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
/////////////////////////////////////////////////////////////////////
// This is the base functions of auto generation.                  //
// Each individual part is supposed to inherit from this class.    //
/////////////////////////////////////////////////////////////////////

#ifndef __PARSER_H__
#define __PARSER_H__

#include <iostream>
#include <fstream>
#include <stack>
#include <list>
#include <map>

#include "lexer.h"
#include "ast_module.h"
#include "container.h"
#include "recursion.h"

class Function;
class Stmt;
class Token;
class RuleTable;
class TableData;
class ASTTree;
class TreeNode;

typedef enum {
  FailWasFailed,
  FailNotIdentifier,
  FailNotLiteral,
  FailChildrenFailed,
  Succ,
  SuccWasSucc,
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
  unsigned     mFinalMatch;       // the final match after sort out.
  SmallVector<unsigned> mMatches; // all of the last matching token.

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
  bool mIsSecondTry; // The node is created after second try. This flag is used during SortOut.
  unsigned int mSimplifiedIndex;  // After SimplifyShrinkEdges, a node could be moved to
                                  // connect to a new 'parent' node, replacing its ancestor.
                                  // To make AST building work, it needs to inherit ancestor's
                                  // index in the rule table.
public:
  union {
    RuleTable *mTable;
    Token     *mToken;
  }mData;

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

  AppealStatus mAfter;

  AppealNode() {mData.mTable=NULL; mParent = NULL;
                mAfter = AppealStatus_NA; mSimplifiedIndex = 0; mIsTable = true;
                mIsSecondTry = false; mStartIndex = 0; mSorted = false; mFinalMatch = 0;
                mIsPseudo = false;}
  ~AppealNode(){mMatches.Release();}

  void AddChild(AppealNode *n) { mChildren.push_back(n); }
  void RemoveChild(AppealNode *n);
  void ClearChildren() { mChildren.clear(); }

  void ReplaceSortedChild(AppealNode *existing, AppealNode *replacement);
  void AddSortedChild(AppealNode *n) { mSortedChildren.push_back(n); }
  bool GetSortedChildIndex(AppealNode*, unsigned &);
  AppealNode* GetSortedChildByIndex(unsigned idx);

  bool IsSucc() { return (mAfter == Succ) || (mAfter == SuccWasSucc); }
  bool IsFail() { return (mAfter == FailWasFailed)
                         || (mAfter == FailNotIdentifier)
                         || (mAfter == FailNotLiteral)
                         || (mAfter == FailChildrenFailed);}
  bool IsNA() {return mAfter == AppealStatus_NA;}

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

// To save all the successful results of a rule, we use a container called Guamian, aka
// Hanging Noodle. We save two types of information, one is the succ AppealNodes. The
// other one is the last matching token index. The second info can be also found through
// AppealNode, but we save it for convenience of query.
//
// This is a per-rule data structure.
// 1. The first 'unsigned' is the start token index. It's the key to the knob.
// 2. The second 'unsigned' is used for 'IsDone' right now. It tells if the current
//    recursion group has finished its parsing on this StartIndex. We conduct the parsing
//    on every recursion group in a wavefront manner. Although after each iteration of
//    the wavefront we got succ/fail info, but it's not complete yet. This field tells
//    if we have reached the fixed point or not.
// 3. The third data is the content which users are looking for, either AppealNodes or
//    matching tokens.

class SuccMatch {
private:
  Guamian<unsigned, unsigned, AppealNode*> mNodes;
  Guamian<unsigned, unsigned, unsigned> mMatches;

public:
  SuccMatch(){}
  ~SuccMatch() {mNodes.Release(); mMatches.Release();}

public:
  // The following functions need be used together, as the first one set the start
  // token (aka the key), the second one add a matching AppealNode and also updates
  // matchings tokens in mMatches.
  void AddStartToken(unsigned token);
  void AddSuccNode(AppealNode *node);
  void AddMatch(unsigned);

  ////////////////////////////////////////////////////////////////////////////
  //                     Query functions.
  // All functions in this section should be used together with GetStartToken()
  // or AddStartToken() above. Internal data is defined in GetStartToken(i);
  ////////////////////////////////////////////////////////////////////////////

  bool        GetStartToken(unsigned t); // trying to get succ info for 't'

  unsigned    GetSuccNodesNum();         // number of matching nodes at a token;
  AppealNode* GetSuccNode(unsigned i);   // get succ node at index i;
  bool        FindNode(AppealNode*);     // can we find the node?
  void        RemoveNode(AppealNode*);

  unsigned    GetMatchNum();             // Num of matchings.
  unsigned    GetOneMatch(unsigned i);   // The ith matching.
  bool        FindMatch(unsigned);       // can we find the matching token?

  // Note, the init value of Knob's data is set to 0, meaning IsDone is false.
  void        SetIsDone();
  bool        IsDone();

  ////////////////////////////////////////////////////////////////////////////
  // Below are independent functions. The start token is in argument.
  ////////////////////////////////////////////////////////////////////////////
  bool FindMatch(unsigned starttoken, unsigned target_match);
};

class RecursionTraversal;
struct RecStackEntry {
  RecursionTraversal *mRecTra;
  RuleTable          *mLeadNode;
  unsigned            mStartToken;
  // mRecTra and mLeadNode are 1-1 mapping, so the operator==
  // uses only one of them.
  bool operator== (const RecStackEntry &right) {
    return ((mLeadNode == right.mLeadNode)
            && (mStartToken == right.mStartToken));
  }
};

class Parser {
private:
  friend class RecursionTraversal;
public:
  Lexer *mLexer;
  const char *filename;

  // debug info
  int  mIndentation;
  bool mTraceTable;         // trace enter/exit rule tables
  bool mTraceLeftRec;       // trace enter/exit rule tables
  bool mTraceAppeal;        // trace appealing
  bool mTraceSecondTry;     // trace second try in parser.
  bool mTraceFailed;        // trace mFailed
  bool mTraceVisited;       // trace mVisitedStack
  bool mTraceSortOut;       // trace Sort out.
  bool mTraceAstBuild;      // trace AST build.
  bool mTracePatchWasSucc;  // trace patching was succ node.
  bool mTraceWarning;       // print the warning.

  void SetLexerTrace() {mLexer->SetTrace();}
  void DumpIndentation();
  void DumpEnterTable(const char *tablename, unsigned indent);
  void DumpExitTable(const char *tablename, unsigned indent,
                     bool succ, AppealStatus reason = Succ);
  void DumpAppeal(RuleTable *table, unsigned token);
  void DumpSuccTokens();
  void DumpSortOut(AppealNode *root, const char * /*hint*/);
  void DumpSortOutNode(AppealNode*);

private:
  std::vector<Token*>   mTokens;         // Storage of all tokens, including active, discarded,
                                         // and pending.
  std::vector<Token*>   mActiveTokens;   // vector for tokens during matching.
  std::vector<unsigned> mStartingTokens; // The starting token of each self-complete statement.
                                         // It's an index of mActiveTokens.
  unsigned              mCurToken;       // index in mActiveTokens, the next token to be matched.
  unsigned              mPending;        // index in mActiveTokens, the first pending token.
                                         // All tokens after it are pending.

  bool                  mInSecondTry;    // A temporary flag to tell we are in second try.

  // I'm using two data structures to record the status of cycle reference.
  // See the detailed comments in the implementation of Parser::Parse().
  //   1. mVisited tells if we are in a loop.
  //   2. mVisitedStack tells the token position of each iteration in the loop
  std::map<RuleTable *, bool> mVisited;
  std::map<RuleTable *, std::vector<unsigned>> mVisitedStack;

  // Using this map to record all the failed tokens for a rule.
  // See the detailed comments in Parser::Parse().
  std::map<RuleTable *, std::vector<unsigned>> mFailed;

  // Using this map to record all the succ info of rules.
  std::map<RuleTable*, SuccMatch*> mSucc;
  std::vector<SuccMatch*> mSuccPool;
  SuccMatch* FindSucc(RuleTable*);
  SuccMatch* FindOrCreateSucc(RuleTable*);
  void RemoveSuccNode(unsigned curr_token, AppealNode *node);
  void ClearSucc();
  void UpdateSuccInfo(unsigned, AppealNode*);

  bool TraverseStmt();                                // success if all tokens are matched.
  bool TraverseRuleTable(RuleTable*, AppealNode*);    // success if all tokens are matched.
  bool TraverseRuleTableRegular(RuleTable*, AppealNode*);    // success if all tokens are matched.
  void TraverseRuleTablePre(AppealNode*, AppealNode*); // success if all tokens are matched.
  bool TraverseTableData(TableData*, AppealNode*);    // success if all tokens are matched.
  bool TraverseConcatenate(RuleTable*, AppealNode*, unsigned start = 0);
  bool TraverseOneof(RuleTable*, AppealNode*);
  bool TraverseZeroormore(RuleTable*, AppealNode*);
  bool TraverseZeroorone(RuleTable*, AppealNode*);

  // There are some special cases we can speed up the traversal.
  // 1. If the target is a token, we just need compare mCurToken with it.
  // 2. If the target is a special rule table, like literal, identifier, we just
  //    need check the type of mCurToken.
  bool TraverseToken(Token*, AppealNode*);
  bool TraverseLiteral(RuleTable*, AppealNode*);
  bool TraverseIdentifier(RuleTable*, AppealNode*);
  void TraverseSpecialTableSucc(RuleTable*, AppealNode*);
  void TraverseSpecialTableFail(RuleTable*, AppealNode*, AppealStatus);

  bool IsVisited(RuleTable*);
  void SetVisited(RuleTable*);
  void ClearVisited(RuleTable*);
  void VisitedPush(RuleTable*);
  void VisitedPop(RuleTable*);

  void ClearFailed() {mFailed.clear();}
  void AddFailed(RuleTable*, unsigned);
  void ResetFailed(RuleTable*, unsigned);
  bool WasFailed(RuleTable*, unsigned);

  bool MoveCurToken();             // move mCurToken one step.
  Token* GetActiveToken(unsigned); // Get an active token.

  // Every language has a fixed number of entry point to start parsing. Eg. in Java
  // the top level language construct is class, so the entry point is ClassDeclaration.
  // In C, the top level construct is either function or statement.
  //
  // In driver function of parser needs to set up these top level rule tables which
  // the traversal will start with.
  std::vector<RuleTable*> mTopTables;

  // Appealing System
  std::vector<AppealNode*> mAppealNodes;
  AppealNode *mRootNode;
  void ClearAppealNodes();

  void Appeal(AppealNode *node, AppealNode *root);

  // Sort Out
  void SortOut();
  void SortOutNode(AppealNode*);
  void SortOutOneof(AppealNode*);
  void SortOutZeroormore(AppealNode*);
  void SortOutZeroorone(AppealNode*);
  void SortOutConcatenate(AppealNode*);
  void SortOutData(AppealNode*);
  void CleanFailedSecondTry(AppealNode*);

  void SimplifySortedTree();
  AppealNode* SimplifyShrinkEdges(AppealNode*);

  // Patch Was Succ
  unsigned mRoundsOfPatching;
  void PatchWasSucc(AppealNode*);
  void FindWasSucc(AppealNode *root);
  void FindPatchingNodes();
  void SupplementalSortOut(AppealNode *root, AppealNode *target);

  // Build AST
  ASTTree*  BuildAST(); // Each top level construct gets a AST

//////////////////////////////////////////////////////////////
// The following section is all about left recursion parsing
/////////////////////////////////////////////////////////////
private:
  RecursionAll               mRecursionAll;
  SmallVector<RecStackEntry> mRecStack;
  SmallVector<AppealNode*>   mSeparatedTrees;   // we may created some seperated
                                                // trees during recursion parsing.

  void PushRecStack(RuleTable *rt, RecursionTraversal *rectra, unsigned cur_token);
  RecursionTraversal* FindRecStack(RuleTable*, unsigned);

  LeftRecursion* FindRecursion(RuleTable *);
  bool IsLeadNode(RuleTable *);
  bool TraverseLeadNode(AppealNode*, AppealNode *parent);
  bool TraverseCircle(AppealNode *lead, Recursion *rec, unsigned idx, unsigned &newcurtoken);
  bool TraverseFronNode(AppealNode *parent, FronNode fnode, Recursion *rec = NULL, unsigned cir=0);
  void ApplySuccInfoOnPath(AppealNode *lead, AppealNode *pseudo, bool succ);
  AppealNode* ConstructPath(AppealNode*, AppealNode*, unsigned*, unsigned);

public:
  void AddSeparatedTree(AppealNode *n) {mSeparatedTrees.PushBack(n);}

public:
  Parser(const char *f);
  ~Parser();

  void SetVerbose(int i) { mLexer->SetVerbose(i); }
  int  GetVerbose() { mLexer->GetVerbose(); }

  void Dump();

  bool Parse();
  bool ParseStmt();
  void InitPredefinedTokens();
  void SetupTopTables();  //Each language parser will implement this by itself. 
  void InitRecursion();
  unsigned LexOneLine();
};

#define MAX_SUCC_TOKENS 16
extern unsigned gSuccTokensNum;
extern unsigned gSuccTokens[MAX_SUCC_TOKENS];

#endif
