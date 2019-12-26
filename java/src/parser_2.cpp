#include "driver.h"
#include "parser.h"
#include "token.h"
#include "common_header_autogen.h"
#include "ruletable_util.h"
#include "gen_debug.h"


//////////////////////////////////////////////////////////////////////////////////
//                           Parsing System
//
// 1. Token Management
//
// Thinking about compound statements which cross multiple lines including multiple
// statement inside, we may first match a few tokens at the beginning just like the
// starting parts of a class, and these tokens must be kept as alive (aka, not done yet).
// Then we need go further to the body. In the body, after we successfully parse a
// sub-statement, its tokens can be discarded, and we read new tokens to parse new
// sub-statements. All the matched tokens of sub statements will be discarded once
// matched. Until in the end, we get the finishing part of a class.
//
// So it's very clear that there are three kinds of tokens while parsing, active,
// discarded and pending.
//   Active    : The tokens are still in the procedure of the matching. e.g. the starting
//               parts of a class declaration, they are always alive until all the
//               sub statements inside are done.
//   Discarded : The tokens finished matching, and we don't need it any more.
//   Pending   : The tokens read in, but not invovled in the matching.
//
// Obviously, we need some data structures to tell these different tokens. We decided
// to have two. (1) One for all the tokens read in. This is the superset of active, discarded,
// and pending. It's the simple reflection of source program. It's the [[mTokens]].
// (2) The other one is for the active tokens. It's [[mActiveTokens]].
//
// During matching, pending tokens are moved to [[mActiveTokens]] per request. Tokens after
// the last active are pending.
//
// 2. Discard Tokens
//
// Now here comes a question, how to identify tokens to be discarded? or when to discard
// what tokens? To address these questions, we need the help of two types of token,
// starting token and ending token.
//   Ending   : Ending tokens are those represent the ending of a complete statement.
//              It's always clearly defined in a language what token are the ending ones.
//              For example, ';' in most languages, and NewLine in Kotlin where a statement
//              doesn't cross multiple line.
//   Starting : Starting tokens are those represent the start of a new complete statement.
//              It's obvious that there are no special characteristics we can tell from
//              a starting token. It could be any token, identifier, keyword, separtor,
//              and anything.
//              To find out the starting tokens, we actually rely on.
//
//              [TODO] Ending token should be configured in .spec file. Right now I'm
//                     just hard coded in the main.cpp.
//
// Actually we just need recognize the ending token, since the one behind ending token is the
// starting token of next statement. However, during matching, we need a stack to record the
// the starting token index-es, [[mStartingTokens]]. Each time we hit an ending token, we
// discard the tokens on the from the starting token (the one on the top of [[mStartingTokens]]
// to the ending token.
// 
//
// 3. Cycle Reference
//
// The most important thing in parsing is how to HANDLE CYCLES in the rules. Take
// rule additiveExpression for example. It calls itself in the second element.
// Should we keep parsing when we find a cycle? If yes, is there any concern we
// need to address?
//
// rule MultiplicativeExpression : ONEOF(
//   UnaryExpression,  ---------------------> it can parse a variable name.
//   MultiplicativeExpression + '*' + UnaryExpression,
//   MultiplicativeExpression + '/' + UnaryExpression,
//   MultiplicativeExpression + '%' + UnaryExpression)
//
// rule AdditiveExpression : ONEOF(
//   MultiplicativeExpression,
//   AdditiveExpression + '+' + MultiplicativeExpression,
//   AdditiveExpression + '-' + MultiplicativeExpression)
//   attr.action.%2,%3 : GenerateBinaryExpr(%1, %2, %3)
//
// The answer is yes, cycle is useful in real life. It needs to keep looping.
// a + b + c + d + ... is a good example showing loops of AdditiveExpression.
//
// Another good example is Block. There are nested blocks. So loop exists.
//
// However, there is a concern which is how to avoid ENDLESS LOOP, and how to
// figure out Valuable Loop vs. Endless Loop. If it's a valuable loop we keep
// parsing, if it's an endless loop, we stop parsing.
//
// A loop is valuable if the mCurToken moves after an iteration of the loop.
// A loop is endless if mCurToken doesn't move between iterations.
// This is determined through the mVisitedStack which records the mCurToken each
// time we hit the ruletable once we are in a loop.
//
// Border conditions:  (1) The first time a table is hit, it's set Visited. We
//                         don's push to the stack since no loop so far.
//                     (2) The second time a table is hit, current token position
//                         is pushed. Now we are forming a loop, but we are not
//                         comparing its current position with the previous one since
//                         it's just a one iteration loop. We allow it go
//                         on until in third time where we can prove it's endless.
//                     (3) From second time on, each time a rule is done, its token
//                         position is popped from the stack.
//                     (4) From the third time on, start checking if the position
//                         has moved compared to the previous one.
//
// 4. Parsing Time Issue
// 
// The rules are referencing each other and could increase the parsing time extremely.
// In order to save time, the first thing is avoiding entering a rule for the second
// time with the same token position if that rule has failed before. This is the
// origin of mFailed.
//
// Also, if a rule was successful for a token and it is trying to traverse it again, it
// can be skipped by using a cached result. This is the origin of mSucc.
//
// 5. Appealing mechanism
//
// Let's look at an example, suppose we have the following rules,
//    rule Primary : ONEOF(PrimaryNoNewArray,
//                         ...)
//    rule PrimaryNoNewArray : ONEOF("this",
//                                   Primary + ...,
//                                   FieldAccess)
//    rule FieldAccess : Primary + '.' + Identifier
//
// And we have a line of code,
//    this.a = 10;
//
// We start with rule Primary for 1st token "this". The traversal of all the rule tables
// form a tree, with the root node as Primary. We depict the tree as below.
//
// Primary  <-- first instance
//    |
//    |--PrimaryNoNewArray  <-- first
//         |--"this"
//         |--Primary  <-- second instance
//         |     |--PrimaryNoNewArray  <-- second
//         |             |--"this"
//         |             |--Primary <-- third instance, failed @ looped
//         |             |--FieldAccess  <-- This is the node to appeal !!!
//         |                   |--Primary <-- forth instance, failed @ looped
//         |--FieldAccess
//
// Looking at the above depict, from the third instance on, all Primary will be found
// endless loop but they are not marked as WasFailed since we don't define loop as failure.
// However, the problem is coming from FieldAccess, which was mistakenly marked as
// WasFailed because its sub-rule Primary failed.
//
// The truth is at the end of the second PrimaryNoNewArray, it's a success since it
// contains "this". And second Primary (so all Primary) are success too. FieldAccess
// doesn't have a chance to clear its mistaken WasFailed. The next time if we are
// traversing FiledAccess with "this", it will give a straightforward failure due to the
// WasFailed.
//
// Obviously something has to be done to correct this problem. We call this Appealing.
// I'll present the rough idea of appealing first and then describe some details.
//
// 1) Appealing is done through the help of tree shown above. We call it AppealTree.
//    It starts with the root node created during the traversal of a top language construct,
//    for example 'class' in Java. Each time we traverse a rule table, we create new
//    children nodes for all of it children tables if they have. In this way, we form
//    the AppealTree.
//
// 2) Suppose we are visiting node N. Before we traverse all its children, it's guaranteed
//    that the node is successful so far. Otherwise, it will return fail before visiting
//    children, or in another word, N will be the leaf node.
//
//    After visiting all children, we check the status of N. It could be marked as
//    FailLooped.
//
// 3) If we see N is of FailLooped, we start check the sub-tree to see if there is
//    any nodes between successful N and the leaf nodes N which are marked as FailChildren.
//    These nodes in between are those to appeal.
//
// == Status of node ==
//
// A node could be marked with different status after visiting it, including
//   * FailLooped
//   * FailChildrenFailed
//   * FailWasFail
//   * FailNotLiteral
//   * FailNotIdentifier
//   * Succ
//
// 6. Second Try
//
// This is the same issue as the second try in lexer. Please refer ruletable_util.cpp
// for details of lexer's second try.
//
// Look at the following example,
//   rule FieldAccess: Primary + '.' + Identifier
//   rule Primary : ONEOF(...
//                        "this"
//                        FieldAccess)
//
// Suppose we have a piece of code containing "this.member", and all three token can
// be matched by Primary, or only the first one can be matched. In my first implementation
// we always match as many tokens as possible. However, here is the problem. If Primary
// taken all the tokens, FieldAccess will get failure. If Primary takes only one token,
// it will be a match. We definitly want a match!
//
// This brings a design which need records all the possible number of tokens a rule can match.
// And give the following rules a chance.
//
// However, this creats another issue. Let's use AppealNode to explain. All nodes form
// a tree. During the traversal, if one node can have multiple possible matchings, the number
// of possible matchings could explode. To limit this explosion we only allow this multiple
// choice happen for temporary, and its parent node will decide the final number of tokens
// immediately. This means the parent node will choose the right one. In this way, the final tree
// has no multipe options at all.
//
// Also this happens when the child node is a ONEOF node and parent node is a Concatenate node.
//
// [NOTE] Second Try will brings in extra branches in the appealing tree. However, it seems doesn't
//        create any serious problem.
//////////////////////////////////////////////////////////////////////////////////


