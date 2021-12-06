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

#ifndef __AST_ADJ_HEADER__
#define __AST_ADJ_HEADER__

#include <stack>
#include <utility>
#include <unordered_map>
#include <unordered_set>
#include "ast_module.h"
#include "ast.h"
#include "ast_type.h"
#include "gen_astvisitor.h"

namespace maplefe {

class AST_ADJ {
 private:
  Module_Handler *mHandler;
  unsigned        mFlags;

 public:
  explicit AST_ADJ(Module_Handler *h, unsigned f) : mHandler(h), mFlags(f) {}
  ~AST_ADJ() {}

  void AdjustAST();
};

class AdjustASTVisitor : public AstVisitor {
 private:
  Module_Handler *mHandler;
  AST_INFO       *mInfo;
  unsigned       mFlags;
  bool           mUpdated;

 public:
  explicit AdjustASTVisitor(Module_Handler *h, unsigned f, bool base = false)
    : mHandler(h), mFlags(f), mUpdated(false), AstVisitor((f & FLG_trace_1) && base) {
      mInfo = mHandler->GetINFO();
      mInfo->SetNameAnonyStruct(true);
    }
  ~AdjustASTVisitor() = default;

  DeclNode *VisitDeclNode(DeclNode *node);
  ImportNode *VisitImportNode(ImportNode *node);
  ExportNode *VisitExportNode(ExportNode *node);
  CondBranchNode *VisitCondBranchNode(CondBranchNode *node);
  ForLoopNode *VisitForLoopNode(ForLoopNode *node);
  LambdaNode *VisitLambdaNode(LambdaNode *node);
  IdentifierNode *VisitIdentifierNode(IdentifierNode *node);
  NamespaceNode *VisitNamespaceNode(NamespaceNode *node);
  StructNode *VisitStructNode(StructNode *node);
  StructLiteralNode *VisitStructLiteralNode(StructLiteralNode *node);
  ClassNode *VisitClassNode(ClassNode *node);
  InterfaceNode *VisitInterfaceNode(InterfaceNode *node);
  FunctionNode *VisitFunctionNode(FunctionNode *node);
  UserTypeNode *VisitUserTypeNode(UserTypeNode *node);
  TypeAliasNode *VisitTypeAliasNode(TypeAliasNode *node);
  LiteralNode *VisitLiteralNode(LiteralNode *node);
  UnaOperatorNode *VisitUnaOperatorNode(UnaOperatorNode *node);
};

}
#endif
