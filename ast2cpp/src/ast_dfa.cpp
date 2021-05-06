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
#include "ast_cfg.h"
#include "ast_dfa.h"

namespace maplefe {

void AST_DFA::Build() {
  SplitDecl();
  BuildReachDefIn();
}

void AST_DFA::SplitDecl() {
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
  // AstVisitor::VisitDeclNode(node);
  // node->Dump(0);
  return node;
}

void AST_DFA::BuildReachDefIn() {
  if (mTrace) std::cout << "============== BuildReachDefIn ==============" << std::endl;
  ReachDefInVisitor visitor(mHandler, mTrace, true);
  AST_Function *func = mHandler->GetFunction();
  visitor.SetCurrentFunction(mHandler->GetFunction());
  visitor.SetCurrentBB(func->GetEntryBB());
  for(auto it: mHandler->GetASTModule()->mTrees) {
    visitor.Visit(it->mRootNode);
  }
}

DeclNode *ReachDefInVisitor::VisitDeclNode(DeclNode *node) {
  // AstVisitor::VisitDeclNode(node);
  // node->Dump(0);
  return node;
}

}