// Lex all tokens in a line, save to mTokens.
// If no valuable in current line, we continue to the next line.
// Returns the number of valuable tokens read. Returns 0 if EOF.
unsigned Parser::LexOneLine() {
  unsigned token_num = 0;

  // Check if there are already pending tokens.
  if (mCurToken < mActiveTokens.size())
    return mActiveTokens.size() - mCurToken;

  while (!token_num) {
    // read untile end of line
    while (!mLexer->EndOfLine() && !mLexer->EndOfFile()) {
      Token* t = mLexer->LexToken_autogen();
      if (t) {
        bool is_whitespace = false;
        if (t->IsSeparator()) {
          SeparatorToken *sep = (SeparatorToken *)t;
          if (sep->IsWhiteSpace())
            is_whitespace = true;
        }
        // Put into the token storage, as Pending tokens.
        if (!is_whitespace && !t->IsComment()) {
          mActiveTokens.push_back(t);
          token_num++;
        }
      } else {
        MASSERT(0 && "Non token got? Problem here!");
        break;
      }
    }
    // Read in the next line.
    if (!token_num) {
      if(!mLexer->EndOfFile())
        mLexer->ReadALine();
      else
        break;
    }
  }

  return token_num;
}

// Move mCurToken one step. If there is no available in mActiveToken, it reads in a new line.
// Return true : if success
//       false : if no more valuable token read, or end of file
bool Parser::MoveCurToken() {
  mCurToken++;
  if (mCurToken == mActiveTokens.size()) {
    unsigned num = LexOneLine();
    if (!num)
      return false;
  }
  return true;
}

