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

#include "ast.h"
#include "ruletable.h"
#include "ast_attr.h"
#include "massert.h"

namespace maplefe {

// Inquiry function for language specific attr keyword
const char* FindAttrKeyword(AttrId id) {
  for (unsigned i = 0; i < AttrKeywordTableSize; i++) {
    if (AttrKeywordTable[i].mId == id)
      return AttrKeywordTable[i].mText;
  }
  return NULL;
}

// Inquiry function for language specific attr keyword
AttrId FindAttrId(const char *keyword) {
  for (unsigned i = 0; i < AttrKeywordTableSize; i++) {
    if (strncmp(AttrKeywordTable[i].mText, keyword, strlen(keyword)) == 0
        && strlen(keyword) == strlen(AttrKeywordTable[i].mText))
      return AttrKeywordTable[i].mId;
  }
  return ATTR_NA;
}

//////////////////////////////////////////////////////////////////////////
//                              AttrPool                                //
//////////////////////////////////////////////////////////////////////////

// The global Pool for AttrNode
AttrPool gAttrPool;

AttrPool::AttrPool() {
  InitAttrs();
}

AttrPool::~AttrPool() {
  mAttrs.Release();
}

void AttrPool::InitAttrs() {
  for (unsigned i = 0; i < ATTR_NA; i++) {
    AttrNode node;
    node.SetId((AttrId)i);
    mAttrs.PushBack(node);
  }
}

// This is used to build AST tree node for attribute.
// It's caller's duty to check if the return value is NULL.
AttrNode* AttrPool::GetAttrNode(const char *keyword) {
  AttrId id = FindAttrId(keyword);
  if (id == ATTR_NA)
    return NULL;
  return GetAttrNode(id);
}

AttrId AttrPool::GetAttrId(const char *keyword) {
  AttrId id = FindAttrId(keyword);
  return id;
}

AttrNode* AttrPool::GetAttrNode(AttrId id) {
  for (unsigned i = 0; i < mAttrs.GetNum(); i++) {
    AttrNode *n = mAttrs.RefAtIndex(i);
    if (n->GetId() == id)
      return n;
  }

  return NULL;
}
}
