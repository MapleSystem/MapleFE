/*
* Copyright (C) [2021] Futurewei Technologies, Inc. All rights reverved.
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

#ifndef __AST_INFO_HEADER__
#define __AST_INFO_HEADER__

#include <stack>
#include <utility>
#include <unordered_map>
#include <unordered_set>
#include "ast_module.h"
#include "ast.h"
#include "ast_type.h"
#include "gen_astvisitor.h"

namespace maplefe {

class FindStrIdxVisitor;

class AST_INFO {
 private:
  Module_Handler *mHandler;
  unsigned        mFlags;
  unsigned        mNum;
  bool            mNameAnonyStruct;
  unsigned        mPass;
  FindStrIdxVisitor *mStrIdxVisitor;

  std::unordered_set<unsigned> mReachableBbIdx;;
  std::unordered_map<unsigned, std::unordered_set<TreeNode *>> mFieldNum2StructNodeMap;
  std::unordered_map<unsigned, SmallVector<TreeNode*>> mStructId2FieldsMap;
  std::unordered_map<unsigned, TreeNode *> mStrIdx2StructMap;
  std::unordered_set<unsigned> mTypeParamStrIdxSet;
  std::unordered_set<unsigned> mWithTypeParamNodeSet;

  void AddField(unsigned nid, TreeNode *node);

 public:
  explicit AST_INFO(Module_Handler *h, unsigned f) : mHandler(h), mFlags(f), mNum(1),
           mNameAnonyStruct(false) {}
  ~AST_INFO() {}

  void CollectInfo();

  unsigned GetPass() { return mPass; }
  TypeId GetTypeId(TreeNode *node);
  unsigned GetFieldsSize(TreeNode *node, bool native = false);
  TreeNode *GetField(TreeNode *node, unsigned i, bool native = false);
  void AddField(TreeNode *container, TreeNode *node);
  TreeNode *GetField(unsigned nid, unsigned stridx);
  unsigned GetSuperSize(TreeNode *node, unsigned idx);
  TreeNode *GetSuper(TreeNode *node, unsigned i, unsigned idx);

  void SetStrIdx2Struct(unsigned stridx, TreeNode *node) { mStrIdx2StructMap[stridx] = node; }
  TreeNode *GetStructFromStrIdx(unsigned stridx) { return mStrIdx2StructMap[stridx]; }

  TreeNode *GetCanonicStructNode(TreeNode *node);

  IdentifierNode *CreateIdentifierNode(unsigned stridx);
  UserTypeNode *CreateUserTypeNode(unsigned stridx, ASTScope *scope = NULL);
  UserTypeNode *CreateUserTypeNode(IdentifierNode *node);
  TypeAliasNode *CreateTypeAliasNode(TreeNode *to, TreeNode *from);
  StructNode *CreateStructFromStructLiteral(StructLiteralNode *node);

  TreeNode *GetAnonymousStruct(TreeNode *node);

  bool IsInterface(TreeNode *node);
  bool IsTypeIdCompatibleTo(TypeId field, TypeId target);
  bool IsTypeCompatible(TreeNode *node1, TreeNode *node2);
  bool IsFieldCompatibleTo(TreeNode *from, TreeNode *to);

  void SetNameAnonyStruct(bool b) { mNameAnonyStruct = b; }
  bool GetNameAnonyStruct() { return mNameAnonyStruct; }

  template <typename T1, typename T2> void SortFields(T1 *node);
  template <typename T1> void ExtendFields(T1 *node, TreeNode *sup);

  bool WithStrIdx(TreeNode *node, unsigned stridx);
  bool WithTypeParam(TreeNode *node);
  bool WithTypeParamFast(TreeNode *node);
  void InsertTypeParamStrIdx(unsigned stridx) { mTypeParamStrIdxSet.insert(stridx); }
  void InsertWithTypeParamNode(TreeNode *node) { mWithTypeParamNodeSet.insert(node->GetNodeId()); }

  void SetTypeId(TreeNode *node, TypeId tid);
  void SetTypeIdx(TreeNode *node, unsigned tidx);
};

class FillNodeInfoVisitor : public AstVisitor {
 private:
  Module_Handler *mHandler;
  AST_INFO       *mInfo;
  unsigned       mFlags;
  bool           mUpdated;

 public:
  explicit FillNodeInfoVisitor(Module_Handler *h, unsigned f, bool base = false)
    : mHandler(h), mFlags(f), mUpdated(false), AstVisitor((f & FLG_trace_1) && base) {
      mInfo= mHandler->GetINFO();
    }
  ~FillNodeInfoVisitor() = default;

  LiteralNode *VisitLiteralNode(LiteralNode *node);
  PrimTypeNode *VisitPrimTypeNode(PrimTypeNode *node);
  UserTypeNode *VisitUserTypeNode(UserTypeNode *node);
  IdentifierNode *VisitIdentifierNode(IdentifierNode *node);
  FunctionNode *VisitFunctionNode(FunctionNode *node);
};

class ClassStructVisitor : public AstVisitor {
 private:
  Module_Handler *mHandler;
  AST_INFO       *mInfo;
  unsigned       mFlags;
  bool           mUpdated;

 public:
  explicit ClassStructVisitor(Module_Handler *h, unsigned f, bool base = false)
    : mHandler(h), mFlags(f), mUpdated(false), AstVisitor((f & FLG_trace_1) && base) {
      mInfo= mHandler->GetINFO();
    }
  ~ClassStructVisitor() = default;

  StructLiteralNode *VisitStructLiteralNode(StructLiteralNode *node);
  StructNode *VisitStructNode(StructNode *node);
  ClassNode *VisitClassNode(ClassNode *node);
  InterfaceNode *VisitInterfaceNode(InterfaceNode *node);
  TypeParameterNode *VisitTypeParameterNode(TypeParameterNode *node);
};

class FindStrIdxVisitor : public AstVisitor {
 private:
  Module_Handler *mHandler;
  AST_INFO       *mInfo;
  unsigned       mFlags;
  unsigned       mStrIdx;
  bool           mFound;

 public:
  explicit FindStrIdxVisitor(Module_Handler *h, unsigned f, bool base = false)
    : mHandler(h), mFlags(f), mStrIdx(0), mFound(false),
      AstVisitor((f & FLG_trace_1) && base) {
      mInfo = mHandler->GetINFO();
    }
  ~FindStrIdxVisitor() = default;

  void Init(unsigned stridx) { mStrIdx = stridx; mFound = false; }
  bool GetFound() { return mFound; }
  IdentifierNode *VisitIdentifierNode(IdentifierNode *node);
};

}
#endif