bool Parser::Parse_autogen() {
  bool succ = false;
  while (1) {
    succ = ParseStmt_autogen();
    if (!succ)
      break;
  }

  return succ;
} 

// Right now I didn't use mempool yet, will come back.
// [TODO] Using mempool.
void Parser::ClearAppealNodes() {
  for (unsigned i = 0; i < mAppealNodes.size(); i++) {
    AppealNode *node = mAppealNodes[i];
    if (node)
      delete node;
  }
  mAppealNodes.clear();
}

// As the above comments mention, this function is going to walk through the appeal
// tree to find the appropriate nodes and clear their failure mark.
//
// 'root_node' is the one who returns success when traversing. So we need check if
// it was marked failure before in its sub-tree.
//
//             ======  Two Issues In The Traversal ==========
//
// 1. The token number is different. Here is an example,
//
//   Primary@9 <-- Succ, mCurToken becomes 12
//    |--Primary@9
//    |    |--FieldAccess@9 <-- FailedChildrenFailed
//    |            |--Primary@9 FailLooped
//    |--SomeOtherRule Succ <-- mCurToken becomes 12
//
//   When FieldAccess is marked as failed, the token was 9. But when we finish the
//   first Primary, it's a success, while token becomes 12 since SomeOtherRule eats
//   more than one tokens. In this way, the token numbers are different at the two
//   points.
//
//   Since RuleTables are marked as WasFailed regarding specific token numbers, we
//   need figure out a way to clear the failure flag regarding a specific token. So
//   we decided to add token num at each AppealNode.
//
// 2. Multiple occurrence. Use the above example again with minor changes.
//
//   Primary@9 <-- Succ, mCurToken becomes 12
//    |--Primary@9
//    |    |--FieldAccess@9 <-- FailedChildrenFailed
//    |            |--Primary@9 FailLooped
//    |--Expression@9
//    |    |--FieldAccess@9 <-- FailedChildrenFailed
//    |            |--Primary@9 FailLooped
//    |--SomeOtherRule Succ <-- mCurToken becomes 12
//
//   As the example shows, FieldAccess got failures at two different places due to
//   Primary failiing in endless loop. The appealing of FieldAccess will be done twice
//   and at the second time we may not find the @9 token when we trying to clean WasFailed.
//
//   We will just ignore the second clearing of FieldAccess if we cannot find the token num
//   in the mFailed.

static std::vector<AppealNode*> traverse_list;
void Parser::AppealTraverse(AppealNode *node, AppealNode *root) {
  traverse_list.push_back(node);

  MASSERT((root->mAfter == Succ) && "root->mAfter is not Succ.");
  if ((node->mTable == root->mTable) && (node->mAfter == FailLooped)) {
    // walk the list, and clear the fail flag for appropriate node
    // we also set the mAfter of node to Succ so that the futural traversal won't
    // modify it again.
    for (unsigned i = 0; i < traverse_list.size(); i++) {
      AppealNode *n = traverse_list[i];
      if ((n->mBefore == Succ) && (n->mAfter == FailChildrenFailed)) {
        if (mTraceAppeal)
          DumpAppeal(n->mTable, n->mToken);
        ResetFailed(n->mTable, n->mToken);
        n->mAfter = Succ;
      }
    }
  }


  for (unsigned i = 0; i < node->mChildren.size(); i++) {
    AppealTraverse(node->mChildren[i], root);
    traverse_list.pop_back();
  }
}

void Parser::Appeal(AppealNode *root) {
  traverse_list.clear();
  traverse_list.push_back(root);

  for (unsigned i = 0; i < root->mChildren.size(); i++) {
    AppealTraverse(root->mChildren[i], root);
    traverse_list.pop_back();
  }
}

// return true : if successful
//       false : if failed
// This is the parsing for highest level language constructs. It could be class
// in Java/c++, or a function/statement in c/c++. In another word, it's the top
// level constructs in a compilation unit (aka Module).
bool Parser::ParseStmt_autogen() {
  // clear status
  mVisited.clear();
  ClearFailed();
  ClearSucc();
  mTokens.clear();
  mStartingTokens.clear();
  ClearAppealNodes();
  mPending = 0;

  // set the root appealing node
  AppealNode *appeal_root = new AppealNode();
  mAppealNodes.push_back(appeal_root);

  // mActiveTokens contain some un-matched tokens from last time of TraverseStmt(),
  // because at the end of every TraverseStmt() when it finishes its matching it always
  // MoveCurToken() which in turn calls LexOneLine() to read new tokens of a new line.
  //
  // This means in LexOneLine() we also need check if there are already tokens pending.
  //
  // [TODO] Later on, we will move thoes pending tokens to a separate data structure.

  unsigned token_num = LexOneLine();
  // No more token, end of file
  if (!token_num)
    return false;

  // Match the tokens against the rule tables.
  // In a rule table there are : (1) separtaor, operator, keyword, are already in token
  //                             (2) Identifier Table won't be traversed any more since
  //                                 lex has got the token from source program and we only
  //                                 need check if the table is &TblIdentifier.
  bool succ = TraverseStmt();
  return succ;
}

