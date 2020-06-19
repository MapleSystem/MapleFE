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

// Returns true : The rule actions in 'table' involves i-th element
// [NOTE] i starts from 1.
bool RuleActionHasElem(RuleTable *table, unsigned target_idx) {
  for (unsigned i = 0; i < table->mNumAction; i++) {
    Action *act = table->mActions + i;
    for (unsigned j = 0; j < act->mNumElem; j++) {
      unsigned index = act->mElems[j];
      if (index = target_idx)
        return true;
    }
  }
  return false;
}

// Returns true if child is found. Also set the 'index'.
// NOTE: We don't go deeper into grandchildren.
bool RuleFindChild(RuleTable *parent, RuleTable *child, unsigned &index) {
  bool found = false;
  EntryType type = parent->mType;

  switch(type) {
  // Concatenate and Oneof, can be handled the same
  case ET_Concatenate:
  case ET_Oneof: {
    for (unsigned i = 0; i < parent->mNum; i++) {
      index = i;
      TableData *data = parent->mData + i;
      switch (data->mType) {
      case DT_Subtable:
        if (data->mData.mEntry == child)
          found = true;
        break;
      default:
        break;
      }

      if (found)
        break;
    }
    break;
  }

  // Zeroorone, Zeroormore and Data can be handled the same way.
  case ET_Data:
  case ET_Zeroorone:
  case ET_Zeroormore: {
    MASSERT((parent->mNum == 1) && "zeroormore node has more than one elements?");
    index = 0; // index is always 0.
    TableData *data = parent->mData;
    switch (data->mType) {
    case DT_Subtable:
      if (data->mData.mEntry == child)
        found = true;
      break;
    default:
      break;
    }
    break;
  }

  default:
    break;
  }

  return found;
}
