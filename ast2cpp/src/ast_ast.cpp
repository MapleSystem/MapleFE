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
  CollectASTInfo();
  AdjustAST();

  if (mTrace) {
    for(auto it: gModule.mTrees) {
      TreeNode *tnode = it->mRootNode;
      tnode->Dump(0);
      std::cout << std::endl;
    }
  }
}

// this calcuates mNodeId2BbMap
void AST_AST::CollectASTInfo() {
  if (mTrace) std::cout << "============== CollectASTInfo ==============" << std::endl;
  AST_Function *func = mHandler->GetFunction();
  MASSERT(func && "null func");
  std::deque<AST_BB *> working_list;
  std::unordered_set<unsigned> done_list;

  working_list.push_back(func->GetEntryBB());

  unsigned bitnum = 0;

  while(working_list.size()) {
    AST_BB *bb = working_list.front();
    MASSERT(bb && "null BB");
    unsigned bbid = bb->GetId();

    // skip bb already visited
    if (done_list.find(bbid) != done_list.end()) {
      working_list.pop_front();
      continue;
    }

    for (int i = 0; i < bb->GetSuccessorsNum(); i++) {
      working_list.push_back(bb->GetSuccessorAtIndex(i));
    }

    for (int i = 0; i < bb->GetStatementsNum(); i++) {
      TreeNode *node = bb->GetStatementAtIndex(i);
      mHandler->SetBbFromNodeId(node->GetNodeId(), bb);
    }

    done_list.insert(bbid);
    working_list.pop_front();
  }
}

void AST_AST::AdjustAST() {
  if (mTrace) std::cout << "============== AdjustAST ==============" << std::endl;
  AdjustASTVisitor visitor(mHandler, mTrace, true);
  AST_Function *func = mHandler->GetFunction();
  visitor.SetCurrentFunction(mHandler->GetFunction());
  visitor.SetCurrentBB(func->GetEntryBB());
  for(auto it: mHandler->GetASTModule()->mTrees) {
    visitor.Visit(it->mRootNode);
  }
}

DeclNode *AdjustASTVisitor::VisitDeclNode(DeclNode *node) {
  TreeNode *var = node->GetVar();

  // Check if need to split Decl
  MASSERT(var->IsIdentifier() && "var not Identifier");

  // move Init on Identifier to Decl
  IdentifierNode *inode = static_cast<IdentifierNode *>(var);
  TreeNode *init = inode->GetInit();
  if (init) {
    node->SetInit(init);
    inode->ClearInit();
  }
  return node;
}

ExportNode *AdjustASTVisitor::VisitExportNode(ExportNode *node) {
  if (node->GetPairsNum() == 1) {
    XXportAsPairNode *p = node->GetPair(0);
    TreeNode *before = p->GetBefore();
    TreeNode *after = p->GetAfter();
    if (before) {
      if (!before->IsIdentifier()) {
        IdentifierNode *n = (IdentifierNode*)gTreePool.NewTreeNode(sizeof(IdentifierNode));
        switch (before->GetKind()) {
          case NK_Function: {
            FunctionNode *func = static_cast<FunctionNode *>(before);
            new (n) IdentifierNode(func->GetName());
            // update p
            p->SetBefore(n);
            // insert before into BB
            AST_BB *bb = mHandler->GetBbFromNodeId(node->GetNodeId());
            bb->InsertStmtAfter(before, node);
            // insert before into AST
            TreeNode *parent = node->GetParent();
            if (parent) {
              if (parent->IsBlock()) {
                BlockNode *blk = static_cast<BlockNode *>(parent);
                blk->InsertStmtAfter(before, node);
              }
            } else {
              // module
            }
            break;
          }
          case NK_Class: {
            break;
          }
        }
      }
    }
  }
  return node;
}

}
