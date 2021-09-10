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

#ifndef __PARSER_H__
#define __PARSER_H__

#include <iostream>
#include <fstream>
#include <stack>
#include <list>

#include "lexer.h"
#include "ast_module.h"
#include "ast_builder.h"
#include "container.h"
#include "recursion.h"
#include "succ_match.h"
#include "rule_summary.h"
#include "appnode_pool.h"

namespace maplefe {

class Function;
class Stmt;
class Token;
class RuleTable;
class TableData;
class ASTTree;
class TreeNode;

typedef enum AppealStatus {
  FailWasFailed,
  FailNotRightToken,
  FailNotIdentifier,
  FailNotLiteral,
  FailNotRegExpr,
  FailChildrenFailed,
  Fail2ndOf1st,
  FailLookAhead,

  // Succ :             Really does the matching, will be saved in SuccMatch
  // SuccWasSucc :      Was matched, not tried traversal for a second timewill,
  //                    It will NOT be saved in SuccMatch
  // SuccStillWasSucc : was matched, but tried traversal again. This happens
  //                    in RecurionNodes where it does multiple instances of
  //                    traversal. But it doesn't make any change compared
  //                    to the last real Succ. It will NOT be saved in SuccMatch
  Succ,
  SuccWasSucc,
  SuccStillWasSucc,

  AppealStatus_NA
}AppealStatus;

typedef enum ParseStatus {
  ParseSucc,
  ParseFail,
  ParseEOF
};

// As in Left Recursion scenario, a rule can have multiple matches on a start token.
// Each AppealNode represents an instance in the recursion, and it matches different
// number of tokens. However, truth is the parent nodes matches more than children
// nodes, since parent nodes means more circles traversed.

class AppealNode{
private:
  AppealNode  *mParent;

  // We do allow the tree to merge in the recursion parsing.
  // they are used during manipulation at certain phases like connecting instances
  // of recursion, if a recursion have multiple cycles with one same leading node.
  // The leading node will be connected multiple times. See ConnectPrevious().
  //
  // However, in sort out, we traverse from parent to child, and these 'secondary'
  // parents are never used. So, I decided not to have dedicated data structure to
  // record these 'second' parents.
  //
  // SmallVector<AppealNode*> mSecondParents;

  unsigned     mStartIndex;       // index of start matching token
  unsigned     mChildIndex;       // index as a child in the parent rule table.
  bool         mSorted;           // already sorted out?
  bool         mIsPseudo;         // A pseudo node, mainly used for sub trees connection
                                  // It has no real program meaning, but can be used
                                  // to transfer information among nodes.
  bool         mAstCreated;       // If the AST is created for this node. People may ask if
                                  // we can use mAstTreeNode to determine. The answer is no,
                                  // because some nodes may have no Ast node.

  unsigned     mFinalMatch;       // the final match after sort out.
  SmallVector<unsigned> mMatches; // all of the last matching token.
                                  // mMatches could be empty even if mResult is succ, e.g.
                                  // Zeroorxxx rules.

  TreeNode    *mAstTreeNode;      // The AST tree node of this AppealNode.

public:
  AppealNode* GetParent()       {return mParent;}
  void        SetParent(AppealNode *n) {mParent = n;}
  void        AddParent(AppealNode *n);

  unsigned GetStartIndex()          {return mStartIndex;}
  void     SetStartIndex(unsigned i){mStartIndex = i;}
  unsigned GetChildIndex()          {return mChildIndex;}
  void     SetChildIndex(unsigned i){mChildIndex = i;}

  bool IsPseudo()   {return mIsPseudo;}
  void SetIsPseudo(){mIsPseudo = true;}
  bool IsSorted()   {return mSorted;}
  void SetSorted()  {mSorted = true;}
  bool AstCreated()    {return mAstCreated;}
  void SetAstCreated() {mAstCreated = true;}

  TreeNode* GetAstTreeNode() {return mAstTreeNode;}
  void      SetAstTreeNode(TreeNode *n) {mAstTreeNode = n;}

