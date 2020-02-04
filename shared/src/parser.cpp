/*
* Copyright (C) [2020] Futurewei Technologies, Inc. All rights reverved.
*
* OpenArkFE is licensed under the Mulan PSL v1.
* You can use this software according to the terms and conditions of the Mulan PSL v1.
* You may obtain a copy of Mulan PSL v1 at:
*
*  http://license.coscl.org.cn/MulanPSL
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR
* FIT FOR A PARTICULAR PURPOSE.
* See the Mulan PSL v1 for more details.
*/
#include <iostream>
#include <string>
#include <cstring>
#include <stack>

#include "parser.h"
#include "massert.h"
#include "token.h"
#include "common_header_autogen.h"
#include "ruletable_util.h"
#include "gen_debug.h"
#include "ast.h"

//////////////////////////////////////////////////////////////////////////////////
//                   Top Issues in Parsing System
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
//
//
// 7. SortOut Process
//
// After traversing the rule tables and successfully matching all the tokens, we have created
// a big tree with Top Table as the root. However, the real matching part is just a small part
// of the tree. We need sort out all the nodes and figure out the exactly matching sub-tree.
// Based on this sub-tree, we can further apply the action of each node to build the MapleIR.
//
// The tree is made up of AppealNode-s, and we will walk the tree and examine the nodes during
// the SortOut process.
//////////////////////////////////////////////////////////////////////////////////

Parser::Parser(const char *name, Module *m) : filename(name), mModule(m) {
  mLexer = new Lexer();
  const std::string file(name);
  // init lexer
  mLexer->PrepareForFile(file);
  mCurToken = 0;

  mTraceTable = false;
  mTraceAppeal = false;
  mTraceSecondTry = false;
  mTraceVisited = false;
  mTraceFailed = false;
  mTraceSortOut = true;
  mTraceWarning = false;

  mIndentation = 0;

  mInSecondTry = false;
}

Parser::Parser(const char *name) : filename(name) {
  mLexer = new Lexer();
  const std::string file(name);
  // init lexer
  mLexer->PrepareForFile(file);
  mCurToken = 0;
  mPending = 0;

  mTraceTable = false;
  mTraceAppeal = false;
  mTraceSecondTry = false;
  mTraceVisited = false;
  mTraceFailed = false;
  mTraceSortOut = true;
  mTraceWarning = false;

  mIndentation = 0;

  mInSecondTry = false;
}

Parser::~Parser() {
  delete mLexer;

  std::vector<ASTTree*>::iterator it = mASTTrees.begin();
  for (; it != mASTTrees.end(); it++) {
    ASTTree *tree = *it;
    if (tree)
      delete tree;
  }
  mASTTrees.clear();
}

void Parser::Dump() {
}


// Utility function to handle visited rule tables.
bool Parser::IsVisited(RuleTable *table) {
  std::map<RuleTable*, bool>::iterator it;
  it = mVisited.find(table);
  if (it == mVisited.end())
    return false;
  if (it->second == false)
    return false;
  return true;
}

void Parser::SetVisited(RuleTable *table) {
  //std::cout << " set visited " << table;
  mVisited[table] = true;
}

void Parser::ClearVisited(RuleTable *table) {
  //std::cout << " clear visited " << table;
  mVisited[table] = false;
}

// Push the current position into stack, as we are entering the table again.
void Parser::VisitedPush(RuleTable *table) {
  //std::cout << " push " << mCurToken << " from " << table;
  mVisitedStack[table].push_back(mCurToken);
}

// Pop the last position in stack, as we are leaving the table again.
void Parser::VisitedPop(RuleTable *table) {
  //std::cout << " pop " << " from " << table;
  mVisitedStack[table].pop_back();
}

// Add one fail case for the table
void Parser::AddFailed(RuleTable *table, unsigned token) {
  //std::cout << " push " << mCurToken << " from " << table;
  mFailed[table].push_back(token);
}

// Remove one fail case for the table
void Parser::ResetFailed(RuleTable *table, unsigned token) {
  std::vector<unsigned>::iterator it = mFailed[table].begin();;
  for (; it != mFailed[table].end(); it++) {
    if (*it == token)
      break;
  }

  if (it != mFailed[table].end())
    mFailed[table].erase(it);
}

bool Parser::WasFailed(RuleTable *table, unsigned token) {
  std::vector<unsigned>::iterator it = mFailed[table].begin();
  for (; it != mFailed[table].end(); it++) {
    if (*it == token)
      return true;
  }
  return false;
}

