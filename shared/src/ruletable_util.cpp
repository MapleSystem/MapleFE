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
#include <cstring>

#include "ruletable_util.h"
#include "lexer.h"
#include "lang_spec.h"
#include "massert.h"
#include "common_header_autogen.h"

////////////////////////////////////////////////////////////////////////////////////
//                Utility Functions to walk tables
////////////////////////////////////////////////////////////////////////////////////

// Return the separator ID, if it's. Or SEP_NA.
//        len : length of matching text.
// Only one of 'str' or 'c' will be valid.
//
// Assumption: Table has been sorted by length.
SepId FindSeparator(const char *str, const char c, unsigned &len) {
  std::string text;
  if (str)
    text = str;
  else
    text = c;

  unsigned i = 0;
  for (; i < SEP_NA; i++) {
    SepTableEntry e = SepTable[i];
    if (!strncmp(text.c_str(), e.mText, strlen(e.mText))) {
      len = strlen(e.mText);
      return e.mId;
    }
  }

  return SEP_NA;
}

// Returen the operator ID, if it's. Or OPR_NA.
// Only one of 'str' or 'c' will be valid.
//
// Assumption: Table has been sorted by length.
OprId FindOperator(const char *str, const char c, unsigned &len) {
  std::string text;
  if (str)
    text = str;
  else
    text = c;

  unsigned i = 0;
  for (; i < OPR_NA; i++) {
    OprTableEntry e = OprTable[i];
    if (!strncmp(text.c_str(), e.mText, strlen(e.mText))) {
      len = strlen(e.mText);
      return e.mId;
    }
  }

  return OPR_NA;
}

// Return the keyword name, or else NULL.
// Only one of 'str' or 'c' will be valid.
//
// Assumption: Table has been sorted by length.
const char* FindKeyword(const char *str, const char c, unsigned &len) {
  std::string text;
  if (str)
    text = str;
  else
    text = c;

  const char *addr = NULL;
  unsigned i = 0;
  for (; i < KeywordTableSize; i++) {
    KeywordTableEntry e = KeywordTable[i];
    if (!strncmp(text.c_str(), e.mText, strlen(e.mText))) {
      len = strlen(e.mText);
      return e.mText;
    }
  }

  return NULL;
}

