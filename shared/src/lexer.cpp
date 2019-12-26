#include <iostream>
#include <cmath>
#include "massert.h"
#include "lexer.h"
#include "token.h"
#include "common_header_autogen.h"
#include "ruletable_util.h"
#include "gen_debug.h"
#include "massert.h"
#include <climits>
#include <cstdlib>

#include "ruletable_util.h"

#define MAX_LINE_SIZE 4096

static unsigned DigitValue(char c) {
  if (c >= '0' && c <= '9')
    return c - '0';
  if (c >= 'A' && c <= 'F')
    return c - 'A' + 10;
  if (c >= 'a' && c <= 'f')
    return c - 'a' + 10;
  std::cout << "Character not a number" << std::endl;
  exit(1);
}

/* Read (next) line from the input file, and return the readed
   number of chars.
   if the line is empty (nothing but a newline), returns 0.
   if EOF, return -1.
   The trailing new-line character has been removed.
 */
int Lexer::ReadALine() {
  if (!srcfile) {
    line[0] = '\0';
    return -1;
  }

  current_line_size = getline(&line, &linebuf_size, srcfile);
  if (current_line_size <= 0) {  // EOF
    fclose(srcfile);
    line[0] = '\0';
    endoffile = true;
  } else {
    if (line[current_line_size - 1] == '\n') {
      line[current_line_size - 1] = '\0';
      current_line_size--;
    }
  }

  curidx = 0;
  return current_line_size;
}

Lexer::Lexer()
  : thename(""),
    theintval(0),
    thefloatval(0.0f),
    thedoubleval(0),
    verboseLevel(0),
    srcfile(nullptr),
    line(nullptr),
    linebuf_size(0),
    current_line_size(0),
    curidx(0),
    endoffile(false),
    mPredefinedTokenNum(0),
    _linenum(0) {
      seencomments.clear();
      keywordmap.clear();

#define KEYWORD(S, T, I)      \
  {                           \
    std::string str(#S);      \
    keywordmap[str] = TK_##T; \
  }
#include "keywords.def"
#undef KEYWORD
#define OPKEYWORD(S, T, I)    \
  {                           \
    std::string str(S);       \
    keywordmap[str] = TK_##I; \
  }
#include "opkeywords.def"
#undef OPKEYWORD
#define SEPARATOR(S, T)     \
  {                           \
    std::string str(S);       \
    keywordmap[str] = TK_##T; \
  }
#include "supported_separators.def"
#undef SEPARATOR
}

void Lexer::PrepareForFile(const std::string filename) {
  // open file
  srcfile = fopen(filename.c_str(), "r");
  if (!srcfile) {
    MASSERT("cannot open file\n");
  }

  // allocate line buffer.
  linebuf_size = (size_t)MAX_LINE_SIZE;
  line = static_cast<char *>(malloc(linebuf_size));  // initial line buffer.
  if (!line) {
    MASSERT("cannot allocate line buffer\n");
  }

  // try to read the first line
  if (ReadALine() < 0) {
    _linenum = 0;
  } else {
    _linenum = 1;
  }
}

void Lexer::GetName(void) {
  int startidx = curidx;
  if ((isalnum(line[curidx]) || line[curidx] == '_' || line[curidx] == '$' ||
       line[curidx] == '@') &&
      (curidx < (size_t)current_line_size)) {
    curidx++;
  }
  if (line[curidx] == '@' && (line[curidx - 1] == 'h' || line[curidx - 1] == 'f')) {
    curidx++;  // special pattern for exception handling labels: catch or finally
  }

  while ((isalnum(line[curidx]) || line[curidx] == '_') && (curidx < (size_t)current_line_size)) {
    curidx++;
  }
  thename = std::string(&line[startidx], curidx - startidx);
}

std::string Lexer::GetTokenString() {
  TK_Kind tk = mToken->mTkKind;
  return GetTokenString(tk);
}

std::string Lexer::GetTokenString(const TK_Kind tk) {
  std::string temp;
  switch (tk) {
    default: {
      temp = "invalid token";
      break;
    }
#define KEYWORD(N, I, T)    \
    case TK_##I: {          \
      temp = #N;            \
      break;                \
    }
#include "keywords.def"
#undef KEYWORD
#define OPKEYWORD(N, I, T)  \
    case TK_##T: {          \
      temp = N;             \
      break;                \
    }
#include "opkeywords.def"
#undef OPKEYWORD
#define SEPARATOR(N, T)   \
    case TK_##T: {          \
      temp = N;             \
      break;                \
    }