  unsigned GetFinalMatch()           {return mFinalMatch;}
  void     SetFinalMatch(unsigned m) {mFinalMatch = m; mSorted = true;}

  unsigned GetMatchNum()        {return mMatches.GetNum();}
  unsigned GetMatch(unsigned i) {return mMatches.ValueAtIndex(i);}
  void     ClearMatch()         {mMatches.Clear();}
  void     AddMatch(unsigned i);
  unsigned LongestMatch();        // find the longest match.
  bool     FindMatch(unsigned m); // if 'm' exists?
  void     CopyMatch(AppealNode *another); // copy match info from another node.
                                           // The existing matching of 'this' is kept.

public:
  bool mIsTable;     // A AppealNode could relate to either rule table or token.
  union {
    RuleTable *mTable;
    Token     *mToken;
  }mData;

  bool         m1stAltTokenMatched;  // See the definition of Alt Tokens in token.h.
                                     // A node could match multiple alt tokens.
                                     // The node matching the first alt token is handled
                                     // different during sortout

  Token       *mAltToken;         // The alt token it matches.

  SmallVector<AppealNode*> mChildren;

  // I use an additional vector for the sorted out children. Why do we have two duplicated
  // children vectors? The reason is coming from sortout. After SortOut we need remove some
  // failed children and only keep the successful children. However, a successful child could
  // be SuccWasSucc, and the real successfully matching sub-tree could be hidden in a previously
  // faild tree.
  //
  // During AST tree generation, for the SuccWasSucc child we need find the original matching
  // tree. That means the original mChildren vector needs to be traversed to locate that tree.
  // So we keep mChildren untouched and define a second vector for the SortOut-ed children.
  SmallVector<AppealNode*> mSortedChildren;

  // A Succ mResult doesn't mean 'really' matching tokens. e.g. Zeroorxxx rules could
  // match nothing, but it is succ.
  AppealStatus mResult;

  AppealNode() {mData.mTable=NULL; mParent = NULL;
                mResult = AppealStatus_NA; mIsTable = true;
                mStartIndex = 0; mSorted = false; mFinalMatch = 0;
                m1stAltTokenMatched = false; mAltToken = NULL;
                mIsPseudo = false; mAstTreeNode = NULL; mAstCreated = false;
                mChildIndex = 0;
                // These two don't need big memory. So set block size to 128.
                mChildren.SetBlockSize(128); mSortedChildren.SetBlockSize(128); }
  ~AppealNode() {Release();}
  void Release(){mMatches.Release(); mChildren.Release(); mSortedChildren.Release();}

  void AddChild(AppealNode *n) { mChildren.PushBack(n); }
  void ClearChildren() { mChildren.Clear(); }

  void ReplaceSortedChild(AppealNode *existing, AppealNode *replacement);
  void AddSortedChild(AppealNode *n) { mSortedChildren.PushBack(n); }
  AppealNode* GetSortedChild(unsigned idx);
  AppealNode* FindIndexedChild(unsigned match, unsigned index);

  bool IsSucc() { return (mResult == Succ) ||
                         (mResult == SuccWasSucc) ||
                         (mResult == SuccStillWasSucc); }
  bool IsFail() { return (mResult == FailWasFailed) ||
                         (mResult == FailNotRightToken) ||
                         (mResult == FailNotIdentifier) ||
                         (mResult == FailNotLiteral) ||
                         (mResult == FailChildrenFailed) ||
                         (mResult == FailLookAhead) ||
                         (mResult == Fail2ndOf1st);}
  bool IsNA() {return mResult == AppealStatus_NA;}

