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
///////////////////////////////////////////////////////////////////////////////
// We need give each symbol, expression a type. The type in AST doesn't have
// any physical representation. It's merely for syntax validation and potential
// optimizations for future R&D project.
//
// Each language should define its own type system, and its own validation rules.
// We only define two categories of types in this file, Primitive and Named types.
///////////////////////////////////////////////////////////////////////////////

#ifndef __AST_TYPE_H__
#define __AST_TYPE_H__

#include "ruletable.h"   // to include TypeId
#include "mempool.h"

class IdentifierNode;

enum TypeCategory {
  Primitive,
  User
};

class ASTType {
public:
  TypeCategory mCat;
  union {
    IdentifierNode *mIdentifier;  // user type
    TypeId          mPrimType;    // primitive type
  }mType;

public:
  ASTType() {mCat = Primitive;}
  ~ASTType(){}

public:
  bool IsPrim() {return mCat == Primitive;}
  bool IsUser() {return mCat == User;}

  IdentifierNode* GetIdentifier() {return mType.mIdentifier;}
  TypeId          GetPrimType()   {return mType.mPrimType;}
  void SetPrimType(TypeId id)            { mCat = Primitive; mType.mPrimType = id; }
  void SetUserType(IdentifierNode *node) { mCat = User; mType.mIdentifier = node; }

  const char* GetName();  // type name
};

///////////////////////////////////////////////////////////////////////////////
//                      ASTTypePool
// The size of ASTType is fixed, so it's good to use MemPool for the storage.
// The ASTType pool is global, across all modules.
///////////////////////////////////////////////////////////////////////////////

class ASTTypePool {
private:
  MemPool               mMemPool;
  std::vector<ASTType*> mTypes;

  void InitSystemTypes();

public:
  ASTTypePool();
  ~ASTTypePool();

  ASTType* FindUserType(IdentifierNode *node);
  ASTType* FindOrCreateUserType(IdentifierNode *node);

  ASTType* FindPrimType(const char *keyword);
  ASTType* FindPrimType(TypeId id);
};

// A global pool for all ASTType-s.
extern ASTTypePool gASTTypePool;

#endif
