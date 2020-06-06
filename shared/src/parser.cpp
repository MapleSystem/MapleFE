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
#include "ast_builder.h"

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
// 3. Left Recursion
//
// MapleFE is an LL parser, and left recursion has to be handled if we allow language
// designer to write left recursion. I personally believe left recursion is a much
// simpler, more human friendly and stronger way to describe language spec. To
// provide this juicy feature, parser has to do extra job.
//
// The main idea of handling left recursion includes two parts.
// (1) Tool 'recdetect' finds out all the left recursions in the language spec.
//     Dump them as tables into source code as part of parser.
// (2) When parser is traversing the rule tables, whenever it sees a left recursion,
//     it simply tries the recursion to see how many tokens the recursion can eat.
//
// Let's look at a few examples.
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
// a + b + c + d + ... is a good example.
//
// Another good example is Block. There are nested blocks. So loop exists.
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

Parser::Parser(const char *name) : filename(name) {
  mLexer = new Lexer();
  const std::string file(name);

  gModule.SetFileName(name);
  mLexer->PrepareForFile(file);
  mCurToken = 0;
  mPending = 0;

  mTraceTable = false;
  mTraceAppeal = false;
  mTraceSecondTry = false;
  mTraceVisited = false;
  mTraceFailed = false;
  mTraceSortOut = false;
  mTraceAstBuild = false;
  mTracePatchWasSucc = false;
  mTraceWarning = false;

  mIndentation = 0;
  mInSecondTry = false;
  mRoundsOfPatching = 0;
}

Parser::~Parser() {
  delete mLexer;
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

  gModule.Dump();

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
//   and at the second time we may not find the @9 token when we trying to clean WasFailed
//   because it was already cleared.
//
//   We will just ignore the second clearing of FieldAccess if we cannot find the token num
//   in the mFailed.

//static std::vector<AppealNode*> traverse_list;
//void Parser::AppealTraverse(AppealNode *node, AppealNode *root) {
//  traverse_list.push_back(node);
//
//  MASSERT((root->mAfter == Succ) && "root->mAfter is not Succ.");
//  if ((node->GetTable() == root->GetTable()) && (node->mAfter == FailLooped)) {
//    // 1. Walk the list, and clear the fail flag for corresponding rule table.
//    // 2. For this specific AppealNode, it's mAfter is set to Fail, we are not changing it.
//    for (unsigned i = 0; i < traverse_list.size(); i++) {
//      AppealNode *n = traverse_list[i];
//      if ((n->mBefore == Succ) && (n->mAfter == FailChildrenFailed)) {
//        if (mTraceAppeal)
//          DumpAppeal(n->GetTable(), n->GetStartIndex());
//        ResetFailed(n->GetTable(), n->GetStartIndex());
//      }
//    }
//  }
//
//  std::vector<AppealNode*>::iterator it = node->mChildren.begin();
//  for (; it != node->mChildren.end(); it++) {
//    AppealNode *child = *it;
//    if (child->IsToken())
//      continue;
//    AppealTraverse(child, root);
//    traverse_list.pop_back();
//  }
//}

//void Parser::Appeal(AppealNode *root) {
//  traverse_list.clear();
//  traverse_list.push_back(root);
//
//  std::vector<AppealNode*>::iterator it = root->mChildren.begin();
//  for (; it != root->mChildren.end(); it++) {
//    AppealNode *child = *it;
//    if (child->IsToken())
//      continue;
//    AppealTraverse(child, root);
//    traverse_list.pop_back();
//  }
//}

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
    PatchWasSucc(mRootNode->mSortedChildren[0]);
    SimplifySortedTree();
    ASTTree *tree = BuildAST();
    if (tree) {
      gModule.AddTree(tree);
    }
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
    else if (reason == FailNotIdentifier)
      std::cout << " fail@NotIdentifer" << "}" << std::endl;
    else if (reason == FailNotLiteral)
      std::cout << " fail@NotLiteral" << "}" << std::endl;
    else if (reason == FailChildrenFailed)
      std::cout << " fail@ChildrenFailed" << "}" << std::endl;
    else if (reason == AppealStatus_NA)
      std::cout << " fail@NA" << "}" << std::endl;
  }
}

// Please read the comments point 6 at the beginning of this file.
// We need prepare certain storage for multiple possible matchings. The successful token
// number could be more than one. I'm using fixed array to save them. If needed to extend
// in the future, just extend it.
#define MAX_SUCC_TOKENS 16
unsigned gSuccTokensNum;
unsigned gSuccTokens[MAX_SUCC_TOKENS];

void Parser::DumpSuccTokens() {
  std::cout << "gSuccTokensNum=" << gSuccTokensNum << ": ";
  for (unsigned i = 0; i < gSuccTokensNum; i++)
    std::cout << gSuccTokens[i] << ",";
  std::cout << std::endl;
}

