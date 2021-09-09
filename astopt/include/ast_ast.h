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
  Module_Handler  *mHandler;
  unsigned         mFlags;
  std::unordered_set<unsigned> mReachableBbIdx;;

 public:
  explicit AST_AST(Module_Handler *h, unsigned f) : mHandler(h), mFlags(f) {}
  ~AST_AST() {}

  void AdjustAST();
};

class AdjustASTVisitor : public AstVisitor {
 private:
  Module_Handler *mHandler;
  unsigned       mFlags;
  bool           mUpdated;
  unsigned       mNum;

  std::unordered_map<unsigned, std::unordered_set<TreeNode *>> mFieldNum2StructNodeIdMap;

 public:
  explicit AdjustASTVisitor(Module_Handler *h, unsigned f, bool base = false)
    : mHandler(h), mFlags(f), mUpdated(false), mNum(1),
      AstVisitor((f & FLG_trace_1) && base) {}
  ~AdjustASTVisitor() = default;

  unsigned GetFieldSize(TreeNode *node);
  TreeNode *GetField(TreeNode *node, unsigned i);
  TreeNode *GetCanonicStructNode(TreeNode *node);
  void AddAnonymousStruct(TreeNode *node);

  TreeNode *CreateTypeNodeFromName(IdentifierNode *node);
  TypeAliasNode *CreateTypeAlias(TreeNode *to, TreeNode *from);

  DeclNode *VisitDeclNode(DeclNode *node);
  ExportNode *VisitExportNode(ExportNode *node);
  CondBranchNode *VisitCondBranchNode(CondBranchNode *node);
  ForLoopNode *VisitForLoopNode(ForLoopNode *node);
  LambdaNode *VisitLambdaNode(LambdaNode *node);
  IdentifierNode *VisitIdentifierNode(IdentifierNode *node);
  StructNode *VisitStructNode(StructNode *node);
  ClassNode *VisitClassNode(ClassNode *node);
  InterfaceNode *VisitInterfaceNode(InterfaceNode *node);
  FunctionNode *VisitFunctionNode(FunctionNode *node);
  UserTypeNode *VisitUserTypeNode(UserTypeNode *node);
};

}
#endif