// Lex all tokens in a line, save to mTokens.
// If no valuable in current line, we continue to the next line.
// Returns the number of valuable tokens read. Returns 0 if EOF.
unsigned Parser::LexOneLine() {
  unsigned token_num = 0;
  Token *t = NULL;

  // Check if there are already pending tokens.
  if (mCurToken < mActiveTokens.size())
    return mActiveTokens.size() - mCurToken;

  while (!token_num) {
    // read untile end of line
    while (!mLexer->EndOfLine() && !mLexer->EndOfFile()) {
      t = mLexer->LexToken();
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

bool Parser::Parse() {
  bool succ = false;
  while (1) {
    succ = ParseStmt();
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
  if ((node->GetTable() == root->GetTable()) && (node->mAfter == FailLooped)) {
    // 1. Walk the list, and clear the fail flag for corresponding rule table.
    // 2. For this specific AppealNode, it's mAfter is set to Fail, we are not changing it.
    for (unsigned i = 0; i < traverse_list.size(); i++) {
      AppealNode *n = traverse_list[i];
      if ((n->mBefore == Succ) && (n->mAfter == FailChildrenFailed)) {
        if (mTraceAppeal)
          DumpAppeal(n->GetTable(), n->mStartIndex);
        ResetFailed(n->GetTable(), n->mStartIndex);
      }
    }
  }

  std::vector<AppealNode*>::iterator it = node->mChildren.begin();
  for (; it != node->mChildren.end(); it++) {
    AppealNode *child = *it;
    if (child->IsToken())
      continue;
    AppealTraverse(child, root);
    traverse_list.pop_back();
  }
}

void Parser::Appeal(AppealNode *root) {
  traverse_list.clear();
  traverse_list.push_back(root);

  std::vector<AppealNode*>::iterator it = root->mChildren.begin();
  for (; it != root->mChildren.end(); it++) {
    AppealNode *child = *it;
    if (child->IsToken())
      continue;
    AppealTraverse(child, root);
    traverse_list.pop_back();
  }
}

// return true : if successful
//       false : if failed
// This is the parsing for highest level language constructs. It could be class
// in Java/c++, or a function/statement in c/c++. In another word, it's the top
// level constructs in a compilation unit (aka Module).
bool Parser::ParseStmt() {
  // clear status
  mVisited.clear();
  ClearFailed();
  ClearSucc();
  mTokens.clear();
  mStartingTokens.clear();
  ClearAppealNodes();
  mPending = 0;

  // set the root appealing node
  mRootNode = new AppealNode();
  mAppealNodes.push_back(mRootNode);

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

  // After matching&SortOut there is a tree with each node being a AppealNode.
  // Now we need build AST based on the AppealNode tree. This involves the rule Actions
  // we are used to build AST. So far we haven't done the syntax checking yet. [TODO]
  //
  // Each top level construct gets a AST tree.
  if (succ) {
    PatchWasSucc(mRootNode);
    SimplifySortedTree(mRootNode);
    ASTTree *tree = BuildAST(mRootNode);
    if (tree)
      mASTTrees.push_back(tree);
  }

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
    succ = TraverseRuleTable(t, mRootNode);
    if (succ) {
      mRootNode->mAfter = Succ;
      SortOut();
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
  mAppealNodes.push_back(appeal);
  appeal->SetTable(rule_table);
  appeal->mStartIndex = mCurToken;
  appeal->mParent = appeal_parent;
  if (mInSecondTry) {
    appeal->mIsSecondTry = true;
    mInSecondTry = false;
  }

  appeal_parent->AddChild(appeal);

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
    if (appeal->IsTable())
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
//
// 'appeal' is the node for this rule table. This is different than TraverseOneof
// or the others where 'appeal' is actually a parent node.
//
// Also since it's actually a token, we will relate 'appeal' to the token.
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

    appeal->mBefore = Succ;
    appeal->mAfter = Succ;
    appeal->SetToken(curr_token);
    appeal->mStartIndex = mCurToken;

    MoveCurToken();
    if (mTraceTable)
      DumpExitTable(name, mIndentation, true);
    mIndentation -= 2;
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
//
// Also since it's actually a token, we will relate 'appeal' to the token.
bool Parser::TraverseIdentifier(RuleTable *rule_table, AppealNode *appeal) {
  Token *curr_token = mActiveTokens[mCurToken];
  const char *name = GetRuleTableName(rule_table);
  bool found = false;
  gSuccTokensNum = 0;

  ClearVisited(rule_table);

  if (curr_token->IsIdentifier()) {
    gSuccTokensNum = 1;
    gSuccTokens[0] = mCurToken;

    appeal->mBefore = Succ;
    appeal->mAfter = Succ;
    appeal->SetToken(curr_token);
    appeal->mStartIndex = mCurToken;

    found = true;
    MoveCurToken();
    if (mTraceTable)
      DumpExitTable(name, mIndentation, true);
    mIndentation -= 2;
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
    gSuccTokensNum = 0;
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

  if (!found) {
    if (gSuccTokensNum != 0)
      std::cout << "weird.... gSuccTokensNum=" << gSuccTokensNum << std::endl;
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
//           one matching in previous node even it actually has multiple matchings.
//        3. Only the last node could be allowed to have multiple matchings to transfer to
//           the caller.
//        4. The gSuccTokensNum/gSuccTokens need be taken care since in a rule like below
//              rule AA : BB + CC + ZEROORONE(xxx)
//           If ZEROORONE(xxx) doesn't match anything, it sets gSuccTokensNum to 0. However
//           rule AA matches multiple tokens. So gSuccTokensNum needs to be accumulated.

bool Parser::TraverseConcatenate(RuleTable *rule_table, AppealNode *parent) {
  bool found = false;
  unsigned prev_succ_tokens_num = 0;
  unsigned prev_succ_tokens[MAX_SUCC_TOKENS];
  unsigned curr_succ_tokens_num = 0;
  unsigned curr_succ_tokens[MAX_SUCC_TOKENS];
  unsigned final_succ_tokens_num = 0;
  unsigned final_succ_tokens[MAX_SUCC_TOKENS];

  // Make sure it's 0 when fail
  gSuccTokensNum = 0;

  for (unsigned i = 0; i < rule_table->mNum; i++) {
    TableData *data = rule_table->mData + i;
    found = TraverseTableData(data, parent);

    curr_succ_tokens_num = gSuccTokensNum;
    for (unsigned id = 0; id < gSuccTokensNum; id++)
      curr_succ_tokens[id] = gSuccTokens[id];

    if (found) {
      // for Zeroorone/Zeroormore node, it set found to true, but actually it may matching
      // nothing. In this case, we don't change final_succ_tokens_num.
      if (gSuccTokensNum > 0) {
        final_succ_tokens_num = gSuccTokensNum;
        for (unsigned id = 0; id < gSuccTokensNum; id++)
          final_succ_tokens[id] = gSuccTokens[id];
      }
    } else {
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

          mInSecondTry = true;

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
        if (!found) {
          mCurToken = old_pos;
        } else {
          // for Zeroorone/Zeroormore node, it set found to true, but actually it may matching
          // nothing. In this case, we don't change final_succ_tokens_num.
          if (gSuccTokensNum > 0) {
            final_succ_tokens_num = gSuccTokensNum;
            for (unsigned id = 0; id < gSuccTokensNum; id++)
              final_succ_tokens[id] = gSuccTokens[id];
          }
        }

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

  if (found) {
     gSuccTokensNum = final_succ_tokens_num;
     for (unsigned id = 0; id < final_succ_tokens_num; id++)
       gSuccTokens[id] = final_succ_tokens[id];
  } else {
    gSuccTokensNum = 0;
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
      AppealNode *appeal = new AppealNode();
      mAppealNodes.push_back(appeal);
      appeal->mBefore = Succ;
      appeal->mAfter = Succ;
      appeal->SetToken(curr_token);
      appeal->mStartIndex = mCurToken;
      appeal->mParent = parent;
      if (mInSecondTry) {
        appeal->mIsSecondTry = true;
        mInSecondTry = false;
      }

      parent->AddChild(appeal);

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

/////////////////////////////////////////////////////////////////////////////
//                              SortOut
//
// The principle of Match-SortOut is very similar as Map-Reduce. During the
// Match phase, the SuccMatch could have multiple choices, and it keeps growing.
// During the SortOut phase, we are reducing the SuccMatch, removing the misleading
// matches.
//
// We will start from the root of the tree composed of AppealNode, and find one
// sub-tree which successfully matched all tokens. The algorithm has a few key
// parts.
//
// 1. Start from mRootNode, and do a traversal similar as matching process, but
//    much simpler.
// 2. If a node is failed, it's removed from its parents children list.
// 3. If a node is success, it's guaranteed its starting token is parent's last
//    token plus one.
// 4. There should be the same amount of succ children as the vector length
//    of parent's SuccMatch.
// 5. There should be only one final matching in the end. Otherwise it's
//    ambiguouse.
// 6. [NOTE] During the matching phase, a node could have multiple successful
//           matching. However, during SortOut, starting from mRootNode, there is
//           only possible matching. We will trim the SuccMatch vector of each
//           child node according to their parent node. In this way, we will finally
//           get a single tree.
/////////////////////////////////////////////////////////////////////////////

// We don't want to use recursive. So a deque is used here.
void Parser::SortOut() {
  // we remove all failed children, leaving only succ child
  std::vector<AppealNode*>::iterator it = mRootNode->mChildren.begin();
  for (; it != mRootNode->mChildren.end(); it++) {
    AppealNode *n = *it;
    if (!n->IsFail())
      mRootNode->mSortedChildren.push_back(n);
  }
  MASSERT(mRootNode->mSortedChildren.size()==1);

  to_be_sorted.clear();
  to_be_sorted.push_back(mRootNode->mSortedChildren.front());

  while(!to_be_sorted.empty()) {
    AppealNode *node = to_be_sorted.front();
    to_be_sorted.pop_front();
    SortOutNode(node);
  }

  if (mTraceSortOut) {
    MASSERT(mRootNode->mSortedChildren.size()==1);
    DumpSortOut(mRootNode->mSortedChildren[0]);
  }
}

// 'node' is already trimmed when passed into this function. This assertion
//  will be done in the SortOutXXX() which handles children node.
void Parser::SortOutNode(AppealNode *node) {
  MASSERT(node->IsSucc() && "Failed node in SortOut?");

  // A token's appeal node is a leaf node. Just return.
  if (node->IsToken())
    return;

  // If node->mAfter is SuccWasSucc, it means we didn't traverse its children
  // during matching. In SortOut, we simple return. However, when generating IR,
  // the children have to be created. A re-traversal of AppealNode tree is needed.
  if (node->mAfter == SuccWasSucc && node->mChildren.size() == 0)
    return;

  RuleTable *rule_table = node->GetTable();
  EntryType type = rule_table->mType;
  switch(type) {
  case ET_Oneof:
    SortOutOneof(node);
    break;
  case ET_Zeroormore:
    SortOutZeroormore(node);
    break;
  case ET_Zeroorone:
    SortOutZeroorone(node);
    break;
  case ET_Concatenate:
    SortOutConcatenate(node);
    break;
  case ET_Data:
    SortOutData(node);
    break;
  case ET_Null:
  default:
    break;
  }

  return;
}

// 'parent' is already trimmed when passed into this function.
void Parser::SortOutOneof(AppealNode *parent) {
  RuleTable *table = parent->GetTable();
  MASSERT(table && "parent is not a table?");

  SuccMatch *succ = FindSucc(table);
  MASSERT(succ && "parent has no SuccMatch?");

  bool found = succ->GetStartToken(parent->mStartIndex);

  // It's possible it actually matches nothing, such as all children are Zeroorxxx
  // and they match nothing.
  unsigned match_num = succ->GetMatchNum();
  if (match_num == 0)
    return;

  // At this point, it has one and only one succ match.
  MASSERT((match_num == 1) && "trimmed parent has >1 matches?");
  unsigned parent_match = succ->GetOneMatch(0);

  unsigned good_children = 0;
  bool     good_child_elected = false;

  std::vector<AppealNode*>::iterator it = parent->mChildren.begin();
  for (; it != parent->mChildren.end(); it++) {
    AppealNode *child = *it;
    if (child->IsFail())
      continue;

    // In OneOf node, A successful child node must have its last matching
    // token the same as parent. Look into the child's SuccMatch, trim it
    // if it has multiple matching.

    if (child->IsToken()) {
      // Token node matches just one token.
      if (child->mStartIndex == parent_match) {
        good_children++;
        parent->mSortedChildren.push_back(child);
      }
    } else {
      SuccMatch *succ = FindSucc(child->GetTable());
      MASSERT(succ && "ruletable node has no SuccMatch?");

      bool found = succ->ReduceMatches(child->mStartIndex, parent_match);
      if (found) {
        good_children++;
        // We only elect the first good child.
        if (!good_child_elected) {
          to_be_sorted.push_back(child);
          parent->mSortedChildren.push_back(child);
          good_child_elected = true;
        }
      }
    }
  }

  if(good_children > 1 && mTraceWarning)
    std::cout << "Warning: Multiple succ children. We only keep one." << std::endl;
}

// 'parent' is already trimmed when passed into this function.
//
// For Zeroormore node, where all children's matching tokens are linked together one after another.
// All children nodes have the same rule table, but each has its unique AppealNode.
//
void Parser::SortOutZeroormore(AppealNode *parent) {
  RuleTable *table = parent->GetTable();
  MASSERT(table && "parent is not a table?");

  SuccMatch *parent_succ = FindSucc(table);
  MASSERT(parent_succ && "parent has no SuccMatch?");

  unsigned parent_start = parent->mStartIndex;
  bool     found = parent_succ->GetStartToken(parent_start);

  // Zeroormore could match nothing.
  unsigned match_num = parent_succ->GetMatchNum();
  if (match_num == 0)
    return;

  unsigned parent_match = parent_succ->GetOneMatch(0);

  // In Zeroormore node, all children should have 'really' matched tokens which means they 
  // did move forward as token index. So there shouldn't be any bad children. 
  // The major work of this loop is to verify the above claim is abided by.

  // There is only failed child which is the last one. Remember in TraverseZeroormore() we
  // keep trying until the last one is faile? So at this point we will first skip the
  // last failed child node.
  AppealNode *failed = parent->mChildren.back();
  std::vector<AppealNode*>::iterator childit = parent->mChildren.begin();
  for (; childit != parent->mChildren.end(); childit++) {
    if (*childit != failed)
      parent->mSortedChildren.push_back(*childit);
  }

  unsigned last_match = parent_start - 1;

  std::vector<AppealNode*>::iterator it = parent->mSortedChildren.begin();
  for (; it != parent->mSortedChildren.end(); it++) {
    AppealNode *child = *it;
    MASSERT(child->IsSucc() && "Successful zeroormore node has a failed child node?");

    MASSERT((last_match + 1 == child->mStartIndex)
             && "Node match index are not connected in SortOut Zeroormore.");

    if (child->IsToken()) {
      // Token node matches just one token.
      last_match++;
    } else {
      SuccMatch *succ = FindSucc(child->GetTable());
      MASSERT(succ && "ruletable node has no SuccMatch?");

      unsigned start_node = last_match + 1;
      bool found = succ->GetStartToken(start_node);  // in order to set mTempIndex
      MASSERT(found && "Couldn't find start token?");

      // We only preserve the longest matching. The other nodes will be reduced.
      unsigned longest = start_node;
      unsigned longest_index = 0;
      for (unsigned i = 0; i < succ->GetMatchNum(); i++) {
        unsigned curr_match = succ->GetOneMatch(i);
        if (curr_match > longest) {
          longest = curr_match;
          longest_index = i;
        }
      }
      succ->ReduceMatches(longest_index);
      last_match = longest;

      // Add child to the working list
      to_be_sorted.push_back(child);
    }
  }

  return;
}

// 'parent' is already trimmed when passed into this function.
void Parser::SortOutZeroorone(AppealNode *parent) {
  RuleTable *table = parent->GetTable();
  MASSERT(table && "parent is not a table?");

  SuccMatch *parent_succ = FindSucc(table);
  MASSERT(parent_succ && "parent has no SuccMatch?");

  unsigned parent_start = parent->mStartIndex;
  bool     found = parent_succ->GetStartToken(parent_start);

  // Zeroorone could match nothing. We just do nothing in this case.
  unsigned match_num = parent_succ->GetMatchNum();
  if (match_num == 0)
    return;

  unsigned parent_match = parent_succ->GetOneMatch(0);

  // At this point, there is one and only one child which may or may not match some tokens.
  // 1. If the child is failed, just remove it.
  // 2. If the child is succ, the major work of this loop is to verify the child's SuccMatch is
  //    consistent with parent's.

  MASSERT((parent->mChildren.size() == 1) && "Zeroorone has >1 valid children?");
  AppealNode *child = parent->mChildren.front();

  if (child->IsFail())
    return;

  unsigned child_start = child->mStartIndex;
  MASSERT((parent_start == child_start)
          && "In Zeroorone node parent and child has different start index");

  if (child->IsToken()) {
    // The only child is a token.
    MASSERT((parent_match == child_start)
            && "Token node match_index != start_index ??");
  } else {
    // 1. We only preserve the one same as parent_match. Else will be reduced.
    // 2. Add child to the working list
    SuccMatch *succ = FindSucc(child->GetTable());
    succ->ReduceMatches(child_start, parent_match);
    to_be_sorted.push_back(child);
  }

  // Finally add the only successful child to mSortedChildren
  parent->mSortedChildren.push_back(child);
}

// 'parent' is already trimmed when passed into this function.
// [NOTE] Concatenate will conduct SecondTry if possible. This means it's possible
//        for a node not to connect to the longest match of its previous node.
void Parser::SortOutConcatenate(AppealNode *parent) {
  RuleTable *table = parent->GetTable();
  MASSERT(table && "parent is not a table?");

  SuccMatch *parent_succ = FindSucc(table);
  MASSERT(parent_succ && "parent has no SuccMatch?");

  unsigned parent_start = parent->mStartIndex;
  bool     found = parent_succ->GetStartToken(parent_start);
  unsigned parent_match = parent_succ->GetOneMatch(0);

  // It's possible for 'parent' to match nothing if all children are Zeroorxxx
  // which matches nothing.
  unsigned match_num = parent_succ->GetMatchNum();
  if (match_num == 0)
    return;

  // In Concatenate node, some ZeroorXXX children could match nothing.i
  // Although they are succ, but we treat them as bad children.
  // This loop mainly verifies if the matches are connected between children.

  unsigned last_match = parent_start - 1;

  // If we did second try, it's possible there are failed children of parent.
  // The failed children will be followed directly by a
  // succ child with the same rule table, same start index, and mIsSecondTry.
  //
  // Decided not to clean failed second try, since it may contain successful matching
  // which cause later good nodes as SuccWasSucc, and we need re-traverse it
  // when creating AST. So the following line is commented.
  //
  //CleanFailedSecondTry(parent);

  std::vector<AppealNode*>::iterator it = parent->mChildren.begin();
  AppealNode *prev_child;

  for (; it != parent->mChildren.end(); it++) {
    AppealNode *child = *it;

    // Failed not should be failed second try node.
    if(child->IsFail())
      continue;

    // Clean the useless matches of prev_child, leaving the only one connecting to
    // the current child.
    if (child->mIsSecondTry) {
      SuccMatch *prev_succ = FindSucc(prev_child->GetTable());
      last_match = child->mStartIndex - 1;
      prev_succ->ReduceMatches(prev_child->mStartIndex, last_match);
    }

    MASSERT((last_match + 1 == child->mStartIndex)
             && "Node match index are not connected in SortOut Concatenate.");

    if (child->IsToken()) {
      // Token node matches just one token.
      last_match++;
    } else {
      SuccMatch *succ = FindSucc(child->GetTable());
      MASSERT(succ && "ruletable node has no SuccMatch?");

      unsigned start_node = last_match + 1;
      bool found = succ->GetStartToken(start_node);  // in order to set mTempIndex
      MASSERT(found && "Couldn't find start token?");

      unsigned match_num = succ->GetMatchNum();

      // if (match_num == 0) 
      // It could be ZeroorXXX node matching nothing. Just keep it in mSortedChildren
      // since during AST generation we need index of a child. So keeping it for
      // the position.

      // we only handle cases where match_num >= 1
      if (match_num == 1) {
        last_match = succ->GetOneMatch(0);
        // Add child to the working list
        to_be_sorted.push_back(child);
      } else if (match_num > 1) {
        // We only preserve the longest matching. Else nodes will be reduced.
        unsigned longest = start_node;
        unsigned longest_index = 0;
        for (unsigned i = 0; i < succ->GetMatchNum(); i++) {
          unsigned curr_match = succ->GetOneMatch(i);
          if (curr_match > longest) {
            longest = curr_match;
            longest_index = i;
          }
        }
        succ->ReduceMatches(longest_index);
        last_match = longest;

        // Add child to the working list
        to_be_sorted.push_back(child);
      }
    }

    parent->mSortedChildren.push_back(child);
    
    prev_child = child;
  }

  // The last valid child's match token should be the same as parent's
  MASSERT((last_match == parent_match) && "Concatenate children not connected?");

  return;
}

// 'parent' is already trimmed when passed into this function.
void Parser::SortOutData(AppealNode *parent) {
  RuleTable *parent_table = parent->GetTable();
  MASSERT(parent_table && "parent is not a table?");

  TableData *data = parent_table->mData;
  switch (data->mType) {
  case DT_Subtable:
    // There should be one child node, which represents the subtable.
    // we just need to add the child node to working list.
    MASSERT((parent->mChildren.size() == 1) && "Should have only one child?");
    to_be_sorted.push_back(parent->mChildren.front());
    parent->mSortedChildren.push_back(parent->mChildren.front());
    break;
  case DT_Token:
    // token in table-data created a Child AppealNode
    // Just keep the child node. Don't need do anything.
    parent->mSortedChildren.push_back(parent->mChildren.front());
    break;
  case DT_Char:
  case DT_String:
  case DT_Type:
  case DT_Null:
  default:
    break;
  }
}

// This is for Concatenate node.
// If there are any failed children node and they are failed second try, clean them.
void Parser::CleanFailedSecondTry(AppealNode *parent) {
  std::vector<AppealNode*>::iterator it = parent->mChildren.begin();
  std::vector<AppealNode*> bad_children;

  bool found = false;
  AppealNode *prev_child = NULL;
  for (; it != parent->mChildren.end(); it++) {
    AppealNode *child = *it;
    if (child->IsFail()) {
      bad_children.push_back(child);
      if (!found) {
        found = true;
      } else {
        // Prev_child is also failed, they should share the same startindex, rule table
        MASSERT( (child->mStartIndex == prev_child->mStartIndex)
                 && (child->GetTable() == prev_child->GetTable())
                 && "Not the same startindex or ruletable in failed 2nd try?");
      }
    } else {
      // For a succ second try, it may or may not have some prev children sharing same
      // start index. (1) If it's a token, we don't create appeal node for failed token.
      // So it have NO failed children. (2) for table, we are sure to have failed children.
      if (found) {
        MASSERT( (child->mStartIndex == prev_child->mStartIndex)
                 && (child->GetTable() == prev_child->GetTable())
                 && "Not the same startindex or ruletable in failed 2nd try?");
        MASSERT( child->mIsSecondTry && "Not a second try after a failed node?");
        // clear the status, ending a session of second try.
        found = false;
      }
    }
    prev_child = child;
  }

  // remove the bad children AppealNode
  std::vector<AppealNode*>::iterator badit = bad_children.begin();
  for (; badit != bad_children.end(); badit++) {
    parent->RemoveChild(*badit);
  }
}

// Dump the result after SortOut. We should see a tree with root being one of the
// top rules. We ignore mRootNode since it's just a fake one.

static std::deque<AppealNode *> to_be_dumped;
static std::deque<unsigned> to_be_dumped_id;
static unsigned seq_num = 1;

void Parser::DumpSortOut(AppealNode *root) {
  std::cout << "=======  Dump SortOut =======  " << std::endl;
  // we start from the only child of mRootNode.
  to_be_dumped.clear();
  to_be_dumped_id.clear();

  to_be_dumped.push_back(root);
  to_be_dumped_id.push_back(seq_num++);

  while(!to_be_dumped.empty()) {
    AppealNode *node = to_be_dumped.front();
    to_be_dumped.pop_front();
    DumpSortOutNode(node);
  }
}

void Parser::DumpSortOutNode(AppealNode *n) {
  unsigned dump_id = to_be_dumped_id.front();
  to_be_dumped_id.pop_front();

  std::cout << "[" << dump_id << "] ";
  if (n->IsToken()) {
    std::cout << "Token" << std::endl;
  } else {
    RuleTable *t = n->GetTable();
    std::cout << "Table " << GetRuleTableName(t) << "@" << n->mStartIndex << ": ";

    if (n->mAfter == SuccWasSucc)
      std::cout << "WasSucc";

    EntryType type = t->mType;
    if (type == ET_Zeroorone || type == ET_Zeroormore) {
      SuccMatch *succ = FindSucc(t);
      bool found = succ->GetStartToken(n->mStartIndex);  // in order to set mTempIndex
      unsigned match_num = succ->GetMatchNum();
      if (match_num == 0)
        std::cout << "KeepPosition";
    }

    std::vector<AppealNode*>::iterator it = n->mSortedChildren.begin();
    for (; it != n->mSortedChildren.end(); it++) {
      std::cout << seq_num << ",";
      to_be_dumped.push_back(*it);
      to_be_dumped_id.push_back(seq_num++);
    }
    std::cout << std::endl;
  }
}

/////////////////////////////////////////////////////////////////////////////
//                       Build AST
/////////////////////////////////////////////////////////////////////////////

// The tree is created dynamically, remember to release it if not used any more.
//
// The AppealNode tree will be depth first traversed. The node will be put in
// a stack and its TreeNode will be created FILO. In this way, the leaf node will
// be first created and the parents.

// A appealnode is popped out and create tree node if and only if all its
// children have been generated a tree node for them.
static std::vector<AppealNode*> done_nodes;
static bool NodeIsDone(AppealNode *n) {
  std::vector<AppealNode*>::const_iterator cit = done_nodes.begin();
  for (; cit != done_nodes.end(); cit++) {
    if (*cit == n)
      return true;
  }
  return false;
}

static std::vector<AppealNode*> was_succ_list;

// The SuccWasSucc node and its patching node is a one-one mapping.
// We don't use a map to maintain this. We use to vectors to do this
// by adding the pair of nodes at the same time. Their index in the
// vectors are the same.
static std::vector<AppealNode*> was_succ_matched_list;
static std::vector<AppealNode*> patching_list;

// Find the nodes which are SuccWasSucc
void Parser::FindWasSucc(AppealNode *root) {
  std::deque<AppealNode*> working_list;
  working_list.push_back(root);
  while (!working_list.empty()) {
    AppealNode *node = working_list.front();
    working_list.pop_front();
    if (node->mAfter == SuccWasSucc)
      was_succ_list.push_back(node);
    else {
      std::vector<AppealNode*>::iterator it = node->mSortedChildren.begin();
      for (; it != node->mSortedChildren.end(); it++)
        working_list.push_back(*it);
    }
  }
  return NULL;
}

// Check if 'n' is a matching for one and only one in was_succ_list.
// Return the one in was_succ_list.
static bool IsGoodMatching(AppealNode *n) {
  // step 1. It should be succ.
  if (n->IsFail())
    return false;

  // step 2. Find the correct SuccWasSucc node
  std::vector<AppealNode*>::iterator it = was_succ_list.begin();
  AppealNode *found = NULL;
  for (; it != was_succ_list.end(); it++) {
    AppealNode *was_succ = *it;
    if (n->SuccEqualTo(was_succ)) {
      MASSERT( !found && "This should be a one-one mapping, got wrong?");
      found = was_succ;
      // We don't break the loop here. Let it finish to verify the
      // one-one mapping
    }
  }

  if (!found)
    return false;

  // step 3. Put the was_succ into was_succ_matched_list.
  //         And put the 'n' into patching_list. This maintains one-one mapping.
  was_succ_matched_list.push_back(found);
  patching_list.push_back(n);

  return true;
}

// For each node in was_succ_list there is one and only patching subtree.
void Parser::FindPatchingNodes(AppealNode *root) {
  std::deque<AppealNode*> working_list;
  working_list.push_back(root);
  while (!working_list.empty()) {
    AppealNode *node = working_list.front();
    working_list.pop_front();
    bool was_succ = IsGoodMatching(node);
    if (!was_succ) {
      std::vector<AppealNode*>::iterator it = node->mChildren.begin();
      for (; it != node->mChildren.end(); it++)
        working_list.push_back(*it);
    }
  }
}

// This is another entry point of sort, similar as SortOut().
// The only difference is we use 'target' as the reference of 'root'
// when trimming 'root'.
void Parser::SupplementalSortOut(AppealNode *root, AppealNode *target) {
  MASSERT(root->mSortedChildren.size()==0 && "root should be un-sorted.");
  MASSERT(root->IsTable() && "root should be a table node.");

  // step 1. Find the only matching token index we want.

  SuccMatch *succ = FindSucc(target->GetTable());
  MASSERT(succ && "ruletable node has no SuccMatch?");

  bool found = succ->GetStartToken(target->mStartIndex);  // in order to set mTempIndex
  MASSERT(found && "Couldn't find start token?");
  unsigned match_num = succ->GetMatchNum();
  MASSERT(match_num == 1 && "target should have been trimmed.");
  unsigned match = succ->GetOneMatch(0);

  // step 2. Trim the root.

  succ = FindSucc(root->GetTable());
  MASSERT(succ && "ruletable node has no SuccMatch?");
  succ->ReduceMatches(root->mStartIndex, match);

  // step 3. Start the sort out

  to_be_sorted.clear();
  to_be_sorted.push_back(root);

  while(!to_be_sorted.empty()) {
    AppealNode *node = to_be_sorted.front();
    to_be_sorted.pop_front();
    SortOutNode(node);
  }

  if (mTraceSortOut)
    DumpSortOut(root);
}

// In the tree after SortOut, some nodes could be SuccWasSucc and we didn't build
// sub-tree for its children. Now it's time to patch the sub-tree.
void Parser::PatchWasSucc(AppealNode *root) {
  return;
  while(1) {
    // step 1. Traverse the sorted tree, find the target node which is SuccWasSucc
    was_succ_list.clear();
    FindWasSucc(root);
    if (was_succ_list.empty())
      break;

    // step 2. Traverse the original tree, find the subtree matching target
    was_succ_matched_list.clear();
    patching_list.clear();
    FindPatchingNodes(root);
    MASSERT( !patching_list.empty() && "Cannot find any patching for SuccWasSucc.");

    // step 3. Assert the subtree is not sorted. Then SupplementalSortOut()
    //         Copy the subtree of patch to was_succ
    for (unsigned i = 0; i < was_succ_matched_list.size(); i++) {
      AppealNode *patch = patching_list[i];
      AppealNode *was_succ = was_succ_matched_list[i];
      SupplementalSortOut(patch, was_succ);
      for (unsigned j = 0; j < patch->mChildren.size(); j++)
        was_succ->AddChild(patch->mChildren[j]);
    }
  }
}

void Parser::SimplifySortedTree(AppealNode *root) {
}

ASTTree* Parser::BuildAST(AppealNode *root) {
  // temporarily disable BuildAST.
  return NULL;

  if (!root)
    return NULL;

  done_nodes.clear();

  ASTTree *tree = new ASTTree();

  std::stack<AppealNode*> appeal_stack;
  appeal_stack.push(root);

  // Need a map between an AppealNode and a TreeNode.
  std::map<AppealNode*, TreeNode*> nodes_map;

  while(!appeal_stack.empty()) {
    AppealNode *node = appeal_stack.top();
    // 1) If all children done. Time to create tree node for 'node'
    // 2) If some are done, some not. Add the first not-done child to stack 
    bool children_done = true;
    std::vector<AppealNode*>::iterator it = node->mChildren.begin();
    for (; it != node->mChildren.end(); it++) {
      AppealNode *child = *it;
      if (!NodeIsDone(child)) {
        appeal_stack.push(child);
        children_done = false;
        break;
      }
    }

    if (children_done) {
      appeal_stack.pop();
      // create the tree node for node.
    }
  }
}

// We take two parameters. One is the AST Tree, since the tree node is part of
// of the tree and will be allocated in the tree's memory pool. The other is
// the appeal node.
//
// For many rules, it's just a Oneof node, which means the appeal_node itself
// doesn't introduce anything into the tree. It's just transferring. 
TreeNode* Parser::NewTreeNode(ASTTree *tree, AppealNode *appeal_node) {
  return NULL;
}

/////////////////////////////////////////////////////////////////////////////
//             Initialized the predefined tokens.
/////////////////////////////////////////////////////////////////////////////

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
// They need be used together as mTempIndex is defined/used between them.
//
// If you do want to use them separately, you need set the mTempIndex correctly through
// GetStartToken(t).

// Temporarily set the num of matches to 0. For the ZeroorXXX node, even there is no
// matching it is still a succ. This will also be saved in SuccMatch, with matching num
// set as 0.
void SuccMatch::AddStartToken(unsigned t) {
  mCache.push_back(t);
  mCache.push_back(0);
  mTempIndex = mCache.size() - 1;
}

void SuccMatch::AddOneMoreMatch(unsigned m) {
  mCache.push_back(m);
  mCache[mTempIndex] += 1;
}

// Below are three Query functions for succ info
// Again, They need be used together as mTempIndex is transferred between them.

// Find the succ info for token 't'. Return true if found.
bool SuccMatch::GetStartToken(unsigned t) {
  // traverse the vector
  // mTempIndex is used as the index of the start token.
  mTempIndex = 0;

  // The last position mTempIndex can be nearest to the end of mCache vector, is
  // when the last start token has 0 matching, so it looks like,
  //  ...., N, 0
  // 'N' represents the position of last mTempIndex.
  while (mTempIndex < (mCache.size() - 1)) {
    if (t != mCache[mTempIndex] ){
      unsigned num = mCache[mTempIndex + 1];
      mTempIndex += 1;   // at the num
      mTempIndex += num; // at last matching token.
      mTempIndex += 1;   // at new index
    } else
      break;
  }

  if (mTempIndex < (mCache.size() - 1))
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

// Reduce all matchings, except the except-th
// Assume mTempIndex is already set through GetStartToken().
//
// Since we are in the vector, we have to manipulate this by ourselves.
void SuccMatch::ReduceMatches(unsigned e_idx) {
  unsigned except = GetOneMatch(e_idx);
  unsigned orig_num = GetMatchNum();
  unsigned reduced_num = orig_num - 1;
  if (!reduced_num)
    return;

  // Step 1. Set the num of match to 1
  mCache[mTempIndex + 1] = 1;

  // Step 2. Set the only match
  mCache[mTempIndex + 2] = except;

  // Step 3. Move all the rest data 'reduced_num' position ahead
  unsigned index = mTempIndex + 3;
  unsigned new_vector_size = mCache.size() - reduced_num;
  for (; index < new_vector_size; index++)
    mCache[index] = mCache[index + reduced_num];

  // Step 4. update the vector size
  mCache.resize(new_vector_size);
}

// 1. Check all matches to verify there is only one matching 'except'
//    if no matching, returns false
//    if >1 matchings, it's an error, ambiguous
//    if 1  matching, success
// 2. Reduce all matches except the one equal to except.
// 3. Returns true : if it successfully remove all rest matchings.
//           false : if the verification fails
//
// Since we are in the vector, we have to manipulate this by ourselves.
bool SuccMatch::ReduceMatches(unsigned start, unsigned except) {
  bool found = GetStartToken(start);
  MASSERT( found && "Couldn't find the start token?");

  // Verify there is one and only one match equal to 'except'

  int except_index = -1;
  unsigned except_num = 0;
  unsigned orig_num = GetMatchNum();
  for (unsigned idx = 0; idx < orig_num; idx++) {
    if (GetOneMatch(idx) == except) {
      except_index = idx;
      except_num++;
    }
  }

  if (except_num == 0) {
    return false;
  } else if (except_num > 1) {
    std::cout << "Oneof node has more than one rules matching same num of tokens? Ambiguous!"
              << std::endl;
    exit(1);
  }

  // At this point, except_num == 1, there is one and only one matching. Now reduce them.
  ReduceMatches(except_index);

  return true;
}

///////////////////////////////////////////////////////////////
//            AppealNode function
///////////////////////////////////////////////////////////////

// Returns true, if both nodes are successful and match the same tokens
// with the same rule table
bool AppealNode::SuccEqualTo(AppealNode *other) {
  if (IsSucc() && other->IsSucc() && mStartIndex == other->mStartIndex) {
    if (IsToken() && other->IsToken()) {
      return GetToken() == other->GetToken();
    } else if (IsTable() && other->IsTable()) {
      return GetTable() == other->GetTable();
    }
  }
  return false;
}

void AppealNode::RemoveChild(AppealNode *child) {
  std::vector<AppealNode*> temp_vector;
  std::vector<AppealNode*>::iterator it = mChildren.begin();
  for (; it != mChildren.end(); it++) {
    if (*it != child)
      temp_vector.push_back(*it);
  }

  mChildren.clear();
  mChildren.assign(temp_vector.begin(), temp_vector.end());
}
