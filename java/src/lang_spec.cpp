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

// Each language has its own format of literal. So this function handles Java literals.
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
    int i = s2v.StringToInt(value_text);
    data.mType = LT_IntegerLiteral;
    data.mData.mInt = i;
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
//          Implementation of Java Lexer
/////////////////////////////////////////////////////////////////////////////////////

Lexer* CreateLexer() {
  Lexer *lexer = new JavaLexer();
  return lexer;
}


}
