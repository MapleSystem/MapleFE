/// Copyright [year] <Copyright Owner>

#ifndef __LEXER_H
#define __LEXER_H

#include "element.h"
#include "tokenkind.h"
#include "stringpool.h"
#include "tokenpool.h"

#include <stdio.h>
#include <string.h>
#include <string>
#include <vector>
#include <unordered_map>

class Lexer {
 public:
  char thechar;
  std::string thename;
  int64_t theintval;
  float thefloatval;
  double thedoubleval;
  TokenKind _thekind;
  std::vector<std::string> seencomments;
  int verboseLevel;

 private:
  StringPool mStringPool;
  TokenPool  mTokenPool;

  FILE *srcfile;
  char *line;           // line buffer
  size_t linebuf_size;  // the allocated size of line(buffer).
  ssize_t current_line_size;
  uint32_t curidx;
  uint32_t _linenum;
  int ReadALine();  // read a line from def file.

  std::unordered_map<std::string, TokenKind> keywordmap;
  // get the identifier name after the % or $ prefix
  void GetName(void);

  TokenKind GetConstVal(void);

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

  // These are for autogen table testing
  TokenKind LexToken_autogen();
  void SavePos();
  void ResetPos();

  void PrepareForFile(const std::string filename);
  void PrepareForString(const std::string &src);
  TokenKind NextToken(void);
  TokenKind LexToken();
  TokenKind ProcessToken();
  // used to process a local string from rule
  TokenKind ProcessLocalToken(char *str) { line=str; curidx=0; return ProcessToken(); }
  TokenKind GetMappedToken(std::string str) const { return keywordmap[str]; }
  TokenKind GetToken() const {
    return _thekind;
  }

  int GetCuridx() const { return curidx; }
  void SetCuridx(int i) { curidx = i; }

  char *GetLine() const { return line; }
  int GetLineNum() const { return _linenum; }
  const std::string &GetTheName() const { return thename; }

  std::string GetTokenString(const TokenKind tk);  // catched string
  std::string GetTokenString();

  static std::string GetTokenKindString(const TokenKind tk);    // Token Kind
  std::string GetTokenKindString();

  void SetFile(FILE *file) { srcfile = file; }
  FILE *GetFile() const { return srcfile; }

  void SetVerbose(int l) { verboseLevel = l; }
  int GetVerbose() { return verboseLevel; }

/*
 private:
  LiteralToken* ProcessLiteralTokenText(LT_Type type, TokenText text);
*/

  friend class RuleTableWalker;
};

inline bool IsVarName(TokenKind tk) {
  return tk == TK_Name;
}

#endif  // INCLUDE_LEXER_H
