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

void AST_AST::ASTCollectAndDBRemoval() {
  CollectASTInfo();
  RemoveDeadBlocks();

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
  AstFunction *func = mHandler->GetFunction();
  MASSERT(func && "null func");
  std::deque<AstBasicBlock *> working_list;
  std::unordered_set<unsigned> done_list;

  working_list.push_back(func->GetEntryBB());

  unsigned bitnum = 0;

  while(working_list.size()) {
    AstBasicBlock *bb = working_list.front();
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
  std::set<AstBasicBlock *> deadBb;
  for (auto it: mHandler->mBbId2BbMap) {
    for (int i = 0; i < it.second->GetPredecessorsNum(); i++) {
      AstBasicBlock *pred = it.second->GetPredecessorAtIndex(i);
      unsigned pid = pred->GetId();
      if (mHandler->mBbId2BbMap.find(pid) == mHandler->mBbId2BbMap.end()) {
        deadBb.insert(pred);
      }
    }
  }
  for (auto it: deadBb) {
    if (mTrace) std::cout << "deleted BB :";
    for (int i = 0; i < it->GetSuccessorsNum(); i++) {
      AstBasicBlock *bb = it->GetSuccessorAtIndex(i);
      bb->mPredecessors.Remove(it);
    }
    if (mTrace) std::cout << " BB" << it->GetId();
    it->~AstBasicBlock();
  }
  if (mTrace) std::cout << std::endl;
}

void AST_AST::AdjustAST() {
  if (mTrace) std::cout << "============== AdjustAST ==============" << std::endl;
  AdjustASTVisitor visitor(mHandler, mTrace, true);
  for(unsigned i = 0; i < mHandler->GetASTModule()->GetTreesNum(); i++) {
    TreeNode *it = mHandler->GetASTModule()->GetTree(i);
    visitor.Visit(it);
  }
}

// move init from identifier to decl
// copy stridx to decl
DeclNode *AdjustASTVisitor::VisitDeclNode(DeclNode *node) {
  TreeNode *var = node->GetVar();

  // Check if need to split Decl
  MASSERT(var->IsIdentifier() && "var not Identifier");

  IdentifierNode *inode = static_cast<IdentifierNode *>(var);

  // copy stridx from Identifier to Decl
  unsigned stridx = inode->GetStrIdx();
  if (stridx) {
    node->SetStrIdx(stridx);
    mUpdated = true;
  }

  // move init from Identifier to Decl
  TreeNode *init = inode->GetInit();
  if (init) {
    node->SetInit(init);
    inode->ClearInit();
    mUpdated = true;
  }
  return node;
}

// split export decl and body
// export {func add(y)}  ==> export {add}; func add(y)
ExportNode *AdjustASTVisitor::VisitExportNode(ExportNode *node) {
  if (node->GetPairsNum() == 1) {
    XXportAsPairNode *p = node->GetPair(0);
    TreeNode *bfnode = p->GetBefore();
    TreeNode *after = p->GetAfter();
    if (bfnode) {
      if (!bfnode->IsIdentifier()) {
        IdentifierNode *n = (IdentifierNode*)gTreePool.NewTreeNode(sizeof(IdentifierNode));
        switch (bfnode->GetKind()) {
          case NK_Function: {
            FunctionNode *func = static_cast<FunctionNode *>(bfnode);
            new (n) IdentifierNode(func->GetStrIdx());
            // update p
            p->SetBefore(n);
            mUpdated = true;
            // insert bfnode into AST after node
            TreeNode *parent = node->GetParent();
            if (parent) {
              if (parent->IsBlock()) {
                static_cast<BlockNode *>(parent)->InsertStmtAfter(bfnode, node);
              } else if (parent->IsModule()) {
                static_cast<ModuleNode *>(parent)->InsertAfter(bfnode, node);
              }
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

// if-then-else : use BlockNode for then and else bodies
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

// for : use BlockNode for body
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

// lamda : use BlockNode for body, add a ReturnNode
LambdaNode *AdjustASTVisitor::VisitLambdaNode(LambdaNode *node) {
  TreeNode *tn = VisitTreeNode(node->GetBody());
  if (tn && !tn->IsBlock()) {
    BlockNode *blk = (BlockNode*)gTreePool.NewTreeNode(sizeof(BlockNode));
    new (blk) BlockNode();
    blk->AddChild(tn);

    ReturnNode *ret = (ReturnNode*)gTreePool.NewTreeNode(sizeof(ReturnNode));
    new (ret) ReturnNode();
    blk->AddChild(ret);

    node->SetBody(blk);
    mUpdated = true;
  }
  return node;
}

}