#include "supported_separators.def"
#undef SEPARATOR
    case TK_Intconst: {
      temp = "intconst";
      break;
    }
    case TK_Floatconst: {
      temp = "floatconst";
      break;
    }
    case TK_Doubleconst: {
      temp = "doubleconst";
      break;
    }
    case TK_Name: {
      temp.append(thename);
      break;
    }
    case TK_Newline: {
      temp = "\\n";
      break;
    }
    case TK_Achar: {
      temp = "\'";
      temp += thechar;
      temp.append("\'");
      break;
    }
    case TK_String: {
      temp = "\"";
      temp.append(thename);
      temp.append("\"");
      break;
    }
    case TK_Eof: {
      temp = "EOF";
      break;
    }
  }
  return temp;
}

std::string Lexer::GetTokenKindString() {
  TK_Kind tk = mToken->mTkKind;
  return GetTokenKindString(tk);
}

std::string Lexer::GetTokenKindString(const TK_Kind tkk) {
  std::string temp;
  switch (tkk) {
    default: {
      temp = "Invalid";
      break;
    }
#define KEYWORD(N, I, T)    \
    case TK_##I: {          \
      temp = #I;            \
      break;                \
    }
#include "keywords.def"
#undef KEYWORD
#define OPKEYWORD(N, I, T)  \
    case TK_##T: {          \
      temp = #T;            \
      break;                \
    }
#include "opkeywords.def"
#undef OPKEYWORD
#define SEPARATOR(N, T)   \
    case TK_##T: {          \
      temp = #T;            \
      break;                \
    }
#include "supported_separators.def"
#undef SEPARATOR
#define TOKEN(N, T)         \
    case TK_##T: {          \
      temp = #T;            \
      break;                \
    }
#include "tokens.def"
#undef TOKEN
  }
  return "TK_" + temp;
}

///////////////////////////////////////////////////////////////////////////
//                Utilities for finding predefined tokens
///////////////////////////////////////////////////////////////////////////

Token* FindSeparatorToken(Lexer * lex, SepId id) {
  for (unsigned i = 0; i < lex->mPredefinedTokenNum; i++) {
    Token *token = lex->mTokenPool.mTokens[i];
    if ((token->mTkType == TT_SP) && (((SeparatorToken*)token)->mSepId == id))
      return token;
  }
}

Token* FindOperatorToken(Lexer * lex, OprId id) {
  for (unsigned i = 0; i < lex->mPredefinedTokenNum; i++) {
    Token *token = lex->mTokenPool.mTokens[i];
    if ((token->mTkType == TT_OP) && (((OperatorToken*)token)->mOprId == id))
      return token;
  }
}

// The caller of this function makes sure 'key' is already in the
// string pool of Lexer.
Token* FindKeywordToken(Lexer * lex, char *key) {
  for (unsigned i = 0; i < lex->mPredefinedTokenNum; i++) {
    Token *token = lex->mTokenPool.mTokens[i];
    if ((token->mTkType == TT_KW) && (((KeywordToken*)token)->mName == key))
      return token;
  }
}

// CommentToken is the last predefined token
Token* FindCommentToken(Lexer * lex) {
  Token *token = lex->mTokenPool.mTokens[lex->mPredefinedTokenNum - 1];
  MASSERT((token->mTkType == TT_CM) && "Last predefined token is not a comment token.");
  return token;
}

/////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////

// Read a token until end of file.
// If no remaining tokens in current line, we move to the next line.
Token* Lexer::LexToken_autogen(void) {
  return LexTokenNoNewLine();
}

// Read a token until end of line.
// Return NULL if no token read.
Token* Lexer::LexTokenNoNewLine(void) {
  bool is_comment = GetComment(this);
  if (is_comment) {
    Token *t = FindCommentToken(this);
    t->Dump();
    return t;
  }

  SepId sep = GetSeparator(this);
  if (sep != SEP_NA) {
    Token *t = FindSeparatorToken(this, sep);
    t->Dump();
    return t;
  }

  OprId opr = GetOperator(this);
  if (opr != OPR_NA) {
    Token *t = FindOperatorToken(this, opr);
    t->Dump();
    return t;
  }

  const char *keyword = GetKeyword(this);
  if (keyword != NULL) {
    Token *t = FindKeywordToken(this, keyword);
    t->Dump();
    return t;
  }

  const char *identifier = GetIdentifier(this);
  if (identifier != NULL) {
    IdentifierToken *t = (IdentifierToken*)mTokenPool.NewToken(sizeof(IdentifierToken)); 
    new (t) IdentifierToken(identifier);
    t->Dump();
    return t;
  }

  LitData ld = GetLiteral(this);
  if (ld.mType != LT_NA) {
    LiteralToken *t = (LiteralToken*)mTokenPool.NewToken(sizeof(LiteralToken)); 
    new (t) LiteralToken(TK_Invalid, ld);
    t->Dump();
    return t;
  }

  return NULL;
}
