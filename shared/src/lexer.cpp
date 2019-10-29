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

Token *Lexer::LexToken(void) {
  // skip spaces
  while ((line[curidx] == ' ' || line[curidx] == '\t') && curidx < (size_t)current_line_size) {
    mToken = new Token(TT_NA, TK_Invalid, ET_WS);
    curidx++;
    return mToken;
  }
  // check end of line
  while (curidx == (size_t)current_line_size || (line[curidx] == '/' && line[curidx+1] == '/')) {
    if (line[curidx] == '/' && line[curidx+1] == '/') {  // process comment contents
      seencomments.push_back(std::string(&line[curidx + 2], current_line_size - curidx - 1));
      mToken = new Token(TT_SP, TK_Invalid, ET_CM);
      // need to store comment
      return mToken;
    }

    if (ReadALine() < 0) {
      mToken = new Token(TT_NA, TK_Eof, ET_NA);
      return mToken;
    }
    _linenum++;  // a new line read.
    // print current line
    if (GetVerbose() >= 3) {
      MMSG2("  >>>> LINE: ", _linenum, GetLine());
    }

    // skip spaces
    while ((line[curidx] == ' ' || line[curidx] == '\t') && curidx < (size_t)current_line_size) {
      mToken = new Token(TT_NA, TK_Invalid, ET_WS);
      curidx++;
      return mToken;
    }
  }
  // handle comments in /* */
  if (line[curidx] == '/' && line[curidx+1] == '*') {
    curidx += 2; // skip : /*
    while (curidx < (size_t)current_line_size && !(line[curidx] == '*' && line[curidx+1] == '/')) {
      curidx++;
      if (curidx == (size_t)current_line_size) {
        if (ReadALine() < 0) {
          mToken = new Token(TT_NA, TK_Eof, ET_NA);
          return mToken;
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

Token *Lexer::ProcessToken() {
  char curchar = line[curidx++];
  switch (curchar) {
    case '\n':
      mToken = new Token(TT_SP, TK_Newline);
      return mToken;
    case '(':
      mToken = new Token(TT_SP, TK_Lparen);
      return mToken;
    case ')':
      mToken = new Token(TT_SP, TK_Rparen);
      return mToken;
    case '{':
      mToken = new Token(TT_SP, TK_Lbrace);
      return mToken;
    case '}':
      mToken = new Token(TT_SP, TK_Rbrace);
      return mToken;
    case '[':
      mToken = new Token(TT_SP, TK_Lbrack);
      return mToken;
    case ']':
      mToken = new Token(TT_SP, TK_Rbrack);
      return mToken;
    case ',':
      mToken = new Token(TT_SP, TK_Comma);
      return mToken;
    case ';':
      mToken = new Token(TT_SP, TK_Semicolon);
      return mToken;
    case '@':
      mToken = new Token(TT_OP, TK_At);
      return mToken;
    case '.':
      if (line[curidx] == '.' && line[curidx + 1] == '.') {
        curidx += 2;
        mToken = new Token(TT_OP, TK_Dotdotdot);
        return mToken;
      } else {
        mToken = new Token(TT_OP, TK_Dot);
        return mToken;
      }
    case '+':
      if (line[curidx] == '+') {
        curidx++;
        mToken = new Token(TT_OP, TK_Inc);
        return mToken;
      } else if (line[curidx] == '=') {
        curidx++;
        mToken = new Token(TT_OP, TK_Addassign);
        return mToken;
      } else {
        mToken = new Token(TT_OP, TK_Add);
        return mToken;
      }
    case '-':
      if (line[curidx] == '-') {
        curidx++;
        mToken = new Token(TT_OP, TK_Dec);
        return mToken;
      } else if (line[curidx] == '=') {
        curidx++;
        mToken = new Token(TT_OP, TK_Subassign);
        return mToken;
      } else if (line[curidx] == '>') {
        curidx++;
        mToken = new Token(TT_OP, TK_Arrow);
        return mToken;
      } else {
        mToken = new Token(TT_OP, TK_Sub);
        return mToken;
      }
    case '*':
      if (line[curidx] == '=') {
        curidx++;
        mToken = new Token(TT_OP, TK_Mulassign);
        return mToken;
      } else {
        mToken = new Token(TT_OP, TK_Mul);
        return mToken;
      }
    case '/':
      if (line[curidx] == '=') {
        curidx++;
        mToken = new Token(TT_OP, TK_Divassign);
        return mToken;
      } else {
        mToken = new Token(TT_OP, TK_Div);
        return mToken;
      }
    case '%':
      if (line[curidx] == '=') {
        curidx++;
        mToken = new Token(TT_OP, TK_Modassign);
        return mToken;
      } else {
        mToken = new Token(TT_OP, TK_Mod);
        return mToken;
      }
    case '&':
      if (line[curidx] == '&') {
        curidx++;
        mToken = new Token(TT_OP, TK_Land);
        return mToken;
      } else if (line[curidx] == '=') {
        curidx++;
        mToken = new Token(TT_OP, TK_Bandassign);
        return mToken;
      } else {
        mToken = new Token(TT_OP, TK_Band);
        return mToken;
      }
    case '|':
      if (line[curidx] == '|') {
        curidx++;
        mToken = new Token(TT_OP, TK_Lor);
        return mToken;
      } else if (line[curidx] == '=') {
        curidx++;
        mToken = new Token(TT_OP, TK_Borassign);
        return mToken;
      } else {
        mToken = new Token(TT_OP, TK_Bor);
        return mToken;
      }
    case '!':
      if (line[curidx] == '=') {
        curidx++;
        mToken = new Token(TT_OP, TK_Ne);
        return mToken;
      } else {
        mToken = new Token(TT_OP, TK_Not);
        return mToken;
      }
    case '^':
      if (line[curidx] == '=') {
        curidx++;
        mToken = new Token(TT_OP, TK_Bxorassign);
        return mToken;
      } else {
        mToken = new Token(TT_OP, TK_Bxor);
        return mToken;
      }
    case '~':
      mToken = new Token(TT_OP, TK_Bcomp);
      return mToken;
    case '>':
      if (line[curidx] == '>' && line[curidx + 1] == '>' && line[curidx + 2] == '=') {
        curidx += 3;
        mToken = new Token(TT_OP, TK_Zextassign);
        return mToken;
      } else if (line[curidx] == '>' && line[curidx + 1] == '>') {
        curidx += 2;
        mToken = new Token(TT_OP, TK_Zext);
        return mToken;
      } else if (line[curidx] == '>' && line[curidx + 1] == '=') {
        curidx += 2;
        mToken = new Token(TT_OP, TK_Shrassign);
        return mToken;
      } else if (line[curidx] == '>') {
        curidx++;
        mToken = new Token(TT_OP, TK_Shr);
        return mToken;
      } else if (line[curidx] == '=') {
        curidx++;
        mToken = new Token(TT_OP, TK_Ge);
        return mToken;
      } else {
        mToken = new Token(TT_OP, TK_Gt);
        return mToken;
      }
    case '<':
      if (line[curidx] == '<' && line[curidx + 1] == '=') {
        curidx += 2;
        mToken = new Token(TT_OP, TK_Shlassign);
        return mToken;
      } else if (line[curidx] == '<') {
        curidx++;
        mToken = new Token(TT_OP, TK_Shl);
        return mToken;
      } else if (line[curidx] == '=') {
        curidx++;
        mToken = new Token(TT_OP, TK_Le);
        return mToken;
      } else if (line[curidx] == '>') {
        curidx++;
        mToken = new Token(TT_OP, TK_Diamond);
        return mToken;
      } else {
        mToken = new Token(TT_OP, TK_Lt);
        return mToken;
      }
    case '=':
      if (line[curidx] == '=') {
        curidx++;
        mToken = new Token(TT_OP, TK_Eq);
        return mToken;
      } else {
        mToken = new Token(TT_OP, TK_Assign);
        return mToken;
      }
    case '?':
      mToken = new Token(TT_OP, TK_Cond);
      return mToken;
    case ':':
      if (line[curidx] == ':') {
        curidx++;
        mToken = new Token(TT_OP, TK_Of);
        return mToken;
      } else {
        mToken = new Token(TT_OP, TK_Colon);
        return mToken;
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
      mToken = new Token(TT_OP, TK_Pound);
      return mToken;
    case '\'':
      if (line[curidx + 1] == '\'') {
        thechar = line[curidx];
        curidx += 2;
        mToken = new Token(TT_LT, TK_Achar);
        return mToken;
      } else {
        mToken = new Token(TT_NA, TK_Invalid);
        return mToken;
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
        mToken = new Token(TT_LT, TK_Invalid);
        return mToken;
      }
      // for empty string
      if (startidx == curidx) {
        thename = std::string("");
      } else {
        thename = std::string(&line[startidx], curidx - startidx - shift);
      }
      curidx++;

      LitData *data = new LitData(LT_StringLiteral);
      char *name = mStringPool.FindString(thename);
      data->mData.mStr = name;
      mToken = new LiteralToken(TK_String, *data);
      return mToken;
    }

    default:
      curidx--;
      if (isalpha(line[curidx]) || line[curidx] == '_') {
        GetName();
        TK_Kind tkk = keywordmap[thename];
        char *name = mStringPool.FindString(thename);
        if (tkk != TK_Invalid) {
          mToken = new KeywordToken(tkk, name);
        } else {
          mToken = new IdentifierToken(name);
        }
        return mToken;
      }

      MASSERT("error in input file\n");
      mToken = new Token(TT_NA, TK_Eof, ET_NA);
      return mToken;
  }
}

// get the constant value
Token *Lexer::GetConstVal(void) {
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
      LitData *data = new LitData(LT_FPLiteral);
      data->mData.mFloat = thefloatval;;
      mToken = new LiteralToken(TK_Floatconst, *data);
      return mToken;
    }
    if (line[curidx] == 'i' && line[curidx + 1] == 'n' && line[curidx + 2] == 'f' && !isalnum(line[curidx + 3])) {
      curidx += 3;
      thedoubleval = -INFINITY;
      LitData *data = new LitData(LT_FPLiteral);
      data->mData.mDouble = thedoubleval;;
      mToken = new LiteralToken(TK_Doubleconst, *data);
      return mToken;
    }
    if (line[curidx] == 'n' && line[curidx + 1] == 'a' && line[curidx + 2] == 'n' && line[curidx + 3] == 'f' &&
        !isalnum(line[curidx + 4])) {
      curidx += 4;
      thefloatval = -NAN;
      LitData *data = new LitData(LT_FPLiteral);
      data->mData.mFloat = thefloatval;;
      mToken = new LiteralToken(TK_Floatconst, *data);
      return mToken;
    }
    if (line[curidx] == 'n' && line[curidx + 1] == 'a' && line[curidx + 2] == 'n' && !isalnum(line[curidx + 3])) {
      curidx += 3;
      thedoubleval = -NAN;
      LitData *data = new LitData(LT_FPLiteral);
      data->mData.mDouble = thedoubleval;;
      mToken = new LiteralToken(TK_Doubleconst, *data);
      return mToken;
    }
    negative = true;
  }
  if (line[curidx] == '0' && line[curidx + 1] == 'x') {
    curidx += 2;
    if (!isxdigit(line[curidx])) {
      thename = std::string(&line[valstart], curidx - valstart);
      mToken = new Token(TT_NA);
      return mToken;
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

    LitData *data = new LitData(LT_IntegerLiteral);
    data->mData.mDouble = theintval;;
    mToken = new LiteralToken(TK_Intconst, *data);
    return mToken;
  }

  uint32_t startidx = curidx;
  while (isdigit(line[curidx])) {
    curidx++;
  }

  if (!isdigit(line[startidx]) && line[curidx] != '.') {
    mToken = new Token(TT_NA);
    return mToken;
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

    LitData *data = new LitData(LT_IntegerLiteral);
    data->mData.mDouble = theintval;;
    mToken = new LiteralToken(TK_Intconst, *data);
    return mToken;
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
        mToken = new Token(TT_NA);
        return mToken;
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
      LitData *data = new LitData(LT_FPLiteral);
      data->mData.mFloat = thefloatval;;
      mToken = new LiteralToken(TK_Floatconst, *data);
      return mToken;
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
      LitData *data = new LitData(LT_FPLiteral);
      data->mData.mDouble = thedoubleval;;
      mToken = new LiteralToken(TK_Doubleconst, *data);
      return mToken;
    }
  }
}

Token *Lexer::NextToken() {
  mToken = LexToken();
  // skip whitespace and comments for now
  while (mToken->EType == ET_WS || mToken->EType == ET_CM)
    mToken = LexToken();
  if (GetVerbose() >= 3)
    std::cout << "Token : " << GetTokenKindString() << "   \t " << GetTokenString() << std::endl;
  return mToken;
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

