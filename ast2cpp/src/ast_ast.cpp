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

#include <stack>
#include <set>
#include "ast_handler.h"
#include "ast_ast.h"

namespace maplefe {

void AST_AST::Build() {
  SplitDecl();
  AdjustDeclInit();
}

void AST_AST::SplitDecl() {
  if (mTrace) std::cout << "============== SplitDecl ==============" << std::endl;
  DeclSplitVisitor visitor(mHandler, mTrace, true);
  AST_Function *func = mHandler->GetFunction();
  visitor.SetCurrentFunction(mHandler->GetFunction());
  visitor.SetCurrentBB(func->GetEntryBB());
  for(auto it: mHandler->GetASTModule()->mTrees) {
    visitor.Visit(it->mRootNode);
  }
}

DeclNode *DeclSplitVisitor::VisitDeclNode(DeclNode *node) {
  MASSERT(node->GetVar()->IsIdentifier() && "need to split VarList decl");
  return node;
}

void AST_AST::AdjustDeclInit() {
  if (mTrace) std::cout << "============== AdjustDeclInit ==============" << std::endl;
  DeclInitVisitor visitor(mHandler, mTrace, true);
  AST_Function *func = mHandler->GetFunction();
  visitor.SetCurrentFunction(mHandler->GetFunction());
  visitor.SetCurrentBB(func->GetEntryBB());
  for(auto it: mHandler->GetASTModule()->mTrees) {
    visitor.Visit(it->mRootNode);
  }
}

DeclNode *DeclInitVisitor::VisitDeclNode(DeclNode *node) {
  TreeNode *var = node->GetVar();
  MASSERT(var->IsIdentifier() && "var not Identifier");
  IdentifierNode *inode = static_cast<IdentifierNode *>(var);
  TreeNode *init = inode->GetInit();
  if (init) {
    node->SetInit(inode->GetInit());
    inode->ClearInit();
  }
  return node;
}

}