// The preparation of TraverseRuleTable().
AppealNode* Parser::TraverseRuleTablePre(RuleTable *rule_table, AppealNode *parent) {
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
  appeal->SetStartIndex(mCurToken);
  appeal->SetParent(parent);
  parent->AddChild(appeal);

  if (mInSecondTry) {
    appeal->mIsSecondTry = true;
    mInSecondTry = false;
  }


  // Check if it was succ. Set the gSuccTokens/gSuccTokensNum appropriately
  // The longest matching is chosen for the next rule table to match.
  SuccMatch *succ = FindSucc(rule_table);
  if (succ) {
    bool was_succ = succ->GetStartToken(mCurToken);
    if (was_succ) {
      gSuccTokensNum = succ->GetMatchNum();
      for (unsigned i = 0; i < gSuccTokensNum; i++) {
        gSuccTokens[i] = succ->GetOneMatch(i);
        // WasSucc nodes need Match info, which will be used later
        // in the sort out.
        appeal->AddMatch(gSuccTokens[i]);
        if (gSuccTokens[i] > mCurToken)
          mCurToken = gSuccTokens[i];
      }

      // In ZeroorXXX cases, it was successful and has SuccMatch. However,
      // it could be a failure. In this case, we shouldn't move mCurToken.
      if (gSuccTokensNum > 0)
        MoveCurToken();

      if (mTraceTable)
        DumpExitTable(name, mIndentation, true, SuccWasSucc);
      mIndentation -= 2;

      appeal->mAfter = SuccWasSucc;
      return appeal;
    }
  }

  if (WasFailed(rule_table, mCurToken)) {
    if (mTraceTable)
      DumpExitTable(name, mIndentation, false, FailWasFailed);
    mIndentation -= 2;
    appeal->mAfter = FailWasFailed;
    return appeal;
  }

  return appeal;
}

// return true : if the rule_table is matched
//       false : if faled.
bool Parser::TraverseRuleTable(RuleTable *rule_table, AppealNode *parent) {

  // Step 1. If the 'rule_table' has already been done, we just reuse the result.
  //         This step is in TraverseRuleTablePre().

  AppealNode *appeal = TraverseRuleTablePre(rule_table, parent);
  if (appeal->IsSucc())
    return true;
  else if (appeal->IsFail())
    return false;

  // At this point, the node is at the initial status. It needs traversal.
  MASSERT(appeal->IsNA());

  // Step 2. If it's a LeadNode, we will go through special handling.

  if (IsLeadNode(rule_table))
    return TraverseLeadNode(appeal, parent);

  // Step 3. It's a regular table. Traverse children in DFS.

  bool matched = false;
  unsigned old_pos = mCurToken;
  Token *curr_token = mActiveTokens[mCurToken];
  gSuccTokensNum = 0;

  // [NOTE] TblLiteral and TblIdentifier don't use the SuccMatch info,
  //        since it's quite simple, we don't need SuccMatch to reduce
  //        the traversal time.
  if ((rule_table == &TblIdentifier))
    return TraverseIdentifier(rule_table, appeal);

  if ((rule_table == &TblLiteral))
    return TraverseLiteral(rule_table, appeal);

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

  if (mTraceTable) {
    const char *name = GetRuleTableName(rule_table);
    DumpExitTable(name, mIndentation, matched, FailChildrenFailed);
  }
  mIndentation -= 2;

  if (mTraceSecondTry)
    DumpSuccTokens();

  if(matched) {
    // It's guaranteed that this rule_table was NOT succ for mCurToken. Need add it.
    SuccMatch *succ_match = FindOrCreateSucc(rule_table);
    succ_match->AddStartToken(old_pos);
    succ_match->AddSuccNode(appeal);
    for (unsigned i = 0; i < gSuccTokensNum; i++) {
      appeal->AddMatch(gSuccTokens[i]);
      succ_match->AddMatch(gSuccTokens[i]);
    }

    // We try to appeal only if it succeeds at the end.
    appeal->mAfter = Succ;
    //if (appeal->IsTable())
    //  Appeal(appeal);
    return true;
  } else {
    appeal->mAfter = FailChildrenFailed;
    mCurToken = old_pos;
    AddFailed(rule_table, mCurToken);
    return false;
  }
}

