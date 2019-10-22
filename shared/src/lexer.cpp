#include <iostream>
#include <cmath>
#include "massert.h"
#include "lexer.h"
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
    _thekind(TK_Invalid),
    verboseLevel(0),
    srcfile(nullptr),
    line(nullptr),
    linebuf_size(0),
    current_line_size(0),
    curidx(0),
    endoffile(false),
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

  _thekind = TK_Invalid;
}

void Lexer::PrepareForString(const std::string &src) {
  // allocate line buffer.
  if (!line) {
    linebuf_size = (size_t)MAX_LINE_SIZE;
    line = static_cast<char *>(malloc(linebuf_size));  // initial line buffer.
    if (!line) {
      MASSERT("cannot allocate line buffer\n");
    }
  }

  MASSERT(src.length() < linebuf_size && "needs larger buffer");
  strcpy(line, src.c_str());
  current_line_size = strlen(line);
  if (line[current_line_size - 1] == '\n') {
    line[current_line_size - 1] = '\0';
    current_line_size--;
  }
  curidx = 0;

  NextToken();
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

TK_Kind Lexer::LexToken(void) {
  // skip spaces
  while ((line[curidx] == ' ' || line[curidx] == '\t') && curidx < (size_t)current_line_size) {
    curidx++;
  }
  // check end of line
  while (curidx == (size_t)current_line_size || (line[curidx] == '/' && line[curidx+1] == '/')) {
    if (line[curidx] == '/' && line[curidx+1] == '/') {  // process comment contents
      seencomments.push_back(std::string(&line[curidx + 2], current_line_size - curidx - 1));
    }

    if (ReadALine() < 0) {
      return TK_Eof;
    }
    _linenum++;  // a new line read.
    // print current line
    if (GetVerbose() >= 3) {
      MMSG2("  >>>> LINE: ", _linenum, GetLine());
    }

    // skip spaces
    while ((line[curidx] == ' ' || line[curidx] == '\t') && curidx < (size_t)current_line_size) {
      curidx++;
    }
  }
  // handle comments in /* */
  if (line[curidx] == '/' && line[curidx+1] == '*') {
    curidx += 2; // skip : /*
    while (curidx < (size_t)current_line_size && !(line[curidx] == '*' && line[curidx+1] == '/')) {
      curidx++;
      if (curidx == (size_t)current_line_size) {
        if (ReadALine() < 0) {
          return TK_Eof;
        }
        _linenum++;  // a new line read.
      }
    }
    curidx += 2; // skip : */
    // skip spaces
    while ((line[curidx] == ' ' || line[curidx] == '\t') && curidx < (size_t)current_line_size) {
      curidx++;
    }
  }

  // process the token
  return ProcessToken();
}

TK_Kind Lexer::ProcessToken() {
  char curchar = line[curidx++];
  switch (curchar) {
    case '\n':
      return TK_Newline;
    case '(':
      return TK_Lparen;
    case ')':
      return TK_Rparen;
    case '{':
      return TK_Lbrace;
    case '}':
      return TK_Rbrace;
    case '[':
      return TK_Lbrack;
    case ']':
      return TK_Rbrack;
    case ',':
      return TK_Comma;
    case ';':
      return TK_Semicolon;
    case '@':
      return TK_At;
    case '.':
      if (line[curidx] == '.' && line[curidx + 1] == '.') {
        curidx += 2;
        return TK_Dotdotdot;
      } else {
        return TK_Dot;
      }
    case '+':
      if (line[curidx] == '+') {
        curidx++;
        return TK_Inc;
      } else if (line[curidx] == '=') {
        curidx++;
        return TK_Addassign;
      } else {
        return TK_Add;
      }
    case '-':
      if (line[curidx] == '-') {
        curidx++;
        return TK_Dec;
      } else if (line[curidx] == '=') {
        curidx++;
        return TK_Subassign;
      } else if (line[curidx] == '>') {
        curidx++;
        return TK_Arrow;
      } else {
        return TK_Sub;
      }
    case '*':
      if (line[curidx] == '=') {
        curidx++;
        return TK_Mulassign;
      } else {
        return TK_Mul;
      }
    case '/':
      if (line[curidx] == '=') {
        curidx++;
        return TK_Divassign;
      } else {
        return TK_Div;
      }
    case '%':
      if (line[curidx] == '=') {
        curidx++;
        return TK_Modassign;
      } else {
        return TK_Mod;
      }
    case '&':
      if (line[curidx] == '&') {
        curidx++;
        return TK_Land;
      } else if (line[curidx] == '=') {
        curidx++;
        return TK_Bandassign;
      } else {
        return TK_Band;
      }
    case '|':
      if (line[curidx] == '|') {
        curidx++;
        return TK_Lor;
      } else if (line[curidx] == '=') {
        curidx++;
        return TK_Borassign;
      } else {
        return TK_Bor;
      }
    case '!':
      if (line[curidx] == '=') {
        curidx++;
        return TK_Ne;
      } else {
        return TK_Not;
      }
    case '^':
      if (line[curidx] == '=') {
        curidx++;
        return TK_Bxorassign;
      } else {
        return TK_Bxor;
      }
    case '~':
      return TK_Bcomp;
    case '>':
      if (line[curidx] == '>' && line[curidx + 1] == '>' && line[curidx + 2] == '=') {
        curidx += 3;
        return TK_Zextassign;
      } else if (line[curidx] == '>' && line[curidx + 1] == '>') {
        curidx += 2;
        return TK_Zext;
      } else if (line[curidx] == '>' && line[curidx + 1] == '=') {
        curidx += 2;
        return TK_Shrassign;
      } else if (line[curidx] == '>') {
        curidx++;
        return TK_Shr;
      } else if (line[curidx] == '=') {
        curidx++;
        return TK_Ge;
      } else {
        return TK_Gt;
      }
    case '<':
      if (line[curidx] == '<' && line[curidx + 1] == '=') {
        curidx += 2;
        return TK_Shlassign;
      } else if (line[curidx] == '<') {
        curidx++;
        return TK_Shl;
      } else if (line[curidx] == '=') {
        curidx++;
        return TK_Le;
      } else if (line[curidx] == '>') {
        curidx++;
        return TK_Diamond;
      } else {
        return TK_Lt;
      }
    case '=':
      if (line[curidx] == '=') {
        curidx++;
        return TK_Eq;
      } else {
        return TK_Assign;
      }
    case '?':
      return TK_Cond;
    case ':':
      if (line[curidx] == ':') {
        curidx++;
        return TK_Of;
      } else {
        return TK_Colon;
      }
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9': {
      curidx--;
      return GetConstVal();
    }
    case '#':
      return TK_Pound;
    case '\'':
      if (line[curidx + 1] == '\'') {
        thechar = line[curidx];
        curidx += 2;
        return TK_Achar;
      } else {
        return TK_Invalid;
      }
    case '\"': {
      unsigned startidx = curidx;
      unsigned shift = 0;
      unsigned c = 0;
      // for \", skip the \ to leave " only internally
      // and also for the pair of chars \ and n become '\n' etc.
      while ((line[curidx] != '\"' || (line[curidx] == '\"' && curidx > 0 && line[curidx - 1] == '\\')) &&
             curidx < (size_t)current_line_size) {
        if (curidx > 0 && line[curidx - 1] == '\\') {
          shift++;
          switch (line[curidx]) {
            case '"':
              line[curidx - shift] = line[curidx];
              break;
            case '\\':
              line[curidx - shift] = line[curidx];
              // avoid 3rd \ in \\\ being treated as an escaped one
              line[curidx] = 0;
              break;
            default:
              line[curidx - shift] = '\\';
              shift--;
              line[curidx - shift] = line[curidx];
              break;
          }
        } else if (shift) {
          line[curidx - shift] = line[curidx];
        }
        curidx++;
      }
      if (line[curidx] != '\"') {
        return TK_Invalid;
      }
      // for empty string
      if (startidx == curidx) {
        thename = std::string("");
      } else {
        thename = std::string(&line[startidx], curidx - startidx - shift);
      }
      curidx++;
      return TK_String;
    }

    default:
      curidx--;
      if (isalpha(line[curidx]) || line[curidx] == '_') {
        GetName();
        TK_Kind tk = keywordmap[thename];
        if (tk == TK_Invalid)
          tk = TK_Name;
        return tk;
      }

      MASSERT("error in input file\n");
      return TK_Eof;
  }
}

