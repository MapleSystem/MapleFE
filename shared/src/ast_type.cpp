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

#include "ast_type.h"
#include "gen_type.h"   // for language specific type keyword
#include "ruletable.h"
#include "ast.h"
#include "massert.h"

//////////////////////////////////////////////////////////////////////////
//                          Local functions                             //
//////////////////////////////////////////////////////////////////////////

static const char* FindPrimTypeName(TypeId id) {
  for (unsigned i = 0; i < TY_NA; i++) {
    if (TypeKeywordTable[i].mId == id)
      return TypeKeywordTable[i].mText;
  }
  return NULL;
}

static TypeId FindPrimTypeId(const char *keyword) {
  for (unsigned i = 0; i < TY_NA; i++) {
    if (strncmp(TypeKeywordTable[i].mText, keyword, strlen(keyword)) == 0
        && strlen(keyword) == strlen(TypeKeywordTable[i].mText))
      return TypeKeywordTable[i].mId;
  }
  return TY_NA;
}

//////////////////////////////////////////////////////////////////////////
//                             PrimTypeNode                             //
//////////////////////////////////////////////////////////////////////////

const char* PrimTypeNode::GetName() {
  const char *name = FindPrimTypeName(GetPrimType());
  return name;
}

void PrimTypeNode::Dump(unsigned indent) {
  DumpIndentation(indent);
  DUMP0_NORETURN(GetName());
}

//////////////////////////////////////////////////////////////////////////
//                           PrimTypePool                               //
//////////////////////////////////////////////////////////////////////////

// The global Pool for 
PrimTypePool gPrimTypePool;

PrimTypePool::PrimTypePool() {
  // 1024 per block could be better.
  mTreePool.SetBlockSize(1024);
  Init();
}

PrimTypePool::~PrimTypePool() {
  mTypes.Release();
}

void PrimTypePool::Init() {
  for (unsigned i = 0; i < TY_NA; i++) {
    PrimTypeNode *n = (PrimTypeNode*)mTreePool.NewTreeNode(sizeof(PrimTypeNode));
    new (n) PrimTypeNode();
    n->SetPrimType((TypeId)i);
    mTypes.PushBack(n);
  }
}

// It's caller's duty to check if the return value is NULL.
PrimTypeNode* PrimTypePool::FindType(const char *keyword) {
  TypeId id = FindPrimTypeId(keyword);
  if (id == TY_NA)
    return NULL;
  return FindType(id);
}

PrimTypeNode* PrimTypePool::FindType(TypeId id) {
  for (unsigned i = 0; i < TY_NA; i++) {
    PrimTypeNode *type = mTypes.ValueAtIndex(i);
    if (type->GetPrimType() == id)
      return type;
  }
  MERROR("Cannot find the prim type of an TypeId.");
}