  bool IsTable(){ return mIsTable; }
  bool IsToken(){ return !mIsTable; }
  void SetTable(RuleTable *t) { mIsTable = true; mData.mTable = t; }
  void SetToken(Token *t)     { mIsTable = false; mData.mToken = t; }
  RuleTable* GetTable() { return mData.mTable; }
  Token*     GetToken() { return mData.mToken; }
};

class RecursionTraversal;
struct RecStackEntry {
  RecursionTraversal *mRecTra;
  unsigned  mGroupId;
  unsigned  mStartToken;
  // mRecTra and mLeadNode are 1-1 mapping, so the operator==
  // uses only one of them.
  bool operator== (const RecStackEntry &right) {
    return ((mGroupId == right.mGroupId)
            && (mStartToken == right.mStartToken));
  }
};

////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////

class Parser {
private:
  friend class RecursionTraversal;

  // Matching on alternative tokens needs a state machine.
  bool        mInAltTokensMatching;  // once it's true, mCurToken is frozen.
  unsigned    mNextAltTokenIndex;    // index of next alt token to be matched.
  unsigned    mATMToken;             // the current input token being processed.
  ModuleNode *mASTModule;            // the AST Module
  ASTBuilder *mASTBuilder;           // the AST Builder

  AppealNodePool mAppealNodePool;

public:
  Lexer *mLexer;
  const char *filename;
  bool mEndOfFile;

  // debug info
  int  mIndentation;
  bool mTraceTable;         // trace enter/exit rule tables
  bool mTraceLeftRec;       // trace enter/exit rule tables
  bool mTraceAppeal;        // trace appealing
  bool mTraceFailed;        // trace gFailed
  bool mTraceTiming;        // trace gFailed
  bool mTraceVisited;       // trace mVisitedStack
  bool mTraceSortOut;       // trace Sort out.
  bool mTraceAstBuild;      // trace AST build.
  bool mTracePatchWasSucc;  // trace patching was succ node.
  bool mTraceWarning;       // print the warning.

  TreeNode *mNormalModeRoot;// For NormalMode, the root node after BuildAST.

  TreeNode *mLineModeRoot;  // For LineMode, the root node after BuildAST.
  bool mLineMode;           // LineMode is for parsing a single line of source code.
                            // It could be from a string in memory, or read from URL.
                            // It's common in dynamic loading of code in web application.

  void SetLexerTrace() {mLexer->SetTrace();}
  void DumpIndentation();
  void DumpEnterTable(const char *tablename, unsigned indent);
  void DumpExitTable(const char *tablename, unsigned indent, AppealNode*);
  void DumpExitTable(const char *tablename, unsigned indent, AppealStatus, AppealNode *n = NULL);
  void DumpAppeal(RuleTable *table, unsigned token);
  void DumpSuccTokens(AppealNode*);
  void DumpSortOut(AppealNode *root, const char * /*hint*/);
  void DumpSortOutNode(AppealNode*);

private:
  SmallVector<Token*>   mActiveTokens;   // vector for tokens during matching.
  unsigned              mCurToken;       // index in mActiveTokens, the next token to be matched.
  unsigned              mPending;        // index in mActiveTokens, the first pending token.
                                         // All tokens after it are pending.

  void RemoveSuccNode(unsigned curr_token, AppealNode *node);
  void ClearSucc();
  void UpdateSuccInfo(unsigned, AppealNode*);

  bool TraverseStmt();
  bool TraverseExpression();
  bool TraverseRuleTable(RuleTable*, AppealNode*, AppealNode *&);
  bool TraverseRuleTableRegular(RuleTable*, AppealNode*);
  bool TraverseTableData(TableData*, AppealNode*, AppealNode *&);
  bool TraverseConcatenate(RuleTable*, AppealNode*);
  bool TraverseOneof(RuleTable*, AppealNode*);
  bool TraverseZeroormore(RuleTable*, AppealNode*);
  bool TraverseZeroorone(RuleTable*, AppealNode*);

  // There are some special cases we can speed up the traversal.
  // 1. If the target is a token, we just need compare mCurToken with it.
  // 2. If the target is a special rule table, like literal, identifier, we just
  //    need check the type of mCurToken.
  bool TraverseToken(Token*, AppealNode*, AppealNode *&);
  bool TraverseLiteral(RuleTable*, AppealNode*);
  bool TraverseIdentifier(RuleTable*, AppealNode*);
  bool TraverseTemplateLiteral(RuleTable*, AppealNode*);
  bool TraverseRegularExpression(RuleTable*, AppealNode*);
  void TraverseSpecialTableSucc(RuleTable*, AppealNode*);

