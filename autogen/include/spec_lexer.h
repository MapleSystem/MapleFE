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
/// Copyright [year] <Copyright Owner>

#ifndef INCLUDE_LEXER_H
#define INCLUDE_LEXER_H

#include "spec_tokens.h"
#include "stdio.h"
#include "string.h"
#include <string>
#include <vector>
#include <unordered_map>

namespace maplefe {

class SPECLexer {
 public:
  char thechar;
  std::string thename;
  int64_t theintval;
  float thefloatval;
  double thedoubleval;
  SPECTokenKind _thekind;
  std::vector<std::string> seencomments;
  int verboseLevel;

 private:
  FILE *deffile;
  char *line;           // line buffer
  size_t linebuf_size;  // the allocated size of line(buffer).
  ssize_t current_line_size;
  uint32_t curidx;
  uint32_t _linenum;
  int ReadALine();  // read a line from def file.

  std::unordered_map<std::string, SPECTokenKind> keywordmap;
  // get the identifier name after the % or $ prefix
  void GetName(void);

  int DigitValue(char c) const {
    switch (c) {
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
        return c - '0';
      case 'a':
      case 'A':
        return 10;
      case 'b':
      case 'B':
        return 11;
      case 'c':
      case 'C':
        return 12;
      case 'd':
      case 'D':
        return 13;
      case 'e':
      case 'E':
        return 14;
      case 'f':
      case 'F':
        return 15;
      default:
        return 0;
    }
  }

  SPECTokenKind GetConstVal(void);

 public:
  SPECLexer(const SPECLexer &p) = default;
  SPECLexer &operator=(const SPECLexer &p) = default;
  SPECLexer(void);
  ~SPECLexer(void) {
    if (line) {
      free(line);
      line = nullptr;
    }
  }

  void PrepareForFile(const std::string filename);
  void PrepareForString(const std::string &src);
  SPECTokenKind NextToken(void);
  SPECTokenKind LexToken();
  SPECTokenKind GetToken() const {
    return _thekind;
  }

  char *GetLine() const { return line; }
  int GetLineNum() const { return _linenum; }
  int GetCuridx() const { return curidx; }
  const std::string &GetTheName() const { return thename; }
  void SetVerbose(int v) { verboseLevel = v; }
  int GetVerbose() { return verboseLevel; }

  std::string GetTokenString(SPECTokenKind tk);  // for error reporting purpose
  std::string GetTokenString();

  void SetFile(FILE *file) { deffile = file; }
  FILE *GetFile() const { return deffile; }
};

inline bool IsVarName(SPECTokenKind tk) {
  return tk == SPECTK_Name;
}

}

#endif  // INCLUDE_LEXER_H
