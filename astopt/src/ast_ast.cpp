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
  RemoveDeadBlocks();
  AdjustAST();

  if (mTrace) {
    for(unsigned i = 0; i < gModule->GetTreesNum(); i++) {
      TreeNode *it = gModule->GetTree(i);
      it->Dump(0);
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

    mHandler->SetBbFromBbId(bbid, bb);

    done_list.insert(bbid);
    working_list.pop_front();
  }
}

void AST_AST::RemoveDeadBlocks() {
  std::set<AST_BB *> deadBb;
  for (auto it: mHandler->mBbId2BbMap) {
    for (int i = 0; i < it.second->GetPredecessorsNum(); i++) {
      AST_BB *pred = it.second->GetPredecessorAtIndex(i);
      unsigned pid = pred->GetId();
      if (mHandler->mBbId2BbMap.find(pid) == mHandler->mBbId2BbMap.end()) {
        deadBb.insert(pred);
      }
    }
  }
  for (auto it: deadBb) {
    if (mTrace) std::cout << "deleted BB :";
    for (int i = 0; i < it->GetSuccessorsNum(); i++) {
      AST_BB *bb = it->GetSuccessorAtIndex(i);
      bb->mPredecessors.Remove(it);
    }
    if (mTrace) std::cout << " BB" << it->GetId();
    it->~AST_BB();
  }
  if (mTrace) std::cout << std::endl;
}

void AST_AST::AdjustAST() {
  if (mTrace) std::cout << "============== AdjustAST ==============" << std::endl;
  AdjustASTVisitor visitor(mHandler, mTrace, true);
  AST_Function *func = mHandler->GetFunction();
  visitor.SetCurrentFunction(mHandler->GetFunction());
  visitor.SetCurrentBB(func->GetEntryBB());
  for(unsigned i = 0; i < mHandler->GetASTModule()->GetTreesNum(); i++) {
    TreeNode *it = mHandler->GetASTModule()->GetTree(i);
    visitor.Visit(it);
  }
}

DeclNode *AdjustASTVisitor::VisitDeclNode(DeclNode *node) {
  TreeNode *var = node->GetVar();

  // Check if need to split Decl
  MASSERT(var->IsIdentifier() && "var not Identifier");

  IdentifierNode *inode = static_cast<IdentifierNode *>(var);

  // copy StrIdx from Identifier to Decl
  if (node->GetStrIdx()) {
    node->SetStrIdx(inode->GetStrIdx());
    mUpdated = true;
  }

  // move Init from Identifier to Decl
  TreeNode *init = inode->GetInit();
  if (init) {
    node->SetInit(init);
    inode->ClearInit();
    mUpdated = true;
  }
  return node;
}

ExportNode *AdjustASTVisitor::VisitExportNode(ExportNode *node) {
  if (node->GetPairsNum() == 1) {
    XXportAsPairNode *p = node->GetPair(0);
    TreeNode *before = p->GetBefore();
    TreeNode *after = p->GetAfter();
    // TODO:
    if (0 && before) {
      if (!before->IsIdentifier()) {
        IdentifierNode *n = (IdentifierNode*)gTreePool.NewTreeNode(sizeof(IdentifierNode));
        switch (before->GetKind()) {
          case NK_Function: {
            FunctionNode *func = static_cast<FunctionNode *>(before);
            new (n) IdentifierNode(func->GetStrIdx());
            // update p
            p->SetBefore(n);
            mUpdated = true;
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

// if-then-else
CondBranchNode *AdjustASTVisitor::VisitCondBranchNode(CondBranchNode *node) {
  TreeNode *tn = VisitTreeNode(node->GetTrueBranch());
  if (tn && !tn->IsBlock()) {
    BlockNode *blk = (BlockNode*)gTreePool.NewTreeNode(sizeof(BlockNode));
    new (blk) BlockNode();
    blk->AddChild(tn);
    node->SetTrueBranch(blk);
    mUpdated = true;
  }
  tn = VisitTreeNode(node->GetFalseBranch());
  if (tn && !tn->IsBlock()) {
    BlockNode *blk = (BlockNode*)gTreePool.NewTreeNode(sizeof(BlockNode));
    new (blk) BlockNode();
    blk->AddChild(tn);
    node->SetFalseBranch(blk);
    mUpdated = true;
  }
  return node;
}

// for
ForLoopNode *AdjustASTVisitor::VisitForLoopNode(ForLoopNode *node) {
  TreeNode *tn = VisitTreeNode(node->GetBody());
  if (tn && !tn->IsBlock()) {
    BlockNode *blk = (BlockNode*)gTreePool.NewTreeNode(sizeof(BlockNode));
    new (blk) BlockNode();
    blk->AddChild(tn);
    node->SetBody(blk);
    mUpdated = true;
  }
  return node;
}
}