  bool IsVisited(RuleTable*);
  void SetVisited(RuleTable*);
  void ClearVisited(RuleTable*);
  void VisitedPush(RuleTable*);
  void VisitedPop(RuleTable*);

  void ClearFailed();
  void AddFailed(RuleTable*, unsigned);
  void ResetFailed(RuleTable*, unsigned);
  bool WasFailed(RuleTable*, unsigned);

  bool LookAheadFail(RuleTable*, unsigned);

  bool MoveCurToken();             // move mCurToken one step.
  Token* GetActiveToken(unsigned); // Get an active token.
  void InsertToken(unsigned, Token*);  //

  // Appealing System
  std::vector<AppealNode*> mAppealNodes;
  AppealNode *mRootNode;
  void ClearAppealNodes();

  void Appeal(AppealNode *node, AppealNode *root);

  // Sort Out
  void SortOut();
  void SortOutNode(AppealNode*);
  void SortOutRecursionHead(AppealNode*);
  void SortOutOneof(AppealNode*);
  void SortOutZeroormore(AppealNode*);
  void SortOutZeroorone(AppealNode*);
  void SortOutConcatenate(AppealNode*);
  void SortOutData(AppealNode*);

  void SimplifySortedTree();
  AppealNode* SimplifyShrinkEdges(AppealNode*);

  // Patch Was Succ
  unsigned mRoundsOfPatching;
  void PatchWasSucc(AppealNode*);
  void FindWasSucc(AppealNode *root);
  void FindPatchingNodes();
  void SupplementalSortOut(AppealNode *root, AppealNode *target);

  // Build AST, for each top level construct.
  TreeNode* BuildAST();
  // We need a set of functions to deal with some common manipulations of
  // most languages during AST Building. You can disable it if some functions
  // are not what you want.
  TreeNode* NewTreeNode(AppealNode*);
  TreeNode* Manipulate(AppealNode*);
  TreeNode* Manipulate2Binary(TreeNode*, TreeNode*);
  TreeNode* Manipulate2Cast(TreeNode*, TreeNode*);
  TreeNode* BuildBinaryOperation(TreeNode *, TreeNode *, OprId);
  TreeNode* BuildPassNode();

  // Handle TemplateLiteralNodes
  void ParseTemplateLiterals();

//////////////////////////////////////////////////////////////
// The following section is all about left recursion parsing
/////////////////////////////////////////////////////////////
private:
  RecursionAll               mRecursionAll;
  SmallVector<RecStackEntry> mRecStack;

  void PushRecStack(unsigned, RecursionTraversal*, unsigned);
  RecursionTraversal* FindRecStack(unsigned /*group id*/, unsigned /*token*/);

  LeftRecursion* FindRecursion(RuleTable *);
  bool IsLeadNode(RuleTable *);
  bool TraverseLeadNode(AppealNode*, AppealNode *parent);
  void SetIsDone(unsigned /*group*/, unsigned /*token*/);
  void SetIsDone(RuleTable*, unsigned);

public:
  Parser(const char *f);
  ~Parser();

  void SetVerbose(int i) { mLexer->SetVerbose(i); }
  int  GetVerbose() { mLexer->GetVerbose(); }

  ModuleNode* GetModule() {return mASTModule;}

  void Dump();

  bool Parse();
  ParseStatus  ParseStmt();

  void InitRecursion();
  unsigned LexOneLine();

  bool   TokenMerge(Token *);
  bool   TokenSplit(Token *);
  Token* GetRegExpr(Token *);
};

// Each language will have its own implementation of lexer. Most of lexer
// are shared with some special functions being language specific.
//
// The implementation of this function is in lang/src/lang_spec.cpp.
extern Lexer* CreateLexer();

}
#endif