bool Parser::TraverseToken(Token *token, AppealNode *parent) {
  Token *curr_token = mActiveTokens[mCurToken];
  bool found = false;
  mIndentation += 2;

  if (mTraceTable)
    DumpEnterTable("token", mIndentation);

  if (token == curr_token) {
    AppealNode *appeal = new AppealNode();
    mAppealNodes.push_back(appeal);
    appeal->mAfter = Succ;
    appeal->SetToken(curr_token);
    appeal->SetStartIndex(mCurToken);
    appeal->AddMatch(mCurToken);
    appeal->SetParent(parent);
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
    DumpExitTable("token", mIndentation, found, AppealStatus_NA);
  if (mTraceSecondTry)
    DumpSuccTokens();

  mIndentation -= 2;
  return found;
}

// Supplemental function invoked when TraverseSpecialToken succeeds.
// It helps set all the data structures.
void Parser::TraverseSpecialTableSucc(RuleTable *rule_table, AppealNode *appeal) {
  const char *name = GetRuleTableName(rule_table);
  Token *curr_token = mActiveTokens[mCurToken];
  gSuccTokensNum = 1;
  gSuccTokens[0] = mCurToken;

  appeal->mAfter = Succ;
  appeal->SetToken(curr_token);
  appeal->SetStartIndex(mCurToken);
  appeal->AddMatch(mCurToken);

  MoveCurToken();

  if (mTraceTable)
    DumpExitTable(name, mIndentation, true);
  mIndentation -= 2;
}

// Supplemental function invoked when TraverseSpecialToken fails.
// It helps set all the data structures.
void Parser::TraverseSpecialTableFail(RuleTable *rule_table,
                                      AppealNode *appeal,
                                      AppealStatus status) {
  const char *name = GetRuleTableName(rule_table);
  AddFailed(rule_table, mCurToken);
  if (mTraceTable)
    DumpExitTable(name, mIndentation, false, status);
  mIndentation -= 2;
  appeal->mAfter = status;
}

// We don't go into Literal table.
// 'appeal' is the node for this rule table. This is different than TraverseOneof
// or the others where 'appeal' is actually a parent node.
bool Parser::TraverseLiteral(RuleTable *rule_table, AppealNode *appeal) {
  Token *curr_token = mActiveTokens[mCurToken];
  const char *name = GetRuleTableName(rule_table);
  bool found = false;
  gSuccTokensNum = 0;

  if (curr_token->IsLiteral()) {
    found = true;
    TraverseSpecialTableSucc(rule_table, appeal);
  } else {
    TraverseSpecialTableFail(rule_table, appeal, FailNotLiteral);
  }

  if (mTraceSecondTry)
    DumpSuccTokens();

  return found;
}

// We don't go into Identifier table.
// 'appeal' is the node for this rule table. In other TraverseXXX(),
// 'appeal' is parent node.
bool Parser::TraverseIdentifier(RuleTable *rule_table, AppealNode *appeal) {
  Token *curr_token = mActiveTokens[mCurToken];
  const char *name = GetRuleTableName(rule_table);
  bool found = false;
  gSuccTokensNum = 0;

  if (curr_token->IsIdentifier()) {
    found = true;
    TraverseSpecialTableSucc(rule_table, appeal);
  } else {
    TraverseSpecialTableFail(rule_table, appeal, FailNotIdentifier);
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

  // Curr status, it could be failure.
  unsigned curr_succ_tokens_num = 0;
  unsigned curr_succ_tokens[MAX_SUCC_TOKENS];

  // Final status. It only saves the latest successful status.
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
        //         We will never go that far. We'll stop at the first matching which matches
        //         the most tokens. So a small sorting is required.

        SmallList<unsigned> sorted;
        for (unsigned j = 0; j < prev_succ_tokens_num; j++)
          sorted.PushBack(prev_succ_tokens[j]);
        sorted.SortDescending();

        bool temp_found = false;
        for (unsigned j = 0; j < sorted.GetNum(); j++) {
          // The longest matching has been proven to be a failure.
          if (sorted.ValueAtIndex(j)== old_pos)
            continue;

          mInSecondTry = true;

          // Start from the one after succ token.
          mCurToken = sorted.ValueAtIndex(j) + 1;
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
          // Usually, for Zeroorone/Zeroormore node, it may set found to true, but actually
          // it may matching nothing. Although it doesn't move mCurToken, it does move the
          // rule. I'll buy this succ.
          final_succ_tokens_num = gSuccTokensNum;
          for (unsigned id = 0; id < gSuccTokensNum; id++)
            final_succ_tokens[id] = gSuccTokens[id];
          curr_succ_tokens_num = gSuccTokensNum;
          for (unsigned id = 0; id < gSuccTokensNum; id++)
            curr_succ_tokens[id] = gSuccTokens[id];
        }

        if (mTraceSecondTry) {
          if (found)
            std::cout << "Second Try succ. mCurToken=" << mCurToken << std::endl;
          else
            std::cout << "Second Try fail." << std::endl;
        }
      }
    }

    // After second try, if it still fails, we quit.
    if (!found)
      break;

    prev_succ_tokens_num = curr_succ_tokens_num;
    for (unsigned id = 0; id < curr_succ_tokens_num; id++)
      prev_succ_tokens[id] = curr_succ_tokens[id];
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
    found = TraverseToken(data->mData.mToken, parent);
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
static std::deque<AppealNode*> to_be_sorted;

void Parser::SortOut() {
  // we remove all failed children, leaving only succ child
  std::vector<AppealNode*>::iterator it = mRootNode->mChildren.begin();
  for (; it != mRootNode->mChildren.end(); it++) {
    AppealNode *n = *it;
    if (!n->IsFail())
      mRootNode->mSortedChildren.push_back(n);
  }
  MASSERT(mRootNode->mSortedChildren.size()==1);
  AppealNode *root = mRootNode->mSortedChildren.front();

  // First sort the root.
  RuleTable *table = root->GetTable();
  MASSERT(table && "root is not a table?");
  SuccMatch *succ = FindSucc(table);
  MASSERT(succ && "root has no SuccMatch?");
  bool found = succ->GetStartToken(root->GetStartIndex());

  // Top level tree can have only one match, otherwise, the language
  // is ambiguous.
  unsigned match_num = succ->GetMatchNum();
  MASSERT(match_num == 1 && "Top level tree has >1 matches?");
  unsigned match = succ->GetOneMatch(0);
  root->SetFinalMatch(match);
  root->SetSorted();

  to_be_sorted.clear();
  to_be_sorted.push_back(root);

  while(!to_be_sorted.empty()) {
    AppealNode *node = to_be_sorted.front();
    to_be_sorted.pop_front();
    SortOutNode(node);
  }

  if (mTraceSortOut)
    DumpSortOut(root, "Main sortout");
}

// 'node' is already trimmed when passed into this function.
void Parser::SortOutNode(AppealNode *node) {
  MASSERT(node->IsSorted() && "Node is NOT sorted?");
  MASSERT(node->IsSucc() && "Failed node in SortOut?");

  // A token's appeal node is a leaf node. Just return.
  if (node->IsToken()) {
    node->SetFinalMatch(node->GetStartIndex());
    return;
  }

  // If node->mAfter is SuccWasSucc, it means we didn't traverse its children
  // during matching. In SortOut, we simple return. However, when generating IR,
  // the children have to be created.
  if (node->mAfter == SuccWasSucc) {
    MASSERT(node->mChildren.size() == 0);
    return;
  }

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

// 'parent' is already sorted when passed into this function.
void Parser::SortOutOneof(AppealNode *parent) {
  MASSERT(parent->IsSorted() && "parent is not sorted?");

  // It's possible it actually matches nothing, such as all children are Zeroorxxx
  // and they match nothing. But they ARE successful.
  unsigned match_num = parent->GetMatchNum();
  if (match_num == 0)
    return;

  unsigned parent_match = parent->GetFinalMatch();
  unsigned good_children = 0;
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
      if (child->GetStartIndex() == parent_match) {
        child->SetSorted();
        child->SetFinalMatch(parent_match);
        good_children++;
        parent->mSortedChildren.push_back(child);
      }
    } else {
      bool found = child->FindMatch(parent_match);
      if (found) {
        good_children++;
        to_be_sorted.push_back(child);
        parent->mSortedChildren.push_back(child);
        child->SetFinalMatch(parent_match);
        child->SetSorted();
      }
    }

    // We only pick the first good child.
    if (good_children > 0)
      break;
  }
}

