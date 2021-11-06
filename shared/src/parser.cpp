/*
* Copyright (C) [2020] Futurewei Technologies, Inc. All rights reverved.
*
* OpenArkFE is licensed under the Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*
*  http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR
* FIT FOR A PARTICULAR PURPOSE.
* See the Mulan PSL v2 for more details.
*/
#include <iostream>
#include <string>
#include <cstring>
#include <stack>
#include <sys/time.h>

#include "parser.h"
#include "massert.h"
#include "token.h"
#include "ruletable_util.h"
#include "rule_summary.h"
#include "ast.h"
#include "ast_builder.h"
#include "ast_mempool.h"
#include "ast_type.h"
#include "parser_rec.h"
#include "ast_fixup.h"

namespace maplefe {

SmallVector<TemplateLiteralNode*> gTemplateLiteralNodes;

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
// origin of gFailed.
//
// Also, if a rule was successful for a token and it is trying to traverse it again, it
// can be skipped by using a cached result. This is the origin of gSucc.
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
  mLexer = CreateLexer();
  const std::string file(name);

  mASTModule = new (gTreePool.NewTreeNode(sizeof(ModuleNode))) ModuleNode();
  mASTModule->SetFilename(name);
  mASTBuilder = new ASTBuilder(mASTModule);
  gPrimTypePool.Init();

  mAppealNodePool.SetBlockSize(16*4096);

  // get source language type
  std::string::size_type lastDot = file.find_last_of('.');
  if (lastDot == std::string::npos) {
    std::cout << "used improper source file" << std::endl;
    return;
  }
  std::string fileExt = file.substr(lastDot);
  if (fileExt.compare(".java") == 0) {
    mASTModule->mSrcLang = SrcLangJava;
  } else if (fileExt.compare(".js") == 0) {
    mASTModule->mSrcLang = SrcLangJavaScript;
  } else if (fileExt.compare(".ts") == 0) {
    mASTModule->mSrcLang = SrcLangTypeScript;
  } else {
    mASTModule->mSrcLang = SrcLangUnknown;
  }

  mLexer->PrepareForFile(file);
  mCurToken = 0;
  mPending = 0;
  mEndOfFile = false;

  mNormalModeRoot = NULL;
  mLineModeRoot = NULL;
  mLineMode = false;

  mTraceTable = false;
  mTraceLeftRec = false;
  mTraceAppeal = false;
  mTraceVisited = false;
  mTraceFailed = false;
  mTraceTiming = false;
  mTraceSortOut = false;
  mTraceAstBuild = false;
  mTracePatchWasSucc = false;
  mTraceWarning = false;

  mIndentation = -2;
  mRoundsOfPatching = 0;

  mInAltTokensMatching = false;
  mNextAltTokenIndex = 0;

  mActiveTokens.SetBlockSize(1024*1024);
}

Parser::~Parser() {
  delete mLexer;
}

void Parser::Dump() {
}

void Parser::ClearFailed() {
  for (unsigned i = 0; i < RuleTableNum; i++)
     gFailed[i].ClearAll();
}

// Add one fail case for the table
void Parser::AddFailed(RuleTable *table, unsigned token) {
  gFailed[table->mIndex].SetBit(token);
}

// Remove one fail case for the table
void Parser::ResetFailed(RuleTable *table, unsigned token) {
  gFailed[table->mIndex].ClearBit(token);
}

bool Parser::WasFailed(RuleTable *table, unsigned token) {
  return gFailed[table->mIndex].GetBit(token);
}

// return true if t can be merged with previous tokens.
// This happens when "-3" is lexed as operator Sub and literal 3.
// For Lexer' stance, this is the right thing to do. However, we do
// need literal -3.
bool Parser::TokenMerge(Token *t) {
  if (!t->IsLiteral())
    return false;
  unsigned size = mActiveTokens.GetNum();
  if (size < 2)
    return false;

  // We take care of a few scenarios.
  //   = -1   <-- sep is an assignment operator
  //   [-1    <-- sep is a separtor
  //
  // Here is also another ugly case in Typescript.
  //  keyword -1
  // such as: x extends -1
  // In normal sense, if it's a keyword we can merge tokens.
  // However, in TS keyword can also be an identifier which means
  // keyword - 1 could be an expression.
  // In this case, we further look at one more token ahead, so if it's
  //   identifier keyword -1
  // then we know keyword is not used an identifier and we can merge tokens.

  Token *sep = mActiveTokens.ValueAtIndex(size - 2);
  bool is_sep = false;
  if (sep->IsSeparator() &&
     (sep->GetSepId() != SEP_Rparen) &&
     (sep->GetSepId() != SEP_Rbrack))
    is_sep = true;
  if (sep->IsOperator() &&
        (sep->GetOprId() == OPR_Assign ||
         sep->GetOprId() == OPR_Bor))
    is_sep = true;

  if (sep->IsKeyword() && mActiveTokens.GetNum() >= 3) {
    Token *idn = mActiveTokens.ValueAtIndex(size - 3);
    if (idn->IsIdentifier())
      is_sep = true;
  }

  if (!is_sep)
    return false;

  Token *opr = mActiveTokens.ValueAtIndex(size - 1);
  if (!opr->IsOperator())
    return false;

  if ((opr->GetOprId() != OPR_Sub) && (opr->GetOprId() != OPR_Add))
    return false;

  LitData data = t->GetLitData();
  if ((data.mType != LT_IntegerLiteral) &&
      (data.mType != LT_FPLiteral) &&
      (data.mType != LT_DoubleLiteral))
    return false;

  if (opr->GetOprId() == OPR_Sub) {
    if ((data.mType == LT_IntegerLiteral)) {
      data.mData.mInt = (-1) * data.mData.mInt;
    } else if (data.mType == LT_FPLiteral) {
      data.mData.mFloat = (-1) * data.mData.mFloat;
    } else if (data.mType == LT_DoubleLiteral) {
      data.mData.mDouble = (-1) * data.mData.mDouble;
    }
    t->SetLiteral(data);
    mActiveTokens.SetElem(size - 1, t);
    return true;
  } else if (opr->GetOprId() == OPR_Add) {
    mActiveTokens.SetElem(size - 1, t);
    return true;
  }

  return false;
}

// return true if t should be split into multiple tokens.
// [NOTE] t is not push into mActiveTokens yet.
//
// We will handle these cases specifically.
//
// We take care of only one scenarios right now..
//   typename<typearg>= initval
// Look at the '>='. It first recognazied by lexer as GE,
// but it's actually a > and a =.

bool Parser::TokenSplit(Token *t) {
  if (!t->IsOperator() || t->GetOprId() != OPR_GE)
    return false;
  unsigned size = mActiveTokens.GetNum();
  if (size < 2)
    return false;

  Token *type_arg = mActiveTokens.ValueAtIndex(size - 1);
  if (!type_arg->IsIdentifier())
    return false;

  Token *lt = mActiveTokens.ValueAtIndex(size - 2);
  if (!lt->IsOperator() || lt->GetOprId() != OPR_LT)
    return false;

  Token *type_name = mActiveTokens.ValueAtIndex(size - 3);
  if (!type_name->IsIdentifier())
    return false;

  // Now we got a matching case.
  Token *gt_token = mLexer->FindOperatorToken(OPR_GT);
  Token *assign_token = mLexer->FindOperatorToken(OPR_Assign);
  mActiveTokens.PushBack(gt_token);
  mActiveTokens.PushBack(assign_token);

  if (mLexer->mTrace) {
    std::cout << "Split >= to > and =" << std::endl;
  }

  return true;
}

// Return a reg expr token instead of t if t is the beginning of a regular expression.
// Or return t.
Token* Parser::GetRegExpr(Token *t) {
  if (!t->IsOperator())
    return t;

  if (t->GetOprId() != OPR_Div)
    return t;

  unsigned size = mActiveTokens.GetNum();
  if (size < 2)
    return t;

  // We take care of only the following scenarios.
  // If more need support, we will add later.
  //   (/abc*/g, )
  //   [/abc*/g, ]
  //   =/abc*/g;
  //   &&  /abc*/g;
  //   ,/abc*/g;
  //   : /abc*/g;
  //   || /abc*/g;

  Token *sep = mActiveTokens.ValueAtIndex(size - 1);
  bool is_sep = false;
  if (sep->IsSeparator() && (sep->GetSepId() == SEP_Lparen))
    is_sep = true;
  if (sep->IsSeparator() && (sep->GetSepId() == SEP_Lbrack))
    is_sep = true;
  if (sep->IsSeparator() && (sep->GetSepId() == SEP_Comma))
    is_sep = true;
  if (sep->IsSeparator() && (sep->GetSepId() == SEP_Colon))
    is_sep = true;
  if (sep->IsOperator() && (sep->GetOprId() == OPR_Assign))
    is_sep = true;
  if (sep->IsOperator() && (sep->GetOprId() == OPR_Land))
    is_sep = true;
  if (sep->IsOperator() && (sep->GetOprId() == OPR_Lor))
    is_sep = true;
  if (!is_sep)
    return t;

  Token *regexpr = mLexer->FindRegExprToken();
  if (regexpr)
    t = regexpr;

  return t;
}