// get the constant value
TK_Kind Lexer::GetConstVal(void) {
  /* patterns of const value:
     1776
     707
     -273
     75         // decimal
     0113       // octal
     0x4b       // hexadecimal
     75         // int
     75u        // unsigned int
     75l        // long
     75ul       // unsigned long
     75lu       // unsigned long
     3.14159    // 3.14159
     6.02e23    // 6.02 x 10^23
     1.6e-19    // 1.6 x 10^-19
     3.0        // 3.0
     3.14159L   // long double (not yet support)
     6.02e23f   // float
     .233  //float
   */
  bool negative = false;
  int valstart = curidx;
  if (line[curidx] == '-') {
    curidx++;
    if (line[curidx] == 'i' && line[curidx + 1] == 'n' && line[curidx + 2] == 'f' && line[curidx + 3] == 'f' &&
        !isalnum(line[curidx + 4])) {
      curidx += 4;
      thefloatval = -INFINITY;
      return TK_Floatconst;
    }
    if (line[curidx] == 'i' && line[curidx + 1] == 'n' && line[curidx + 2] == 'f' && !isalnum(line[curidx + 3])) {
      curidx += 3;
      thedoubleval = -INFINITY;
      return TK_Doubleconst;
    }
    if (line[curidx] == 'n' && line[curidx + 1] == 'a' && line[curidx + 2] == 'n' && line[curidx + 3] == 'f' &&
        !isalnum(line[curidx + 4])) {
      curidx += 4;
      thefloatval = -NAN;
      return TK_Floatconst;
    }
    if (line[curidx] == 'n' && line[curidx + 1] == 'a' && line[curidx + 2] == 'n' && !isalnum(line[curidx + 3])) {
      curidx += 3;
      thedoubleval = -NAN;
      return TK_Doubleconst;
    }
    negative = true;
  }
  if (line[curidx] == '0' && line[curidx + 1] == 'x') {
    curidx += 2;
    if (!isxdigit(line[curidx])) {
      thename = std::string(&line[valstart], curidx - valstart);
      return TK_Invalid;
    }
    theintval = DigitValue(line[curidx++]);
    while (isxdigit(line[curidx])) {
      theintval = (theintval << 4) + DigitValue(line[curidx++]);
    }
    if (negative) {
      theintval = -theintval;
    }
    thefloatval = static_cast<float>(theintval);
    thedoubleval = static_cast<double>(theintval);
    if (negative && theintval == 0) {
      thefloatval = -thefloatval;
      thedoubleval = -thedoubleval;
    }
    thename = std::string(&line[valstart], curidx - valstart);
    return TK_Intconst;
  }

  uint32_t startidx = curidx;
  while (isdigit(line[curidx])) {
    curidx++;
  }

  if (!isdigit(line[startidx]) && line[curidx] != '.') {
    return TK_Invalid;
  }

  if (line[curidx] != '.' && line[curidx] != 'f' && line[curidx] != 'F' && line[curidx] != 'e' && line[curidx] != 'E') {
    curidx = startidx;
    theintval = DigitValue(line[curidx++]);
    if (theintval == 0)  // octal
      while (isdigit(line[curidx])) {
        theintval = ((static_cast<uint64_t>(theintval)) << 3) + DigitValue(line[curidx++]);
      }
    else
      while (isdigit(line[curidx])) {
        theintval = (theintval * 10) + DigitValue(line[curidx++]);
      }
    if (negative) {
      theintval = -theintval;
    }
    if (line[curidx] == 'u' || line[curidx] == 'U') {  // skip 'u' or 'U'
      curidx++;
      if (line[curidx] == 'l' || line[curidx] == 'L') {
        curidx++;
      }
    }
    if (line[curidx] == 'l' || line[curidx] == 'L') {
      curidx++;
      if (line[curidx] == 'l' || line[curidx] == 'L' || line[curidx] == 'u' || line[curidx] == 'U') {
        curidx++;
      }
    }
    thename = std::string(&line[valstart], curidx - valstart);
    thefloatval = static_cast<float>(theintval);
    thedoubleval = static_cast<double>(theintval);
    if (negative && theintval == 0) {
      thefloatval = -thefloatval;
      thedoubleval = -thedoubleval;
    }
    return TK_Intconst;
  } else {  // float value
    if (line[curidx] == '.') {
      curidx++;
    }
    while (isdigit(line[curidx])) {
      curidx++;
    }
    bool doublePrec = true;
    if (line[curidx] == 'e' || line[curidx] == 'E') {
      curidx++;
      if (!isdigit(line[curidx]) && line[curidx] != '-' && line[curidx] != '+') {
        thename = std::string(&line[valstart], curidx - valstart);
        return TK_Invalid;
      }

      if (line[curidx] == '-' || line[curidx] == '+') {
        curidx++;
      }
      while (isdigit(line[curidx])) {
        curidx++;
      }
    }
    if (line[curidx] == 'f' || line[curidx] == 'F') {
      doublePrec = false;
      curidx++;
    }
    if (line[curidx] == 'l' || line[curidx] == 'L') {
      MASSERT("warning: not yet support long double\n");
      curidx++;
    }
    // copy to buffer
    char buffer[30];
    int i;
    int length = curidx - startidx;
    for (i = 0; i < length; i++, startidx++) {
      buffer[i] = line[startidx];
    }
    buffer[length] = 0;
    // get the float constant value
    if (!doublePrec) {
      int eNum = sscanf(buffer, "%e", &thefloatval);
      if (eNum == -1) {
        MASSERT("sscanf_s failed");
      }
      if (negative) {
        thefloatval = -thefloatval;
      }
      theintval = static_cast<int>(thefloatval);
      thedoubleval = static_cast<double>(thedoubleval);
      if (thefloatval == -0) {
        thedoubleval = -thedoubleval;
      }
      thename = std::string(&line[valstart], curidx - valstart);
      return TK_Floatconst;
    } else {
      int eNum = sscanf(buffer, "%le", &thedoubleval);
      if (eNum == -1) {
        MASSERT("sscanf_s failed");
      }
      if (negative) {
        thedoubleval = -thedoubleval;
      }
      theintval = static_cast<int>(thedoubleval);
      thefloatval = static_cast<float>(thedoubleval);
      if (thedoubleval == -0) {
        thefloatval = -thefloatval;
      }
      thename = std::string(&line[valstart], curidx - valstart);
      return TK_Doubleconst;
    }
  }
}

TK_Kind Lexer::NextToken() {
  _thekind = LexToken();
  if (GetVerbose() >= 3)
    std::cout << "Token : " << GetTokenKindString() << "   \t " << GetTokenString() << std::endl;
  return _thekind;
}

std::string Lexer::GetTokenString() {
  TK_Kind tk = _thekind;
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
  TK_Kind tk = _thekind;
  return GetTokenKindString(tk);
}

std::string Lexer::GetTokenKindString(const TK_Kind tk) {
  std::string temp;
  switch (tk) {
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

