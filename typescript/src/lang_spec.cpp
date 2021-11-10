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
#include "lang_spec.h"
#include "stringpool.h"

namespace maplefe {

// For all the string to value functions below, we assume the syntax of 's' is correct
// for a literal in Java.

float  StringToValueImpl::StringToFloat(std::string &s) {
  return stof(s);
}

// Java use 'd' or 'D' as double suffix. C++ use 'l' or 'L'.
double StringToValueImpl::StringToDouble(std::string &s) {
  std::string str = s;
  char suffix = str[str.length() - 1];
  if (suffix == 'd' || suffix == 'D')
    str[str.length() - 1] = 'L';
  return stod(str);
}

bool StringToValueImpl::StringToBool(std::string &s) {
  if ((s.size() == 4) && (s.compare("true") == 0))
    return true;
  else if ((s.size() == 5) && (s.compare("false") == 0))
    return false;
  else
    MERROR("unknown bool literal");
}

bool StringToValueImpl::StringIsNull(std::string &s) {return false;}

static char DeEscape(char c) {
  switch(c) {
  case 'b':
    return '\b';
  case 't':
    return '\t';
  case 'n':
    return '\n';
  case 'f':
    return '\f';
  case 'r':
    return '\r';
  case '"':
    return '\"';
  case '\'':
    return '\'';
  case '\\':
    return '\\';
  case '0':
    return '\0';
  default:
    MERROR("Unsupported in DeEscape().");
  }
}

static int char2int(char c) {
  if (c >= '0' && c <= '9')
    return c - '0';
  else if (c >= 'a' && c <= 'f')
    return c - 'a' + 10;
  else if (c >= 'A' && c <= 'F')
    return c - 'A' + 10;
  else
    MERROR("Unsupported char in char2int().");
}

Char StringToValueImpl::StringToChar(std::string &s) {
  Char ret_char;
  ret_char.mIsUnicode = false;
  MASSERT (s[0] == '\'');
  if (s[1] == '\\') {
    if (s[2] == 'u') {
      ret_char.mIsUnicode = true;
      int first = char2int(s[3]);
      int second = char2int(s[4]);
      int third = char2int(s[5]);
      int forth = char2int(s[6]);
      MASSERT(s[7] == '\'');
      ret_char.mData.mUniValue = (first << 12) + (second << 8)
                               + (third << 4) + forth;
    } else {
      ret_char.mData.mChar = DeEscape(s[2]);
    }
  } else {
    MASSERT(s[2] == '\'');
    ret_char.mData.mChar = s[1];
  }
  return ret_char;
}

// Each language has its own format of literal. So this function handles Typescript literals.
// It translate a string into a literal.
//
// 'str' is in the Lexer's string pool.
//
LitData ProcessLiteral(LitId id, const char *str) {
  LitData data;
  std::string value_text(str);
  StringToValueImpl s2v;

  switch (id) {
  case LT_IntegerLiteral: {
    long l = s2v.StringToLong(value_text);
    data.mType = LT_IntegerLiteral;
    data.mData.mInt = l;
    break;
  }
  case LT_FPLiteral: {
    // Java spec doesn't define rules for double. Both float and double
    // are covered by Float Point. But we need differentiate here.
    // Check if it's a float of double. Non-suffix means double.
    char suffix = value_text[value_text.length() - 1];
    if (suffix == 'f' || suffix == 'F') {
      float f = s2v.StringToFloat(value_text);
      data.mType = LT_FPLiteral;
      data.mData.mFloat = f;
    } else {
      double d = s2v.StringToDouble(value_text);
      data.mType = LT_DoubleLiteral;
      data.mData.mDouble = d;
    }
    break;
  }
  case LT_BooleanLiteral: {
    bool b = s2v.StringToBool(value_text);
    data.mType = LT_BooleanLiteral;
    data.mData.mBool = b;
    break; }
  case LT_CharacterLiteral: {
    Char c = s2v.StringToChar(value_text);
    data.mType = LT_CharacterLiteral;
    data.mData.mChar = c;
    break; }
  case LT_StringLiteral: {
    const char *s = s2v.StringToString(value_text);
    data.mType = LT_StringLiteral;
    data.mData.mStrIdx = gStringPool.GetStrIdx(s);
    break; }
  case LT_NullLiteral: {
    // Just need set the id
    data.mType = LT_NullLiteral;
    break; }
  case LT_NA:    // N/A,
  default:
    data.mType = LT_NA;
    break;
  }

  return data;
}

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
//                   Implementation of typescript Lexer
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

// Add a ';' to the end of _line.
void TypescriptLexer::AppendSemiColon() {
  //line[current_line_size] = ';';
  //current_line_size++;
}

// NOTE: right now we rely on 'tsc' to assure the input is legal,
//       so I'll make many things easier and will skip many lexical
//       checks. Just make it easy for now.
//       Also, I assume we don't handle multiple line template literal
//       for the time being.
TempLitData* TypescriptLexer::GetTempLit() {
  TempLitData *tld = NULL;
  unsigned old_cur_idx = curidx;

  if (line[curidx] == '`') {
    // It's certain that this is a template literal because tsc assures it.
    tld = new TempLitData;

    unsigned start_idx;
    unsigned end_idx;
    start_idx = curidx + 1;
    while(1) {
      // Try string
      end_idx = 0;
      std::string fmt_str = "";
      bool s_found = FindNextTLFormat(start_idx, fmt_str, end_idx);
      const char *addr = NULL;
      if (s_found) {
        MASSERT(fmt_str.size() > 0 && "found token has 0 data?");
        addr = gStringPool.FindString(fmt_str);
        start_idx = end_idx + 1;
      }

      // Try pattern
      end_idx = 0;
      const char *addr_ph = NULL;
      std::string pl_str = "";
      bool p_found = FindNextTLPlaceHolder(start_idx, pl_str, end_idx);
      if (p_found) {
        unsigned len = pl_str.size();
        MASSERT(len > 0 && "found token has 0 data?");
        addr_ph = gStringPool.FindString(pl_str);
        // We need skip the ending '}' of a pattern.
        start_idx = end_idx + 2;
      }

      // If both string and pattern failed to be found
      if (!s_found && !p_found) {
        break;
      } else {
        tld->mStrings.PushBack(addr);
        tld->mStrings.PushBack(addr_ph);
      }
    }

    // It's for sure that this is the ending '`'.
    MASSERT(line[start_idx] == '`');
    curidx = start_idx + 1;
  }

  return tld;
}

// Find the pure string of a template literal.
// Set end_idx as the last char of string.
bool TypescriptLexer::FindNextTLFormat(unsigned start_idx, std::string &str, unsigned& end_idx) {
  unsigned working_idx = start_idx;
  while(1) {
    if ((line[working_idx] == '$' && line[working_idx+1] == '{')
        || line[working_idx] == '`' ){
      end_idx = working_idx - 1;
      break;
    }

    // Template Literal allows \n in format and place holder.
    // the \n was removed by Lexer in the beginning of ReadALine();
    if (working_idx == current_line_size) {
      str += '\n';
      ReadALine();
      if (endoffile)
        return false;
      working_idx = 0;
      continue;
    }

    str += line[working_idx];
    working_idx++;
  }

  if (str.size() > 0)
    return true;
  else
    return false;
}

// Find the pattern string of a template literal.
// Set end_idx as the last char of string.
//
// [NOTE] For nested template literals, we will matching the outmost
//        template literal and treat the inner template literal as
//        a plain string. The innter template literal will later
//        be handled by the parsing of the outmost template literal.
//        This makes things easier.
//
// [NOTE] We only support two level of nesting temp lit !!

bool TypescriptLexer::FindNextTLPlaceHolder(unsigned start_idx, std::string& str, unsigned& end_idx) {
  unsigned working_idx = start_idx;
  if (line[working_idx] != '$' || line[working_idx+1] != '{')
    return false;

  working_idx = start_idx + 2;

  // There could be {..} inside placeholder.
  unsigned num_left_brace = 0;

  // There could be string literal inside placeholder,
  bool in_string_literal = false;

  // There could be nested template literal inside a place holder.
  bool in_nested_temp_lit = false;
  bool waiting_right_brace = false;
  unsigned num_left_brace_inner = 0; // there could be {..} for inner temp lit.

  while(1) {

    if (line[working_idx] == '`') {
      if (in_nested_temp_lit) {
        // finish inner temp lit. Need clear the status.
        in_nested_temp_lit = false;
        num_left_brace_inner = 0;
      } else {
        in_nested_temp_lit = true;
      }
    } else if (line[working_idx] == '$' && line[working_idx+1] == '{') {
      MASSERT(in_nested_temp_lit);
      str += line[working_idx];
      str += line[working_idx + 1];
      working_idx += 2;
      waiting_right_brace = true;
      continue;
    } else if (line[working_idx] == '\'' || line[working_idx] == '\"') {
      in_string_literal = in_string_literal ? false : true;
    } else if (line[working_idx] == '{') {
      if (!in_string_literal) {
        if (in_nested_temp_lit)
          num_left_brace_inner++;
        else
          num_left_brace++;
      }
    } else if (line[working_idx] == '}') {
      if (!in_string_literal) {
        if (waiting_right_brace)
          waiting_right_brace = false;
        else if (!in_nested_temp_lit && num_left_brace > 0)
          num_left_brace--;
        else if (in_nested_temp_lit && num_left_brace_inner > 0)
          num_left_brace_inner--;
        else
          break;
      }
    }

    // Template Literal allows \n in format and place holder.
    // the \n was removed by Lexer in the beginning of ReadALine();
    // 
    // I don't need worry about if the template literal is ended or not,
    // since tsc guarantees it's correct.
    if (working_idx == current_line_size) {
      str += '\n';
      ReadALine();
      if (endoffile)
        return false;
      working_idx = 0;
    } else {
      str += line[working_idx];
      working_idx++;
    }
  }

  end_idx = working_idx - 1;
  return true;
}

Lexer* CreateLexer() {
  Lexer *lexer = new TypescriptLexer();
  return lexer;
}

}