// return true : if all tokens in mActiveTokens are matched.
//       false : if faled.
bool Parser::TraverseStmt() {
  // right now assume statement is just one line.
  // I'm doing a simple separation of one-line class declaration.
  bool succ = false;

  // Go through the top level construct, find the right one.
  std::vector<RuleTable*>::iterator it = mTopTables.begin();
  for (; it != mTopTables.end(); it++) {
    RuleTable *t = *it;
    succ = TraverseRuleTable(t, mAppealNodes[0]);
    if (succ) {
      //MASSERT((mPending == mTokens.size()) && "Num of matched token is wrong.");
      break;
    }
  }

  if (!succ)
    std::cout << "Illegal syntax detected!" << std::endl;
  else
    std::cout << "Matched " << mCurToken << " tokens." << std::endl;

  return succ;
}

void Parser::DumpAppeal(RuleTable *table, unsigned token) {
  for (unsigned i = 0; i < mIndentation + 2; i++)
    std::cout << " ";
  const char *name = GetRuleTableName(table);
  std::cout << "!!Reset the Failed flag of " << name << " @" << token << std::endl;
}

void Parser::DumpEnterTable(const char *table_name, unsigned indent) {
  for (unsigned i = 0; i < indent; i++)
    std::cout << " ";
  std::cout << "Enter " << table_name << "@" << mCurToken << "{" << std::endl;
}

void Parser::DumpExitTable(const char *table_name, unsigned indent, bool succ, AppealStatus reason) {
  for (unsigned i = 0; i < indent; i++)
    std::cout << " ";
  std::cout << "Exit  " << table_name << "@" << mCurToken;
  if (succ) {
    if (reason == SuccWasSucc)
      std::cout << " succ@WasSucc" << "}" << std::endl;
    else
      std::cout << " succ" << "}" << std::endl;
  } else {
    if (reason == FailWasFailed)
      std::cout << " fail@WasFailed" << "}" << std::endl;
    else if (reason == FailLooped)
      std::cout << " fail@Looped" << "}" << std::endl;
    else if (reason == FailNotIdentifier)
      std::cout << " fail@NotIdentifer" << "}" << std::endl;
    else if (reason == FailNotLiteral)
      std::cout << " fail@NotLiteral" << "}" << std::endl;
    else if (reason == FailChildrenFailed)
      std::cout << " fail@ChildrenFailed" << "}" << std::endl;
    else if (reason == NA)
      std::cout << " fail@NA" << "}" << std::endl;
  }
}

// Please read the comments point 6 at the beginning of this file.
// We need prepare certain storage for multiple possible matchings. The successful token
// number could be more than one. I'm using fixed array to save them. If needed to extend
// in the future, just extend it.
#define MAX_SUCC_TOKENS 16
static unsigned gSuccTokensNum;
static unsigned gSuccTokens[MAX_SUCC_TOKENS];

void Parser::DumpSuccTokens() {
  std::cout << "gSuccTokensNum=" << gSuccTokensNum << ": ";
  for (unsigned i = 0; i < gSuccTokensNum; i++)
    std::cout << gSuccTokens[i] << ",";
  std::cout << std::endl;
}