// For Zeroormore node, where all children's matching tokens are linked together one after another.
// All children nodes have the same rule table, but each has its unique AppealNode.
//
void Parser::SortOutZeroormore(AppealNode *parent) {
  MASSERT(parent->IsSorted());

  // Zeroormore could match nothing.
  unsigned match_num = parent->GetMatchNum();
  if (match_num == 0)
    return;

  unsigned parent_start = parent->GetStartIndex();
  unsigned parent_match = parent->GetFinalMatch();

  // In Zeroormore node, all children should have 'really' matched tokens which means they 
  // did move forward as token index. So there shouldn't be any bad children. 
  // The major work of this loop is to verify the above claim is abided by.
  //
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

    MASSERT((last_match + 1 == child->GetStartIndex())
             && "Node match index are not connected in SortOut Zeroormore.");

    if (child->IsToken()) {
      // Token node matches just one token.
      last_match++;
      child->SetSorted();
      child->SetFinalMatch(last_match);
    } else {
      // We choose the longest match.
      unsigned longest = last_match + 1;
      for (unsigned i = 0; i < child->GetMatchNum(); i++) {
        unsigned curr_match = child->GetMatch(i);
        if (curr_match > longest)
          longest = curr_match;
      }
      child->SetFinalMatch(longest);
      child->SetSorted();
      last_match = longest;
      to_be_sorted.push_back(child);
    }
  }

  return;
}

// 'parent' is already sorted when passed into this function.
void Parser::SortOutZeroorone(AppealNode *parent) {
  MASSERT(parent->IsSorted());
  RuleTable *table = parent->GetTable();
  MASSERT(table && "parent is not a table?");

  // Zeroorone could match nothing. We just do nothing in this case.
  unsigned match_num = parent->GetMatchNum();
  if (match_num == 0)
    return;

  unsigned parent_match = parent->GetFinalMatch();

  // At this point, there is one and only one child which may or may not match some tokens.
  // 1. If the child is failed, just remove it.
  // 2. If the child is succ, the major work of this loop is to verify the child's SuccMatch is
  //    consistent with parent's.

  MASSERT((parent->mChildren.size() == 1) && "Zeroorone has >1 valid children?");
  AppealNode *child = parent->mChildren.front();

  if (child->IsFail())
    return;

  unsigned parent_start = parent->GetStartIndex();
  unsigned child_start = child->GetStartIndex();
  MASSERT((parent_start == child_start)
          && "In Zeroorone node parent and child has different start index");

  if (child->IsToken()) {
    // The only child is a token.
    MASSERT((parent_match == child_start)
            && "Token node match_index != start_index ??");
    child->SetFinalMatch(child_start);
    child->SetSorted();
  } else {
    // 1. We only preserve the one same as parent_match. Else will be reduced.
    // 2. Add child to the working list
    bool found = child->FindMatch(parent_match);
    MASSERT(found && "The only child has different match than parent.");
    child->SetFinalMatch(parent_match);
    child->SetSorted();
    to_be_sorted.push_back(child);
  }

  // Finally add the only successful child to mSortedChildren
  parent->mSortedChildren.push_back(child);
}

