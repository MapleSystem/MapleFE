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
#include "a2c_cfg.h"

namespace maplefe {

  void ModuleVisitor::InitializeVisitor() {
    // Create an artificial function for current module
    A2C_Function *func = mModule->NewFunction();
    mModule->SetFunction(func);

    // Create the entry BB and exit BB of current function
    A2C_BB *bb = mModule->NewBB();
    func->SetEntryBB(bb);
    func->SetExitBB(mModule->NewBB());

    // Initialize the working function and BB
    mCurrentFunction = func;
    mCurrentBB = mModule->NewBB();
    bb->AddSuccessor(mCurrentBB);
  }

  void ModuleVisitor::FinalizeVisitor() {
    // Add the exit BB as a successor of the entry BB of current function
    A2C_BB *exit = mCurrentFunction->GetExitBB();
    A2C_BB *entry = mCurrentFunction->GetEntryBB();
    entry->AddSuccessor(exit);
    mCurrentBB->AddSuccessor(exit);

    mCurrentFunction = nullptr;
    mCurrentBB = nullptr;
  }

  FunctionNode *ModuleVisitor::VisitFunctionNode(FunctionNode *node) {
    if(mTrace)
      std::cout << "ModuleVisitor: enter FunctionNode, id=" << node->GetNodeId() << std::endl;

    // Save both mCurrentFunction and mCurrentBB
    A2C_Function *current_func = mCurrentFunction;
    A2C_BB *current_bb = mCurrentBB;

    // Create a new function and add it as a nested function to current function
    mCurrentFunction = mModule->NewFunction();
    current_func->AddNestedFunction(mCurrentFunction);

    // Create both entry BB and exit BB of the new function
    A2C_BB *mCurrentBB = mModule->NewBB();
    mCurrentFunction->SetEntryBB(mCurrentBB);
    mCurrentFunction->SetExitBB(mModule->NewBB());

    // Visit the FunctionNode 'node'
    AstVisitor::VisitFunctionNode(node);

    // Add the exit BB as a successor of the entry BB of the new function
    A2C_BB *exit = mCurrentFunction->GetExitBB();
    A2C_BB *entry = mCurrentFunction->GetEntryBB();
    entry->AddSuccessor(exit);

    // Restore both mCurrentFunction and mCurrentBB
    mCurrentFunction = current_func;
    mCurrentBB = current_bb;

    if(mTrace)
      std::cout << "ModuleVisitor: exit FunctionNode, id=" << node->GetNodeId() << std::endl;
    return node;
  }

  ReturnNode *ModuleVisitor::VisitReturnNode(ReturnNode *node) {
    //AstVisitor::VisitReturnNode(node);
    mCurrentBB->AddStatement(node);
    mCurrentBB->SetKind(BK_Return);

    A2C_BB *current_bb = mCurrentBB;
    //TODO
    //A2C_BB *exit = mCurrentFunction->GetExitBB();
    //mCurrentBB->AddSuccessor(exit);

    // Create a new BB for the code following this return statement
    mCurrentBB = mModule->NewBB();
    current_bb->AddSuccessor(mCurrentBB);
    return node;
  }

  CondBranchNode *ModuleVisitor::VisitCondBranchNode(CondBranchNode *node) {
    if(mTrace)
      std::cout << "ModuleVisitor: enter CondBranchNode, id=" << node->GetNodeId() << std::endl;

    mCurrentBB->SetKind(BK_Branch);

    TreeNode *cond = node->GetCond();
    // Set predicate of current BB
    mCurrentBB->SetPredicate(cond);
    //VisitTreeNode(cond);

    // Save current BB
    A2C_BB *current_bb = mCurrentBB;

    // Create a new BB for true branch
    mCurrentBB = mModule->NewBB();
    current_bb->AddSuccessor(mCurrentBB);

    // Visit true branch first
    VisitTreeNode(node->GetTrueBranch());

    // Create a BB for the join point
    A2C_BB *join = mModule->NewBB();
    mCurrentBB->AddSuccessor(join);

    TreeNode *false_branch = node->GetFalseBranch();
    if(false_branch == nullptr)
      current_bb->AddSuccessor(join);
    else {
      mCurrentBB = mModule->NewBB();
      current_bb->AddSuccessor(mCurrentBB);
      VisitTreeNode(false_branch);
      mCurrentBB->AddSuccessor(join);
    }

    // Keep going with the BB at the join point
    mCurrentBB = join;

    if(mTrace)
      std::cout << "ModuleVisitor: exit CondBranchNode, id=" << node->GetNodeId() << std::endl;
    return node;
  }

