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
#include <cstring>

#include "ruletable_util.h"
#include "lexer.h"
#include "massert.h"
#include "rule_summary.h"
#include "container.h"

namespace maplefe {

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
  for (; i < SepTableSize; i++) {
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
  for (; i < OprTableSize; i++) {
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
//
// If there is an action which has no elem as its argument. It could
// take all element, or any number of element. In this case, we think
// it HasElem.  Please look at typescript's stmt.spec.
//   rule JSIdentifier: is a good example.
//
bool RuleActionHasElem(RuleTable *table, unsigned target_idx) {
  for (unsigned i = 0; i < table->mNumAction; i++) {
    Action *act = table->mActions + i;
    if (act->mNumElem == 0)
      return true;
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
bool RuleFindChildIndex(RuleTable *parent, RuleTable *child, unsigned &index) {
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

// Returns the child table if found.
// [NOTE] Caller needs check if the return is NULL if needed.
RuleTable* RuleFindChild(RuleTable *parent, unsigned index) {
  RuleTable *child = NULL;
  TableData *data = parent->mData + index;
  switch (data->mType) {
  case DT_Subtable:
    child = data->mData.mEntry;
    break;
  default:
    break;
  }

  return child;
}

// If it's reachable from 'from' to 'to'.
bool RuleReachable(RuleTable *from, RuleTable *to) {
  bool found = false;
  SmallList<RuleTable*> working_list;
  SmallVector<RuleTable*> done_list;

  working_list.PushBack(from);
  while (!working_list.Empty()) {
    RuleTable *rt = working_list.Front();
    working_list.PopFront();

    if (rt == to) {
      found = true;
      break;
    }

    bool is_done = false;
    for (unsigned i = 0; i < done_list.GetNum(); i++) {
      if (rt == done_list.ValueAtIndex(i)) {
        is_done = true;
        break;
      }
    }
    if (is_done)
      continue;

    // Add all table children to working list.
    for (unsigned i = 0; i < rt->mNum; i++) {
      TableData *data = rt->mData + i;
      if (data->mType == DT_Subtable) {
        RuleTable *child = data->mData.mEntry;
        working_list.PushBack(child);
      }
    }

    done_list.PushBack(rt);
  }

  return found;
}

bool LookAheadEqual(LookAhead la_a, LookAhead la_b) {
  if (la_a.mType == LA_Char && la_b.mType == LA_Char) {
    return (la_a.mData.mChar == la_b.mData.mChar);
  } else if (la_a.mType == LA_Token && la_b.mType == LA_Token) {
    return (la_a.mData.mTokenId == la_b.mData.mTokenId);
  } else if (la_a.mType == LA_String && la_b.mType == LA_String) {
    size_t len_a = strlen(la_a.mData.mString);
    size_t len_b = strlen(la_b.mData.mString);
    if (len_a != len_b)
      return false;
    return strncmp(la_a.mData.mString, la_b.mData.mString, len_a);
  } else if (la_a.mType == LA_Identifier && la_b.mType == LA_Identifier) {
    return true;
  } else if (la_a.mType == LA_Literal && la_b.mType == LA_Literal) {
    return true;
  } else
    return false;
}
}
