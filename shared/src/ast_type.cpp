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

#include "ast_type.h"
#include "ruletable.h"
#include "rule_summary.h"
#include "ast.h"
#include "massert.h"

namespace maplefe {

//////////////////////////////////////////////////////////////////////////
//                           UserTypeNode                               //
//////////////////////////////////////////////////////////////////////////

void UserTypeNode::AddUnionInterType(TreeNode *args) {
  if (args->IsIdentifier() ||
      args->IsPrimType() ||
      args->IsPrimArrayType() ||
      args->IsUserType() ||
      args->IsLiteral() ||
      args->IsLambda() ||
      args->IsTypeOf() ||
      args->IsTupleType() ||
      args->IsArrayElement() ||
      args->IsConditionalType() ||
      args->IsNew() ||
      args->IsKeyOf() ||
      args->IsStruct()) {
    mUnionInterTypes.PushBack(args);
    SETPARENT(args);
  } else if (args->IsPass()) {
    PassNode *p = (PassNode*)args;
    for (unsigned i = 0; i < p->GetChildrenNum(); i++) {
      TreeNode *a = p->GetChild(i);
      AddTypeGeneric(a);
    }
  } else {
    MASSERT(0 && "Unsupported tree node in UserTypeNode::AddUnionInterType()");
  }
}

void UserTypeNode::AddTypeGeneric(TreeNode *args) {
  if (args->IsIdentifier() ||
      args->IsPrimType() ||
      args->IsPrimArrayType() ||
      args->IsUserType() ||
      args->IsTypeParameter() ||
      args->IsLiteral() ||
      args->IsTypeOf() ||
      args->IsArrayElement() ||
      args->IsStruct() ||
      args->IsTupleType() ||
      args->IsLambda() ||
      args->IsKeyOf() ||
      args->IsField() ||
      args->IsConditionalType() ||
      args->IsInfer()) {
    mTypeGenerics.PushBack(args);
    SETPARENT(args);
  } else if (args->IsPass()) {
    PassNode *p = (PassNode*)args;
    for (unsigned i = 0; i < p->GetChildrenNum(); i++) {
      TreeNode *a = p->GetChild(i);
      AddTypeGeneric(a);
    }
  } else {
    MASSERT(0 && "Unsupported tree node in UserTypeNode::AddTypeArgs()");
  }
}

// If the two UserTypeNodes are equivalent.
bool UserTypeNode::TypeEquivalent(UserTypeNode *type) {
  // For now, I just check the name. The name is in the global string pool,
  // so two same names should be in the same address.
  if (GetStrIdx() == type->GetStrIdx())
    return true;
  else
    return false;
}

void UserTypeNode::Dump(unsigned ind) {
  if (mType == UT_Union)
    DUMP0_NORETURN("union ");
  else if (mType == UT_Inter)
    DUMP0_NORETURN("intersect ");

  if (mId)
    mId->Dump(0);

  unsigned size = mTypeGenerics.GetNum();
  if (size > 0) {
    DUMP0_NORETURN('<');
    for (unsigned i = 0; i < size; i++) {
      TreeNode *inode = mTypeGenerics.ValueAtIndex(i);
      inode->Dump(0);
      if (i < size - 1)
        DUMP0_NORETURN(',');
    }
    DUMP0_NORETURN('>');
  }

  size = mUnionInterTypes.GetNum();
  if (size > 0) {
    DUMP0_NORETURN(" = ");
    for (unsigned i = 0; i < size; i++) {
      TreeNode *inode = mUnionInterTypes.ValueAtIndex(i);
      inode->Dump(0);
      if (i < size - 1) {
        if (mType == UT_Union)
          DUMP0_NORETURN(" | ");
        else if (mType == UT_Inter)
          DUMP0_NORETURN(" & ");
      }
    }
  }

  if (mDims) {
    for (unsigned i = 0; i < GetDimsNum(); i++)
      DUMP0_NORETURN("[]");
  }
}

//////////////////////////////////////////////////////////////////////////
//                          PrimArrayTypeNode                           //
//////////////////////////////////////////////////////////////////////////

void PrimArrayTypeNode::Dump(unsigned ind) {
  DUMP0_NORETURN("prim array-TBD");
}

//////////////////////////////////////////////////////////////////////////
//                          Local functions                             //
//////////////////////////////////////////////////////////////////////////

static const char* FindPrimTypeName(TypeId id) {
  for (unsigned i = 0; i < TypeKeywordTableSize; i++) {
    if (TypeKeywordTable[i].mId == id)
      return TypeKeywordTable[i].mText;
  }
  return NULL;
}

static TypeId FindPrimTypeId(const char *keyword) {
  for (unsigned i = 0; i < TypeKeywordTableSize; i++) {
    if (strncmp(TypeKeywordTable[i].mText, keyword, strlen(keyword)) == 0
        && strlen(keyword) == strlen(TypeKeywordTable[i].mText))
      return TypeKeywordTable[i].mId;
  }
  return TY_NA;
}

//////////////////////////////////////////////////////////////////////////
//                             PrimTypeNode                             //
//////////////////////////////////////////////////////////////////////////

const char* PrimTypeNode::GetTypeName() {
  const char *name = FindPrimTypeName(GetPrimType());
  return name;
}

void PrimTypeNode::Dump(unsigned indent) {
  if (mIsUnique)
    DUMP0_NORETURN("unique ");

  DumpIndentation(indent);
  DUMP0_NORETURN(GetTypeName());
}

//////////////////////////////////////////////////////////////////////////
//                           PrimTypePool                               //
//////////////////////////////////////////////////////////////////////////

// The global Pool for
PrimTypePool gPrimTypePool;

PrimTypePool::PrimTypePool() {}

PrimTypePool::~PrimTypePool() {
  mTypes.Release();
}

void PrimTypePool::Init() {
  for (unsigned i = 0; i < TypeKeywordTableSize; i++) {
    PrimTypeNode *n = (PrimTypeNode*)gTreePool.NewTreeNode(sizeof(PrimTypeNode));
    new (n) PrimTypeNode();
    n->SetPrimType((TypeId)TypeKeywordTable[i].mId);
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
  for (unsigned i = 0; i < TypeKeywordTableSize; i++) {
    PrimTypeNode *type_float = mTypes.ValueAtIndex(6);
    PrimTypeNode *type = mTypes.ValueAtIndex(i);
    if (type->GetPrimType() == id)
      return type;
  }
  MASSERT(0 && "Cannot find the prim type of an TypeId.");
}
}
