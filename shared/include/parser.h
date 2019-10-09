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

class Parser {
public:
  Lexer mLexer;
  const char *filename;
  Automata *mAutomata;
  Module *mModule;
  Function *currfunc;

  std::vector<std::string> mVars;

public:
  Parser(const char *f, Module *m);
  Parser(const char *f);
  ~Parser() {};

  // for all ParseXXX routines
  // Return true  : succeed
  //        false : failed
  bool Parse();
  bool Parse_autogen();
  bool ParseFunction(Function *func);
  bool ParseFuncArgs(Function *func);
  bool ParseFuncBody(Function *func);
  bool ParseStmt(Function *func);

  TokenKind GetTokenKind(const char c);
  TokenKind GetTokenKind(const char *str);

  std::string GetTokenKindString(const TokenKind tk) { return mLexer.GetTokenKindString(tk); }

  FEOpcode GetFEOpcode(const char c);
  FEOpcode GetFEOpcode(const char *str);

  void SetVerbose(int i) { mLexer.SetVerbose(i); }
  int GetVerbose() { mLexer.GetVerbose(); }

  void Dump();
};

#endif
