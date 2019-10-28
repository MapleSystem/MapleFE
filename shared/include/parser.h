/////////////////////////////////////////////////////////////////////
// This is the base functions of auto generation.                  //
// Each individual part is supposed to inherit from this class.    //
/////////////////////////////////////////////////////////////////////

#ifndef __PARSER_H__
#define __PARSER_H__

#include <iostream>
#include <fstream>
#include <stack>

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

class Parser {
public:
  Lexer *mLexer;
  const char *filename;
  Automata *mAutomata;
  Module *mModule;
  Function *currfunc;

  std::vector<std::string> mVars;

private:
  std::vector<Token*>   mTokens;   // vector for tokens during matching.
  unsigned              mCurToken; // index in mTokens, 1st to-be-matched token.
                                   // tokens before it have been matched.

  bool TraverseRuleTable(RuleTable*); // success if all tokens are matched.
  bool TraverseTableData(TableData*); // success if all tokens are matched.
  bool TraverseStmt();                // success if all tokens are matched.

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
};

#endif
