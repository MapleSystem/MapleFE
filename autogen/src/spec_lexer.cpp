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
#include <iostream>
#include <cmath>
#include "massert.h"
#include "spec_lexer.h"
#include <climits>
#include <cstdlib>

namespace maplefe {

#define MAX_LINE_SIZE 4096

/* Read (next) line from the input file, and return the readed
   number of chars.
   if the line is empty (nothing but a newline), returns 0.
   if EOF, return -1.
   The trailing new-line character has been removed.
 */
int SPECLexer::ReadALine() {
  if (!deffile) {
    line[0] = '\0';
    return -1;
  }

  current_line_size = getline(&line, &linebuf_size, deffile);
  if (current_line_size <= 0) {  // EOF
    fclose(deffile);
    line[0] = '\0';
  } else {
    if (line[current_line_size - 1] == '\n') {
      line[current_line_size - 1] = '\0';
      current_line_size--;
    }
  }

  curidx = 0;
  return current_line_size;
}

SPECLexer::SPECLexer()
  : thename(""),
    theintval(0),
    thefloatval(0.0f),
    thedoubleval(0),
    _thekind(SPECTK_Invalid),
    verboseLevel(0),
    deffile(nullptr),
    line(nullptr),
    linebuf_size(0),
    current_line_size(0),
    curidx(0),
    _linenum(0) {
      seencomments.clear();
      keywordmap.clear();

#define KEYWORD(S, T)     \
  {                             \
    std::string str;            \
    str = #S;                 \
    keywordmap[str] = SPECTK_##T; \
  }
#include "spec_keywords.h"
#undef KEYWORD
}

void SPECLexer::PrepareForFile(const std::string filename) {
  // open file
  deffile = fopen(filename.c_str(), "r");
  if (!deffile) {
    MASSERT(0 && "cannot open file\n");
  }

  // allocate line buffer.
  linebuf_size = (size_t)MAX_LINE_SIZE;
  line = static_cast<char *>(malloc(linebuf_size));  // initial line buffer.
  if (!line) {
    MASSERT(0 && "cannot allocate line buffer\n");
  }

  // try to read the first line
  if (ReadALine() < 0) {
    _linenum = 0;
  } else {
    _linenum = 1;
  }

  _thekind = SPECTK_Invalid;
}

void SPECLexer::PrepareForString(const std::string &src) {
  // allocate line buffer.
  if (!line) {
    linebuf_size = (size_t)MAX_LINE_SIZE;
    line = static_cast<char *>(malloc(linebuf_size));  // initial line buffer.
    if (!line) {
      MASSERT(0 && "cannot allocate line buffer\n");
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

void SPECLexer::GetName(void) {
  int startidx = curidx;
  if ((isalnum(line[curidx]) || line[curidx] == '_' || line[curidx] == '$' ||
       line[curidx] == '@') &&
      (curidx < (size_t)current_line_size)) {
    curidx++;
  }
  if (line[curidx] == '@' && (line[curidx - 1] == 'h' || line[curidx - 1] == 'f')) {
    curidx++;  // special pattern for exception handling labels: catch or finally
  }

  while ((isalnum(line[curidx]) || line[curidx] == '_' || line[curidx] == '$' ||
          line[curidx] == ';' || line[curidx] == '/' || line[curidx] == '|' ||
          line[curidx] == '?' || line[curidx] == '@') &&
         (curidx < (size_t)current_line_size)) {
    curidx++;
  }
  thename = std::string(&line[startidx], curidx - startidx);
}

SPECTokenKind SPECLexer::LexToken(void) {
  // skip spaces
  while ((line[curidx] == ' ' || line[curidx] == '\t') && curidx < (size_t)current_line_size) {
    curidx++;
  }
  // check end of line
  while (curidx == (size_t)current_line_size || line[curidx] == '#') {
    if (line[curidx] == '#') {  // process comment contents
      seencomments.push_back(std::string(&line[curidx + 1], current_line_size - curidx - 1));
    }

    if (ReadALine() < 0) {
      return SPECTK_Eof;
    }
    _linenum++;  // a new line readed.

    // skip spaces
    while ((line[curidx] == ' ' || line[curidx] == '\t') && curidx < (size_t)current_line_size) {
      curidx++;
    }
  }
  char curchar = line[curidx++];
  switch (curchar) {
    case '\n':
      return SPECTK_Newline;
    case '(':
      return SPECTK_Lparen;
    case ')':
      return SPECTK_Rparen;
    case '{':
      return SPECTK_Lbrace;
    case '}':
      return SPECTK_Rbrace;
    case '[':
      return SPECTK_Lbrack;
    case ']':
      return SPECTK_Rbrack;
    case '<':
      return SPECTK_Langle;
    case '>':
      return SPECTK_Rangle;
    case '=':
      if (line[curidx] == '=' || line[curidx+1] == '>') {
        curidx += 2;
        return SPECTK_Actionfunc;
      } else {
        return SPECTK_Eqsign;
      }
    case ',':
      return SPECTK_Coma;
    case ':':
      return SPECTK_Colon;
    case ';':
      return SPECTK_Semicolon;
    case '*':
      return SPECTK_Asterisk;
    case '+':
      return SPECTK_Concat;
    case '.':
      if (line[curidx] == '.' && line[curidx + 1] == '.') {
        curidx += 2;
        return SPECTK_Dotdotdot;
      } else {
        return SPECTK_Dot;
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
    case '9':
    case '-': {
      curidx--;
      return GetConstVal();
    }

    case '%':
      return SPECTK_Percent;
    case '&':
      if (isalpha(line[curidx]) || line[curidx] == '_') {
        GetName();
        return SPECTK_Fname;
      } else {
        // for error reporting.
        thename = std::string(&line[curidx - 1], 2);
        return SPECTK_Invalid;
      }
    case '\'':
      if (line[curidx + 1] == '\'') {
        thechar = line[curidx];
        curidx += 2;
        return SPECTK_Char;
      } else {
        return SPECTK_Invalid;
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
        return SPECTK_Invalid;
      }
      // for empty string
      if (startidx == curidx) {
        thename = std::string("");
      } else {
        thename = std::string(&line[startidx], curidx - startidx - shift);
      }
      curidx++;
      return SPECTK_String;
    }

    default:
      curidx--;
      if (isalpha(line[curidx]) || line[curidx] == '_') {
        GetName();
        SPECTokenKind tk = keywordmap[thename];
        if (tk == SPECTK_Invalid)
          tk = SPECTK_Name;
        return tk;
      }

      MASSERT(0 && "error in input file\n");
      return SPECTK_Eof;
  }
}

// get the constant value
SPECTokenKind SPECLexer::GetConstVal(void) {
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
      return SPECTK_Floatconst;
    }
    if (line[curidx] == 'i' && line[curidx + 1] == 'n' && line[curidx + 2] == 'f' && !isalnum(line[curidx + 3])) {
      curidx += 3;
      thedoubleval = -INFINITY;
      return SPECTK_Doubleconst;
    }
    if (line[curidx] == 'n' && line[curidx + 1] == 'a' && line[curidx + 2] == 'n' && line[curidx + 3] == 'f' &&
        !isalnum(line[curidx + 4])) {
      curidx += 4;
      thefloatval = -NAN;
      return SPECTK_Floatconst;
    }
    if (line[curidx] == 'n' && line[curidx + 1] == 'a' && line[curidx + 2] == 'n' && !isalnum(line[curidx + 3])) {
      curidx += 3;
      thedoubleval = -NAN;
      return SPECTK_Doubleconst;
    }
    negative = true;
  }
  if (line[curidx] == '0' && line[curidx + 1] == 'x') {
    curidx += 2;
    if (!isxdigit(line[curidx])) {
      thename = std::string(&line[valstart], curidx - valstart);
      return SPECTK_Invalid;
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
    return SPECTK_Intconst;
  }

  uint32_t startidx = curidx;
  while (isdigit(line[curidx])) {
    curidx++;
  }

  if (!isdigit(line[startidx]) && line[curidx] != '.') {
    return SPECTK_Invalid;
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
    return SPECTK_Intconst;
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
        return SPECTK_Invalid;
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
      MASSERT(0 && "warning: not yet support long double\n");
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
        MASSERT(0 && "sscanf_s failed");
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
      return SPECTK_Floatconst;
    } else {
      int eNum = sscanf(buffer, "%le", &thedoubleval);
      if (eNum == -1) {
        MASSERT(0 && "sscanf_s failed");
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
      return SPECTK_Doubleconst;
    }
  }
}

SPECTokenKind SPECLexer::NextToken() {
  _thekind = LexToken();
  if (GetVerbose() >= 3)
    MMSG("Token : ", GetTokenString());
  return _thekind;
}

std::string SPECLexer::GetTokenString(SPECTokenKind thekind) {
  std::string temp;
  switch (thekind) {
    default: {
      temp = "invalid token";
      break;
    }
    case SPECTK_Intconst:
    case SPECTK_Floatconst:
    case SPECTK_Doubleconst: {
      temp.append(thename);
      break;
    }
    case SPECTK_Name: {
      temp.append(thename);
      break;
    }
    case SPECTK_Newline: {
      temp = "\\n";
      break;
    }
    case SPECTK_Lparen: {
      temp = "(";
      break;
    }
    case SPECTK_Rparen: {
      temp = ")";
      break;
    }
    case SPECTK_Lbrace: {
      temp = "{";
      break;
    }
    case SPECTK_Rbrace: {
      temp = "}";
      break;
    }
    case SPECTK_Lbrack: {
      temp = "[";
      break;
    }
    case SPECTK_Rbrack: {
      temp = "]";
      break;
    }
    case SPECTK_Langle: {
      temp = "<";
      break;
    }
    case SPECTK_Rangle: {
      temp = ">";
      break;
    }
    case SPECTK_Eqsign: {
      temp = "=";
      break;
    }
    case SPECTK_Coma: {
      temp = ",";
      break;
    }
    case SPECTK_Dot: {
      temp = ".";
      break;
    }
    case SPECTK_Dotdotdot: {
      temp = "...";
      break;
    }
    case SPECTK_Colon: {
      temp = ":";
      break;
    }
    case SPECTK_Semicolon: {
      temp = ";";
      break;
    }
    case SPECTK_Asterisk: {
      temp = "*";
      break;
    }
    case SPECTK_Percent: {
      temp = "%";
      break;
    }
    case SPECTK_Rule: {
      temp = "rule";
      break;
    }
    case SPECTK_Struct: {
      temp = "STRUCT";
      break;
    }
    case SPECTK_Oneof: {
      temp = "ONEOF";
      break;
    }
    case SPECTK_Zeroorone: {
      temp = "ZEROORONE";
      break;
    }
    case SPECTK_Zeroormore: {
      temp = "ZEROORMORE";
      break;
    }
    case SPECTK_Concat: {
      temp = "+";
      break;
    }
    case SPECTK_Char: {
      temp = "\'";
      temp += thechar;
      temp.append("\'");
      break;
    }
    case SPECTK_String: {
      temp = "\"";
      temp.append(thename);
      temp.append("\"");
      break;
    }
    case SPECTK_Actionfunc: {
      temp = "==>";
      break;
    }
    case SPECTK_Attr: {
      temp = "attr";
      break;
    }
    case SPECTK_Datatype: {
      temp = "datatype";
      break;
    }
    case SPECTK_Tokentype: {
      temp = "tokentype";
      break;
    }
    case SPECTK_Action: {
      temp = "action";
      break;
    }
    case SPECTK_Validity: {
      temp = "validity";
      break;
    }
    case SPECTK_Eof: {
      temp = "EOF";
      break;
    }
  }
  return temp;
}


std::string SPECLexer::GetTokenString() {
  return GetTokenString(_thekind);
}

}
