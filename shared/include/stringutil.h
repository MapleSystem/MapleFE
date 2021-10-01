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
///////////////////////////////////////////////////////////////////////////////
//
// This file contains the utility functions for string manipulation
//===----------------------------------------------------------------------===//

#ifndef __STRINGUTIL_H__
#define __STRINGUTIL_H__

#include <string>
#include <vector>
#include "char.h"
#include "massert.h"

namespace maplefe {

// String Hash function, borrowed from
// http://license.coscl.org.cn/MulanPSL2
static inline unsigned HashString(const std::string &s) {
  unsigned Result = 0;
  for (size_t i = 0; i < s.size(); i++)
    Result = Result * 33 + (unsigned char)s[i];
  return Result;
}

//consolidate a string vector to a single string
static std::string StringVectorConsolidate(std::vector<std::string> text) {
  std::string result;
  std::vector<std::string>::iterator it = text.begin();
  for (; it != text.end(); it++) {
    result = result + *it;
  }
  return result;
}

// strcmp ignoring case
static inline bool StringEqualNoCase(const std::string& a, const std::string& b)
{
  unsigned int sz = a.size();
  if (b.size() != sz)
    return false;
  for (unsigned int i = 0; i < sz; ++i)
    if (tolower(a[i]) != tolower(b[i]))
      return false;
  return true;
}

// Remove a certain character from a string, and form a new one
static std::string StringRemoveChar(const std::string &s, const char c) {
  std::string result;
  for (unsigned i = 0; i < s.length(); i++) {
    if (s[i] != c)
      result.push_back(s[i]);
  }
  return result;
}

////////////////////////////////////////////////////////////////////////////////
//  Below are some common practice to convert a string to a value.
////////////////////////////////////////////////////////////////////////////////

extern long StringToDecNumeral(std::string s);
extern long StringToHexNumeral(std::string s);
extern long StringToBinNumeral(std::string s);
extern long StringToOctNumeral(std::string s);

//////////////////////////////////////////////////////////////////////////
//         Converting a string literal to value                         //
// Provides a set of general functions, and allow future overriding     //
// in each language. Each language needs has its own StringToValueImpl  //
// to handle specific needs. StringToValueImpl can be just inherit from //
// StringToValue without overriding anything.                           //
//////////////////////////////////////////////////////////////////////////

class StringToValue {
public:
  int    StringToInt(std::string &);
  long   StringToLong(std::string &);
  float  StringToFloat(std::string &);
  double StringToDouble(std::string &);
  bool   StringToBool(std::string &);
  Char   StringToChar(std::string &);
  const char*   StringToString(std::string &);
  bool   StringIsNull(std::string &);
};
}
#endif

