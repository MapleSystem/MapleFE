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
#include "gen_type.h"   // to include the type keyword table, language specific
#include "ast.h"
#include "massert.h"

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
//                               ASTType                                //
//////////////////////////////////////////////////////////////////////////

const char* ASTType::GetName() {
  const char *name = NULL;
  if (IsPrim()) {
    // Get name from keyword table
    name = FindPrimTypeName(GetPrimType());
  } else if (IsUser()) {
    // Get name from Identifier stringpool
    IdentifierNode *node = GetIdentifier();
    name = node->GetName();
  }

  MASSERT(name && "Could not find prim type name!");
  return name;
}

//////////////////////////////////////////////////////////////////////////
//                           ASTTypePool                                //
//////////////////////////////////////////////////////////////////////////

// The global Pool for ASTType
ASTTypePool gASTTypePool;

ASTTypePool::ASTTypePool() {
  InitSystemTypes();
}

ASTTypePool::~ASTTypePool() {
  std::vector<ASTType*>::iterator it = mTypes.begin();
  for (; it != mTypes.end(); it++ ){
    ASTType *t = *it;
    delete t;
  }
  mTypes.clear();
}

void ASTTypePool::InitSystemTypes() {
  for (unsigned i = 0; i < TY_NA; i++) {
    char *addr = mMemPool.Alloc(sizeof(ASTType));
    ASTType *type = new (addr) ASTType();
    type->SetPrimType((TypeId)i);
    mTypes.push_back(type);
  }
}

// It's caller's duty to check if the return value is NULL.
ASTType* ASTTypePool::FindPrimType(const char *keyword) {
  TypeId id = FindPrimTypeId(keyword);
  if (id == TY_NA)
    return NULL;
  return FindPrimType(id);
}

// Just need search the first TY_NA since primitive types are created
// at the beginning.
ASTType* ASTTypePool::FindPrimType(TypeId id) {
  for (unsigned i = 0; i < TY_NA; i++) {
    ASTType *type = mTypes[i];
    if (type->IsPrim() && type->GetPrimType() == id)
      return type;
  }
  MERROR("Cannot find the prim type of an TypeId.");
}

// Try to find the type of identifier 'node'.
// If not found, return NULL.
//
// [NOTE] Assume all types' IdentifierNode are the same. This requires
//        an extra pass to Reduce the IdentifierNode-s, so as to make
//        sure there is only one IdentifierNode for all appearance of a
//        type name.
ASTType* ASTTypePool::FindUserType(IdentifierNode *node) {
  for (unsigned i = 0; i < mTypes.size(); i++) {
    ASTType *type = mTypes[i];
    if (type->IsUser() && type->GetIdentifier() == node)
      return type;
  }
  return NULL;
}

// Try to find the type of identifier 'node'.
// If not found, create one.
//
// [NOTE] Assume all types' IdentifierNode are the same. This requires
//        an extra pass to Reduce the IdentifierNode-s, so as to make
//        sure there is only one IdentifierNode for all appearance of a
//        type name.
ASTType* ASTTypePool::FindOrCreateUserType(IdentifierNode *node) {
  ASTType *t = FindUserType(node);
  if (t)
    return t;

  char *addr = mMemPool.Alloc(sizeof(ASTType));
  t = new (addr) ASTType();
  t->SetUserType(node);
  mTypes.push_back(t);
  return t;
}