// Lex all tokens in a line, save to mActiveTokens.
// If no valuable in current line, we continue to the next line.
// Returns the number of valuable tokens read. Returns 0 if EOF.
unsigned Parser::LexOneLine() {
  unsigned token_num = 0;
  Token *t = NULL;

  // Check if there are already pending tokens.
  if (mCurToken < mActiveTokens.GetNum())
    return mActiveTokens.GetNum() - mCurToken;

  while (!token_num) {
    // read until end of line
    while (!mLexer->EndOfLine() && !mLexer->EndOfFile()) {
      t = mLexer->LexToken();
      if (t) {
        bool is_whitespace = false;
        if (t->IsSeparator()) {
          if (t->IsWhiteSpace())
            is_whitespace = true;
        }
        bool is_tab = false;
        if (t->IsSeparator()) {
          if (t->IsTab())
            is_tab = true;
        }
        // Put into the token storage
        if (!is_whitespace && !is_tab && !t->IsComment()) {
          // 1. if need to merge
          if (TokenMerge(t))
            continue;

          // 2. if need split tokens
          if (TokenSplit(t))
            continue;

          // 3. handle regular expression
          t = GetRegExpr(t);

          mActiveTokens.PushBack(t);
          token_num++;
        }
      } else {
        MASSERT(0 && "Non token got? Problem here!");
        break;
      }
    }
    // Read in the next line.
    if (!token_num && !mLineMode) {
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
  if (mCurToken == mActiveTokens.GetNum()) {
    // In line mode, we won't read new line any more.
    if (mLineMode) {
      mEndOfFile = true;
      return true;
    }
    unsigned num = LexOneLine();
    if (!num) {
      mEndOfFile = true;
      return false;
    }
  }
  return true;
}

Token* Parser::GetActiveToken(unsigned i) {
  if (i >= mActiveTokens.GetNum())
    MASSERT(0 && "mActiveTokens OutOfBound");
  return mActiveTokens.ValueAtIndex(i);
}

// insert token at position idx.
void Parser::InsertToken(unsigned idx, Token *token) {
  if (idx >= mActiveTokens.GetNum())
    MASSERT(0 && "mActiveTokens OutOfBound");
  // enlarge the size by 1.
  mActiveTokens.PushBack(NULL);
  // Copy each of them forwards.
  unsigned i = mActiveTokens.GetNum() - 2;
  for (; i >= idx; i--) {
    Token *move_t = mActiveTokens.ValueAtIndex(i);
    mActiveTokens.SetElem(i + 1, move_t);
  }
  mActiveTokens.SetElem(idx, token);
}

bool Parser::Parse() {
  gTemplateLiteralNodes.Clear();
  mASTBuilder->SetTrace(mTraceAstBuild);
  ParseStatus res;
  while (1) {
    res = ParseStmt();
    if (res == ParseFail || res == ParseEOF)
      break;
  }

  if (gTemplateLiteralNodes.GetNum() > 0)
    ParseTemplateLiterals();

  FixUpVisitor worker(mASTModule);
  worker.FixUp();

  mASTModule->Dump(0);
  return (res==ParseFail)? false: true;
}

void Parser::ParseTemplateLiterals() {

  mLineMode = true;
  mLexer->SetLineMode();
  for (unsigned i = 0; i < gTemplateLiteralNodes.GetNum(); i++) {
    TemplateLiteralNode *tl = gTemplateLiteralNodes.ValueAtIndex(i);
    for (unsigned j = 1; j < tl->GetStringsNum(); j += 2) {
      // Create tree node for format
      const char *fmt_str = tl->GetStringAtIndex(j-1);
      if (fmt_str) {
        //Create a string literal node
        LitData litdata;
        litdata.mType = LT_StringLiteral;
        litdata.mData.mStrIdx = gStringPool.GetStrIdx(fmt_str);
        LiteralNode *n = (LiteralNode*)gTreePool.NewTreeNode(sizeof(LiteralNode));
        new (n) LiteralNode(litdata);
        tl->AddTree(n);
      } else {
        tl->AddTree(NULL);
      }

      const char *ph_str = tl->GetStringAtIndex(j);
      if (ph_str) {
        mLexer->PrepareForString(ph_str);
        // Clear some status
        mEndOfFile = false;
        ParseStatus result = ParseStmt();
        MASSERT(result == ParseSucc);
        MASSERT(mLineModeRoot);
        tl->AddTree(mLineModeRoot);
      } else {
        tl->AddTree(NULL);
      }
    }
  }
  mLineMode = false;
  mLexer->ResetLineMode();
}

void Parser::ClearAppealNodes() {
  for (unsigned i = 0; i < mAppealNodes.size(); i++) {
    AppealNode *node = mAppealNodes[i];
    if (node)
      node->Release();
  }
  mAppealNodes.clear();
  mAppealNodePool.Clear();
}

// This is for the appealing of mistaken Fail cases created during the first instance
// of LeadNode traversal. We appeal all nodes from start up to root. We do backwards
// traversal from 'start' upto 'root'.
//
// [NOTE] We will clear the Fail info, so that it won't be
//        treated as WasFail during TraverseRuleTable(). However, the appeal tree
//        should be remained as fail, because it IS a fail indeed for this sub-tree.
void Parser::Appeal(AppealNode *start, AppealNode *root) {
  MASSERT((root->IsSucc()) && "root->mResult is not Succ.");

  AppealNode *node = start;

  // It's possible that this sub-tree could be separated. For example, the last
  // instance of RecursionTraversal, which is a Fake Succ, and is separated
  // from the main tree. However, the tree itself is useful, and we do want to
  // clear the mistaken Fail flag of useful rule tables.
  //
  // So a check for 'node' Not Null is to handle separated trees.
  while(node && (node != root)) {
    if ((node->mResult == FailChildrenFailed)) {
      if (mTraceAppeal)
        DumpAppeal(node->GetTable(), node->GetStartIndex());
      ResetFailed(node->GetTable(), node->GetStartIndex());
    }
    node = node->GetParent();
  }
}

// return true : if successful
//       false : if failed
// This is the parsing for highest level language constructs. It could be class
// in Java/c++, or a function/statement in c/c++. In another word, it's the top
// level constructs in a compilation unit (aka Module).
ParseStatus Parser::ParseStmt() {
  // clear status
  ClearFailed();
  ClearSucc();
  ClearAppealNodes();
  mPending = 0;

  // set the root appealing node
  mRootNode = mAppealNodePool.NewAppealNode();
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
    return ParseEOF;

  // Match the tokens against the rule tables.
  // In a rule table there are : (1) separtaor, operator, keyword, are already in token
  //                             (2) Identifier Table won't be traversed any more since
  //                                 lex has got the token from source program and we only
  //                                 need check if the table is &TblIdentifier.

  struct timeval stop, start;
  if (mTraceTiming)
    gettimeofday(&start, NULL);

  bool succ = false;
  if (mLineMode)
    succ = TraverseTempLiteral();
  else
    succ = TraverseStmt();

  if (mTraceTiming) {
    gettimeofday(&stop, NULL);
    std::cout << "Parse Time: " << (stop.tv_sec - start.tv_sec) * 1000000 + stop.tv_usec - start.tv_usec;
    std::cout << " us" << std::endl;
  }

  // Each top level construct gets a AST tree.
  if (succ) {
    if (mTraceTiming)
      gettimeofday(&start, NULL);

    PatchWasSucc(mRootNode->mSortedChildren.ValueAtIndex(0));
    if (mTraceTiming) {
      gettimeofday(&stop, NULL);
      std::cout << "PatchWasSucc Time: " << (stop.tv_sec - start.tv_sec) * 1000000 + stop.tv_usec - start.tv_usec;
      std::cout << " us" << std::endl;
    }

    if (mTraceTiming)
      gettimeofday(&start, NULL);
    SimplifySortedTree();
    if (mTraceTiming) {
      gettimeofday(&stop, NULL);
      std::cout << "Simplify Time: " << (stop.tv_sec - start.tv_sec) * 1000000 + stop.tv_usec - start.tv_usec;
      std::cout << " us" << std::endl;
    }

    if (mTraceTiming)
      gettimeofday(&start, NULL);
    TreeNode *tree = BuildAST();
    if (tree) {
      if (!mLineMode)
        mASTModule->AddTree(tree);
    }

    if (mTraceTiming) {
      gettimeofday(&stop, NULL);
      std::cout << "BuildAST Time: " << (stop.tv_sec - start.tv_sec) * 1000000 + stop.tv_usec - start.tv_usec;
      std::cout << " us" << std::endl;
    }
  }
  return succ? ParseSucc: ParseFail;
}

// return true : if all tokens in mActiveTokens are matched.
//       false : if faled.
// For the place holders in Typescript Template Literal, there are usually two
// syntax, expression and type.

bool Parser::TraverseTempLiteral() {
  bool succ_expr = false;
  bool succ_type = false;
  unsigned saved_mCurToken = mCurToken;
  unsigned new_mCurToken_expr = 0;
  unsigned new_mCurToken_type = 0;

  mRootNode->ClearChildren();

  RuleTable *t = &TblExpression;
  AppealNode *child = NULL;
  succ_expr = TraverseRuleTable(t, mRootNode, child);
  if (succ_expr) {
    MASSERT(child);
    mRootNode->CopyMatch(child);
    // Need adjust the mCurToken. A rule could try multiple possible
    // children rules, although there is one and only one valid child
    // for a Top table. However, the mCurToken could deviate from
    // the valid children and reflect the invalid children.
    //MASSERT(mRootNode->mChildren.GetNum() == 1);
    //AppealNode *topnode = mRootNode->mChildren.ValueAtIndex(0);
    //MASSERT(topnode->IsSucc());
    new_mCurToken_expr = mCurToken;
  }

  // Type TblType
  mCurToken = saved_mCurToken;
  t = &TblType;
  child = NULL;
  succ_type = TraverseRuleTable(t, mRootNode, child);
  if (succ_type) {
    MASSERT(child);
    mRootNode->CopyMatch(child);
    // Need adjust the mCurToken. A rule could try multiple possible
    // children rules, although there is one and only one valid child
    // for a Top table. However, the mCurToken could deviate from
    // the valid children and reflect the invalid children.
    //MASSERT(mRootNode->mChildren.GetNum() == 1);
    //AppealNode *topnode = mRootNode->mChildren.ValueAtIndex(0);
    //MASSERT(topnode->IsSucc());
    new_mCurToken_type = mCurToken;
  }

  mCurToken = new_mCurToken_expr > new_mCurToken_type ? new_mCurToken_expr : new_mCurToken_type;

  bool succ = succ_expr | succ_type;
  if (succ) {
    mRootNode->mResult = Succ;
    SortOut();
  }

  if (!succ)
    std::cout << "Illegal syntax detected!" << std::endl;
  else
    std::cout << "Matched " << mCurToken << " tokens." << std::endl;

  return succ;
}

// return true : if all tokens in mActiveTokens are matched.
//       false : if faled.
bool Parser::TraverseStmt() {
  // right now assume statement is just one line.
  // I'm doing a simple separation of one-line class declaration.
  bool succ = false;

  // Go through the top level construct, find the right one.
  for (unsigned i = 0; i < gTopRulesNum; i++){
    RuleTable *t = gTopRules[i];
    mRootNode->ClearChildren();
    AppealNode *child = NULL;
    succ = TraverseRuleTable(t, mRootNode, child);
    if (succ) {
      MASSERT(child);
      mRootNode->CopyMatch(child);
      // Need adjust the mCurToken. A rule could try multiple possible
      // children rules, although there is one and only one valid child
      // for a Top table. However, the mCurToken could deviate from
      // the valid children and reflect the invalid children.
      MASSERT(mRootNode->mChildren.GetNum() == 1);
      AppealNode *topnode = mRootNode->mChildren.ValueAtIndex(0);
      MASSERT(topnode->IsSucc());

      // Top level table should have only one valid matching. Otherwise,
      // the language is ambiguous.
      MASSERT(topnode->GetMatchNum() == 1);
      mCurToken = topnode->GetMatch(0) + 1;

      mRootNode->mResult = Succ;
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

void Parser::DumpIndentation() {
  for (unsigned i = 0; i < mIndentation; i++)
    std::cout << " ";
}

void Parser::DumpEnterTable(const char *table_name, unsigned indent) {
  for (unsigned i = 0; i < indent; i++)
    std::cout << " ";
  std::cout << "Enter " << table_name << "@" << mCurToken << "{" << std::endl;
}

void Parser::DumpExitTable(const char *table_name, unsigned indent, AppealNode *appeal) {
  DumpExitTable(table_name, indent, appeal->mResult, appeal);
}

void Parser::DumpExitTable(const char *table_name, unsigned indent,
                           AppealStatus reason, AppealNode *appeal) {
  for (unsigned i = 0; i < indent; i++)
    std::cout << " ";
  std::cout << "Exit  " << table_name << "@" << mCurToken;
  if (reason == SuccWasSucc) {
    std::cout << " succ@WasSucc" << "}";
    DumpSuccTokens(appeal);
    std::cout << std::endl;
  } else if (reason == SuccStillWasSucc) {
    std::cout << " succ@StillWasSucc" << "}";
    DumpSuccTokens(appeal);
    std::cout << std::endl;
  } else if (reason == Succ) {
    std::cout << " succ" << "}";
    DumpSuccTokens(appeal);
    std::cout << std::endl;
  } else if (reason == FailWasFailed)
    std::cout << " fail@WasFailed" << "}" << std::endl;
  else if (reason == FailNotRightToken)
    std::cout << " fail@NotRightToken" << "}" << std::endl;
  else if (reason == FailNotIdentifier)
    std::cout << " fail@NotIdentifer" << "}" << std::endl;
  else if (reason == FailNotLiteral)
    std::cout << " fail@NotLiteral" << "}" << std::endl;
  else if (reason == FailNotRegExpr)
    std::cout << " fail@NotRegExpr" << "}" << std::endl;
  else if (reason == FailChildrenFailed)
    std::cout << " fail@ChildrenFailed" << "}" << std::endl;
  else if (reason == Fail2ndOf1st)
    std::cout << " fail@2ndOf1st" << "}" << std::endl;
  else if (reason == FailLookAhead)
    std::cout << " fail@LookAhead" << "}" << std::endl;
  else if (reason == AppealStatus_NA)
    std::cout << " fail@NA" << "}" << std::endl;
}

void Parser::DumpSuccTokens(AppealNode *appeal) {
  std::cout << " " << appeal->GetMatchNum() << ": ";
  for (unsigned i = 0; i < appeal->GetMatchNum(); i++)
    std::cout << appeal->GetMatch(i) << ",";
}

// Update SuccMatch using 'node'.
void Parser::UpdateSuccInfo(unsigned curr_token, AppealNode *node) {
  MASSERT(node->IsTable());
  RuleTable *rule_table = node->GetTable();
  SuccMatch *succ_match = &gSucc[rule_table->mIndex];
  succ_match->AddStartToken(curr_token);

  // AddSuccNode will update mMatches from 'node'.
  succ_match->AddSuccNode(node);
}

// Remove 'node' from its SuccMatch
void Parser::RemoveSuccNode(unsigned curr_token, AppealNode *node) {
  MASSERT(node->IsTable());
  RuleTable *rule_table = node->GetTable();
  SuccMatch *succ_match = &gSucc[rule_table->mIndex];
  MASSERT(succ_match);
  succ_match->GetStartToken(curr_token);
  succ_match->RemoveNode(node);
}

bool Parser::LookAheadFail(RuleTable *rule_table, unsigned token) {
  Token *curr_token = GetActiveToken(token);

  LookAheadTable latable = gLookAheadTable[rule_table->mIndex];

  bool found = false;
  for (unsigned i = 0; i < latable.mNum; i++) {
    LookAhead la = latable.mData[i];

    switch(la.mType) {
    case LA_Char:
    case LA_String:
      // We are not ready to handle non-literal or non-identifier Char/String
      // which are not recoganized by lexer.
      break;
    case LA_Token:
      if (curr_token == &gSystemTokens[la.mData.mTokenId])
        found = true;
      // TemplateLiteral, Regular Expression is treated as a special keyword.
      {
        Token *t = &gSystemTokens[la.mData.mTokenId];
        if (t->IsKeyword() && !strncmp(t->GetName(), "this_is_for_fake_rule", 21)) {
          if (curr_token->IsTempLit() || curr_token->IsRegExpr())
            found = true;
        }
      }
      break;
    case LA_Identifier:
      if (curr_token->IsIdentifier())
        found = true;
      break;
    case LA_Literal:
      if (curr_token->IsLiteral())
        found = true;
      break;
    case LA_NA:
    default:
      MASSERT(0 && "Unknown LookAhead Type.");
      break;
    }

    if (found)
      break;
  }

  if (found)
    return false;
  else
    return true;
}

// return true : if the rule_table is matched
//       false : if faled.
//
// [NOTE] About how to move mCurToken
//        1. TraverseRuleTable will restore the mCurToken if it fails.
//        2. TraverseRuleTable will let the children's traverse to move mCurToken
//           if they succeeded.
//        3. TraverseOneof, TraverseZeroxxxx, TraverseConcatenate follow rule 1&2.
//        3. TraverseLeadNode exit early, so need follow the rule 1&2.
//        4. TraverseLeadNode() also follows the rule 1&2. It moves mCurToken
//           when succ and restore it when fail.
//
// 'child' is the AppealNode of 'rule_table'.
bool Parser::TraverseRuleTable(RuleTable *rule_table, AppealNode *parent, AppealNode *&child) {
  if (mEndOfFile)
    return false;

  mIndentation += 2;
  const char *name = NULL;
  if (mTraceTable) {
    name = GetRuleTableName(rule_table);
    DumpEnterTable(name, mIndentation);
  }

  // Lookahead fail is fast to check, even faster than check WasFailed.
  if (LookAheadFail(rule_table, mCurToken) &&
      (rule_table->mType != ET_Zeroormore) &&
      (rule_table->mType != ET_Zeroorone)) {
    if (mTraceTable)
      DumpExitTable(name, mIndentation, FailLookAhead);
    mIndentation -= 2;
    return false;
  }

  AppealNode *appeal = NULL;
  unsigned saved_mCurToken = mCurToken;
  bool is_done = false;

  // Check if it was succ. The longest matching is chosen for the next rule table to match.
  SuccMatch *succ = &gSucc[rule_table->mIndex];
  if (succ) {
    bool was_succ = succ->GetStartToken(mCurToken);
    if (was_succ) {
      // Those affected by the 1st appearance of 1st instance which returns false.
      // 1stOf1st is not add to WasFail, but those affected will be added to WasFail.
      // The affected can be succ later. So there is possibility both succ and fail
      // exist at the same time.
      //
      // I still keep this assertion. We will see. Maybe we'll remove it.
      MASSERT(!WasFailed(rule_table, mCurToken));

      // set the apppeal node
      appeal = mAppealNodePool.NewAppealNode();
      mAppealNodes.push_back(appeal);
      appeal->SetTable(rule_table);
      appeal->SetStartIndex(mCurToken);
      appeal->SetParent(parent);
      parent->AddChild(appeal);
      child = appeal;

      is_done = succ->IsDone();

      unsigned num = succ->GetMatchNum();
      for (unsigned i = 0; i < num; i++) {
        unsigned match = succ->GetOneMatch(i);
        // WasSucc nodes need Match info, which will be used later
        // in the sort out.
        appeal->AddMatch(match);
        if (match > mCurToken)
          mCurToken = match;
      }
      appeal->mResult = SuccWasSucc;

      // In ZeroorXXX cases, it was successful and has SuccMatch. However,
      // it could be a failure. In this case, we shouldn't move mCurToken.
      if (num > 0)
        MoveCurToken();
    }
  }

  unsigned group_id;
  bool in_group = FindRecursionGroup(rule_table, group_id);

  // 1. In a recursion, a rule could fail in the first a few instances,
  //    but could match in a later instance. So I need check is_done.
  //    Here is an example. Node A is one of the circle node.
  //    (a) In the first recursion instance, A is failed, but luckly it
  //        gets appealed due to lead node is 2ndOf1st.
  //    (b) In the second instance, it still fail because its children
  //        failed. But the whole recursion actually matches tokens, and
  //        those matching rule tables are not related to A.
  //    (c) Finally, A matches because leading node goes forward and gives
  //        A new opportunity.
  // 2. For A not-in-group rule, a WasFailed is a real fail.

  bool was_failed = WasFailed(rule_table, saved_mCurToken);
  if (was_failed && (!in_group || is_done)) {
    if (mTraceTable)
      DumpExitTable(name, mIndentation, FailWasFailed);
    mIndentation -= 2;
    return false;
  }

  // If the rule is NOT in any recursion group, we simply return the result.
  // If the rule is done, we also simply return the result.

  // If a rule is succ at some token, it doesn't mean it's finished, and
  // there could be more matchings.

  if (appeal && appeal->IsSucc()) {
    if (!in_group || is_done) {
      if (mTraceTable)
        DumpExitTable(name, mIndentation, appeal);
      mIndentation -= 2;
      return true;
    } else {
      if (mTraceTable) {
        DumpIndentation();
        std::cout << "Traverse-Pre WasSucc, mCurToken:" << saved_mCurToken;
        std::cout << std::endl;
      }
    }
  }

  RecursionTraversal *rec_tra = FindRecStack(group_id, saved_mCurToken);

  // group_id is 0 which is the default value if rule_table is not in a group
  // Need to reset rec_tra;
  if (!in_group)
    rec_tra = NULL;

  // This part is to handle a special case: The second appearance in the first instance
  // (wave) in the Wavefront algorithm. At this moment, the first appearance in this
  // instance hasn't finished its traversal, so there is no previous succ or fail case.
  //
  // We need to simply return false, but we cannot add them to the Fail mapping.
  // A rule is AddFailed() in TraverseRuleTableRegular() which is in the end of this function.

  if (rec_tra &&
      rec_tra->GetInstance() == InstanceFirst &&
      rec_tra->LeadNodeVisited(rule_table)) {
    rec_tra->AddAppealPoint(parent);
    if (mTraceTable)
      DumpExitTable(name, mIndentation, Fail2ndOf1st);
    mIndentation -= 2;
    return false;
  }

  // We delay creation of AppealNode as much as possible.
  if (!appeal) {
    appeal = mAppealNodePool.NewAppealNode();
    mAppealNodes.push_back(appeal);
    appeal->SetTable(rule_table);
    appeal->SetStartIndex(saved_mCurToken);
    appeal->SetParent(parent);
    parent->AddChild(appeal);
    child = appeal;
  }

  // If the rule is already traversed in this iteration(instance), we return the result.
  if (rec_tra && rec_tra->RecursionNodeVisited(rule_table)) {
    if (mTraceTable)
      DumpExitTable(name, mIndentation, appeal);
    mIndentation -= 2;
    return true;
  }

  // This part is to handle the 2nd appearance of the Rest Instances
  //
  // IsSucc() assures it's not 2nd appearance of 1st Instance?
  // Because the 1st instantce is not done yet and cannot be IsSucc().
  if (appeal->IsSucc() && mRecursionAll.IsLeadNode(rule_table)) {
    // If we are entering a lead node which already succssfully matched some
    // tokens and not IsDone yet, it means we are in second or later instances.
    // We should find RecursionTraversal for it.
    MASSERT(rec_tra);

    // Check if it's visited, assure it's 2nd appearance.
    // There are only two appearances of the Leading rule tables in one single
    // wave (instance) of the Wavefront traversal, the 1st is not visited, the
    // 2nd is visited.

    if (rec_tra->LeadNodeVisited(rule_table)) {
      if (mTraceLeftRec) {
        DumpIndentation();
        std::cout << "<LR>: ConnectPrevious " << GetRuleTableName(rule_table)
                  << "@" << appeal->GetStartIndex()
                  << " node:" << appeal << std::endl;
      }
      // It will be connect to the previous instance, which have full appeal tree.
      // WasSucc node is used for succ node which has no full appeal tree. So better
      // change the status to Succ.
      appeal->mResult = Succ;
      if (mTraceTable)
        DumpExitTable(name, mIndentation, appeal);

      mIndentation -= 2;
      return rec_tra->ConnectPrevious(appeal);
    }
  }

  // Restore the mCurToken since TraverseRuleTablePre() update the mCurToken
  // if succ. And we need use the old mCurToken.
  mCurToken = saved_mCurToken;

  // Now it's time to do regular traversal on a LeadNode.
  // The scenarios of LeadNode is one of the below.
  // 1. The first time we hit the LeadNode
  // 2. The first time in an instance we hit the LeadNode. It WasSucc, but
  //    we have to do re-traversal.
  //
  // The match info of 'appeal' and its SuccMatch will be updated
  // inside TraverseLeadNode().

  if (mRecursionAll.IsLeadNode(rule_table)) {
    bool found = TraverseLeadNode(appeal, parent);
    if (mTraceTable) {
      const char *name = GetRuleTableName(rule_table);
      DumpExitTable(name, mIndentation, appeal);
    }
    mIndentation -= 2;
    return found;
  }

  // It's a regular (non leadnode) table, either inside or outside of a
  // recursion, we just need do the regular traversal.
  // If it's inside a Left Recursion, it will finally goes to that
  // recursion. I don't need take care here.

  bool matched = TraverseRuleTableRegular(rule_table, appeal);
  if (rec_tra)
    rec_tra->AddVisitedRecursionNode(rule_table);

  if (!in_group && matched)
    SetIsDone(rule_table, saved_mCurToken);

  if (mTraceTable)
    DumpExitTable(name, mIndentation, appeal);

  mIndentation -= 2;
  return matched;
}

// 'appeal' is the AppealNode of 'rule_table'.
//
// [NOTE] !!!!
//
// This is the most important note. The merge of children's succ match to parent's
// is done inside :
//   1. TraverseIdentifier, TraverseLiteral, TraverseOneof, TraverseZeroorXXx, etc
//      since we konw the relation between parent and children
//   2. Or TraverseTableData in this function, because we know the relation.
//   3. When TraverseRuleTable pre-check was succ. Because we know the relation.
// These are the only places of colleting succ match for a parent node.
//

bool Parser::TraverseRuleTableRegular(RuleTable *rule_table, AppealNode *appeal) {
  // In TraverseToken(), alt tokens are traversed. The intermediate status of matching
  // are needed. However, if a matching failed in the middle of alt token serial, the
  // status is not cleared in TraverseToken(). It's hard to clear in TraverseToken()
  // as it doesn't know the context of traversal.
  //
  // A better solution is to clear each time entering a top rule table.
  if (rule_table->mProperties & RP_Top) {
    mInAltTokensMatching = false;
    mNextAltTokenIndex = 0;
    if (mTraceTable)
      std::cout << "Clear alt token status." << std::endl;
  }

  bool matched = false;
  unsigned saved_mCurToken = mCurToken;

  bool was_succ = (appeal->mResult == SuccWasSucc) || (appeal->mResult == SuccStillWasSucc);
  unsigned match_num = appeal->GetMatchNum();
  unsigned longest_match = 0;
  if (was_succ)
    longest_match = appeal->LongestMatch();

  // [NOTE] 1. TblLiteral and TblIdentifier don't use the SuccMatch info,
  //           since it's quite simple, we don't need SuccMatch to reduce
  //           the traversal time.
  //        2. Actually TraverseIdentifier and TraverseLiteral don't create new AppealNode
  //           since we don't go inside.
  if ((rule_table == &TblIdentifier))
    return TraverseIdentifier(rule_table, appeal);

  if ((rule_table == &TblLiteral))
    return TraverseLiteral(rule_table, appeal);

  if ((rule_table == &TblTemplateLiteral))
    return TraverseTemplateLiteral(rule_table, appeal);

  if ((rule_table == &TblRegularExpression))
    return TraverseRegularExpression(rule_table, appeal);

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
  case ET_Data: {
    // This is a rare case where a rule table contains only table, either a token
    // or a single child rule. In this case, we need merge the child's match into
    // parent. However, we cannot do the merge in TraverseTableData() since this
    // function will be used in multiple places where we cannot merge.
    AppealNode *child = NULL;
    matched = TraverseTableData(rule_table->mData, appeal, child);
    if (child) {
      child->SetChildIndex(0);
      appeal->CopyMatch(child);
    }
    break;
  }
  case ET_Null:
  default:
    break;
  }

  if(matched) {
    UpdateSuccInfo(saved_mCurToken, appeal);
    appeal->mResult = Succ;
    ResetFailed(rule_table, saved_mCurToken);
    return true;
  } else {
    appeal->mResult = FailChildrenFailed;
    mCurToken = saved_mCurToken;
    if (mEndOfFile)
      mEndOfFile = false;
    AddFailed(rule_table, mCurToken);
    return false;
  }
}

// Returns 1. true if succ.
//         2. child_node which represents 'token'.
bool Parser::TraverseToken(Token *token, AppealNode *parent, AppealNode *&child_node) {
  Token *curr_token = GetActiveToken(mCurToken);
  bool found = false;
  mIndentation += 2;

  if (mTraceTable) {
    std::string name = "token:";
    name += token->GetName();
    name += " curr_token:";
    name += curr_token->GetName();
    DumpEnterTable(name.c_str(), mIndentation);
  }

  bool use_alt_token = false;
  AppealNode *appeal = NULL;

  // [TODO]
  // We enable skipping semi-colon. Later we will implement TS specific version of parser
  // which overried TraverseToken().
  // We handle one case: if we see } instead of ';', we can skip
  // There are many other cases. Will handle later.
  if (token->IsSeparator() && token->GetSepId() == SEP_Semicolon) {
    if (curr_token->IsSeparator() && curr_token->GetSepId() == SEP_Rbrace) {
      // 1. There are rule like ZEROORMORE(';'). In this case, we don't insert
      RuleTable *parent_rt = parent->GetTable();
      bool need_insert = true;
      if (parent_rt->mType == ET_Zeroormore || parent_rt->mType == ET_Zeroorone)
        need_insert = false;

      // 2. we need check cases where we already have one previous ';'.
      Token *prev = mActiveTokens.ValueAtIndex(mCurToken - 1);
      if (prev != token && need_insert) {
        // The simpliest way is to insert a semicolon token in mActiveTokens.
        // Just pretend we lex a semicolon.
        InsertToken(mCurToken, token);
        curr_token = token;
        if (mTraceTable) {
          std::cout << "Auto-insert one semicolon." << std::endl;
        }
      }
    }
  }

  if (token == curr_token) {
    appeal = mAppealNodePool.NewAppealNode();
    child_node = appeal;
    mAppealNodes.push_back(appeal);
    appeal->SetToken(curr_token);
    appeal->SetStartIndex(mCurToken);
    appeal->SetParent(parent);
    parent->AddChild(appeal);
    appeal->mResult = Succ;
    appeal->AddMatch(mCurToken);
    found = true;
    MoveCurToken();
  } else {
    // TraverseToken handles system tokens which could have alternative tokens.
    if (curr_token->mAltTokens) {
      bool alt_found = false;
      AltToken *pat = curr_token->mAltTokens;
      if (token == &gSystemTokens[pat->mAltTokenId]) {
        appeal = mAppealNodePool.NewAppealNode();
        child_node = appeal;
        mAppealNodes.push_back(appeal);
        appeal->SetToken(curr_token);
        appeal->SetStartIndex(mCurToken);
        appeal->SetParent(parent);
        parent->AddChild(appeal);

        found = true;
        alt_found = true;
        mATMToken = mCurToken;

        if (mTraceTable) {
          std::cout << "Work on alt token, index : " << mNextAltTokenIndex << std::endl;
        }

        if (!mInAltTokensMatching) {
          mInAltTokensMatching = true;
          appeal->m1stAltTokenMatched = true;
          if (mTraceTable) {
            std::cout << "Turn On mInAltTokensMatching " << std::endl;
          }
        }
        mNextAltTokenIndex++;

        appeal->mResult = Succ;
        appeal->AddMatch(mCurToken);
        appeal->mAltToken = token;
        use_alt_token = true;

        // when it's done with all alt tokens
        if (mNextAltTokenIndex == pat->mNum) {
          // We only move cursor when all alt-tokens are matched
          MoveCurToken();
          mInAltTokensMatching = false;
          mNextAltTokenIndex = 0;
          if (mTraceTable) {
            std::cout << "Work on alt token is successfully finised. Set mNextAltTokenIndex to : " << mNextAltTokenIndex << std::endl;
            std::cout << "Turn Off mInAltTokensMatching " << std::endl;
          }
        }
      }
    }
  }

  if (mTraceTable) {
    std::string name;
    if (use_alt_token)
      name = "token (alttoken used):";
    else
      name = "token:";
    name += token->GetName();
    if (appeal)
      DumpExitTable(name.c_str(), mIndentation, appeal);
    else
      DumpExitTable(name.c_str(), mIndentation, FailNotRightToken);
  }

  mIndentation -= 2;
  return found;
}

// Supplemental function invoked when TraverseSpecialToken succeeds.
// It helps set all the data structures.
void Parser::TraverseSpecialTableSucc(RuleTable *rule_table, AppealNode *appeal) {
  Token *curr_token = GetActiveToken(mCurToken);
  appeal->mResult = Succ;
  appeal->SetToken(curr_token);
  appeal->SetStartIndex(mCurToken);
  appeal->AddMatch(mCurToken);

  MoveCurToken();
}

// We don't go into Literal table.
// 'appeal' is the node for this rule table. This is different than TraverseOneof
// or the others where 'appeal' is actually a parent node.
bool Parser::TraverseLiteral(RuleTable *rule_table, AppealNode *appeal) {
  Token *curr_token = GetActiveToken(mCurToken);
  const char *name = GetRuleTableName(rule_table);
  bool found = false;

  if (curr_token->IsLiteral()) {
    found = true;
    TraverseSpecialTableSucc(rule_table, appeal);
  } else {
    appeal->mResult = FailNotLiteral;
    AddFailed(rule_table, mCurToken);
  }

  return found;
}

// We don't go into TemplateLiteral table.
// 'appeal' is the node for this rule table. This is different than TraverseOneof
// or the others where 'appeal' is actually a parent node.
bool Parser::TraverseTemplateLiteral(RuleTable *rule_table, AppealNode *appeal) {
  Token *curr_token = GetActiveToken(mCurToken);
  const char *name = GetRuleTableName(rule_table);
  bool found = false;

  if (curr_token->IsTempLit()) {
    found = true;
    TraverseSpecialTableSucc(rule_table, appeal);
  } else {
    appeal->mResult = FailNotLiteral;
    AddFailed(rule_table, mCurToken);
  }

  return found;
}

// We don't go into RegularExpressionLiteral table.
// 'appeal' is the node for this rule table. This is different than TraverseOneof
// or the others where 'appeal' is actually a parent node.
bool Parser::TraverseRegularExpression(RuleTable *rule_table, AppealNode *appeal) {
  Token *curr_token = GetActiveToken(mCurToken);
  const char *name = GetRuleTableName(rule_table);
  bool found = false;

  if (curr_token->IsRegExpr()) {
    found = true;
    TraverseSpecialTableSucc(rule_table, appeal);
  } else {
    appeal->mResult = FailNotRegExpr;
    AddFailed(rule_table, mCurToken);
  }

  return found;
}

// We don't go into Identifier table.
// 'appeal' is the node for this rule table.
bool Parser::TraverseIdentifier(RuleTable *rule_table, AppealNode *appeal) {
  Token *curr_token = GetActiveToken(mCurToken);
  const char *name = GetRuleTableName(rule_table);
  bool found = false;

  if (curr_token->IsIdentifier()) {
    found = true;
    TraverseSpecialTableSucc(rule_table, appeal);
  } else {
    appeal->mResult = FailNotIdentifier;
    AddFailed(rule_table, mCurToken);
  }

  return found;
}

// It always return true.
// Moves until hit a NON-target data
// [Note]
//   We don't count the 'mCurToken' as a succ matching to return. It means catch 'zero'.
//
// 'appeal' is the node of 'rule_table'.

bool Parser::TraverseZeroormore(RuleTable *rule_table, AppealNode *appeal) {
  unsigned saved_mCurToken = mCurToken;
  bool found_real = false;

  MASSERT((rule_table->mNum == 1) && "zeroormore node has more than one elements?");
  TableData *data = rule_table->mData;

  // prepare the prev_succ_tokens] for the 1st iteration.
  SmallVector<unsigned> prev_succ_tokens;
  prev_succ_tokens.SetBlockSize(1024);
  prev_succ_tokens.PushBack(mCurToken - 1);

  // Need to avoid duplicated mCurToken. Look at the rule
  // rule SwitchBlock : '{' + ZEROORMORE(ZEROORMORE(SwitchBlockStatementGroup) + ZEROORMORE(SwitchLabel)) + '}'
  // The inner "ZEROORMORE(SwitchBlockStatementGroup) + ZEROORMORE(SwitchLabel)" could return multiple
  // succ matches including the 'zero' match. The next time we go through the outer ZEROORMORE(...) it
  // could traverse the same mCurToken again, at least 'zero' is always duplicated. This will be
  // an endless loop.
  SmallVector<unsigned> visited;
  visited.SetBlockSize(1024);

  while(1) {
    // A set of results of current instance
    bool found_subtable = false;
    SmallVector<unsigned> subtable_succ_tokens;
    subtable_succ_tokens.SetBlockSize(1024);

    // Like TraverseConcatenate, we will try all good matchings of previous instance.
    for (unsigned j = 0; j < prev_succ_tokens.GetNum(); j++) {
      unsigned prev = prev_succ_tokens.ValueAtIndex(j);
      mCurToken = prev + 1;
      visited.PushBack(prev);

      AppealNode *child = NULL;
      bool temp_found = TraverseTableData(data, appeal, child);
      found_subtable |= temp_found;

      if (temp_found && child) {
        unsigned match_num = child->GetMatchNum();
        for (unsigned id = 0; id < match_num; id++) {
          unsigned match = child->GetMatch(id);
          subtable_succ_tokens.PushBack(match);
          appeal->AddMatch(match);
        }
      }
    }

    prev_succ_tokens.Clear();

    // It's possible that sub-table is also a ZEROORxxx, and is succ without
    // real matching. This will be considered as a STOP.
    unsigned subtable_tokens_num = subtable_succ_tokens.GetNum();
    if (found_subtable && subtable_tokens_num > 0) {
      found_real = true;
      // set prev
      for (unsigned id = 0; id < subtable_tokens_num; id++) {
        unsigned t = subtable_succ_tokens.ValueAtIndex(id);
        if (!visited.Find(t))
          prev_succ_tokens.PushBack(t);
      }
    } else {
      break;
    }
  }

  mCurToken = saved_mCurToken;
  appeal->mResult = Succ;
  if (found_real && appeal->GetMatchNum() > 0)
    mCurToken = appeal->LongestMatch() + 1;

  return true;
}

// 'appeal' is the node of 'rule_table'.
bool Parser::TraverseZeroorone(RuleTable *rule_table, AppealNode *appeal) {
  MASSERT((rule_table->mNum == 1) && "zeroorone node has more than one elements?");
  TableData *data = rule_table->mData;
  AppealNode *child = NULL;
  bool found = TraverseTableData(data, appeal, child);
  if (child)
    appeal->CopyMatch(child);
  return true;
}

// 1. Save all the possible matchings from children.
//    There is one exception. If the rule-table is a top rule it should
//    get the longest match.
// 2. As return value we choose the longest matching.
//
// 'appeal' is the node of 'rule_table'.
bool Parser::TraverseOneof(RuleTable *rule_table, AppealNode *appeal) {
  bool found = false;
  unsigned new_mCurToken = mCurToken; // position after most tokens eaten
  unsigned old_mCurToken = mCurToken;

  for (unsigned i = 0; i < rule_table->mNum; i++) {
    TableData *data = rule_table->mData + i;
    AppealNode *child = NULL;
    bool temp_found = TraverseTableData(data, appeal, child);
    found = found | temp_found;
    if (temp_found) {
      if (child)
        appeal->CopyMatch(child);
      if (mCurToken > new_mCurToken)
        new_mCurToken = mCurToken;

      // Restore the position of original mCurToken, for the next child traversal.
      mCurToken = old_mCurToken;

      // Some ONEOF rules can have only children matching current token seq.
      // Or the language desiner just want to match the first children rule.
      if (rule_table->mProperties & RP_Single) {
        break;
      }
    }
  }

  if (found && (rule_table->mProperties & RP_Top)) {
    unsigned longest = appeal->LongestMatch();
    appeal->ClearMatch();
    appeal->AddMatch(longest);
  }

  // move position according to the longest matching
  mCurToken = new_mCurToken;
  return found;
}

//
// [NOTE] 1. There could be an issue of matching number explosion. Each node could have
//           multiple matchings, and when they are concatenated the number would be
//           matchings_1 * matchings_2 * ... This needs to be taken care of since we need
//           all the possible matches so that later nodes can still have opportunity.
//        2. Each node will try all starting tokens which are the ending token of previous
//           node.
//        3. We need take care of the final_succ_tokens carefully.
//           e.g. in a rule like below
//              rule AA : BB + CC + ZEROORONE(xxx)
//           If ZEROORONE(xxx) doesn't match anything, it sets subtable_succ_tokens to 0. However
//           rule AA matches 'BB + CC'. So final_succ_tokens needs to be calculated carefully.
//        4. We are going to take succ match info from SuccMatch, not from a specific
//           AppealNode. SuccMatch has the complete info.
//
// 'appeal' is the node of 'rule_table'.

bool Parser::TraverseConcatenate(RuleTable *rule_table, AppealNode *appeal) {
  // Init found to true.
  // 'found' is tricky. If all child rule tables are Zeroorxxx, it always returns true.
  bool found = true;

  SmallVector<unsigned> prev_succ_tokens;
  SmallVector<unsigned> subtable_succ_tokens;

  unsigned saved_mCurToken = mCurToken;

  // prepare the prev_succ_tokens[_num] for the 1st iteration.
  int last_matched = mCurToken - 1;
  prev_succ_tokens.PushBack(last_matched);

  // This is regarding the matching of Alternative Tokens. For example,
  //    a<b<c>>
  // >> needs to be matched as two '>'.
  // mInAltTokensMatching is used to control if we are in the middle of matching such tokens.
  // However, there is a complicated case, that is  "c>", which could be matched as part
  // RelationshipExpression, and it turned on mInAltTokensMatching. But it actually fails
  // to be a RelationshipExpression and mInAltTokensMatching should be turned off.
  bool turned_on_AltToken = false;

  for (unsigned i = 0; i < rule_table->mNum; i++) {
    bool is_zeroxxx = false;
    bool is_token = false;
    bool old_mInAltTokensMatching = mInAltTokensMatching;

    TableData *data = rule_table->mData + i;
    if (data->mType == DT_Subtable) {
      RuleTable *zero_rt = data->mData.mEntry;
      if (zero_rt->mType == ET_Zeroormore || zero_rt->mType == ET_Zeroorone)
        is_zeroxxx = true;
    } else if (data->mType == DT_Token) {
      is_token = true;
    }

    SmallVector<unsigned> carry_on_prev;
    bool found_subtable = false;

    subtable_succ_tokens.Clear();

    // We will iterate on all previous succ matches.
    for (unsigned j = 0; j < prev_succ_tokens.GetNum(); j++) {
      unsigned prev = prev_succ_tokens.ValueAtIndex(j);
      mCurToken = prev + 1;

      // In ATM, we don't move mCurToken.
      // Alt Token Matching only applies to that specific 'prev'.
      if (mInAltTokensMatching && (prev == mATMToken))
        mCurToken = mATMToken;

      AppealNode *child = NULL;
      bool temp_found = TraverseTableData(data, appeal, child);
      if (child)
        child->SetChildIndex(i);
      found_subtable |= temp_found;

      if (temp_found && child) {
        for (unsigned id = 0; id < child->GetMatchNum(); id++) {
          unsigned match = child->GetMatch(id);
          if (!subtable_succ_tokens.Find(match))
            subtable_succ_tokens.PushBack(match);
        }
      }
    }

    // for Zeroorone/Zeroormore node it always returns true. NO matter how
    // many tokens it really matches, 'zero' is also a correct match. we
    // need take it into account so that the next rule table can try
    // on it.
    if (!is_zeroxxx)
      prev_succ_tokens.Clear();

    // is_zeroxxx seems redundant because the traversal should always be true.
    // However, it's not true. In mLineMode, mEndOfFile could be set before
    // traversing this ZEROORXXX table. It will return false.
    // Since we do treat this case as success, so is_zeroxxx is included in this
    // condition expression.
    if (found_subtable || is_zeroxxx) {
      for (unsigned id = 0; id < subtable_succ_tokens.GetNum(); id++) {
        unsigned token = subtable_succ_tokens.ValueAtIndex(id);
        if (!prev_succ_tokens.Find(token))
          prev_succ_tokens.PushBack(token);
      }
      // alert mInAltTokensMatching is turned on
      if (is_token && mInAltTokensMatching && !old_mInAltTokensMatching) {
        turned_on_AltToken = true;
      }
    } else {
      // Once a child rule fails, the 'appeal' fails.
      found = false;
      break;
    }
  }

  mCurToken = saved_mCurToken;

  if (found) {
    for (unsigned id = 0; id < prev_succ_tokens.GetNum(); id++) {
      unsigned token = prev_succ_tokens.ValueAtIndex(id);
      if (token != last_matched)
        appeal->AddMatch(token);
    }
    // mCurToken doesn't have much meaning in current algorithm when
    // transfer to the next rule table, because the next rule will take
    // the succ info and get all matching token of prev, and set them
    // as the starting mCurToken.
    //
    // However, we do set mCurToken to the biggest matching.
    appeal->mResult = Succ;
    if (appeal->GetMatchNum() > 0)
      mCurToken = appeal->LongestMatch() + 1;
  } else if (turned_on_AltToken) {
    mInAltTokensMatching = false;
    if (mTraceTable)
      std::cout << "Turned Off mInAltTokensMatching." << std::endl;
  }

  return found;
}

// 1. Returns (1) true if found, (2) the child_node represents 'data'.
// 2. We don't merge the succ matches to 'appeal' in this function. It will be handled by
//    the TraverseXXX(), since we don't know what type of rule table of appeal. For example,
//    Oneof and Zeroormore should be handled differently.
// 3. The mCurToken moves if found target, or restore the original location.
bool Parser::TraverseTableData(TableData *data, AppealNode *appeal, AppealNode *&child_node) {
  // Usually mCurToken is a new token to be matched. So if it's end of file, we simply return false.
  // However, if mCurToken is actually an ATMToken, which means it needs to be matched
  // multiple times, we are NOT at the end yet.
  if (mEndOfFile) {
    if (!(mInAltTokensMatching && (mCurToken == mATMToken)))
      return false;
  }

  unsigned old_pos = mCurToken;
  bool     found = false;
  Token   *curr_token = GetActiveToken(mCurToken);

  switch (data->mType) {
  case DT_Char:
  case DT_String:
    //MASSERT(0 && "Hit Char/String in TableData during matching!");
    //TODO: Need compare literal. But so far looks like it's impossible to
    //      have a literal token able to match a string/char in rules.
    break;
  // separator, operator, keywords are generated as DT_Token.
  // just need check the pointer of token
  case DT_Token:
    found = TraverseToken(&gSystemTokens[data->mData.mTokenId], appeal, child_node);
    break;
  case DT_Type:
    break;
  case DT_Subtable: {
    RuleTable *t = data->mData.mEntry;
    found = TraverseRuleTable(t, appeal, child_node);
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

void Parser::SetIsDone(unsigned group_id, unsigned start_token) {
  Group2Rule g2r = gGroup2Rule[group_id];
  for (unsigned i = 0; i < g2r.mNum; i++) {
    RuleTable *rt = g2r.mRuleTables[i];
    SuccMatch *succ = &gSucc[rt->mIndex];
    bool found = succ->GetStartToken(start_token);
    if(found)
      succ->SetIsDone();
  }
}

void Parser::SetIsDone(RuleTable *rt, unsigned start_token) {
  // We don't save SuccMatch for TblLiteral and TblIdentifier
  if((rt == &TblLiteral) ||
     (rt == &TblIdentifier) ||
     (rt == &TblRegularExpression) ||
     (rt == &TblTemplateLiteral))
    return;

  SuccMatch *succ = &gSucc[rt->mIndex];
  bool found = succ->GetStartToken(start_token);
  MASSERT(found);
  succ->SetIsDone();
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

// We don't want to use recursion. So a deque is used here.
static std::deque<AppealNode*> to_be_sorted;

void Parser::SortOut() {
  // we remove all failed children, leaving only succ child
  AppealNode *root = NULL;
  if (!mLineMode) {
    for (unsigned i = 0; i < mRootNode->mChildren.GetNum(); i++) {
      AppealNode *n = mRootNode->mChildren.ValueAtIndex(i);
      if (!n->IsFail() && !n->IsNA())
        mRootNode->mSortedChildren.PushBack(n);
    }
    MASSERT(mRootNode->mSortedChildren.GetNum()==1);
    root = mRootNode->mSortedChildren.ValueAtIndex(0);
  } else {
    // LineMode could have >1 matching children
    // Find the longest match
    unsigned longest = mRootNode->LongestMatch();
    for (unsigned i = 0; i < mRootNode->mChildren.GetNum(); i++) {
      AppealNode *n = mRootNode->mChildren.ValueAtIndex(i);
      if (n->LongestMatch() == longest) {
        root = n;
        mRootNode->mSortedChildren.PushBack(n);
        break;
      }
    }
  }

  // First sort the root.
  RuleTable *table = root->GetTable();
  MASSERT(table && "root is not a table?");
  SuccMatch *succ = &gSucc[table->mIndex];
  MASSERT(succ && "root has no SuccMatch?");
  bool found = succ->GetStartToken(root->GetStartIndex());

  // In regular parsing, Top level tree can have only one match, otherwise, the language
  // is ambiguous.
  // In LineMode parsing, we are parsing an expression, and it could be multiple matching
  // with some partial matchings. We pick the longest matching and it should be the same
  // as mActiveTokens.GetNum() meaning it matches all token so far.
  unsigned match_num = succ->GetMatchNum();
  unsigned match = 0;
  if (mLineMode) {
    match = root->LongestMatch();
    MASSERT(match + 1 == mActiveTokens.GetNum());
  } else {
    MASSERT(match_num == 1 && "Top level tree has >1 matches?");
    match = succ->GetOneMatch(0);
  }
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

  // If node->mResult is SuccWasSucc, it means we didn't traverse its children
  // during matching. In SortOut, we simple return. However, when generating IR,
  // the children have to be created.
  if (node->mResult == SuccWasSucc) {
    MASSERT(node->mChildren.GetNum() == 0);
    return;
  }

  // The last instance of recursion traversal doesn't need SortOut.
  if (node->mResult == SuccStillWasSucc)
    return;

  RuleTable *rule_table = node->GetTable();

  // Table Identifier and Literal don't need sort.
  if (rule_table == &TblIdentifier || rule_table == &TblLiteral || rule_table == &TblTemplateLiteral)
    return;

  // The lead node of a traversal group need special solution, if they are
  // simply connect to previous instance(s).
  if (mRecursionAll.IsLeadNode(rule_table)) {
    bool connect_only = true;
    for (unsigned i = 0; i < node->mChildren.GetNum(); i++) {
      AppealNode *child = node->mChildren.ValueAtIndex(i);
      if (!child->IsTable() || child->GetTable() != rule_table) {
        connect_only = false;
        break;
      }
    }

    if (connect_only) {
      SortOutRecursionHead(node);
      return;
    }
  }

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

// RecursionHead is any of the LeadNodes of a recursion group. There could be
// multiple leaders in a group, but only one master leader which is mSelf of
// class TraverseRecurson.
// The children of them could be
//   1. multiple lead nodes of multiple instance, for the master leader of group.
//   2. The single node of previous instance, for non-master leaders.
// In any case, parent and children have the same rule table.
void Parser::SortOutRecursionHead(AppealNode *parent) {
  unsigned parent_match = parent->GetFinalMatch();

  //Find the first child having the same match as parent.
  for (unsigned i = 0; i < parent->mChildren.GetNum(); i++) {
    AppealNode *child = parent->mChildren.ValueAtIndex(i);
    if (child->IsFail() || child->IsNA())
      continue;
    bool found = child->FindMatch(parent_match);
    if (found) {
      to_be_sorted.push_back(child);
      parent->mSortedChildren.PushBack(child);
      child->SetFinalMatch(parent_match);
      child->SetSorted();
      child->SetParent(parent);
      break;
    }
  }
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
  for (unsigned i = 0; i < parent->mChildren.GetNum(); i++) {
    AppealNode *child = parent->mChildren.ValueAtIndex(i);
    if (child->IsFail() || child->IsNA())
      continue;

    // In OneOf node, A successful child node must have its last matching
    // token the same as parent. Look into the child's SuccMatch, trim it
    // if it has multiple matching.

    if (child->IsToken()) {
      // Token node matches just one token.
      if (child->GetStartIndex() == parent_match) {
        child->SetSorted();
        child->SetFinalMatch(parent_match);
        child->SetParent(parent);
        good_children++;
        parent->mSortedChildren.PushBack(child);
      }
    } else {
      bool found = child->FindMatch(parent_match);
      if (found) {
        good_children++;
        to_be_sorted.push_back(child);
        parent->mSortedChildren.PushBack(child);
        child->SetFinalMatch(parent_match);
        child->SetSorted();
        child->SetParent(parent);
      }
    }

    // We only pick the first good child.
    if (good_children > 0)
      break;
  }

  // If we find good_children in the DIRECT children, good to return.
  if(good_children)
    return;

  // If no good child found in direct children, we look into
}

// For Zeroormore node, where all children's matching tokens are linked together one after another.
// All children nodes have the same rule table.
void Parser::SortOutZeroormore(AppealNode *parent) {
  MASSERT(parent->IsSorted());

  // Zeroormore could match nothing.
  unsigned match_num = parent->GetMatchNum();
  if (match_num == 0)
    return;

  unsigned parent_start = parent->GetStartIndex();
  unsigned parent_match = parent->GetFinalMatch();

  unsigned last_match = parent_match;

  // We look into all children, and find out the one with matching token.
  // We do it in a backwards way. Find the one matching 'last_match' at first,
  // and move backward until find the one starting with parent_start.
  SmallVector<AppealNode*> sorted_children;
  while(1) {
    AppealNode *good_child = NULL;
    for (unsigned i = 0; i < parent->mChildren.GetNum(); i++) {
      AppealNode *child = parent->mChildren.ValueAtIndex(i);
      if (sorted_children.Find(child))
        continue;
      if (child->IsSucc() && child->FindMatch(last_match)) {
        good_child = child;
        break;
      }
    }
    MASSERT(good_child);

    sorted_children.PushBack(good_child);
    good_child->SetFinalMatch(last_match);
    good_child->SetParent(parent);
    good_child->SetSorted();
    last_match = good_child->GetStartIndex() - 1;

    // Finished the backward searching.
    if (good_child->GetStartIndex() == parent_start)
      break;
  }

  MASSERT(last_match + 1 == parent->GetStartIndex());

  for (int i = sorted_children.GetNum() - 1; i >= 0; i--) {
    AppealNode *child = sorted_children.ValueAtIndex(i);
    parent->mSortedChildren.PushBack(child);
    if (child->IsTable())
      to_be_sorted.push_back(child);
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

  MASSERT((parent->mChildren.GetNum() == 1) && "Zeroorone has >1 valid children?");
  AppealNode *child = parent->mChildren.ValueAtIndex(0);

  if (child->IsFail() || child->IsNA())
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
  parent->mSortedChildren.PushBack(child);
  child->SetParent(parent);
}

// [NOTE] Concatenate could have more than one succ matches. These matches
//        logically form different trees. But they are all children AppealNodes
//        of 'parent'. So sortout will find the matching subtree.
//
// The algorithm is going from the children of last sub-rule element. And traverse
// backwards until reaching the beginning.

void Parser::SortOutConcatenate(AppealNode *parent) {
  MASSERT(parent->IsSorted());
  RuleTable *rule_table = parent->GetTable();
  MASSERT(rule_table && "parent is not a table?");

  unsigned parent_start = parent->GetStartIndex();
  unsigned parent_match = parent->GetFinalMatch();

  // It's possible for 'parent' to match nothing if all children are Zeroorxxx
  // which matches nothing.
  unsigned match_num = parent->GetMatchNum();
  if (match_num == 0)
    return;

  unsigned last_match = parent_match;

  // We look into all children, and find out the one with matching ruletable/token
  // and token index.
  SmallVector<AppealNode*> sorted_children;
  for (int i = rule_table->mNum - 1; i >= 0; i--) {
    TableData *data = rule_table->mData + i;
    AppealNode *child = parent->FindIndexedChild(last_match, i);
    // It's possible that we find NO child if 'data' is a ZEROORxxx table
    bool good_child = false;
    if (!child) {
      if (data->mType == DT_Subtable) {
        RuleTable *table = data->mData.mEntry;
        if (table->mType == ET_Zeroorone || table->mType == ET_Zeroormore)
          good_child = true;
      }
      MASSERT(good_child);
    } else {
      sorted_children.PushBack(child);
      child->SetFinalMatch(last_match);
      child->SetParent(parent);
      child->SetSorted();
      // If 'child' is matching an alternative token and is NOT the first index
      // we want to keep the last_match
      if (child->mAltToken && !(child->m1stAltTokenMatched))
        last_match = last_match;
      else
        last_match = child->GetStartIndex() - 1;
    }
  }
  MASSERT(last_match + 1 == parent->GetStartIndex());

  for (int i = sorted_children.GetNum() - 1; i >= 0; i--) {
    AppealNode *child = sorted_children.ValueAtIndex(i);
    parent->mSortedChildren.PushBack(child);
    if (child->IsTable())
      to_be_sorted.push_back(child);
  }

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
    MASSERT((parent->mChildren.GetNum() == 1) && "Should have only one child?");
    AppealNode *child = parent->mChildren.ValueAtIndex(0);
    child->SetFinalMatch(parent->GetFinalMatch());
    child->SetSorted();
    to_be_sorted.push_back(child);
    parent->mSortedChildren.PushBack(child);
    child->SetParent(parent);
    break;
  }
  case DT_Token: {
    // token in table-data created a Child AppealNode
    // Just keep the child node. Don't need do anything.
    AppealNode *child = parent->mChildren.ValueAtIndex(0);
    child->SetFinalMatch(child->GetStartIndex());
    parent->mSortedChildren.PushBack(child);
    child->SetParent(parent);
    break;
  }
  case DT_Char:
  case DT_String:
  case DT_Type:
  case DT_Null:
  default:
    break;
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

  std::cout << "[" << dump_id << ":" << n->GetChildIndex() << "] ";
  if (n->IsToken()) {
    n->mData.mToken->Dump();
  } else {
    RuleTable *t = n->GetTable();
    std::cout << "Table " << GetRuleTableName(t) << "@" << n->GetStartIndex() << ": ";

    if (n->mResult == SuccWasSucc)
      std::cout << "WasSucc";

    for (unsigned i = 0; i < n->mSortedChildren.GetNum(); i++) {
      std::cout << seq_num << ",";
      to_be_dumped.push_back(n->mSortedChildren.ValueAtIndex(i));
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

static std::vector<AppealNode*> was_succ_list;

// The SuccWasSucc node and its patching node is a one-one mapping.
// We don't use a map to maintain this. We use vectors to do this
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
    if (node->mResult == SuccWasSucc) {
      was_succ_list.push_back(node);
      if (mTracePatchWasSucc) {
        std::cout << "Find WasSucc ";
        if (node->IsTable())
          std::cout << GetRuleTableName(node->GetTable()) << std::endl;
        else
          std::cout << "a token?" << std::endl;
      }
    } else {
      for (unsigned i = 0; i < node->mSortedChildren.GetNum(); i++)
        working_list.push_back(node->mSortedChildren.ValueAtIndex(i));
    }
  }
  return;
}

// For each node in was_succ_list there are one or more patching subtree.
// So far looks like every matching work. But we use the first one which
// usually is the real complete matching.

void Parser::FindPatchingNodes() {
  std::vector<AppealNode*>::iterator it = was_succ_list.begin();
  for (; it != was_succ_list.end(); it++) {
    AppealNode *was_succ = *it;
    MASSERT(was_succ->IsSorted());
    unsigned final_match = was_succ->GetFinalMatch();

    SuccMatch *succ_match = &gSucc[was_succ->GetTable()->mIndex];
    MASSERT(succ_match && "WasSucc's rule has no SuccMatch?");
    bool found = succ_match->GetStartToken(was_succ->GetStartIndex());
    MASSERT(found && "WasSucc cannot find start index in SuccMatch?");

    AppealNode *patch = NULL;
    for (unsigned i = 0; i < succ_match->GetSuccNodesNum(); i++) {
      AppealNode *node = succ_match->GetSuccNode(i);
      if (node->FindMatch(final_match)) {
        patch = node;
        break;
      }
    }
    MASSERT(patch && "succ matching node is missing?");

    if (mTracePatchWasSucc)
      std::cout << "Find one match " << patch << std::endl;

    was_succ_matched_list.push_back(was_succ);
    patching_list.push_back(patch);
  }
}

// This is another entry point of sort, similar as SortOut().
// The only difference is we use 'reference' as the refrence of final match.
void Parser::SupplementalSortOut(AppealNode *root, AppealNode *reference) {
  MASSERT(root->mSortedChildren.GetNum()==0 && "root should be un-sorted.");
  MASSERT(root->IsTable() && "root should be a table node.");

  // step 1. Find the last matching token index we want.
  MASSERT(reference->IsSorted() && "reference is not sorted?");

  // step 2. Set the root.
  SuccMatch *succ = &gSucc[root->GetTable()->mIndex];
  MASSERT(succ && "root node has no SuccMatch?");
  root->SetFinalMatch(reference->GetFinalMatch());
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
      was_succ->mResult = Succ;

      // We can copy only sorted nodes. The original mChildren cannot be copied since
      // it's the original tree. We don't want to mess it up. Think about it, if you
      // copy the mChildren to was_succ, there are duplicated tree nodes. This violates
      // the definition of the original tree.
      for (unsigned j = 0; j < patch->mSortedChildren.GetNum(); j++)
        was_succ->AddSortedChild(patch->mSortedChildren.ValueAtIndex(j));
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
  working_list.push_back(mRootNode->mSortedChildren.ValueAtIndex(0));

  while(!working_list.empty()) {
    AppealNode *node = working_list.front();
    working_list.pop_front();
    MASSERT(node->IsSucc() && "Sorted node is not succ?");

    // Shrink edges
    if (node->IsToken())
      continue;
    node = SimplifyShrinkEdges(node);

    for (unsigned i = 0; i < node->mSortedChildren.GetNum(); i++) {
      working_list.push_back(node->mSortedChildren.ValueAtIndex(i));
    }
  }

  if (mTraceSortOut)
    DumpSortOut(mRootNode->mSortedChildren.ValueAtIndex(0), "Simplify AppealNode Trees");
}

// Reduce an edge is (1) Pred has only one succ
//                   (2) Succ has only one pred (this is always true)
//                   (3) There is no Rule Action of pred's rule table, regarding this succ.
//                       Or, the rule is a LeadNode of recursion and succ is one of the
//                       instance. In this case, we don't care about the rule action.
// Keep this reduction until the conditions are violated
//
// Returns the new 'node' which stops the shrinking.
AppealNode* Parser::SimplifyShrinkEdges(AppealNode *node) {

  // index will be defined only once since it's the index-child of the 'node'
  // transferred into this function.
  unsigned index = 0;

  while(1) {
    // step 1. Check condition (1) (2)
    if (node->mSortedChildren.GetNum() != 1)
      break;
    AppealNode *child = node->mSortedChildren.ValueAtIndex(0);

    // step 2. Find out the index of child, through looking into sub-ruletable or token.

    // There is one case where it cannot find child_index. In the left recursion
    // parsing, each instance is connected to its previous one through the lead node.
    // The connected two nodes are both lead rule table. We need remove one of them.
    //
    // In this case we don't worry about action since one of them is kept and the
    // actions are kept actually.

    bool skip = false;
    RuleTable *rt_p = node->GetTable();
    RuleTable *rt_c = child->GetTable();
    if (rt_p == rt_c && mRecursionAll.IsLeadNode(rt_p))
      skip = true;

    unsigned child_index = child->GetChildIndex();
    if (!skip) {
      // step 3. check condition (3)
      //         [NOTE] in RuleAction, element index starts from 1.
      RuleTable *rt = node->GetTable();
      bool has_action = RuleActionHasElem(rt, child_index + 1);
      if (has_action)
        break;
    }

    // step 4. Shrink the edge. This is to remove 'node' by connecting 'node's father
    //         to child. We need go on shrinking with child.
    AppealNode *parent = node->GetParent();
    parent->ReplaceSortedChild(node, child);

    index = node->GetChildIndex();
    child->SetChildIndex(index);

    // step 5. keep going
    node = child;
  }

  return node;
}

////////////////////////////////////////////////////////////////////////////////////
//                             Build the AST
////////////////////////////////////////////////////////////////////////////////////

TreeNode* Parser::BuildAST() {
  mLineModeRoot = NULL;
  mNormalModeRoot = NULL;

  std::stack<AppealNode*> appeal_stack;
  appeal_stack.push(mRootNode->mSortedChildren.ValueAtIndex(0));

  // 1) If all children done. Time to create tree node for 'appeal_node'
  // 2) If some are done, some not. Add the first not-done child to stack
  while(!appeal_stack.empty()) {
    AppealNode *appeal_node = appeal_stack.top();
    bool children_done = true;
    for (unsigned i = 0; i < appeal_node->mSortedChildren.GetNum(); i++) {
      AppealNode *child = appeal_node->mSortedChildren.ValueAtIndex(i);
      if (!child->AstCreated()) {
        appeal_stack.push(child);
        children_done = false;
        break;
      }
    }

    if (children_done) {
      // Create tree node when there is a rule table, or meanful tokens.
      MASSERT(!appeal_node->GetAstTreeNode());
      TreeNode *sub_tree = NewTreeNode(appeal_node);
      if (sub_tree) {
        appeal_node->SetAstTreeNode(sub_tree);
        // mNormalModeRoot is overwritten each time until the last one which is
        // the real root node.
        mNormalModeRoot = sub_tree;
      }
      // pop out the 'appeal_node'
      appeal_node->SetAstCreated();
      appeal_stack.pop();
    }
  }

  // The tree could be an empty statement like: ;

  if (mLineMode)
    mLineModeRoot = mNormalModeRoot;

  return mNormalModeRoot;
}

// Create tree node. Its children have been created tree nodes.
// There are couple issueshere.
//
// 1. An sorted AppealNode could have NO tree node, because it may have NO RuleAction to
//    create the sub tree. This happens if the RuleTable is just a temporary intermediate
//    table created by Autogen, or its rule is just ONEOF without real syntax. Here
//    is an example.
//
//       The AST after BuildAST() for a simple statment: c=a+b;
//
//       ======= Simplify Trees Dump SortOut =======
//       [1] Table TblExpressionStatement@0: 2,3,
//       [2:1] Table TblAssignment@0: 4,5,6,
//       [3] Token
//       [4:1] Token
//       [5:2] Token
//       [6:3] Table TblArrayAccess_sub1@2: 7,8,  <-- supposed to get a binary expression
//       [7:1] Token                              <-- a
//       [8:2] Table TblUnaryExpression_sub1@3: 9,10, <-- +b
//       [9] Token
//       [10:2] Token
//
//    Node [1] won't have a tree node at all since it has no Rule Action attached.
//    Node [6] won't have a tree node either.
//
// 2. A binary operation like a+b could be parsed as (1) expression: a, and (2) a
//    unary operation: +b. This is because we parse them in favor to ArrayAccess before
//    Binary Operation. Usually to handle this issue, in some system like ANTLR,
//    they require you to list the priority, by writing rules from higher priority to
//    lower priority.
//
//    We are going to do a consolidation of the sub-trees, by converting smaller trees
//    to a more compact bigger trees. However, to do this we want to set some rules.
//    *) The parent AppealNode of these sub-trees has no tree node. So the conversion
//       helps make the tree complete.

TreeNode* Parser::NewTreeNode(AppealNode *appeal_node) {
  TreeNode *sub_tree = NULL;

  if (appeal_node->IsToken()) {
    sub_tree = mASTBuilder->CreateTokenTreeNode(appeal_node->GetToken());
    return sub_tree;
  }

  RuleTable *rule_table = appeal_node->GetTable();

  for (unsigned i = 0; i < rule_table->mNumAction; i++) {
    Action *action = rule_table->mActions + i;
    mASTBuilder->mActionId = action->mId;
    mASTBuilder->ClearParams();

    for (unsigned j = 0; j < action->mNumElem; j++) {
      // find the appeal node child
      unsigned elem_idx = action->mElems[j];
      AppealNode *child = appeal_node->GetSortedChild(elem_idx - 1);
      Param p;
      p.mIsEmpty = true;
      // There are 3 cases to handle.
      // 1. child is token, we pass the token to param.
      // 2. child is a sub appeal tree, but has no legal AST tree. For example,
      //    a parameter list: '(' + param-lists + ')'.
      //    if param-list is empty, it has no AST tree.
      //    In this case, we sset mIsEmpty to true.
      // 3. chidl is a sub appeal tree, and has a AST tree too.
      if (child) {
        TreeNode *tree_node = child->GetAstTreeNode();
        if (!tree_node) {
          if (child->IsToken()) {
            p.mIsEmpty = false;
            p.mIsTreeNode = false;
            p.mData.mToken = child->GetToken();
          }
        } else {
          p.mIsEmpty = false;
          p.mIsTreeNode = true;
          p.mData.mTreeNode = tree_node;
        }
      }
      mASTBuilder->AddParam(p);
    }

    // For multiple actions of a rule, there should be only action which create tree.
    // The others are just for adding attribute or else, and return the same tree
    // with additional attributes.
    sub_tree = mASTBuilder->Build();
  }

  if (sub_tree)
    return sub_tree;

  // It's possible that the Rule has no action, meaning it cannot create tree node.
  // Now we have to do some manipulation. Please check if you need all of them.
  sub_tree = Manipulate(appeal_node);

  // It's possible that the sub tree is actually empty. For example, in a Parameter list
  // ( params ). If 'params' is empty, it returns NULL.

  return sub_tree;
}

// It's possible that we get NULL tree.
TreeNode* Parser::Manipulate(AppealNode *appeal_node) {
  TreeNode *sub_tree = NULL;

  std::vector<TreeNode*> child_trees;
  for (unsigned i = 0; i < appeal_node->mSortedChildren.GetNum(); i++) {
    AppealNode *a_node = appeal_node->mSortedChildren.ValueAtIndex(i);
    TreeNode *t_node = a_node->GetAstTreeNode();
    if (t_node)
      child_trees.push_back(t_node);
  }

  // If we have one and only one child's tree node, we take it.
  if (child_trees.size() == 1) {
    sub_tree = child_trees[0];
    if (sub_tree)
      return sub_tree;
    else
      MERROR("We got a broken AST tree, not connected sub tree.");
  }

  // For the tree having two children, there are a few approaches to further
  // manipulate them in order to obtain better AST.
  //
  // 1. There are cases like (type)value, but they are not recoganized as cast.
  //    Insteand they are seperated into two nodes, one is (type), the other value.
  //    So we define ParenthesisNode for (type), and build a CastNode over here.
  //
  // 2. There are cases like a+b could be parsed as "a" and "+b", a symbol and a
  //    unary operation. However, we do prefer binary operation than unary. So a
  //    combination is needed here, especially when the parent node is NULL.
  if (child_trees.size() == 2) {
    TreeNode *child_a = child_trees[0];
    TreeNode *child_b = child_trees[1];

    sub_tree = Manipulate2Cast(child_a, child_b);
    if (sub_tree)
      return sub_tree;
  }

  // In the end, if we still have no suitable solution to create the tree,
  //  we will put subtrees into a PassNode to pass to parent.
  if (child_trees.size() > 0) {
    PassNode *pass = (PassNode*)BuildPassNode();
    std::vector<TreeNode*>::iterator child_it = child_trees.begin();
    for (; child_it != child_trees.end(); child_it++)
      pass->AddChild(*child_it);
    return pass;
  }

  // It's possible that we get a Null tree.
  return sub_tree;
}

TreeNode* Parser::Manipulate2Cast(TreeNode *child_a, TreeNode *child_b) {
  if (child_a->IsParenthesis()) {
    ParenthesisNode *type = (ParenthesisNode*)child_a;
    CastNode *n = (CastNode*)gTreePool.NewTreeNode(sizeof(CastNode));
    new (n) CastNode();
    n->SetDestType(type->GetExpr());
    n->SetExpr(child_b);
    return n;
  }
  return NULL;
}

TreeNode* Parser::Manipulate2Binary(TreeNode *child_a, TreeNode *child_b) {
  if (child_b->IsUnaOperator()) {
    UnaOperatorNode *unary = (UnaOperatorNode*)child_b;
    unsigned property = GetOperatorProperty(unary->GetOprId());
    if ((property & Binary) && (property & Unary)) {
      std::cout << "Convert unary --> binary" << std::endl;
      TreeNode *unary_sub = unary->GetOpnd();
      TreeNode *binary = BuildBinaryOperation(child_a, unary_sub, unary->GetOprId());
      return binary;
    }
  }
  return NULL;
}

TreeNode* Parser::BuildBinaryOperation(TreeNode *childA, TreeNode *childB, OprId id) {
  BinOperatorNode *n = (BinOperatorNode*)gTreePool.NewTreeNode(sizeof(BinOperatorNode));
  new (n) BinOperatorNode(id);
  n->SetOpndA(childA);
  n->SetOpndB(childB);
  return n;
}

TreeNode* Parser::BuildPassNode() {
  PassNode *n = (PassNode*)gTreePool.NewTreeNode(sizeof(PassNode));
  new (n) PassNode();
  return n;
}

////////////////////////////////////////////////////////////////////////////
//         Initialize the Left Recursion Information
// It collects all information from gen_recursion.h/cpp into RecursionAll,
// and calculate the LeadFronNode and FronNode accordingly.
////////////////////////////////////////////////////////////////////////////

void Parser::InitRecursion() {
  mRecursionAll.Init();
}

////////////////////////////////////////////////////////////////////////////
//                          Succ Info Related
////////////////////////////////////////////////////////////////////////////

void Parser::ClearSucc() {
  for (unsigned i = 0; i < RuleTableNum; i++) {
    SuccMatch *sm = &gSucc[i];
    if (sm)
      sm->Clear();
  }
}

/////////////////////////////////////////////////////////////////////////
//                     SuccMatch Implementation
////////////////////////////////////////////////////////////////////////

void SuccMatch::AddStartToken(unsigned t) {
  mNodes.PairedFindOrCreateKnob(t);
  mMatches.PairedFindOrCreateKnob(t);
}

// The container Guamian assures 'n' is not duplicated.
// It also assures 'm' is not duplicated.
void SuccMatch::AddSuccNode(AppealNode *n) {
  MASSERT(mNodes.PairedGetKnobKey() == n->GetStartIndex());
  MASSERT(mMatches.PairedGetKnobKey() == n->GetStartIndex());
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

// Below are paired functions. They need be used together
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

void SuccMatch::RemoveNode(AppealNode *n) {
  return mNodes.PairedRemoveElem(n);
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

void SuccMatch::SetIsDone() {
  mNodes.PairedSetKnobData(1);
}

bool SuccMatch::IsDone() {
  unsigned u1 = mNodes.PairedGetKnobData();
  return (bool)u1;
}

///////////////////////////////////////////////////////////////
//            AppealNode function
///////////////////////////////////////////////////////////////

void AppealNode::AddParent(AppealNode *p) {
  if (!mParent || mParent->IsPseudo())
    mParent = p;
  //else
  //  mSecondParents.PushBack(p);
  return;
}

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

unsigned AppealNode::LongestMatch() {
  unsigned longest = 0;
  MASSERT(IsSucc());
  MASSERT(mMatches.GetNum() > 0);
  for (unsigned i = 0; i < mMatches.GetNum(); i++) {
    unsigned m = mMatches.ValueAtIndex(i);
    longest = m > longest ? m : longest;
  }
  return longest;
}

// The existing match of 'this' is kept.
// mResult is changed only when it's changed from fail to succ.
//
// The node is not added to SuccMatch since we can use 'another'.
void AppealNode::CopyMatch(AppealNode *another) {
  for (unsigned i = 0; i < another->GetMatchNum(); i++) {
    unsigned m = another->GetMatch(i);
    AddMatch(m);
  }
  if (IsFail() || IsNA())
    mResult = another->mResult;
}

void AppealNode::ReplaceSortedChild(AppealNode *existing, AppealNode *replacement) {
  unsigned index;
  bool found = false;
  for (unsigned i = 0; i < mSortedChildren.GetNum(); i++) {
    if (mSortedChildren.ValueAtIndex(i) == existing) {
      index = i;
      found = true;
      break;
    }
  }
  MASSERT(found && "ReplaceSortedChild could not find existing node?");

  *(mSortedChildren.RefAtIndex(index)) = replacement;
  replacement->SetParent(this);
}

AppealNode* AppealNode::GetSortedChild(unsigned index) {
  for (unsigned i = 0; i < mSortedChildren.GetNum(); i++) {
    AppealNode *child = mSortedChildren.ValueAtIndex(i);
    unsigned id = child->GetChildIndex();
    if (id == index)
      return child;
  }
  return NULL;
}

// Look for a specific un-sorted child having the child index and match.
AppealNode* AppealNode::FindIndexedChild(unsigned match, unsigned index) {
  AppealNode *ret_child = NULL;
  for (unsigned i = 0; i < mChildren.GetNum(); i++) {
    AppealNode *child = mChildren.ValueAtIndex(i);
    if (child->IsSucc() &&
        child->FindMatch(match) &&
        (index == child->GetChildIndex())) {
      ret_child = child;
      break;
    }
  }
  return ret_child;
}

}