// return true : if the rule_table is matched
//       false : if faled.
bool Parser::TraverseRuleTable(RuleTable *rule_table, AppealNode *appeal_parent) {
  bool matched = false;
  unsigned old_pos = mCurToken;
  Token *curr_token = mActiveTokens[mCurToken];
  gSuccTokensNum = 0;

  mIndentation += 2;
  const char *name = NULL;
  if (mTraceTable) {
    name = GetRuleTableName(rule_table);
    DumpEnterTable(name, mIndentation);
  }

  // set the apppeal node
  AppealNode *appeal = new AppealNode();
  appeal->mTable = rule_table;
  appeal->mToken = mCurToken;
  appeal->mParent = appeal_parent;
  mAppealNodes.push_back(appeal);
  appeal_parent->mChildren.push_back(appeal);

  // Check if it was succ. Set the gSuccTokens/gSuccTokensNum appropriately
  SuccMatch *succ = FindSucc(rule_table);
  if (succ) {
    bool was_succ = succ->GetStartToken(mCurToken);
    if (was_succ) {
      gSuccTokensNum = succ->GetMatchNum();
      for (unsigned i = 0; i < gSuccTokensNum; i++) {
        gSuccTokens[i] = succ->GetOneMatch(i);
        if (gSuccTokens[i] > mCurToken)
          mCurToken = gSuccTokens[i];
      }
      MoveCurToken();

      if (mTraceTable)
        DumpExitTable(name, mIndentation, true, SuccWasSucc);

      mIndentation -= 2;
      appeal->mBefore = Succ;
      appeal->mAfter = SuccWasSucc;
      return true;
    }
  }

  //
  if (WasFailed(rule_table, mCurToken)) {
    if (mTraceTable) {
      DumpExitTable(name, mIndentation, false, FailWasFailed);
    }
    mIndentation -= 2;
    appeal->mBefore = FailWasFailed;
    appeal->mAfter = FailWasFailed;
    return false;
  }

  if (IsVisited(rule_table)) {
    // If there is already token position in stack, it means we are at at least the
    // 3rd instance. So need check if we made any progress between the two instances.
    //
    // This is not a failure case, don't need put into mFailed.
    if (mVisitedStack[rule_table].size() > 0) {
      unsigned prev = mVisitedStack[rule_table].back();
      if (mCurToken == prev) {
        if (mTraceTable)
          DumpExitTable(name, mIndentation, false, FailLooped);
        mIndentation -= 2;
        appeal->mBefore = FailLooped;
        appeal->mAfter = FailLooped;
        return false; 
      }
    }
    // push the current token position
    VisitedPush(rule_table);
  } else {
    SetVisited(rule_table);
  }
    
  if ((rule_table == &TblIdentifier))
    return TraverseIdentifier(rule_table, appeal);

  if ((rule_table == &TblLiteral))
    return TraverseLiteral(rule_table, appeal);

  // Once reaching this point, the node was successful.
  // Gonna look into rule_table's data
  appeal->mBefore = Succ;

  EntryType type = rule_table->mType;
  switch(type) {
  case ET_Oneof:
    matched = TraverseOneof(rule_table, appeal);
    break;
  case ET_Zeroormore:
    matched = TraverseZeroormore(rule_table, appeal);
    break;
  case ET_Zeroorone:
    matched = TraverseZeroorone(rule_table, appeal);
    break;
  case ET_Concatenate:
    matched = TraverseConcatenate(rule_table, appeal);
    break;
  case ET_Data:
    matched = TraverseTableData(rule_table->mData, appeal);
    break;
  case ET_Null:
  default:
    break;
  }

  // If we are leaving the first instance of this rule_table, so clear the flag.
  // Or we pop out the last token position.
  if (mVisitedStack[rule_table].size() == 0)
    ClearVisited(rule_table);
  else
    VisitedPop(rule_table);

  if (mTraceTable)
    DumpExitTable(name, mIndentation, matched, FailChildrenFailed);
  mIndentation -= 2;

  if (mTraceSecondTry)
    DumpSuccTokens();

  if(matched) {
    // It's guaranteed that this rule_table was NOT succ for mCurToken. Need add it.
    SuccMatch *succ_match = FindOrCreateSucc(rule_table);
    succ_match->AddStartToken(old_pos);
    for (unsigned i = 0; i < gSuccTokensNum; i++)
      succ_match->AddOneMoreMatch(gSuccTokens[i]);

    // We try to appeal only if it succeeds at the end.
    appeal->mAfter = Succ;
    Appeal(appeal);
    return true;
  } else {
    appeal->mAfter = FailChildrenFailed;
    mCurToken = old_pos;
    AddFailed(rule_table, mCurToken);
    return false;
  }
}

// We don't go into Literal table.
// No mVisitedStack invovled for literal table.
bool Parser::TraverseLiteral(RuleTable *rule_table, AppealNode *appeal) {
  Token *curr_token = mActiveTokens[mCurToken];
  const char *name = GetRuleTableName(rule_table);
  bool found = false;
  gSuccTokensNum = 0;

  ClearVisited(rule_table);

  if (curr_token->IsLiteral()) {
    found = true;
    gSuccTokensNum = 1;
    gSuccTokens[0] = mCurToken;
    MoveCurToken();
    if (mTraceTable)
      DumpExitTable(name, mIndentation, true);
    mIndentation -= 2;
    appeal->mBefore = Succ;
    appeal->mAfter = Succ;
  } else {
    AddFailed(rule_table, mCurToken);
    if (mTraceTable)
      DumpExitTable(name, mIndentation, false, FailNotLiteral);
    mIndentation -= 2;
    appeal->mBefore = FailNotLiteral;
    appeal->mAfter = FailNotLiteral;
  }

  if (mTraceSecondTry)
    DumpSuccTokens();

  return found;
}

// We don't go into Identifier table.
// No mVisitedStack invovled for identifier table.
//
// 'appeal' is the node for this rule table. This is different than TraverseOneof
// or the others where 'appeal' is actually a parent node.
bool Parser::TraverseIdentifier(RuleTable *rule_table, AppealNode *appeal) {
  Token *curr_token = mActiveTokens[mCurToken];
  const char *name = GetRuleTableName(rule_table);
  bool found = false;
  gSuccTokensNum = 0;

  ClearVisited(rule_table);

  if (curr_token->IsIdentifier()) {
    gSuccTokensNum = 1;
    gSuccTokens[0] = mCurToken;
    found = true;
    MoveCurToken();
    if (mTraceTable)
      DumpExitTable(name, mIndentation, true);
    mIndentation -= 2;
    appeal->mBefore = Succ;
    appeal->mAfter = Succ;
  } else {
    AddFailed(rule_table, mCurToken);
    if (mTraceTable)
      DumpExitTable(name, mIndentation, false, FailNotIdentifier);
    mIndentation -= 2;
    appeal->mBefore = FailNotIdentifier;
    appeal->mAfter = FailNotIdentifier;
  }

  if (mTraceSecondTry)
    DumpSuccTokens();

  return found;
}


