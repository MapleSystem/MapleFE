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
  bool mTraceFailed;        // trace mFailed
  bool mTraceVisited;       // trace mVisitedStack
  const char* GetRuleTableName(const RuleTable*);
  void DumpEnterTable(const char *tablename, unsigned indent);
  void DumpExitTable(const char *tablename, unsigned indent,
                     bool succ, AppealStatus reason = Succ);
  void DumpAppeal(RuleTable *table, unsigned token);

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

  bool TraverseRuleTable(RuleTable*, AppealNode*);  // success if all tokens are matched.
  bool TraverseTableData(TableData*, AppealNode*);  // success if all tokens are matched.
  bool TraverseConcatenate(RuleTable*, AppealNode*);

  bool TraverseStmt();                // success if all tokens are matched.
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

  // for all ParseXXX routines
  // Return true  : succeed
  //        false : failed
  bool Parse();
  bool ParseFunction(Function *func);
  bool ParseFuncArgs(Function *func);
  bool ParseFuncBody(Function *func);
  bool ParseStmt(Function *func);

  TK_Kind GetTokenKind(const char c);
  TK_Kind GetTokenKind(const char *str);

  std::string GetTokenKindString(const TK_Kind tk) { return mLexer->GetTokenKindString(tk); }

  FEOpcode GetFEOpcode(const char c);
  FEOpcode GetFEOpcode(const char *str);

  void SetVerbose(int i) { mLexer->SetVerbose(i); }
  int GetVerbose() { mLexer->GetVerbose(); }

  void Dump();

  //////////////////////////////////////////////////////////
  //        Framework based on Autogen+Token
  //////////////////////////////////////////////////////////

  bool Parse_autogen();
  bool ParseStmt_autogen();
  void InitPredefinedTokens();
  void SetupTopTables();  //Each language parser will implement this by itself. 
  unsigned LexOneLine();
};

#endif
