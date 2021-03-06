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
#include <list>
#include "base_struct.h"
#include "stringpool.h"

namespace maplefe {

void StructData::SetString(const std::string name, StringPool *pool) {
  mKind = DK_String;
  const char *str = pool->FindString(name);
  mValue.mString = str;
}

void StructData::SetName(const std::string name, StringPool *pool) {
  mKind = DK_Name;
  const char *str = pool->FindString(name);
  mValue.mName = str;
}

void StructBase::Dump() {
  std::cout << "STRUCT " << mName << " : (";
  for (auto it: mStructElems) {
    std::cout << "\n  (";
    it->Dump();
    std::cout << ")";
    if (it != mStructElems.back())
      std::cout << ", ";
  }
  std::cout << ")" << std::endl;
}

void StructElem::Dump() {
  for (auto it: mDataVec) {
    it->Dump();
    if (it != mDataVec.back())
      std::cout << ", ";
  }
}

void StructData::Dump() {
  switch (mKind) {
    case DK_Char:
      std::cout << "'" << mValue.mChar << "'";
      break;
    case DK_Int:
      std::cout << mValue.mInt;
      break;
    case DK_Float:
      std::cout << mValue.mFloat;
      break;
    case DK_Double:
      std::cout << mValue.mDouble;
      break;
    case DK_Name:
      std::cout << mValue.mName;
      break;
    case DK_String:
      std::cout << "\"" << mValue.mString << "\"";
      break;
  }
}

// Some STRUCT has certain requirement of order. Eg. the operator
// need to be sorted by the length of their keywords, with the longer
// keyword operator being ahead of shorter one. In this way, the language
// parser can find the correct operator if it find the first matching
// keyword. Take a look at '+' and '++'. '++' should be saved before '+'.
//
// 'idx' is the index of the element in the StructElem, and this element
// must be a string.
//
// The original data structure is a vector. The sorting is using a bubble
// sorting. To accomodate the frequent insertion of element, I'm using a list
// for temporary data structure. After it's done, it will be copied back to
// the original vector.
void StructBase::Sort(unsigned idx) {
  std::list<StructElem*> templist;
  std::vector<StructElem *>::iterator eit = mStructElems.begin();

  for (; eit != mStructElems.end(); eit++) {
    StructElem *elem = *eit;
    // insert the element into the templist, making the longer elem at front
    std::list<StructElem*>::iterator lit = templist.begin();
    for (; lit != templist.end(); lit++) {
      StructElem *tempelem = *lit;
      const char *elem_str = elem->GetString(idx);
      const char *tempelem_str = tempelem->GetString(idx);
      if (strlen(elem_str) >= strlen(tempelem_str))
        break;
    }
    templist.insert(lit, elem);
  }

  // copy list back into vector.
  mStructElems.clear();
  std::list<StructElem*>::iterator lit = templist.begin();
  for (; lit != templist.end(); lit++) {
    mStructElems.push_back(*lit);
  }
}

}