  ForLoopNode *ModuleVisitor::VisitForLoopNode(ForLoopNode *node) {
    // Visit all inits
    for (unsigned i = 0; i < node->GetInitsNum(); ++i) {
      VisitTreeNode(node->GetInitAtIndex(i));
    }

    A2C_BB *current_bb = mCurrentBB;
    // Create a new BB for loop header
    mCurrentBB = mModule->NewBB();
    current_bb->AddSuccessor(mCurrentBB);
    mCurrentBB->SetKind(BK_LoopHeader);
    // Set current_bb to be loop header
    current_bb = mCurrentBB;

    TreeNode *cond = node->GetCond();
    // Set predicate of current BB
    mCurrentBB->SetPredicate(cond);
    //VisitTreeNode(node->GetCond());

    // Create a BB for loop body
    mCurrentBB = mModule->NewBB();
    current_bb->AddSuccessor(mCurrentBB);

    VisitTreeNode(node->GetBody());
    for (unsigned i = 0; i < node->GetUpdatesNum(); ++i) {
      VisitTreeNode(node->GetUpdateAtIndex(i));
    }
    // Add a back edge to loop header
    mCurrentBB->AddSuccessor(current_bb);

    // Create a new BB to get out of the loop
    A2C_BB *loop_exit = mModule->NewBB();
    current_bb->AddSuccessor(loop_exit);
    mCurrentBB = loop_exit;
    return node;
  }

  BlockNode *ModuleVisitor::VisitBlockNode(BlockNode *node) {
    // Check if current block constains any JS_Let or JS_Const DeclNode
    unsigned i, num = node->GetChildrenNum();
    for (i = 0; i < num; ++i) {
      TreeNode *child = node->GetChildAtIndex(i);
      if(child->GetKind() != NK_Decl)
        continue;
      DeclNode *decl = static_cast<DeclNode *>(child);
      if(decl->GetProp() == JS_Let || decl->GetProp() == JS_Const)
        break;
    }
    if(i <= num)
      // Do not create BB for current block when no JS_Let or JS_Const DeclNode inside
      AstVisitor::VisitBlockNode(node);
    else {
      // Needs BBs for current block
      // Save current BB
      A2C_BB *current_bb = mCurrentBB;
      current_bb->SetKind(BK_Block);

      // Create a new BB for current block node
      mCurrentBB = mModule->NewBB();
      current_bb->AddSuccessor(mCurrentBB);

      // Visit all children nodes
      AstVisitor::VisitBlockNode(node);

      // Create a BB for the join point
      A2C_BB *join = mModule->NewBB();
      mCurrentBB->AddSuccessor(join);
      // This edge is to determine the block range for JS_Let or JS_Const DeclNode
      current_bb->AddSuccessor(join);

      mCurrentBB = join;
    }
    return node;
  }

  DeclNode *ModuleVisitor::VisitDeclNode(DeclNode *node) {
    mCurrentBB->AddStatement(node);
    // AstVisitor::VisitDeclNode(node);
    return node;
  }

  CallNode *ModuleVisitor::VisitCallNode(CallNode *node) {
    mCurrentBB->AddStatement(node);
    return node;
  }

  void A2C_Function::Dump() {
    unsigned num = GetNestedFunctionsNum();
    if(num > 0) {
      std::cout << "Nested Functions: " << num << " {" << std::endl;
      for(unsigned i = 0; i < num; ++i) {
        std::cout << "Function: " << i + 1 << std::endl;
        GetNestedFunctionAtIndex(i)->Dump();
      }
      std::cout << "}" << std::endl;
    }
    std::cout << "Basic blocks: {" << std::endl;

    std::stack<A2C_BB*> bb_stack;
    A2C_BB *entry = GetEntryBB(), *exit = GetExitBB();
    bb_stack.push(exit);
    bb_stack.push(entry);
    std::set<A2C_BB*> visited;
    visited.insert(exit);
    visited.insert(entry);
    std::string dot("---\ndigraph CFG {");
    const char* fake = " [style=dashed];";
    while(!bb_stack.empty()) {
      A2C_BB *bb = bb_stack.top();
      bb_stack.pop();
      unsigned succ_num = bb->GetSuccessorsNum();
      std::cout << "BB" << bb->GetId() << (succ_num ? " ( succ: " : " ( Exit ");
      for(unsigned i = 0; i < succ_num; ++i) {
        A2C_BB *curr = bb->GetSuccessorAtIndex(i);
        std::cout << "BB" << curr->GetId() << " ";
        dot += "\nBB" + std::to_string(bb->GetId()) + " -> BB" + std::to_string(curr->GetId())
          + (bb == entry ? (curr == exit ? fake : ";") :
              succ_num == 1 ? ";" : i ? bb->GetKind() == BK_Block ? fake :
              " [color=darkred];" : " [color=darkgreen];");
        if(visited.find(curr) == visited.end()) {
          bb_stack.push(curr);
          visited.insert(curr);
        }
      }
      std::cout << ")" << std::endl;
      unsigned num = bb->GetStatementsNum();
      if(num)
        for(unsigned i = 0; i < num; ++i)
          std::cout << "  " << i + 1 << ". TreeNode: "
            << bb->GetStatementAtIndex(i)->GetNodeId() << std::endl;
    }
    std::cout << "}" << std::endl;
    dot += "\n}";
    std::cout << dot << std::endl;
  }

  void A2C_Module::BuildCFG() {
    // Set the init function for current module
    SetFunction(NewFunction());


    ModuleVisitor visitor(this, mTraceModule, true);

    visitor.InitializeVisitor();

    for(auto it: mASTModule->mTrees)
          visitor.Visit(it->mRootNode);

    visitor.FinalizeVisitor();
  }

  void A2C_Module::Dump(char *msg) {
    std::cout << std::endl << msg << ":" << std::endl;
    A2C_Function *func = GetFunction();
    func->Dump();
  }


}

