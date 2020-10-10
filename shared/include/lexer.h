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
/// Copyright [year] <Copyright Owner>

#ifndef __LEXER_H
#define __LEXER_H

#include "element.h"
#include "stringpool.h"
#include "token.h"
#include "tokenpool.h"
#include "ruletable.h"

#include <stdio.h>
#include <string.h>
#include <string>
#include <vector>
#include <unordered_map>

class Token;

class Lexer {
public:
  Token *mToken;
  char thechar;
  std::string thename;
  int64_t theintval;
  float thefloatval;
  double thedoubleval;
  std::vector<std::string> seencomments;
  int verboseLevel;
  TokenPool  mTokenPool;
  unsigned   mPredefinedTokenNum;   // number of predefined tokens.
  bool       mTrace;

public:
  FILE *srcfile;
  char *line;           // line buffer
  size_t linebuf_size;  // the allocated size of line(buffer).
  ssize_t current_line_size;
  uint32_t curidx;
  uint32_t _linenum;
  bool endoffile;
  int ReadALine();  // read a line from def file.

  // get the identifier name after the % or $ prefix
  void GetName(void);

  Token *GetConstVal(void);

public:
  Lexer(const Lexer &p) = default;
  Lexer &operator=(const Lexer &p) = default;
  Lexer(void);
  ~Lexer(void) {
    if (line) {
      free(line);
      line = nullptr;
    }
  }

  void SetTrace() {mTrace = true;}

  bool EndOfLine() { return curidx == current_line_size; }
  bool EndOfFile() { return endoffile; }
  void SavePos();
  void ResetPos();

  void PrepareForFile(const std::string filename);
  void PrepareForString(const std::string &src);

  int GetCuridx() const { return curidx; }
  void SetCuridx(int i) { curidx = i; }

  char *GetLine() const { return line; }
  int GetLineNum() const { return _linenum; }
  const std::string &GetTheName() const { return thename; }

  void SetFile(FILE *file) { srcfile = file; }
  FILE *GetFile() const { return srcfile; }

  void SetVerbose(int l) { verboseLevel = l; }
  int GetVerbose() { return verboseLevel; }

  friend class Parser;

  // These are for autogen table testing
  Token* LexToken();  // always return token until end of file.
  Token* LexTokenNoNewLine(); // try to get token untile end of line.

  ///////////////////////////////////////////////////////////////////////////////////
  // NOTE: (1) All interfaces will not go the new line.
  //       (2) All interfaces will move the 'curidx' of Lexer right after the target.
  //           They won't move 'curidx' if target is not hit.
  ///////////////////////////////////////////////////////////////////////////////////

  SepId       GetSeparator();
  OprId       GetOperator();
  LitData     GetLiteral();
  const char* GetKeyword();
  const char* GetIdentifier();
  bool        GetComment();

  // replace keyword/opr/sep... with tokens
  //void PlantTokens();
  //void PlantTraverseRuleTable(RuleTable*);
  //void PlantTraverseTableData(TableData*);

  //
  Token* FindSeparatorToken(SepId id);
  Token* FindOperatorToken(OprId id);
  Token* FindKeywordToken(char *key);
  Token* FindCommentToken();

  // When we start walk a rule table to find a token, do we need check if
  // the following data is a separator?
  bool mCheckSeparator;

  // Need take care of token during traversing a rule table?
  bool mMatchToken;

  // When lexing a literal, it's easier if we know the type of literal before
  // we do ProcessLiteral()
  LitId mLastLiteralId;

  // It returns true : if RuleTable is met
  //           false : if failed
  // The found token's string is saved at mText, length at mLen. The string is already
  // put in the string pool.
  bool        Traverse(const RuleTable*);
  bool        TraverseTableData(TableData*);
  bool        TraverseSecondTry(const RuleTable*);  // See comments in the implementation.

  SepId       TraverseSepTable();        // Walk the separator table
  OprId       TraverseOprTable();        // Walk the operator table
  const char* TraverseKeywordTable();    //
  const char* TraverseIdentifierTable(); //
  bool        MatchToken(Token*);
};

// This is language specific function. Please implement this in LANG/src,
// such as java/src/lang_spec.cpp

extern LitData ProcessLiteral(LitId type, const char *str);

#endif  // INCLUDE_LEXER_H
