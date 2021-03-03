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
#include "container.h"
#include "recursion.h"
#include "succ_match.h"
#include "gen_summary.h"

namespace maplefe {

class Function;
class Stmt;
class Token;
class RuleTable;
class TableData;
class ASTTree;
class TreeNode;

typedef enum {
  FailWasFailed,
  FailNotRightToken,
  FailNotIdentifier,
  FailNotLiteral,
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

// As in Left Recursion scenario, a rule can have multiple matches on a start token.
// Each AppealNode represents an instance in the recursion, and it matches different
// number of tokens. However, truth is the parent nodes matches more than children
// nodes, since parent nodes means more circles traversed.

class AppealNode{
private:
  // In theory a tree shouldn't merge. But we do allow merge in the recursion
  // parsing. mParent is the first level parent. mSecondParents are second level, and
  // they are used during manipulation at certain phases like connecting instances
  // of recursion. However, after SortOut, only mParent is valid.
  AppealNode  *mParent;
  SmallVector<AppealNode*> mSecondParents;

  unsigned     mStartIndex;       // index of start matching token
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
  AppealNode* GetSecondParentsNum() {return mSecondParents.GetNum();}
  AppealNode* GetSecondParent(unsigned i) {return mSecondParents.ValueAtIndex(i);}
  void        ClearSecondParents() {mSecondParents.Clear();}
  AppealNode* GetParent()       {return mParent;}
  void        SetParent(AppealNode *n) {mParent = n;}
  void        AddParent(AppealNode *n);

  unsigned GetStartIndex()      {return mStartIndex;}
  void SetStartIndex(unsigned i){mStartIndex = i;}

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
  void     AddMatch(unsigned i);
  unsigned LongestMatch();        // find the longest match.
  bool     FindMatch(unsigned m); // if 'm' exists?
  void     CopyMatch(AppealNode *another); // copy match info from another node.
                                           // The existing matching of 'this' is kept.

public:
  bool mIsTable;     // A AppealNode could relate to either rule table or token.
  unsigned int mSimplifiedIndex;  // After SimplifyShrinkEdges, a node could be moved to
                                  // connect to a new 'parent' node, replacing its ancestor.
                                  // To make AST building work, it needs to inherit ancestor's
                                  // index in the rule table.

public:
  union {
    RuleTable *mTable;
    Token     *mToken;
  }mData;

  bool         m1stAltTokenMatched;  // See the definition of Alt Tokens in token.h.
                                     // A node could match multiple alt tokens.
                                     // The node matching the first alt token is handled
                                     // different during sortout

  Token       *mAltToken;         // The alt token it matches.

  std::vector<AppealNode*> mChildren;

  // I use an additional vector for the sorted out children. Why do we have two duplicated
  // children vectors? The reason is coming from sortout. After SortOut we need remove some
  // failed children and only keep the successful children. However, a successful child could
  // be SuccWasSucc, and the real successfully matching sub-tree could be hidden in a previously
  // faild tree.
  //
  // During AST tree generation, for the SuccWasSucc child we need find the original matching
  // tree. That means the original mChildren vector needs to be traversed to locate that tree.
  // So we keep mChildren untouched and define a second vector for the SortOut-ed children.
  std::vector<AppealNode*> mSortedChildren;

  // A Succ mResult doesn't mean 'really' matching tokens. e.g. Zeroorxxx rules could
  // match nothing, but it is succ.
  AppealStatus mResult;

  AppealNode() {mData.mTable=NULL; mParent = NULL;
                mResult = AppealStatus_NA; mSimplifiedIndex = 0; mIsTable = true;
                mStartIndex = 0; mSorted = false; mFinalMatch = 0;
                m1stAltTokenMatched = false; mAltToken = NULL;
                mIsPseudo = false; mAstTreeNode = NULL; mAstCreated = false;}
  ~AppealNode(){mMatches.Release();}

  void AddChild(AppealNode *n) { mChildren.push_back(n); }
  void RemoveChild(AppealNode *n);
  void ClearChildren() { mChildren.clear(); }

  void ReplaceSortedChild(AppealNode *existing, AppealNode *replacement);
  void AddSortedChild(AppealNode *n) { mSortedChildren.push_back(n); }
  bool GetSortedChildIndex(AppealNode*, unsigned &);
  AppealNode* GetSortedChildByIndex(unsigned idx);
  AppealNode* FindSpecChild(TableData *tdata, unsigned match);

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

  bool SuccEqualTo(AppealNode*);

  // If 'this' is a descendant of 'p'.
  bool DescendantOf(AppealNode *p);
};

class RecursionTraversal;
struct RecStackEntry {
  RecursionTraversal *mRecTra;
  unsigned *mGroupId;
  unsigned  mStartToken;
  // mRecTra and mLeadNode are 1-1 mapping, so the operator==
  // uses only one of them.
  bool operator== (const RecStackEntry &right) {
    return ((mGroupId == right.mGroupId)
            && (mStartToken == right.mStartToken));
  }
};

class Parser {
private:
  friend class RecursionTraversal;

  // Matching on alternative tokens needs a state machine.
  bool     mInAltTokensMatching;  // once it's true, mCurToken is frozen.
  unsigned mNextAltTokenIndex;    // index of next alt token to be matched.
  unsigned mATMToken;             // the current input token being processed.

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

  void SetLexerTrace() {mLexer->SetTrace();}
  void DumpIndentation();
  void DumpEnterTable(const char *tablename, unsigned indent);
  void DumpExitTable(const char *tablename, unsigned indent, AppealNode*);
  void DumpAppeal(RuleTable *table, unsigned token);
  void DumpSuccTokens(AppealNode*);
  void DumpSortOut(AppealNode *root, const char * /*hint*/);
  void DumpSortOutNode(AppealNode*);

private:
  std::vector<Token*>   mActiveTokens;   // vector for tokens during matching.
  unsigned              mCurToken;       // index in mActiveTokens, the next token to be matched.
  unsigned              mPending;        // index in mActiveTokens, the first pending token.
                                         // All tokens after it are pending.

  void RemoveSuccNode(unsigned curr_token, AppealNode *node);
  void ClearSucc();
  void UpdateSuccInfo(unsigned, AppealNode*);

  bool TraverseStmt();
  bool TraverseRuleTable(RuleTable*, AppealNode*, AppealNode *&);
  bool TraverseRuleTableRegular(RuleTable*, AppealNode*);
  bool TraverseRuleTablePre(AppealNode*);
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
  ASTTree*  BuildAST();


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

  void Dump();

  bool Parse();
  bool ParseStmt();
  void InitRecursion();
  unsigned LexOneLine();
};

}
#endif
