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
// So we a TreeNode of the type identifier can be a type, and we don't have
// to give any special data struct for it. A user type is created as a treenode
// (IdentifierNode) at the beginning, but later we will do consolidation,
// and it may be turned into a function, struct, etc. So a TreeNode is good here.
//
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
  UT_Inter,   // Intersection of two other types.
  UT_Alias,   // Intersection of two other types.
};

class UserTypeNode : public TreeNode {
private:
  TreeNode *mId;  // A regular UT always has an Id (or name), or lambda, etc.
                  // A union or intersection UT may or may not have an ID.
  UT_Type   mType;
  TreeNode *mChildA;    // first child type in UT_Union or UT_Inter.
                        // or it's the orig type in UT_Alias.
  TreeNode *mChildB;    // seconf child type.

  DimensionNode *mDims;

  SmallVector<IdentifierNode*> mTypeArguments;

public:
  UserTypeNode() : mId(NULL), mChildA(NULL), mChildB(NULL), mType(UT_Regular), mDims(NULL) {
    mKind = NK_UserType;
  }
  UserTypeNode(TreeNode *n) : mId(n), mChildA(NULL), mChildB(NULL), mType(UT_Regular), mDims(NULL) {
    mKind = NK_UserType;
  }
  ~UserTypeNode(){Release();}

  TreeNode* GetId() {return mId;}
  void SetId(TreeNode *n) {mId = n;}

  const char* GetName() {return mId->GetName();}

  unsigned GetTypeArgumentsNum() {return mTypeArguments.GetNum();}
  IdentifierNode* GetTypeArgument(unsigned i) {return mTypeArguments.ValueAtIndex(i);}
  void     AddTypeArgs(TreeNode *n);

  UT_Type GetType() {return mType;}
  void SetType(UT_Type t) {mType = t;}

  TreeNode* GetChildA() {return mChildA;}
  TreeNode* GetChildB() {return mChildB;}
  void SetChildA(TreeNode *t) {mChildA = t;}
  void SetChildB(TreeNode *t) {mChildB = t;}

  void SetAlias(TreeNode *t) {mChildA = t; mType = UT_Alias;}

  DimensionNode* GetDims()       {return mDims;}
  void SetDims(DimensionNode *d) {mDims = d;}

  unsigned GetDimsNum()          {return mDims->GetDimensionsNum();}
  bool     IsArray()             {return mDims && GetDimsNum() > 0;}
  unsigned AddDim(unsigned i = 0){mDims->AddDim(i);}           // 0 means unspecified
  unsigned GetNthNum(unsigned n) {return mDims->GetDimension(n);} // 0 means unspecified.
  void     SetNthNum(unsigned n, unsigned i) {mDims->SetDimension(n, i);}

  bool TypeEquivalent(UserTypeNode *);

  void Release() {mTypeArguments.Release();}
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
public:
  PrimTypeNode() {mKind = NK_PrimType;}
  ~PrimTypeNode(){}

  TypeId    GetPrimType()     {return mPrimType;}
  void SetPrimType(TypeId id) {mPrimType = id; }
  const char* GetName();  // type name

  void Dump(unsigned);
};

class PrimArrayTypeNode : public TreeNode {
private:
  PrimTypeNode  *mPrim;
  DimensionNode *mDims;
public:
  PrimArrayTypeNode() : mPrim(NULL), mDims(NULL) {mKind = NK_PrimArrayType;}
  ~PrimArrayTypeNode(){}

  void SetPrim(PrimTypeNode *p) {mPrim = p;}
  void SetDims(DimensionNode *d) {mDims = d;}
  PrimTypeNode*  GetPrim() {return mPrim;}
  DimensionNode* GetDims(){return mDims;}

  void Dump(unsigned);
};

class PrimTypePool {
private:
  TreePool                   mTreePool;
  SmallVector<PrimTypeNode*> mTypes;

  void Init();

public:
  PrimTypePool();
  ~PrimTypePool();

  PrimTypeNode* FindType(const char *keyword);
  PrimTypeNode* FindType(TypeId id);
};

// A global pool for Primitive TypeNodes.
extern PrimTypePool gPrimTypePool;

}
#endif