// It always return true.
// Moves until hit a NON-target data
// [Note]
//   1. Every iteration we go through all table data, and pick the one matching the most tokens.
//   2. If noone of table data can match, it quit.
//   3. gSuccTokens and gSuccTokensNum are a little bit complicated. Since Zeroormore can match
//      any number of tokens it can, the number of matchings will grow each time one instance
//      is matched.
bool Parser::TraverseZeroormore(RuleTable *rule_table, AppealNode *parent) {
  unsigned succ_tokens_num = 0;
  unsigned succ_tokens[MAX_SUCC_TOKENS];
  bool matched = false;

  gSuccTokensNum = 0;

  while(1) {
    bool found = false;
    unsigned old_pos = mCurToken;
    unsigned new_pos = mCurToken;

    MASSERT((rule_table->mNum == 1) && "zeroormore node has more than one elements?");
    for (unsigned i = 0; i < rule_table->mNum; i++) {
      // every table entry starts from the old_pos
      mCurToken = old_pos;
      TableData *data = rule_table->mData + i;
      found = found | TraverseTableData(data, parent);
      if (mCurToken > new_pos)
        new_pos = mCurToken;
    }

    // 1. If hit the first non-target, stop it.
    // 2. Sometimes 'found' is true, but actually nothing was read becauser the 'true'
    //    is coming from a Zeroorone or Zeroormore. So need check this.
    if ((!found) || (new_pos == old_pos) )
      break;
    else {
      matched = true;
      mCurToken = new_pos;
      // Take care of the successful matchings.
      for (unsigned i = 0; i < gSuccTokensNum; i++)
        succ_tokens[succ_tokens_num++] = gSuccTokens[i];
    }
  }

  if (matched) {
    gSuccTokensNum = succ_tokens_num;
    for (unsigned i = 0; i < succ_tokens_num; i++)
      gSuccTokens[i] = succ_tokens[i];
  } else {
    // Even if nothing is found, we still treat it as one success without moving
    // mCurToken forward. Since gSuccTokens[] record the success matched tokens, and
    // mCurToken is not matched, we have to set the previous token as successful token.
    gSuccTokensNum = 1;
    gSuccTokens[0] = mCurToken - 1;
  }

  return true;
}

// For Zeroorone node it's easier to handle gSuccTokens(Num). Just let the elements
// handle themselves.
bool Parser::TraverseZeroorone(RuleTable *rule_table, AppealNode *parent) {
  gSuccTokensNum = 0;
  bool found = false;
  for (unsigned i = 0; i < rule_table->mNum; i++) {
    TableData *data = rule_table->mData + i;
    found = TraverseTableData(data, parent);
    // The first element is hit, then stop.
    if (found)
      break;
  }

  // Even if nothing is found, we still treat it as one success without moving
  // mCurToken forward. Since gSuccTokens[] record the success matched tokens, and
  // mCurToken is not matched, we have to set the previous token as successful token.
  if (!found) {
    gSuccTokensNum = 1;
    gSuccTokens[0] = mCurToken - 1;
  }

  return true;
}

// 1. Save all the possible matchings from children.
// 2. As return value we choose the longest matching.
bool Parser::TraverseOneof(RuleTable *rule_table, AppealNode *parent) {
  bool found = false;
  unsigned succ_tokens_num = 0;
  unsigned succ_tokens[MAX_SUCC_TOKENS];
  unsigned new_mCurToken = mCurToken; // position after most tokens eaten
  unsigned old_mCurToken = mCurToken;

  gSuccTokensNum = 0;

  for (unsigned i = 0; i < rule_table->mNum; i++) {
    TableData *data = rule_table->mData + i;
    bool temp_found = TraverseTableData(data, parent);
    found = found | temp_found;
    if (temp_found) {
      // 1. Save the possilbe matchings
      // 2. Remove be duplicated matchings
      for (unsigned j = 0; j < gSuccTokensNum; j++) {
        bool duplicated = false;
        for (unsigned k = 0; k < succ_tokens_num; k++) {
          if (succ_tokens[k] == gSuccTokens[j]) {
            duplicated = true;
            break;
          }
        }
        if (!duplicated)
          succ_tokens[succ_tokens_num++] = gSuccTokens[j];
      }

      if (mCurToken > new_mCurToken)
        new_mCurToken = mCurToken;
      // Restore the position of original mCurToken.
      mCurToken = old_mCurToken;
    }
  }

  gSuccTokensNum = succ_tokens_num;
  for (unsigned k = 0; k < succ_tokens_num; k++)
    gSuccTokens[k] = succ_tokens[k];

  // move position according to the longest matching
  mCurToken = new_mCurToken;
  return found;
}

