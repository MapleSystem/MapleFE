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

void AST_AST::ASTCollectAndDBRemoval(CfgFunc *func) {
  CollectASTInfo(func);
  RemoveDeadBlocks(func);

  ModuleNode *module = mHandler->GetASTModule();
  if (mTrace) {
    for(unsigned i = 0; i < module->GetTreesNum(); i++) {
      TreeNode *it = module->GetTree(i);
      it->Dump(0);
      std::cout << std::endl;
    }
  }
}

// this calcuates mNodeId2BbMap
void AST_AST::CollectASTInfo(CfgFunc *func) {
  if (mTrace) std::cout << "============== CollectASTInfo ==============" << std::endl;
  mReachableBbIdx.clear();
  std::deque<CfgBB *> working_list;

  working_list.push_back(func->GetEntryBB());

  unsigned bitnum = 0;

  while(working_list.size()) {
    CfgBB *bb = working_list.front();
    MASSERT(bb && "null BB");
    unsigned bbid = bb->GetId();

    // skip bb already visited
    if (mReachableBbIdx.find(bbid) != mReachableBbIdx.end()) {
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

    mReachableBbIdx.insert(bbid);
    working_list.pop_front();
  }
}

void AST_AST::RemoveDeadBlocks(CfgFunc *func) {
  std::set<CfgBB *> deadBb;
  CfgBB *bb = nullptr;
  for (auto id: mReachableBbIdx) {
    bb = mHandler->mBbId2BbMap[id];
    for (int i = 0; i < bb->GetPredecessorsNum(); i++) {
      CfgBB *pred = bb->GetPredecessorAtIndex(i);
      unsigned pid = pred->GetId();
      if (mHandler->mBbId2BbMap.find(pid) == mHandler->mBbId2BbMap.end()) {
        deadBb.insert(pred);
      }
    }
  }
  for (auto it: deadBb) {
    if (mTrace) std::cout << "deleted BB :";
    for (int i = 0; i < it->GetSuccessorsNum(); i++) {
      bb = it->GetSuccessorAtIndex(i);
      bb->mPredecessors.Remove(it);
    }
    if (mTrace) std::cout << " BB" << it->GetId();
    it->~CfgBB();
  }
  if (mTrace) std::cout << std::endl;
}

void AST_AST::AdjustAST() {
  if (mTrace) std::cout << "============== AdjustAST ==============" << std::endl;
  AdjustASTVisitor visitor(mHandler, mTrace, true);
  ModuleNode *module = mHandler->GetASTModule();
  for(unsigned i = 0; i < module->GetTreesNum(); i++) {
    TreeNode *it = module->GetTree(i);
    it->SetParent(module);
    visitor.Visit(it);
  }
}

// set parent for some identifier's type
IdentifierNode *AdjustASTVisitor::VisitIdentifierNode(IdentifierNode *node) {
  (void) AstVisitor::VisitIdentifierNode(node);
  TreeNode *type = node->GetType();
  if (type && type->IsUserType()) {
    type->SetParent(node);
  }
  return node;
}

TreeNode *AdjustASTVisitor::CreateTypeNodeFromName(IdentifierNode *node) {
  UserTypeNode *ut = (UserTypeNode*)gTreePool.NewTreeNode(sizeof(UserTypeNode));
  new (ut) UserTypeNode(node);
  return ut;
}

// set UserTypeNode's mStrIdx to be its mId's if exist
UserTypeNode *AdjustASTVisitor::VisitUserTypeNode(UserTypeNode *node) {
  (void) AstVisitor::VisitUserTypeNode(node);
  TreeNode *id = node->GetId();
  if (id && id->IsIdentifier()) {
    node->SetStrIdx(id->GetStrIdx());
  }
  return node;
}

FunctionNode *AdjustASTVisitor::VisitFunctionNode(FunctionNode *node) {
  (void) AstVisitor::VisitFunctionNode(node);
  TreeNode *type = node->GetType();
  if (type && type->IsUserType()) {
    type->SetParent(node);
  }
  // Refine function TypeParames
  for(unsigned i = 0; i < node->GetTypeParamsNum(); i++) {
    type = node->GetTypeParamAtIndex(i);
    if (type->IsIdentifier()) {
      IdentifierNode *inode = static_cast<IdentifierNode *>(type);
      TreeNode *newtype = CreateTypeNodeFromName(inode);
      node->SetTypeParamAtIndex(i, newtype);
      newtype->SetParent(node);
      newtype->SetStrIdx(inode->GetStrIdx());
    }
  }
  return node;
}

// move init from identifier to decl
// copy stridx to decl
DeclNode *AdjustASTVisitor::VisitDeclNode(DeclNode *node) {
  (void) AstVisitor::VisitDeclNode(node);
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
  TreeNode *parent = node->GetParent();
  if (!parent || parent->IsNamespace()) {
    // Export declarations are not permitted in a namespace
    return node;
  }
  if (node->GetPairsNum() == 1) {
    XXportAsPairNode *p = node->GetPair(0);
    TreeNode *bfnode = p->GetBefore();
    TreeNode *after = p->GetAfter();
    if (bfnode) {
      if (!bfnode->IsIdentifier()) {
        switch (bfnode->GetKind()) {
          case NK_Function: {
            FunctionNode *func = static_cast<FunctionNode *>(bfnode);
            IdentifierNode *n = (IdentifierNode*)gTreePool.NewTreeNode(sizeof(IdentifierNode));
            new (n) IdentifierNode(func->GetStrIdx());
            // update p
            p->SetBefore(n);
            n->SetParent(p);
            mUpdated = true;
            // insert func into AST after node
            if (parent->IsBlock()) {
              static_cast<BlockNode *>(parent)->InsertStmtAfter(func, node);
            } else if (parent->IsModule()) {
              static_cast<ModuleNode *>(parent)->InsertAfter(func, node);
            }
            // cp annotation from node to func
            for (unsigned i = 0; i < node->GetAnnotationsNum(); i++) {
              func->AddAnnotation(node->GetAnnotationAtIndex(i));
            }
            break;
          }
          case NK_Class: {
            ClassNode *classnode = static_cast<ClassNode *>(bfnode);
            IdentifierNode *n = (IdentifierNode*)gTreePool.NewTreeNode(sizeof(IdentifierNode));
            new (n) IdentifierNode(classnode->GetStrIdx());
            // update p
            p->SetBefore(n);
            n->SetParent(p);
            mUpdated = true;
            // insert classnode into AST after node
            if (parent->IsBlock()) {
              static_cast<BlockNode *>(parent)->InsertStmtAfter(classnode, node);
            } else if (parent->IsModule()) {
              static_cast<ModuleNode *>(parent)->InsertAfter(classnode, node);
            }
            // cp annotation from node to classnode
            for (unsigned i = 0; i < node->GetAnnotationsNum(); i++) {
              classnode->AddAnnotation(node->GetAnnotationAtIndex(i));
            }
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
  TreeNode *tn = VisitTreeNode(node->GetCond());
  tn = VisitTreeNode(node->GetTrueBranch());
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
  TreeNode *tn = NULL;
  for (int i = 0; i < node->GetInitsNum(); i++) {
    tn = VisitTreeNode(node->GetInitAtIndex(i));
  }
  tn = VisitTreeNode(node->GetCond());
  tn = VisitTreeNode(node->GetBody());
  if (tn && !tn->IsBlock()) {
    BlockNode *blk = (BlockNode*)gTreePool.NewTreeNode(sizeof(BlockNode));
    new (blk) BlockNode();
    blk->AddChild(tn);
    node->SetBody(blk);
    mUpdated = true;
  }
  return node;
}

static unsigned uniq_number = 1;

// lamda : create a FunctionNode for it
//         use BlockNode for body, add a ReturnNode
LambdaNode *AdjustASTVisitor::VisitLambdaNode(LambdaNode *node) {
  FunctionNode *func = (FunctionNode*)gTreePool.NewTreeNode(sizeof(FunctionNode));
  new (func) FunctionNode();

  // func name
  std::string str("__lambda_");
  str += std::to_string(uniq_number++);
  str += "__";
  IdentifierNode *name = (IdentifierNode*)gTreePool.NewTreeNode(sizeof(IdentifierNode));
  unsigned stridx = gStringPool.GetStrIdx(str);
  new (name) IdentifierNode(stridx);
  func->SetStrIdx(stridx);
  func->SetFuncName(name);

  // func parameters
  for (int i = 0; i < node->GetParamsNum(); i++) {
    func->AddParam(node->GetParam(i));
  }

  // func body
  TreeNode *tn = VisitTreeNode(node->GetBody());
  if (tn) {
    if (tn->IsBlock()) {
      func->SetBody(static_cast<BlockNode*>(tn));
      func->SetType(node->GetType());
    } else {
      BlockNode *blk = (BlockNode*)gTreePool.NewTreeNode(sizeof(BlockNode));
      new (blk) BlockNode();

      ReturnNode *ret = (ReturnNode*)gTreePool.NewTreeNode(sizeof(ReturnNode));
      new (ret) ReturnNode();
      ret->SetResult(tn);
      blk->AddChild(ret);

      func->SetBody(blk);
    }
  }

  // func return type
  if (node->GetType()) {
    func->SetType(node->GetType());
  }

  mUpdated = true;
  // note: the following conversion is only for the visitor to notice the node is updated
  return (LambdaNode*)func;
}

}
