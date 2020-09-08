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
#include "lang_spec.h"

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

bool   StringToValueImpl::StringToBool(std::string &s) {return false;}
char   StringToValueImpl::StringToChar(std::string &s) {return 'c';}
bool   StringToValueImpl::StringIsNull(std::string &s) {return false;}


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
    // Check if it's a float of double. Non-suffix means double.
    char suffix = value_text[value_text.length() - 1];
    if (suffix == 'f' || suffix == 'F') {
      float f = s2v.StringToFloat(value_text);
      data.mType = LT_FPLiteral;
      data.mData.mFloat = f;
      data.mIsDouble = false;
    } else {
      double d = s2v.StringToDouble(value_text);
      data.mType = LT_FPLiteral;  // we don't have doubleLiteral
      data.mData.mDouble = d;
      data.mIsDouble = true;
    }
    break;
  }
  case LT_BooleanLiteral: {
    bool b = s2v.StringToBool(value_text);
    data.mType = LT_BooleanLiteral;
    data.mData.mBool = b;
    break; }
  case LT_CharacterLiteral: {
    char c = s2v.StringToChar(value_text);
    data.mType = LT_CharacterLiteral;
    data.mData.mChar = c;
    break; }
  case LT_StringLiteral: {
    const char *s = s2v.StringToString(value_text);
    data.mType = LT_StringLiteral;
    data.mData.mStr = s;
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


