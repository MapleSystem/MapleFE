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

#ifndef __AST_AST_HEADER__
#define __AST_AST_HEADER__

#include <stack>
#include <utility>
#include <unordered_map>
#include <unordered_set>
#include "ast_module.h"
#include "ast.h"
#include "ast_type.h"
#include "gen_astvisitor.h"

namespace maplefe {

class AST_AST {
 private:
  Module_Handler *mHandler;
  unsigned        mFlags;
  unsigned        mNum;
  bool            mNameAnonyStruct;

  std::unordered_set<unsigned> mReachableBbIdx;;
  std::unordered_map<unsigned, std::unordered_set<TreeNode *>> mFieldNum2StructNodeMap;

 public:
  explicit AST_AST(Module_Handler *h, unsigned f) : mHandler(h), mFlags(f), mNum(1),
           mNameAnonyStruct(false) {}
  ~AST_AST() {}

  void AdjustAST();

  unsigned GetFieldSize(TreeNode *node);
  TreeNode *GetField(TreeNode *node, unsigned i);
  TreeNode *GetCanonicStructNode(TreeNode *node);

  IdentifierNode *CreateIdentifierNode(unsigned stridx);
  UserTypeNode *CreateUserTypeNode(unsigned stridx);
  UserTypeNode *CreateUserTypeNode(IdentifierNode *node);
  TypeAliasNode *CreateTypeAliasNode(TreeNode *to, TreeNode *from);
  StructNode *CreateStructFromStructLiteral(StructLiteralNode *node);

  TreeNode *GetAnonymousStruct(TreeNode *node);

  bool IsInterface(TreeNode *node);
  bool IsFieldCompatibleTo(IdentifierNode *from, IdentifierNode *to);

  void SetNameAnonyStruct(bool b) { mNameAnonyStruct = b; }
  bool GetNameAnonyStruct() { return mNameAnonyStruct; }
};

class CollectClassStructVisitor : public AstVisitor {
 private:
  Module_Handler *mHandler;
  AST_AST        *mAst;
  unsigned       mFlags;
  bool           mUpdated;

 public:
  explicit CollectClassStructVisitor(Module_Handler *h, unsigned f, bool base = false)
    : mHandler(h), mFlags(f), mUpdated(false), AstVisitor((f & FLG_trace_1) && base) {
      mAst = mHandler->GetAST();
    }
  ~CollectClassStructVisitor() = default;

  virtual StructNode *VisitStructNode(StructNode *node);
  virtual ClassNode *VisitClassNode(ClassNode *node);
  virtual InterfaceNode *VisitInterfaceNode(InterfaceNode *node);
};

class SortFieldsVisitor : public AstVisitor {
 private:
  Module_Handler *mHandler;
  AST_AST        *mAst;
  unsigned       mFlags;
  bool           mUpdated;

 public:
  explicit SortFieldsVisitor(Module_Handler *h, unsigned f, bool base = false)
    : mHandler(h), mFlags(f), mUpdated(false), AstVisitor((f & FLG_trace_1) && base) {
      mAst = mHandler->GetAST();
      mAst->SetNameAnonyStruct(false);
    }
  ~SortFieldsVisitor() = default;

  StructNode *VisitStructNode(StructNode *node);
  StructLiteralNode *VisitStructLiteralNode(StructLiteralNode *node);
  ClassNode *VisitClassNode(ClassNode *node);
  InterfaceNode *VisitInterfaceNode(InterfaceNode *node);
};

class AdjustASTVisitor : public AstVisitor {
 private:
  Module_Handler *mHandler;
  AST_AST        *mAst;
  unsigned       mFlags;
  bool           mUpdated;

 public:
  explicit AdjustASTVisitor(Module_Handler *h, unsigned f, bool base = false)
    : mHandler(h), mFlags(f), mUpdated(false), AstVisitor((f & FLG_trace_1) && base) {
      mAst = mHandler->GetAST();
      mAst->SetNameAnonyStruct(true);
    }
  ~AdjustASTVisitor() = default;

  DeclNode *VisitDeclNode(DeclNode *node);
  ExportNode *VisitExportNode(ExportNode *node);
  CondBranchNode *VisitCondBranchNode(CondBranchNode *node);
  ForLoopNode *VisitForLoopNode(ForLoopNode *node);
  LambdaNode *VisitLambdaNode(LambdaNode *node);
  IdentifierNode *VisitIdentifierNode(IdentifierNode *node);
  StructNode *VisitStructNode(StructNode *node);
  StructLiteralNode *VisitStructLiteralNode(StructLiteralNode *node);
  ClassNode *VisitClassNode(ClassNode *node);
  InterfaceNode *VisitInterfaceNode(InterfaceNode *node);
  FunctionNode *VisitFunctionNode(FunctionNode *node);
  UserTypeNode *VisitUserTypeNode(UserTypeNode *node);
  TypeAliasNode *VisitTypeAliasNode(TypeAliasNode *node);
};

}
#endif
