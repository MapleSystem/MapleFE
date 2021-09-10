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
#include <string>

#include "stringutil.h"
#include "stringpool.h"

namespace maplefe {

// Assume the 'l' or 'L' is removed by the caller.
long StringToDecNumeral(std::string s) {
  return std::stol(s);
}

// Assume the '0x' or '0X' is removed by the caller.
long StringToHexNumeral(std::string s){
  long value = 0;
  for (unsigned i = 0; i < s.length(); i++) {
    char c = s[i];
    int c_val;
    if (c <= '9' && c >= '0')
      c_val = c - '0';
    else if (c <= 'f' && c >= 'a')
      c_val = c - 'a' + 10;
    else if (c <= 'F' && c >= 'A')
      c_val = c - 'A' + 10;

    value = value * 16 + c_val;
  }
  return value;
}

// Assume the '0b' or '0B' is removed by the caller.
long StringToBinNumeral(std::string s){
  int value = 0;
  for (unsigned i = 0; i < s.length(); i++) {
    char c = s[i];
    int c_val;
    c_val = c - '0';
    value = value * 2 + c_val;
  }
  return value;
}

// Assume the leading '0' is removed by the caller.
long StringToOctNumeral(std::string s){
  return 0;
}

///////////////////////////////////////////////////////////////////////////////////////
//                  Common Implementation of StringToValue
// Most languages have the same numeral format. We'll reuse this as much as possible.
///////////////////////////////////////////////////////////////////////////////////////

int StringToValue::StringToInt(std::string &str) {
  int value;
  value = (int)StringToLong(str);
  return value;
}

long StringToValue::StringToLong(std::string &s) {
  long value;
  std::string value_string;
  bool is_hex = false;
  bool is_oct = false;
  bool is_bin = false;

  // check if it's hex
  if (s[0] == '0' && s[1] == 'x')
    is_hex = true;
  else if (s[0] == '0' && s[1] == 'X')
    is_hex = true;
  else if (s[0] == '0' && s[1] == 'b')
    is_bin = true;
  else if (s[0] == '0' && s[1] == 'B')
    is_bin = true;
  else if (s[0] == '0' && s.length() > 1)
    // Octal integer starts with '0' and followed by 0 to 7.
    // We dont check the following '0' to '7' since it's done by lexer.
    is_oct = true;

  if (is_hex || is_bin)
    value_string = s.substr(2);
  else if (is_oct)
    value_string = s.substr(1);
  else
    value_string = s;

  // Java allows '_', remove it.
  value_string = StringRemoveChar(value_string, '_');

  if (is_hex)
    value = StringToHexNumeral(value_string);
  else if (is_bin)
    value = StringToBinNumeral(value_string);
  else if (is_oct)
    value = StringToOctNumeral(value_string);
  else
    value = StringToDecNumeral(value_string);

  return value;
}

float StringToValue::StringToFloat(std::string &str) {
  return stof(str);
}

double StringToValue::StringToDouble(std::string &str) {
  return stod(str);
}

bool StringToValue::StringToBool(std::string &str) {
  bool b = false;
  std::string true_text("true");
  std::string false_text("false");
  if (StringEqualNoCase(true_text, str))
    b = true;
  else if (StringEqualNoCase(false_text, str))
    b = false;
  else
    MASSERT(0 && "Wrong boolean string, only accepts true/false");
  return b;
}

// Just return 'c'.
// This function should be overriden by the language implementation.
Char StringToValue::StringToChar(std::string &str) {
  Char ret;
  ret.mIsUnicode = false;
  ret.mData.mChar = 'c';
  return ret;
}

// [NOTE] This is to handle escape character in different language.
//        This implementation is common in many languages. If you have
//        special escape character solution, please do it in you-lang directory.

// There are 4 different places involved in escape character.
//
// 1. In the .spec file. Here is an example
//      rule ESCAPE : ONEOF('\' + 'b', '\' + 't', '\' + 'n')
//    When Autogen read the .spec file, it reads a plain text file. so '\' will be
//    read as 3 characters. The backslash is already himself. Don't need be escaped.
//
// 2. In the gen_xxx.cpp file created by autogen, the output should be
//      TableData TblESCAPE_sub1_data[2] ={{DT_Char, {.mChar='\\'}},{DT_Char, {.mChar='b'}}};
//    Because we are creating C++ source code, and it's a character literal, so we need
//    the additional backslash to escape. So in autogen we will have to check if the character
//    in .spec is \ or ', and handle them specially, like
//      case ET_Char:
//        data += "DT_Char, {.mChar=\'";
//        if (elem->mData.mChar == '\\')
//          data += "\\\\";   <-- Need four backslash to output two in .cpp file.
//        else if (elem->mData.mChar == '\'')
//          data += "\\\'";
//        else
//          data += elem->mData.mChar;
//        data += "\'}";
//        break;
//
// 3. The application program of user programming language write:
//      printf("test\n");
//    When parser read it, it's just a plain text file. So it will see 6 characters actually.
//    The backslash is itself a character.
//    After parser read this string into buffer, you can see it in gdb is "test\\n".
//    Lexer and Parser will keep what they saw.

const char* StringToValue::StringToString(std::string &in_str) {
  std::string target;

  // For most languages, the input 'in_str' still contains the leading " or ' and the
  // ending " or '. They need to be removed.
  std::string str;

  // If empty string literal, return the empty 'target'.
  if (in_str.size() == 2) {
    const char *s = gStringPool.FindString(target);
    return s;
  } else {
    str.assign(in_str, 1, in_str.size() - 2);
  }

  const char *s = gStringPool.FindString(str);
  return s;
}

// Some languares have 'null' keyword with Null type.
bool StringToValue::StringIsNull(std::string &str) {
  bool b = false;
  if (StringEqualNoCase("null", str))
    b = true;
  return b;
}

}

