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

class Token;

class Lexer {
 public:
  char thechar;
  std::string thename;
  int64_t theintval;
  float thefloatval;
  double thedoubleval;
  TK_Kind _thekind;
  std::vector<std::string> seencomments;
  int verboseLevel;
  StringPool mStringPool;
  TokenPool  mTokenPool;
  unsigned   mPredefinedTokenNum;   // number of predefined tokens.

 private:

  FILE *srcfile;
  char *line;           // line buffer
  size_t linebuf_size;  // the allocated size of line(buffer).
  ssize_t current_line_size;
  uint32_t curidx;
  uint32_t _linenum;
  bool endoffile;
  int ReadALine();  // read a line from def file.

  std::unordered_map<std::string, TK_Kind> keywordmap;
  // get the identifier name after the % or $ prefix
  void GetName(void);

  TK_Kind GetConstVal(void);

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

  void DumpKeywordMap() {
    int i = 0;
    std::cout << "keywordmap.size() " << std::dec << keywordmap.size() << std::endl;
    std::unordered_map<std::string, TK_Kind>::iterator it = keywordmap.begin();
    for (; it != keywordmap.end(); it++) {
      std::cout << std::dec << i << " " << std::hex << &(it->first) << " " << it->first << std::endl;
      i++;
    }
  }

  // These are for autogen table testing
  Token* LexToken_autogen();
  
  bool EndOfLine() { return curidx == current_line_size; }
  bool EndOfFile() { return endoffile; }
  void SavePos();
  void ResetPos();

  void PrepareForFile(const std::string filename);
  void PrepareForString(const std::string &src);
  TK_Kind NextToken(void);
  TK_Kind LexToken();
  TK_Kind ProcessToken();
  // used to process a local string from rule
  TK_Kind ProcessLocalToken(char *str) { line=str; curidx=0; return ProcessToken(); }
  TK_Kind GetMappedToken(std::string str) const { return keywordmap[str]; }
  TK_Kind GetToken() const {
    return _thekind;
  }

  int GetCuridx() const { return curidx; }
  void SetCuridx(int i) { curidx = i; }

  char *GetLine() const { return line; }
  int GetLineNum() const { return _linenum; }
  const std::string &GetTheName() const { return thename; }

  std::string GetTokenString(const TK_Kind tk);  // catched string
  std::string GetTokenString();

  static std::string GetTokenKindString(const TK_Kind tk);    // Token Kind
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
  friend class Parser;
};

inline bool IsVarName(TK_Kind tk) {
  return tk == TK_Name;
}

#endif  // INCLUDE_LEXER_H
