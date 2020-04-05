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

class Automata;
class Function;
class Stmt;
class Token;
class RuleTable;
class TableData;
class ASTTree;
class TreeNode;

typedef enum {
  FailWasFailed,
  FailLooped,
  FailNotIdentifier,
  FailNotLiteral,
  FailChildrenFailed,
  Succ,
  SuccWasSucc,
  AppealStatus_NA
}AppealStatus;

class AppealNode;
class AppealNode{
private:
  unsigned  mStartIndex;     // index of start matching token
  unsigned  mNumTokens;      // num of matched token, set when sorted.
  bool      mSorted;         //
public:
  unsigned GetStartIndex()        {return mStartIndex;}
  unsigned GetNumTokens()         {return mNumTokens;}
  bool     IsSorted()             {return mSorted;}
  void SetStartIndex(unsigned i)  {mStartIndex = i;}
  void SetNumTokens(unsigned i)   {mNumTokens = i; mSorted = true;}
  void SetSorted()                {mSorted = true;}

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

  AppealNode  *mParent;
  AppealStatus mBefore;
  AppealStatus mAfter;

  AppealNode() {mData.mTable=NULL; mParent = NULL; mBefore = AppealStatus_NA;
                mAfter = AppealStatus_NA; mSimplifiedIndex = 0; mIsTable = true;
                mIsSecondTry = false; mStartIndex = 0; mNumTokens = 0; mSorted = false;}
  ~AppealNode(){}

  void AddChild(AppealNode *n) { mChildren.push_back(n); }
  void RemoveChild(AppealNode *n);
  void ClearChildren() { mChildren.clear(); }

  void ReplaceSortedChild(AppealNode *existing, AppealNode *replacement);
  void AddSortedChild(AppealNode *n) { mSortedChildren.push_back(n); }
  bool GetSortedChildIndex(AppealNode*, unsigned &);
  AppealNode* GetSortedChildByIndex(unsigned idx);

  bool IsSucc() { return (mAfter == Succ) || (mAfter == SuccWasSucc); }
  bool IsFail() { return !IsSucc(); }

  bool IsTable(){ return mIsTable; }
  bool IsToken(){ return !mIsTable; }
  void SetTable(RuleTable *t) { mIsTable = true; mData.mTable = t; }
  void SetToken(Token *t)     { mIsTable = false; mData.mToken = t; }
  RuleTable* GetTable() { return mData.mTable; }
  Token*     GetToken() { return mData.mToken; }

  bool SuccEqualTo(AppealNode*);

  // If 'p' is a parent of 'this'
  bool IsParent(AppealNode *p);
};

// Design of the cached success info.
// Starting from a certian token, a rule can have multiple matchings, for example,
// an expression,  this.x, can be matched by Primary either by one token 'this', or
// three tokens 'this.x'.
//
// Assuming index of 'this' is 9, we say Primary has two matches indicated by two numbers
// 9 and 11, which represent the last successfully matched token.
//
// To cache all the successful results of a rule, we use a container called Guamian, aka
// Hanging Noodle. Each knob of Guamian has index of start token. Each string of noodle
// contains the successful matches.
//
// This is a per-rule data structure, saving the success info of a rule.

class SuccMatch {
private:
  Guamian<unsigned, unsigned> mCache;
public:
  SuccMatch(){}
  ~SuccMatch() {mCache.Release();}
public:
  // Usually a rule may have a few matches, less than 3. So we define 3 special interfaces.
  void AddOneMatch(unsigned t, unsigned m);                               // rule has only 1 match
  void AddTwoMatch(unsigned t, unsigned m1, unsigned m2);                 // rule has only 2 match
  void AddThreeMatch(unsigned t, unsigned m1, unsigned m2, unsigned m3);  // rule has only 3 match
  // For general cases we will append matches one by one. The following two
  // functions will be used together, as the first one set the start token, the second one
  // add the end matching tokens one by one.
  void AddStartToken(unsigned token);
  void AddOneMoreMatch(unsigned last_succ_token);  // add one more match to the cach

  // Reduce tokens, used during SortOut

  // Query functions.
  // The four function need to be used together since internal data is defiined in
  // GetStartToken(unsigned).
  bool     GetStartToken(unsigned t);    // trying to get succ info for 't'
  bool     IsReduced();                  // It's already reduced.
  bool     FindMatch(unsigned i);        // If a match exist?
  unsigned GetMatchNum();                // number of matches at a token;
  unsigned GetOneMatch(unsigned i);      // Get the i-th matching token. Starts from 0.
  void     ReduceMatches(unsigned idx);  // Reduce all matches except idx-th.

  // Below are independent functions. The start token is in argument.
  bool ReduceMatches(unsigned starttoken, unsigned except);
  bool FindMatch(unsigned starttoken, unsigned target);
};

class Parser {
public:
  Lexer *mLexer;
  const char *filename;
  ASTModule mModule;     // A source file has a module

  // debug info
  unsigned mIndentation;    //
  bool mTraceTable;         // trace enter/exit rule tables
  bool mTraceAppeal;        // trace appealing
  bool mTraceSecondTry;     // trace second try in parser.
  bool mTraceFailed;        // trace mFailed
  bool mTraceVisited;       // trace mVisitedStack
  bool mTraceSortOut;       // trace Sort out.
  bool mTraceAstBuild;      // trace AST build.
  bool mTracePatchWasSucc;  // trace patching was succ node.
  bool mTraceWarning;       // print the warning.

  void SetLexerTrace() {mLexer->SetTrace();}
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
  void ClearSucc();

  bool TraverseStmt();                              // success if all tokens are matched.
  bool TraverseRuleTable(RuleTable*, AppealNode*);  // success if all tokens are matched.
  bool TraverseTableData(TableData*, AppealNode*);  // success if all tokens are matched.
  bool TraverseConcatenate(RuleTable*, AppealNode*);
  bool TraverseOneof(RuleTable*, AppealNode*);
  bool TraverseZeroormore(RuleTable*, AppealNode*);
  bool TraverseZeroorone(RuleTable*, AppealNode*);

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

  bool MoveCurToken();  // move mCurToken one step.

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
  void Appeal(AppealNode*);
  void AppealTraverse(AppealNode *node, AppealNode *root);

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
  SmallVector<AppealNode*> mOrigPatchedNodes;
  unsigned mRoundsOfPatching;
  void PatchWasSucc(AppealNode*);
  void FindWasSucc(AppealNode *root);
  void FindPatchingNodes(AppealNode *root);
  void FindGoodMatching(AppealNode *node);
  void CleanPatchingNodes();
  void SupplementalSortOut(AppealNode *root, AppealNode *target);

  // Build AST
  ASTTree*  BuildAST(); // Each top level construct gets a AST

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
  unsigned LexOneLine();
};

#endif