// For concatenate rule, we need take care second try.
// There could be many different scenarios, but we only do it for a simplest case, in which
// a multiple matching element followed by a failed element.
//
// We also need take care of the gSuccTokensNum and gSuccTokens for Concatenate node.
// [NOTE] 1. There could be an issue of matching number explosion. Each node could have
//           multiple matchings, and when they are concatenated the number would be
//           matchings_1 * matchings_2 * ...
//        2. The good thing is each following node start to match from the new mCurToken
//           after the previous node. This means we are actually assuming there is only
//           one matching from previous node even it actually has multiple matchings.
//        3. Only the last node could be allowed to have multiple matchings to transfer to
//           the caller.
// But we are lucky as the last element of Concatenate will take care of gSuccTokens by itself.
bool Parser::TraverseConcatenate(RuleTable *rule_table, AppealNode *parent) {
  bool found = false;
  unsigned prev_succ_tokens_num = 0;
  unsigned prev_succ_tokens[MAX_SUCC_TOKENS];
  unsigned curr_succ_tokens_num = 0;
  unsigned curr_succ_tokens[MAX_SUCC_TOKENS];

  // Make sure it's 0 when fail
  gSuccTokensNum = 0;

  for (unsigned i = 0; i < rule_table->mNum; i++) {
    TableData *data = rule_table->mData + i;
    found = TraverseTableData(data, parent);

    curr_succ_tokens_num = gSuccTokensNum;
    for (unsigned id = 0; id < gSuccTokensNum; id++)
      curr_succ_tokens[id] = gSuccTokens[id];

    if (!found) {
      if (mTraceSecondTry)
        DumpSuccTokens();

      MASSERT((gSuccTokensNum == 0) || ((gSuccTokensNum == 1) && (data->mType == DT_Token))
               && "failed case has >=1 successful matching?");

      // If the previous element has single matching or fail, we give up. Or we'll give a
      // second try.
      if (prev_succ_tokens_num > 1) {
        if (mTraceSecondTry)
          std::cout << "Decided to second try." << std::endl;

        // Step 1. Save the mCurToken, it supposed to be the longest matching.
        unsigned old_pos = mCurToken;

        // Step 2. Iterate and try
        //         There are again multiple possiblilities. Could be multiple matching again.
        //         We will never go that far. We'll stop at the first matching.
        //         We don't want the matching number to explode.
        bool temp_found = false;
        for (unsigned j = 0; j < prev_succ_tokens_num; j++) {
          // The longest matching has been proven to be a failure.
          if (prev_succ_tokens[j] == old_pos)
            continue;
          // Start from the one after succ token.
          mCurToken = prev_succ_tokens[j] + 1;
          temp_found = TraverseTableData(data, parent);
          // As mentioned above, we stop at the first successfuly second try.
          if (temp_found)
            break;
        }

        // Step 3. Set the 'found'
        found = temp_found;

        // Step 4. If still fail after second try, we reset the mCurToken
        if (!found)
          mCurToken = old_pos;

        if (mTraceSecondTry) {
          if (found)
            std::cout << "Second Try succ." << std::endl;
          else
            std::cout << "Second Try fail." << std::endl;
        }
      }
    }

    prev_succ_tokens_num = curr_succ_tokens_num;
    for (unsigned id = 0; id < curr_succ_tokens_num; id++)
      prev_succ_tokens[id] = curr_succ_tokens[id];

    // After second try, if it still fails, we quit.
    if (!found)
      break;
  }

  return found;
}

// The mCurToken moves if found target, or restore the original location.
bool Parser::TraverseTableData(TableData *data, AppealNode *parent) {
  unsigned old_pos = mCurToken;
  bool     found = false;
  Token   *curr_token = mActiveTokens[mCurToken];
  gSuccTokensNum = 0;

  switch (data->mType) {
  case DT_Char:
  case DT_String:
    //MASSERT(0 && "Hit Char/String in TableData during matching!");
    //TODO: Need compare literal. But so far looks like it's impossible to
    //      have a literal token able to match a string/char in rules.
    break;
  // separator, operator, keywords are planted as DT_Token.
  // just need check the pointer of token
  case DT_Token:
    mIndentation += 2;
    if (mTraceTable)
      DumpEnterTable("token", mIndentation);
    if (data->mData.mToken == curr_token) {
      found = true;
      gSuccTokensNum = 1;
      gSuccTokens[0] = mCurToken;
      MoveCurToken();
    }
    if (mTraceTable)
      DumpExitTable("token", mIndentation, found, NA);
    if (mTraceSecondTry)
      DumpSuccTokens();
    mIndentation -= 2;
    break;
  case DT_Type:
    break;
  case DT_Subtable: {
    RuleTable *t = data->mData.mEntry;
    found = TraverseRuleTable(t, parent);
    if (!found)
      mCurToken = old_pos;
    break;
  }
  case DT_Null:
  default:
    break;
  }

  return found;
}



