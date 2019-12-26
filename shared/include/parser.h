/////////////////////////////////////////////////////////////////////
// This is the base functions of auto generation.                  //
// Each individual part is supposed to inherit from this class.    //
/////////////////////////////////////////////////////////////////////

#ifndef __PARSER_H__
#define __PARSER_H__

#include <iostream>
#include <fstream>
#include <stack>
#include <map>

#include "feopcode.h"
#include "lexer.h"

// tyidx for int for the time being
#define inttyidx 1

class Automata;
class Module;
class Function;
class Stmt;
class Token;
class RuleTable;
class TableData;

typedef enum {
  FailWasFailed,
  FailLooped,
  FailNotIdentifier,
  FailNotLiteral,
  FailChildrenFailed,
  Succ,
  SuccWasSucc,
  NA
}AppealStatus;

class AppealNode;
class AppealNode{
public:
  RuleTable *mTable;
  unsigned   mToken;
  std::vector<AppealNode*> mChildren;
  AppealNode *mParent;
  AppealStatus mBefore;
  AppealStatus mAfter;

  AppealNode() {mTable=NULL; mParent = NULL; mBefore = NA; mAfter = NA;}
  ~AppealNode(){}
};

// Design of the cached success info.
// Starting from a certian token, a rule can have multiple matchings, for example,
// an expression,  this.x, can be matched by Primary either by one token 'this', or
// three tokens 'this.x'.
//
// Assuming index of 'this' is 9, we say Primary has two matches indicated by two numbers
// 9 and 11, which represent the last successfully matched token.
//
// To cache all the successful results of a rule, we define a vector. The vector is composed
// of a set of segments with variable length. One segment represents the successful matches
// starting from a specific token. The structure is as below.
//      9, 2, 9, 11
// the first number 9 is the starting token. The second number, 2, is the number of matches.
// The third number and the forth number are the last successfully matched tokens.
// If the rule has 3 matches for token 20, its segment is 20, 3, 20, 22, 23
// Put them together, we get the mSucc for this rule 9, 2, 9, 11, 20, 3, 20, 22, 23

// This is a per-rule data structure, saving the success info of a rule.
class SuccMatch {
public:
  std::vector<unsigned> mCache;
private:
  unsigned mTempIndex;          // A temporary index
public:
  SuccMatch(){}
  ~SuccMatch() {mCache.clear();}
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

  // query functions.
  // The three function need to be used together.
  bool     GetStartToken(unsigned t);    // trying to get succ info for 't'
  unsigned GetMatchNum();                // number of matches at a token;
  unsigned GetOneMatch(unsigned i);      // Get the i-th matching token. Starts from 0.
};

class Parser {
public:
  Lexer *mLexer;
  const char *filename;
  Automata *mAutomata;
  Module *mModule;
  Function *currfunc;

  std::vector<std::string> mVars;

  // debug info
  unsigned mIndentation;    //
  bool mTraceTable;         // trace enter/exit rule tables
  bool mTraceAppeal;        // trace appealing
  bool mTraceSecondTry;     // trace second try in parser.
  bool mTraceFailed;        // trace mFailed
  bool mTraceVisited;       // trace mVisitedStack
  const char* GetRuleTableName(const RuleTable*);
  void DumpEnterTable(const char *tablename, unsigned indent);
  void DumpExitTable(const char *tablename, unsigned indent,
                     bool succ, AppealStatus reason = Succ);
  void DumpAppeal(RuleTable *table, unsigned token);
  void DumpSuccTokens();

private:
  std::vector<Token*>   mTokens;         // Storage of all tokens, including active, discarded,
                                         // and pending.
  std::vector<Token*>   mActiveTokens;   // vector for tokens during matching.
  std::vector<unsigned> mStartingTokens; // The starting token of each self-complete statement.
                                         // It's an index of mActiveTokens.
  unsigned              mCurToken;       // index in mActiveTokens, the next token to be matched.
  unsigned              mPending;        // index in mActiveTokens, the first pending token.
                                         // All tokens after it are pending.

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
  void ClearAppealNodes();
  void Appeal(AppealNode*);
  void AppealTraverse(AppealNode *node, AppealNode *root);

public:
  Parser(const char *f, Module *m);
  Parser(const char *f);
  ~Parser() { delete mLexer; }

  TK_Kind GetTokenKind(const char c);
  TK_Kind GetTokenKind(const char *str);

  std::string GetTokenKindString(const TK_Kind tk) { return mLexer->GetTokenKindString(tk); }

  FEOpcode GetFEOpcode(const char c);
  FEOpcode GetFEOpcode(const char *str);

  void SetVerbose(int i) { mLexer->SetVerbose(i); }
  int GetVerbose() { mLexer->GetVerbose(); }

  void Dump();

  bool Parse();
  bool ParseStmt();
  void InitPredefinedTokens();
  void SetupTopTables();  //Each language parser will implement this by itself. 
  unsigned LexOneLine();
};

#endif