// [NOTE] Concatenate will conduct SecondTry if possible. This means it's possible
//        for a successful node not to connect to the longest match of its
//        previous node.
void Parser::SortOutConcatenate(AppealNode *parent) {
  MASSERT(parent->IsSorted());
  RuleTable *table = parent->GetTable();
  MASSERT(table && "parent is not a table?");

  unsigned parent_start = parent->GetStartIndex();
  unsigned parent_match = parent->GetFinalMatch();

  // It's possible for 'parent' to match nothing if all children are Zeroorxxx
  // which matches nothing.
  unsigned match_num = parent->GetMatchNum();
  if (match_num == 0)
    return;

  // In Concatenate node, some ZeroorXXX children could match nothing.
  // Although they are succ, but we treat them as bad children.
  // This loop mainly verifies if the matches are connected between children.

  unsigned last_match = parent_start - 1;

  // If we did second try, it's possible there are failed children of parent.
  // The failed children will be followed directly by a
  // succ child with the same rule table, same start index, and mIsSecondTry.
  //
  // We did not remove failed second try, since it may contain successful matching
  // which later good nodes as SuccWasSucc will need.

  std::vector<AppealNode*>::iterator it = parent->mChildren.begin();
  AppealNode *prev_child = NULL;

  for (; it != parent->mChildren.end(); it++) {
    AppealNode *child = *it;

    // A Failed node, it should be failed second try node.
    if(child->IsFail())
      continue;

    // Clean the useless matches of prev_child, leaving the only one connecting to
    // the current child.
    if (child->mIsSecondTry) {
      last_match = child->GetStartIndex() - 1;
      MASSERT(prev_child->FindMatch(last_match));
      prev_child->SetFinalMatch(last_match);
      prev_child->SetSorted();
    }

    if (child->IsToken()) {
      // Token node matches just one token.
      last_match++;
      MASSERT(last_match == child->GetStartIndex());
      child->SetFinalMatch(last_match);

      parent->mSortedChildren.push_back(child);
      prev_child = child;
    } else {
      unsigned match_num = child->GetMatchNum();

      // if match_num == 0, It could be ZeroorXXX node matching nothing.
      // We just skip this child.
      if (match_num == 1) {
        last_match = child->GetMatch(0);
        child->SetFinalMatch(last_match);
        child->SetSorted();

        to_be_sorted.push_back(child);
        parent->mSortedChildren.push_back(child);
        prev_child = child;
      } else if (match_num > 1) {
        // We only preserve the longest matching.
        unsigned longest = child->GetStartIndex();
        for (unsigned i = 0; i < child->GetMatchNum(); i++) {
          unsigned curr_match = child->GetMatch(i);
          if (curr_match > longest)
            longest = curr_match;
        }
        child->SetFinalMatch(longest);
        last_match = longest;

        // Add child to the working list
        to_be_sorted.push_back(child);
        parent->mSortedChildren.push_back(child);

        prev_child = child;
      }
    }
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
  case DT_Subtable: {
    // There should be one child node, which represents the subtable.
    // we just need to add the child node to working list.
    MASSERT((parent->mChildren.size() == 1) && "Should have only one child?");
    AppealNode *child = parent->mChildren.front();
    child->SetFinalMatch(parent->GetFinalMatch());
    child->SetSorted();
    to_be_sorted.push_back(child);
    parent->mSortedChildren.push_back(child);
    break;
  }
  case DT_Token:
    // token in table-data created a Child AppealNode
    // Just keep the child node. Don't need do anything.
    AppealNode *child = parent->mChildren.front();
    child->SetFinalMatch(child->GetStartIndex());
    parent->mSortedChildren.push_back(child);
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
        MASSERT( (child->GetStartIndex() == prev_child->GetStartIndex())
                 && (child->GetTable() == prev_child->GetTable())
                 && "Not the same startindex or ruletable in failed 2nd try?");
      }
    } else {
      // For a succ second try, it may or may not have some prev children sharing same
      // start index. (1) If it's a token, we don't create appeal node for failed token.
      // So it have NO failed children. (2) for table, we are sure to have failed children.
      if (found) {
        MASSERT( (child->GetStartIndex() == prev_child->GetStartIndex())
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

// 'root' cannot be mRootNode which is just a fake node.
void Parser::DumpSortOut(AppealNode *root, const char *phase) {
  std::cout << "======= " << phase << " Dump SortOut =======" << std::endl;
  // we start from the only child of mRootNode.
  to_be_dumped.clear();
  to_be_dumped_id.clear();
  seq_num = 1;

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

  if (n->mSimplifiedIndex > 0)
    std::cout << "[" << dump_id << ":" << n->mSimplifiedIndex<< "] ";
  else
    std::cout << "[" << dump_id << "] ";
  if (n->IsToken()) {
    std::cout << "Token" << std::endl;
  } else {
    RuleTable *t = n->GetTable();
    std::cout << "Table " << GetRuleTableName(t) << "@" << n->GetStartIndex() << ": ";

    if (n->mAfter == SuccWasSucc)
      std::cout << "WasSucc";

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
    if (node->mAfter == SuccWasSucc) {
      was_succ_list.push_back(node);
      if (mTracePatchWasSucc)
        std::cout << "Find WasSucc " << node << std::endl;
    } else {
      std::vector<AppealNode*>::iterator it = node->mSortedChildren.begin();
      for (; it != node->mSortedChildren.end(); it++)
        working_list.push_back(*it);
    }
  }
  return NULL;
}

// For each node in was_succ_list there are one or more patching subtree.
// A succ parent node contains the matching of succ children nodes. But we
// only want the real matching which comes from the children. So, we look into
// those nodes and find the node being the youngest descendant, which has the
// smallest sub-tree.

void Parser::FindPatchingNodes() {
  std::vector<AppealNode*>::iterator it = was_succ_list.begin();
  for (; it != was_succ_list.end(); it++) {
    AppealNode *was_succ = *it;

    SuccMatch *succ = FindSucc(was_succ->GetTable());
    MASSERT(succ && "WasSucc's rule has no SuccMatch?");
    bool found = succ->GetStartToken(was_succ->GetStartIndex());
    MASSERT(found && "WasSucc cannot find start index in SuccMatch?");

    AppealNode *youngest = succ->GetSuccNode(0);
    for (unsigned i = 1; i < succ->GetSuccNodesNum(); i++) {
      AppealNode *node = succ->GetSuccNode(i);
      if (node->DescendantOf(youngest)) {
        youngest = node;
      } else {
        // Any two nodes should be in a ancestor-descendant relationship.
        MASSERT(youngest->DescendantOf(node));
      }
    }
    MASSERT(youngest && "succ matching node is missing?");

    if (mTracePatchWasSucc)
      std::cout << "Find one match " << youngest << std::endl;

    was_succ_matched_list.push_back(was_succ);
    patching_list.push_back(youngest);
  }
}

// This is another entry point of sort, similar as SortOut().
// The only difference is we use 'target' as the refrence that we want
// 'root' to be sorted.
void Parser::SupplementalSortOut(AppealNode *root, AppealNode *target) {
  if(root->mSortedChildren.size()!=0)
    std::cout << "got one sorted again." << std::endl;
  MASSERT(root->mSortedChildren.size()==0 && "root should be un-sorted.");
  MASSERT(root->IsTable() && "root should be a table node.");

  // step 1. Find the last matching token index we want.
  MASSERT(target->IsSorted() && "target is not sorted?");

  // step 2. Set the root.
  SuccMatch *succ = FindSucc(root->GetTable());
  MASSERT(succ && "root node has no SuccMatch?");
  root->SetFinalMatch(target->GetFinalMatch());
  root->SetSorted();

  // step 3. Start the sort out
  to_be_sorted.clear();
  to_be_sorted.push_back(root);

  while(!to_be_sorted.empty()) {
    AppealNode *node = to_be_sorted.front();
    to_be_sorted.pop_front();
    SortOutNode(node);
  }

  if (mTraceSortOut)
    DumpSortOut(root, "supplemental sortout");
}

// In the tree after SortOut, some nodes could be SuccWasSucc and we didn't build
// sub-tree for its children. Now it's time to patch the sub-tree.
void Parser::PatchWasSucc(AppealNode *root) {
  while(1) {
    mRoundsOfPatching++;
    if (mTracePatchWasSucc)
      std::cout << "=== In round " << mRoundsOfPatching << std::endl;

    // step 1. Traverse the sorted tree, find the target node which is SuccWasSucc
    was_succ_list.clear();
    FindWasSucc(root);
    if (was_succ_list.empty())
      break;

    // step 2. Traverse the original tree, find the subtree matching target
    was_succ_matched_list.clear();
    patching_list.clear();
    FindPatchingNodes();
    MASSERT( !patching_list.empty() && "Cannot find any patching for SuccWasSucc.");

    MASSERT( was_succ_list.size() == was_succ_matched_list.size() && "Some WasSucc not matched.");

    // step 3. Assert the sorted subtree is not sorted. Then SupplementalSortOut()
    //         Copy the subtree of patch to was_succ
    for (unsigned i = 0; i < was_succ_matched_list.size(); i++) {
      AppealNode *patch = patching_list[i];
      AppealNode *was_succ = was_succ_matched_list[i];
      SupplementalSortOut(patch, was_succ);
      was_succ->mAfter = Succ;

      // We can copy only sorted nodes. The original mChildren cannot be copied since
      // it's the original tree. We don't want to mess it up. Think about it, if you
      // copy the mChildren to was_succ, there are duplicated tree nodes. This violates
      // the definition of the original tree.
      for (unsigned j = 0; j < patch->mSortedChildren.size(); j++)
        was_succ->AddSortedChild(patch->mSortedChildren[j]);
    }
  }

  if (mTraceSortOut)
    DumpSortOut(root, "patch-was-succ");
}

// The idea is to make the Sorted tree simplest. After PatchWasSucc(), there are many
// edges with pred having only one succ and succ having only one pred. If there is no
// any Action (either for building tree or checking validity), this edge can be shrinked
// and reduce the problem size.
//
// However, there is one problem as whether we need add additional fields in the AppealNode
// to save this simplified tree, or we just modify the mSortedChildren on the tree?
// The answer is: We just modify the mSortedChildren to shrink the useless edges.

void Parser::SimplifySortedTree() {
  // start with the only child of mRootNode.
  std::deque<AppealNode*> working_list;
  working_list.push_back(mRootNode->mSortedChildren[0]);

  while(!working_list.empty()) {
    AppealNode *node = working_list.front();
    working_list.pop_front();
    MASSERT(node->IsSucc() && "Sorted node is not succ?");

    // Shrink edges
    if (node->IsToken())
      continue;
    node = SimplifyShrinkEdges(node);

    std::vector<AppealNode*>::iterator it = node->mSortedChildren.begin();
    for (; it != node->mSortedChildren.end(); it++) {
      working_list.push_back(*it);
    }
  }

  if (mTraceSortOut)
    DumpSortOut(mRootNode->mSortedChildren[0], "Simplify AppealNode Trees");
}

// Reduce an edge is (1) Pred has only one succ
//                   (2) Succ has only one pred (this is always true)
//                   (3) There is no Rule Action of pred's rule table, regarding this succ.
// Keep this reduction until the conditions are violated
//
// Returns the new 'node' which stops the shrinking.
AppealNode* Parser::SimplifyShrinkEdges(AppealNode *node) {

  // index will be defined only once since it's the index-child of the 'node'
  // transferred into this function.
  unsigned index = 0;

  while(1) {
    // step 1. Check condition (1) (2)
    if (node->mSortedChildren.size() != 1)
      break;
    AppealNode *child = node->mSortedChildren[0];

    // step 2. Find out the index of child, through looking into sub-ruletable or token.
    //         At this point, there is only one sorted child. It's guaranteed not
    //         a concatenate table. It could be Oneof, Zeroorxxx, etc.
    unsigned child_index;
    bool found = node->GetSortedChildIndex(child, child_index);
    MASSERT(found && "Could not find child index?");

    // step 3. check condition (3)
    //         [NOTE] in RuleAction, element index starts from 1.
    RuleTable *rt = node->GetTable();
    bool has_action = RuleActionHasElem(rt, child_index);
    if (has_action)
      break;

    // step 4. Shrink the edge. This is to remove 'node' by connecting 'node's father
    //         to child. We need go on shrinking with child.
    AppealNode *parent = node->GetParent();
    parent->ReplaceSortedChild(node, child);

    // 1. mRootNode won't have RuleAction, so the index is never used.
    // 2. 'index' just need be calculated once, at the first ancestor which is 'node'
    //    transferred into this function.
    if (parent != mRootNode && index == 0) {
      found = parent->GetSortedChildIndex(node, index);
      MASSERT(found && "Could not find child index?");
    }
    child->mSimplifiedIndex = index;

    // step 5. keep going
    node = child;
  }

  return node;
}

////////////////////////////////////////////////////////////////////////////////////
//                             Build the AST
////////////////////////////////////////////////////////////////////////////////////

ASTTree* Parser::BuildAST() {
  done_nodes.clear();

  ASTTree *tree = new ASTTree();
  tree->SetTraceBuild(mTraceAstBuild);

  std::stack<AppealNode*> appeal_stack;
  appeal_stack.push(mRootNode->mSortedChildren[0]);

  // A map between an AppealNode and a TreeNode.
  std::map<AppealNode*, TreeNode*> nodes_map;

  // 1) If all children done. Time to create tree node for 'appeal_node'
  // 2) If some are done, some not. Add the first not-done child to stack
  while(!appeal_stack.empty()) {
    AppealNode *appeal_node = appeal_stack.top();
    bool children_done = true;
    std::vector<AppealNode*>::iterator it = appeal_node->mSortedChildren.begin();
    for (; it != appeal_node->mSortedChildren.end(); it++) {
      AppealNode *child = *it;
      if (!NodeIsDone(child)) {
        appeal_stack.push(child);
        children_done = false;
        break;
      }
    }

    if (children_done) {
      // Create tree node when there is a rule table, or meanful tokens.
      // Only put in the nodes_map if tree node is really created, since some
      // some tokens like separators don't need tree nodes.
      TreeNode *sub_tree = tree->NewTreeNode(appeal_node, nodes_map);
      if (sub_tree) {
        nodes_map.insert(std::pair<AppealNode*, TreeNode*>(appeal_node, sub_tree));
        // mRootNode is overwritten each time until the last one which is
        // the real root node.
        tree->mRootNode = sub_tree;
      }

      // pop out the 'appeal_node'
      appeal_stack.pop();
      done_nodes.push_back(appeal_node);
    }
  }

  if (tree->mRootNode)
    tree->Dump(0);
  else
    MERROR("We got a statement failed to create AST!");

  return tree;
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

/////////////////////////////////////////////////////////////////////////
//                     SuccMatch Implementation
////////////////////////////////////////////////////////////////////////

void SuccMatch::AddStartToken(unsigned t) {
  mNodes.PairedFindOrCreateKnob(t);
  mMatches.PairedFindOrCreateKnob(t);
}

// The container Guamian assures 'n' is not duplicated.
void SuccMatch::AddSuccNode(AppealNode *n) {
  MASSERT(mNodes.PairedGetKnobData() == n->GetStartIndex());
  MASSERT(mMatches.PairedGetKnobData() == n->GetStartIndex());
  mNodes.PairedAddElem(n);
  for (unsigned i = 0; i < n->GetMatchNum(); i++) {
    unsigned m = n->GetMatch(i);
    mMatches.PairedAddElem(m);
  }
}

// The container Guamian assures 'm' is not duplicated.
void SuccMatch::AddMatch(unsigned m) {
  mMatches.PairedAddElem(m);
}

// Below are Query functions. They need be used together
// with GetStartToken().

// Find the succ info for token 't'. Return true if found.
bool SuccMatch::GetStartToken(unsigned t) {
  bool found_n = mNodes.PairedFindKnob(t);
  bool found_m = mMatches.PairedFindKnob(t);
  MASSERT(found_n == found_m);
  return found_n;
}

unsigned SuccMatch::GetSuccNodesNum() {
  return mNodes.PairedNumOfElem();
}

AppealNode* SuccMatch::GetSuccNode(unsigned idx) {
  return mNodes.PairedGetElemAtIndex(idx);
}

bool SuccMatch::FindNode(AppealNode *n) {
  return mNodes.PairedFindElem(n);
}

unsigned SuccMatch::GetMatchNum() {
  return mMatches.PairedNumOfElem();
}

// idx starts from 0.
unsigned SuccMatch::GetOneMatch(unsigned idx) {
  return mMatches.PairedGetElemAtIndex(idx);
}

bool SuccMatch::FindMatch(unsigned m) {
  return mMatches.PairedFindElem(m);
}

// Return true : if target is found
//       false : if fail
bool SuccMatch::FindMatch(unsigned start, unsigned target) {
  bool found = GetStartToken(start);
  MASSERT( found && "Couldn't find the start token?");
  found = FindMatch(target);
  return found;
}

///////////////////////////////////////////////////////////////
//            AppealNode function
///////////////////////////////////////////////////////////////

// If match 'm' is in the node?
bool AppealNode::FindMatch(unsigned m) {
  for (unsigned i = 0; i < mMatches.GetNum(); i++) {
    if (m == mMatches.ValueAtIndex(i))
      return true;
  }
  return false;
}

void AppealNode::AddMatch(unsigned m) {
  if (FindMatch(m))
    return;
  mMatches.PushBack(m);
}

// return true if 'parent' is a parent of this.
bool AppealNode::DescendantOf(AppealNode *parent) {
  AppealNode *node = mParent;
  while (node) {
    if (node == parent)
      return true;
    node = node->mParent;
  }
  return false;
}

// Returns true, if both nodes are successful and match the same tokens
// with the same rule table
bool AppealNode::SuccEqualTo(AppealNode *other) {
  if (IsSucc() && other->IsSucc() && mStartIndex == other->GetStartIndex()) {
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

void AppealNode::ReplaceSortedChild(AppealNode *existing, AppealNode *replacement) {
  unsigned index;
  bool found = false;
  for (unsigned i = 0; i < mSortedChildren.size(); i++) {
    if (mSortedChildren[i] == existing) {
      index = i;
      found = true;
      break;
    }
  }
  MASSERT(found && "ReplaceSortedChild could not find existing node?");

  mSortedChildren[index] = replacement;
  replacement->mParent = this;
}

// Returns true : if successfully found the index.
// [NOTE] This is the index in the Rule Spec description, which are used in the
//        building of AST. So remember it starts from 1.
//
// The AppealNode tree has many messy nodes generated during second try, or others.
// It's not a good idea to find the index through the tree. The final real solution
// is to go through the RuleTable and locate the child's index.
bool AppealNode::GetSortedChildIndex(AppealNode *child, unsigned &index) {
  bool found = false;
  MASSERT(IsTable() && "Parent node is not a RuleTable");
  RuleTable *rule_table = GetTable();

  // In SimplifyShrinkEdge, the tree could be simplified and a node could be given an index
  // to his ancestor.
  if (child->mSimplifiedIndex != 0) {
    index = child->mSimplifiedIndex;
    return true;
  }

  // If the edge is not shrinked, we just look into the rule tabls or tokens.
  for (unsigned i = 0; i < rule_table->mNum; i++) {
    TableData *data = rule_table->mData + i;
    switch (data->mType) {
    case DT_Token: {
      Token *t = data->mData.mToken;
      if (child->IsToken() && child->GetToken() == t) {
        found = true;
        index = i+1;
      }
      break;
    }
    case DT_Subtable: {
      RuleTable *t = data->mData.mEntry;
      if (t == &TblIdentifier) {
        if (child->IsToken()) {
          Token *token = child->GetToken();
          if (token->IsIdentifier()) {
            found = true;
            index = i+1;
          }
        }
      } else if (t == &TblLiteral) {
        if (child->IsToken()) {
          Token *token = child->GetToken();
          if (token->IsLiteral()) {
            found = true;
            index = i+1;
          }
        }
      } else if (child->IsTable() && child->GetTable() == t) {
        found = true;
        index = i+1;
      }
      break;
    }
    default:
      MASSERT(0 && "Unknown entry in TableData");
      break;
    }
  }

  return found;
}

AppealNode* AppealNode::GetSortedChildByIndex(unsigned index) {
  std::vector<AppealNode*>::iterator it = mSortedChildren.begin();
  for (; it != mSortedChildren.end(); it++) {
    AppealNode *child = *it;
    unsigned id = 0;
    bool found = GetSortedChildIndex(child, id);
    MASSERT(found && "sorted child has no index..");
    if (id == index)
      return child;
  }
  return NULL;
}