// Initialized the predefined tokens.
void Parser::InitPredefinedTokens() {
  // 1. create separator Tokens.
  for (unsigned i = 0; i < SEP_NA; i++) {
    Token *t = (Token*)mLexer->mTokenPool.NewToken(sizeof(SeparatorToken));
    new (t) SeparatorToken(i);
    //std::cout << "init a pre token " << t << std::endl;
    //t->Dump();
  }
  mLexer->mPredefinedTokenNum += SEP_NA;

  // 2. create operator Tokens.
  for (unsigned i = 0; i < OPR_NA; i++) {
    Token *t = (Token*)mLexer->mTokenPool.NewToken(sizeof(OperatorToken));
    new (t) OperatorToken(i);
    //std::cout << "init a pre token " << t << std::endl;
    //t->Dump();
  }
  mLexer->mPredefinedTokenNum += OPR_NA;

  // 3. create keyword Tokens.
  for (unsigned i = 0; i < KeywordTableSize; i++) {
    Token *t = (Token*)mLexer->mTokenPool.NewToken(sizeof(KeywordToken));
    char *s = mLexer->mStringPool.FindString(KeywordTable[i].mText);
    new (t) KeywordToken(s);
    //std::cout << "init a pre token " << t << std::endl;
    //t->Dump();
  }
  mLexer->mPredefinedTokenNum += KeywordTableSize;

  // 4. Create a single comment token
  //    This is the last predefined token, and we refer to it directly.
  Token *t = (Token*)mLexer->mTokenPool.NewToken(sizeof(CommentToken));
  new (t) CommentToken();
  mLexer->mPredefinedTokenNum += 1;
}

// Set up the top level rule tables.
void Parser::SetupTopTables() {
  mTopTables.push_back(&TblStatement);
  mTopTables.push_back(&TblClassDeclaration);
  mTopTables.push_back(&TblInterfaceDeclaration);
}

const char* Parser::GetRuleTableName(const RuleTable* addr) {
  for (unsigned i = 0; i < RuleTableNum; i++) {
    RuleTableName name = gRuleTableNames[i];
    if (name.mAddr == addr)
      return name.mName;
  }
  return NULL;
}

////////////////////////////////////////////////////////////////////////////
//                          Succ Info Related
////////////////////////////////////////////////////////////////////////////

void Parser::ClearSucc() {
  // clear the map
  mSucc.clear();
  // clear the pool
  std::vector<SuccMatch*>::iterator it = mSuccPool.begin();
  for (; it != mSuccPool.end(); it++) {
    SuccMatch *m = *it;
    delete m;
  }
}

// Find the succ info of 'table'.
// Return NULL if not found.
SuccMatch* Parser::FindSucc(RuleTable *table) {
  std::map<RuleTable*, SuccMatch*>::iterator it = mSucc.find(table);
  SuccMatch *succ = NULL;
  if (it != mSucc.end())
    succ = it->second;
  return succ;
}

// Find the succ info of 'table'.
// If not found, create one.
SuccMatch* Parser::FindOrCreateSucc(RuleTable *table) {
  std::map<RuleTable*, SuccMatch*>::iterator it = mSucc.find(table);
  SuccMatch *succ = NULL;
  if (it != mSucc.end())
    succ = it->second;
  else {
    succ = new SuccMatch();
    mSucc.insert(std::pair<RuleTable*, SuccMatch*>(table, succ));
  }
  return succ;
}

// Rule has only one match at this token.
void SuccMatch::AddOneMatch(unsigned t, unsigned m) {
  mCache.push_back(t);
  mCache.push_back(1);
  mCache.push_back(m);
}

// Rule has only two matches at this token.
void SuccMatch::AddTwoMatch(unsigned t, unsigned m1, unsigned m2) {
  mCache.push_back(t);
  mCache.push_back(2);
  mCache.push_back(m1);
  mCache.push_back(m2);
}

// Rule has only three matches at this token.
void SuccMatch::AddThreeMatch(unsigned t, unsigned m1, unsigned m2, unsigned m3) {
  mCache.push_back(t);
  mCache.push_back(3);
  mCache.push_back(m1);
  mCache.push_back(m2);
  mCache.push_back(m3);
}

///////////////////
// The following two functions are used to handle general cases where the
// total number of matches could be unknown, or could be more than 3.
// They need be used together as mTempIndex is def-use in between.

void SuccMatch::AddStartToken(unsigned t) {
  mCache.push_back(t);
  mCache.push_back(0);    // temporarily set the num of matches to 0
  mTempIndex = mCache.size() - 1;
}

void SuccMatch::AddOneMoreMatch(unsigned m) {
  mCache.push_back(m);
  mCache[mTempIndex] += 1;
}

// Below are three Query functions for succ info
// Again, They need be used together as mTempIndex is def-use in between.
//

// Find the succ info for token 't'. Return true if found.
bool SuccMatch::GetStartToken(unsigned t) {
  // traverse the vector
  // mTempIndex is used as the index of the start token.
  mTempIndex = 0;

  while (mTempIndex < (mCache.size() - 2)) {
    if (t != mCache[mTempIndex] ){
      unsigned num = mCache[mTempIndex + 1];
      mTempIndex += 1;   // at the num
      mTempIndex += num; // at last num tokens.
      mTempIndex += 1;   // at new index
    } else
      break;
  }

  if (mTempIndex < (mCache.size() - 2))
    return true;
  else
    return false;
}

unsigned SuccMatch::GetMatchNum() {
  return mCache[mTempIndex + 1];
}

// idx starts from 0.
unsigned SuccMatch::GetOneMatch(unsigned idx) {
  return mCache[mTempIndex + 1 + idx + 1];
}
