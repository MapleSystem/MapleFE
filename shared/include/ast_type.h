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
// The type in AST doesn't have any physical representation. It's merely for
// syntax validation and potential optimizations for future R&D project.
//
// Each language should define its own type system, and its own validation rules.
// We only define two categories of types in this file, Primitive and User types.
//
//
//                    About the User Type
//
// The user type could come from class, struct, interface, ...
// To accomodate the above multiple cases, we simply put TreeNode*
// as the type info. And there are some issues.
// (1) During parsing, each instance of identifier is given a new tree node,
//     and multiple instance of a same user type are treated as different types.
// (2) Of course, situation in (1) is wrong, we need consolidate all these
//     instance into one single same type. This has to be done when the tree
//     is done creation.
// (3) To consolidate the multiple instances of a type symbol, the scope
//     info has to kick in. This is another issue.
// Right now, I just let each instance represent a separate type, and will
// come back to this.
//
// We define 3 different types.
// 1. UserType
//    It's an identifier which defines a class, interface, struct, etc.
//    It also includes DimensionNode to tell if it's array.
// 2. PrimType
//    This is coming from language's type keyword which are primitive types.
//    PrimType-s have limited number, and we pre-created AST nodes for them.
//    All same prim type nodes are pointing to the same one.
// 3. PrimArrayType
//    This is a special case. e.g. int[]
//
// The reason we split primary types into PrimType and PrimArrayType is to
// share the same PrimType since they can be predefined in the Prim Pool.
//
//////////////////////////////////////////////////////////////////////////

#ifndef __AST_TYPE_H__
#define __AST_TYPE_H__

#include "ruletable.h"
#include "mempool.h"
#include "ast.h"
#include "ast_mempool.h"

namespace maplefe {

///////////////////////////////////////////////////////////////////////////////
//                          UserTypeNode
// User type is complicated in Typescript. It could a union or intersection
// of other types.
///////////////////////////////////////////////////////////////////////////////

enum UT_Type {
  UT_Regular, // the normal user type, it could be just a name.
  UT_Union,   // Union of two other types.
  UT_Inter,   // Intersection of other types.
};

class UserTypeNode : public TreeNode {
private:
  // A regular UT always has an Id (or name), or lambda, etc.
  // A union or intersection UT may or may not have an ID.
  TreeNode *mId;

  UT_Type   mType;
  DimensionNode *mDims;

  // the set of types in union or intersection.
  SmallVector<TreeNode*> mUnionInterTypes;

  // There are two scenarios type generic info are used.
  // 1. It's a type argument
  // 2. It's a type parameter. Type parameter may have default value.
  SmallVector<TreeNode*> mTypeGenerics;

  SmallVector<AttrId> mAttrs;

public:
  UserTypeNode(TreeNode *n) : TreeNode(NK_UserType),
    mId(n), mType(UT_Regular), mDims(NULL) { if(n) n->SetParent(this); }
  UserTypeNode() : UserTypeNode(NULL) {}
  ~UserTypeNode(){Release();}

  TreeNode* GetId() {return mId;}
  void SetId(TreeNode *n) {mId = n; if(n) n->SetParent(this);}

  unsigned  GetUnionInterTypesNum()                    {return mUnionInterTypes.GetNum();}
  void      AddUnionInterType(TreeNode *n);
  TreeNode* GetUnionInterType(unsigned i)              {return mUnionInterTypes.ValueAtIndex(i);}
  void      SetUnionInterType(unsigned i, TreeNode* n) {*(mUnionInterTypes.RefAtIndex(i)) = n;}

  unsigned  GetTypeGenericsNum()                    {return mTypeGenerics.GetNum();}
  void      AddTypeGeneric(TreeNode *n);
  TreeNode* GetTypeGeneric(unsigned i)              {return mTypeGenerics.ValueAtIndex(i);}
  void      SetTypeGeneric(unsigned i, TreeNode* n) {*(mTypeGenerics.RefAtIndex(i)) = n;}

  UT_Type GetType() {return mType;}
  void SetType(UT_Type t) {mType = t;}

  DimensionNode* GetDims()       {return mDims;}
  void SetDims(DimensionNode *d) {mDims = d;}

  unsigned GetDimsNum()          {return mDims->GetDimensionsNum();}
  bool     IsArray()             {return mDims && GetDimsNum() > 0;}
  unsigned AddDim(unsigned i = 0){mDims->AddDimension(i);}        // 0 means unspecified
  unsigned GetNthNum(unsigned n) {return mDims->GetDimension(n);} // 0 means unspecified.
  void     SetNthNum(unsigned n, unsigned i) {mDims->SetDimension(n, i);}

  // Attributes related
  unsigned GetAttrsNum() const        {return mAttrs.GetNum();}
  void     AddAttr(AttrId a)          {mAttrs.PushBack(a);}
  AttrId   GetAttrAtIndex(unsigned i) {return mAttrs.ValueAtIndex(i);}
  void     SetAttrAtIndex(unsigned i, AttrId n) {*(mAttrs.RefAtIndex(i)) = n;}

  bool TypeEquivalent(UserTypeNode *);

  void Release() {mTypeGenerics.Release(); mUnionInterTypes.Release();}
  void Dump(unsigned);
};

///////////////////////////////////////////////////////////////////////////////
//                          PrimTypeNode & PrimTypePool
// The size of PrimTypeNode is fixed, so it's good to use container for the storage.
// The PrimTypeNode pool is global, across all modules.
///////////////////////////////////////////////////////////////////////////////

class PrimTypeNode : public TreeNode {
private:
  TypeId    mPrimType; // primitive type
  bool      mIsUnique; // This is specifically for TS "unique symbol". TS creates many
                       // opaque syntax.
public:
  PrimTypeNode() : TreeNode(NK_PrimType), mIsUnique(false) {}
  ~PrimTypeNode(){}

  bool IsUnique()                 {return mIsUnique;}
  void SetIsUnique(bool b = true) {mIsUnique = b;}

  TypeId    GetPrimType()     {return mPrimType;}
  void SetPrimType(TypeId id) {mPrimType = id; }
  const char* GetTypeName();  // type name

  void Dump(unsigned);
};

class PrimArrayTypeNode : public TreeNode {
private:
  PrimTypeNode  *mPrim;
  DimensionNode *mDims;
  SmallVector<AttrId> mAttrs;

public:
  PrimArrayTypeNode() : TreeNode(NK_PrimArrayType), mPrim(NULL), mDims(NULL) {}
  ~PrimArrayTypeNode(){}

  void SetPrim(PrimTypeNode *p) {mPrim = p;}
  void SetDims(DimensionNode *d) {mDims = d;}
  PrimTypeNode*  GetPrim() {return mPrim;}
  DimensionNode* GetDims(){return mDims;}

  // Attributes related
  unsigned GetAttrsNum() const        {return mAttrs.GetNum();}
  void     AddAttr(AttrId a)          {mAttrs.PushBack(a);}
  AttrId   GetAttrAtIndex(unsigned i) {return mAttrs.ValueAtIndex(i);}
  void     SetAttrAtIndex(unsigned i, AttrId n) {*(mAttrs.RefAtIndex(i)) = n;}

  void Dump(unsigned);
};

class PrimTypePool {
private:
  SmallVector<PrimTypeNode*> mTypes;
public:
  PrimTypePool();
  ~PrimTypePool();

  void Init();
  PrimTypeNode* FindType(const char *keyword);
  PrimTypeNode* FindType(TypeId id);
};

// A global pool for Primitive TypeNodes.
extern PrimTypePool gPrimTypePool;

}
#endif
