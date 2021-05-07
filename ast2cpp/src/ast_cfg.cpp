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
#include "ast_cfg.h"
#include "ast_dfa.h"
#include "ast_handler.h"
#include "gen_astdump.h"

namespace maplefe {

// Initialize a AST_Function node
void CFGVisitor::InitializeFunction(AST_Function *func) {
  // Create the entry BB and exit BB of current function
  AST_BB *bb = NewBB(BK_Uncond);
  func->SetEntryBB(bb);
  func->SetExitBB(NewBB(BK_Join));
  // Initialize the working function and BB
  mCurrentFunction = func;
  mCurrentBB = NewBB(BK_Uncond);
  bb->AddSuccessor(mCurrentBB);
}

// Finalize a AST_Function node
void CFGVisitor::FinalizeFunction() {
  AST_BB *exit = mCurrentFunction->GetExitBB();
  mCurrentBB->AddSuccessor(exit);
  mCurrentFunction->SetLastBBId(AST_BB::GetLastId());
  mCurrentFunction = nullptr;
  mCurrentBB = nullptr;
}

// Handle a function
FunctionNode *CFGVisitor::VisitFunctionNode(FunctionNode *node) {
  if(mTrace) std::cout << "CFGVisitor: enter FunctionNode, id=" << node->GetNodeId() << std::endl;

  // Save both mCurrentFunction and mCurrentBB
  AST_Function *current_func = mCurrentFunction;
  AST_BB *current_bb = mCurrentBB;

  // Create a new function and add it as a nested function to current function
  mCurrentFunction = NewFunction();
  current_func->AddNestedFunction(mCurrentFunction);

  InitializeFunction(mCurrentFunction);
  // Visit the FunctionNode 'node'
  AstVisitor::VisitFunctionNode(node);
  FinalizeFunction();

  // Restore both mCurrentFunction and mCurrentBB
  mCurrentFunction = current_func;
  mCurrentBB = current_bb;

  if(mTrace) std::cout << "CFGVisitor: exit FunctionNode, id=" << node->GetNodeId() << std::endl;
  return node;
}

// Handle a lambda
LambdaNode *CFGVisitor::VisitLambdaNode(LambdaNode *node) {
  return node;
}

// For control flow
ReturnNode *CFGVisitor::VisitReturnNode(ReturnNode *node) {
  mCurrentBB->AddStatement(node);
  AST_BB *exit = mCurrentFunction->GetExitBB();
  mCurrentBB->AddSuccessor(exit);
  mCurrentBB->SetKind(BK_Terminated);
  return node;
}

// For control flow
CondBranchNode *CFGVisitor::VisitCondBranchNode(CondBranchNode *node) {
  mCurrentBB->SetKind(BK_Branch);
  mCurrentBB->AddStatement(node);

  TreeNode *cond = node->GetCond();
  // Set predicate of current BB
  mCurrentBB->SetPredicate(cond);
  //VisitTreeNode(cond);

  // Save current BB
  AST_BB *current_bb = mCurrentBB;

  // Create a new BB for true branch
  mCurrentBB = NewBB(BK_Uncond);
  current_bb->AddSuccessor(mCurrentBB);

  // Visit true branch first
  VisitTreeNode(node->GetTrueBranch());

  // Create a BB for the join point
  AST_BB *join = NewBB(BK_Join);
  mCurrentBB->AddSuccessor(join);

  TreeNode *false_branch = node->GetFalseBranch();
  if(false_branch == nullptr) {
    current_bb->AddSuccessor(join);
  } else {
    mCurrentBB = NewBB(BK_Uncond);
    current_bb->AddSuccessor(mCurrentBB);
    // Visit false branch if it exists
    VisitTreeNode(false_branch);
    mCurrentBB->AddSuccessor(join);
  }
  // Keep going with the BB at the join point
  mCurrentBB = join;
  return node;
}

// For control flow
// ForLoopProp: FLP_Regular, FLP_JSIn, FLP_JSOf
ForLoopNode *CFGVisitor::VisitForLoopNode(ForLoopNode *node) {
  // Visit all inits
  for (unsigned i = 0; i < node->GetInitsNum(); ++i) {
    VisitTreeNode(node->GetInitAtIndex(i));
  }

  AST_BB *current_bb = mCurrentBB;
  // Create a new BB for loop header
  mCurrentBB = NewBB(BK_LoopHeader);

  // Add current node to the loop header BB
  mCurrentBB->AddStatement(node);
  mCurrentBB->SetAuxNode(node);
  current_bb->AddSuccessor(mCurrentBB);
  // Set current_bb to be loop header
  current_bb = mCurrentBB;

  if(node->GetProp() == FLP_Regular) {
    TreeNode *cond = node->GetCond();
    // Set predicate of current BB
    mCurrentBB->SetPredicate(cond);
  } else
    // Set predicate to be current ForLoopNode when it is FLP_JSIn or FLP_JSOf
    mCurrentBB->SetPredicate(node);

  // Create a BB for loop body
  mCurrentBB = NewBB(BK_Uncond);
  current_bb->AddSuccessor(mCurrentBB);
  // Create a new BB for getting out of the loop
  AST_BB *loop_exit = NewBB(BK_Join);

  // Push loop_exit and current_bb to mTargetBBs for 'break' and 'continue'
  mTargetBBs.push(std::pair<AST_BB*,AST_BB*>{loop_exit, current_bb});
  // Visit loop body
  VisitTreeNode(node->GetBody());
  mTargetBBs.pop();

  // Visit all updates
  for (unsigned i = 0; i < node->GetUpdatesNum(); ++i) {
    VisitTreeNode(node->GetUpdateAtIndex(i));
  }
  // Add a back edge to loop header
  mCurrentBB->AddSuccessor(current_bb);
  current_bb->AddSuccessor(loop_exit);
  mCurrentBB = loop_exit;
  return node;
}

// For control flow
WhileLoopNode *CFGVisitor::VisitWhileLoopNode(WhileLoopNode *node) {
  AST_BB *current_bb = mCurrentBB;
  // Create a new BB for loop header
  mCurrentBB = NewBB(BK_LoopHeader);
  // Add current node to the loop header BB
  mCurrentBB->AddStatement(node);
  mCurrentBB->SetAuxNode(node);
  current_bb->AddSuccessor(mCurrentBB);
  // Set current_bb to be loop header
  current_bb = mCurrentBB;

  TreeNode *cond = node->GetCond();
  // Set predicate of current BB
  mCurrentBB->SetPredicate(cond);
  //VisitTreeNode(node->GetCond());

  // Create a BB for loop body
  mCurrentBB = NewBB(BK_Uncond);
  current_bb->AddSuccessor(mCurrentBB);
  // Create a new BB for getting out of the loop
  AST_BB *loop_exit = NewBB(BK_Join);

  // Push loop_exit and current_bb to mTargetBBs for 'break' and 'continue'
  mTargetBBs.push(std::pair<AST_BB*,AST_BB*>{loop_exit, current_bb});
  // Visit loop body
  VisitTreeNode(node->GetBody());
  mTargetBBs.pop();

  // Add a back edge to loop header
  mCurrentBB->AddSuccessor(current_bb);
  current_bb->AddSuccessor(loop_exit);
  mCurrentBB = loop_exit;
  return node;
}

// For control flow
DoLoopNode *CFGVisitor::VisitDoLoopNode(DoLoopNode *node) {
  AST_BB *current_bb = mCurrentBB;
  // Create a new BB for loop header
  mCurrentBB = NewBB(BK_LoopHeader);
  // Add current node to the loop header BB
  mCurrentBB->AddStatement(node);
  mCurrentBB->SetAuxNode(node);
  current_bb->AddSuccessor(mCurrentBB);
  // Set current_bb to be loop header
  current_bb = mCurrentBB;

  // Create a BB for loop body
  mCurrentBB = NewBB(BK_Uncond);
  current_bb->AddSuccessor(mCurrentBB);
  // Create a new BB for getting out of the loop
  AST_BB *loop_exit = NewBB(BK_Join);

  // Push loop_exit and current_bb to mTargetBBs for 'break' and 'continue'
  mTargetBBs.push(std::pair<AST_BB*,AST_BB*>{loop_exit, current_bb});
  // Visit loop body
  VisitTreeNode(node->GetBody());
  mTargetBBs.pop();

  TreeNode *cond = node->GetCond();
  // Set predicate of current BB
  mCurrentBB->SetPredicate(cond);
  //VisitTreeNode(node->GetCond());

  // Add a back edge to loop header
  mCurrentBB->AddSuccessor(current_bb);
  mCurrentBB->AddSuccessor(loop_exit);
  mCurrentBB = loop_exit;
  return node;
}

// For control flow
ContinueNode *CFGVisitor::VisitContinueNode(ContinueNode *node) {
  mCurrentBB->AddStatement(node);
  // Get the loop header
  AST_BB *loop_header = mTargetBBs.top().second;
  // Add the loop header as a successor of current BB
  mCurrentBB->AddSuccessor(loop_header);
  mCurrentBB->SetKind(BK_Terminated);
  return node;
}

BreakNode *CFGVisitor::VisitBreakNode(BreakNode *node) {
  mCurrentBB->AddStatement(node);
  // Get the target BB for a loop or switch statement
  AST_BB *exit = mTargetBBs.top().first;
  // Add the target as a successor of current BB
  mCurrentBB->AddSuccessor(exit);
  mCurrentBB->SetKind(BK_Terminated);
  return node;
}

// For control flow
SwitchNode *CFGVisitor::VisitSwitchNode(SwitchNode *node) {
  mCurrentBB->SetKind(BK_Switch);
  mCurrentBB->AddStatement(node);
  // Set the root node of current BB
  mCurrentBB->SetAuxNode(node);

  // Save current BB
  AST_BB *current_bb = mCurrentBB;

  // Create a new BB for getting out of the switch block
  AST_BB *exit = NewBB(BK_Join);
  mTargetBBs.push(std::pair<AST_BB*,AST_BB*>{exit,
      (mTargetBBs.empty() ? nullptr : mTargetBBs.top().second)});
  AST_BB *prev_block = nullptr;
  TreeNode *switch_expr = node->GetExpr();
  for (unsigned i = 0; i < node->GetCasesNum(); ++i) {
    AST_BB *case_bb = NewBB(BK_Case);
    current_bb->AddSuccessor(case_bb);

    TreeNode *case_node = node->GetCaseAtIndex(i);
    // Add current case node to current case BB
    case_bb->AddStatement(case_node);
    // Set the auxiliary node and predicate for current case BB
    case_bb->SetAuxNode(case_node);
    case_bb->SetPredicate(switch_expr);

    bool is_default = false;
    TreeNode *case_expr = nullptr;
    if(case_node->GetKind() == NK_SwitchCase) {
      // Use the first label node of current SwitchCaseNode
      TreeNode *label_node = static_cast<SwitchCaseNode *>(case_node)->GetLabelAtIndex(0);
      if(label_node->GetKind() == NK_SwitchLabel) {
        is_default = static_cast<SwitchLabelNode *>(label_node)->IsDefault();
        case_expr = static_cast<SwitchLabelNode *>(label_node)->GetValue();
      }
    }

    // Optimize for default case
    if(is_default) {
      mCurrentBB = case_bb;
      case_bb->SetKind(BK_Uncond);
    } else {
      mCurrentBB = NewBB(BK_Uncond);
      case_bb->AddSuccessor(mCurrentBB);
    }

    // Add a fall-through edge if needed
    if(prev_block) {
      prev_block->AddSuccessor(mCurrentBB);
    }

    // Prepare for next case
    prev_block = mCurrentBB;
    current_bb = case_bb;
  }
  mTargetBBs.pop();

  // Connect to the exit BB of this switch statement
  prev_block->AddSuccessor(exit);
  if(prev_block != current_bb) {
    current_bb->AddSuccessor(exit);
  }
  mCurrentBB = exit;
  return node;
}

// For control flow
BlockNode *CFGVisitor::VisitBlockNode(BlockNode *node) {
  mCurrentBB->AddStatement(node);
  // Check if current block constains any JS_Let or JS_Const DeclNode
  unsigned i, num = node->GetChildrenNum();
  for (i = 0; i < num; ++i) {
    TreeNode *child = node->GetChildAtIndex(i);
    if(child->GetKind() != NK_Decl) {
      continue;
    }
    DeclNode *decl = static_cast<DeclNode *>(child);
    if(decl->GetProp() == JS_Let || decl->GetProp() == JS_Const) {
      break;
    }
  }
  if(i >= num) {
    // Do not create BB for current block when no JS_Let or JS_Const DeclNode inside
    // Visit all child nodes
    AstVisitor::VisitBlockNode(node);
  } else {
    mCurrentBB->SetKind(BK_Block);
    // Set the auxiliary node of this BB
    mCurrentBB->SetAuxNode(node);

    // Create a BB for the join point
    AST_BB *join = NewBB(BK_Join);

    // Needs BBs for current block
    // Save current BB
    AST_BB *current_bb = mCurrentBB;
    // Create a new BB for current block node
    mCurrentBB = NewBB(BK_Uncond);
    current_bb->AddSuccessor(mCurrentBB);

    // Visit all child nodes
    AstVisitor::VisitBlockNode(node);

    mCurrentBB->AddSuccessor(join);
    // This edge is to determine the block range for JS_Let or JS_Const DeclNode
    //current_bb->AddSuccessor(join);

    mCurrentBB = join;
  }
  return node;
}

// For PassNode
PassNode *CFGVisitor::VisitPassNode(PassNode *node) {
  mCurrentBB->AddStatement(node);
  return node;
}

// For statement of current BB
ImportNode *CFGVisitor::VisitImportNode(ImportNode *node) {
  mCurrentBB->AddStatement(node);
  return node;
}

// For statement of current BB
DeclNode *CFGVisitor::VisitDeclNode(DeclNode *node) {
  mCurrentBB->AddStatement(node);
  return node;
}

// For statement of current BB
ParenthesisNode *CFGVisitor::VisitParenthesisNode(ParenthesisNode *node) {
  mCurrentBB->AddStatement(node);
  return node;
}

// For statement of current BB
CastNode *CFGVisitor::VisitCastNode(CastNode *node) {
  mCurrentBB->AddStatement(node);
  return node;
}

// For statement of current BB
ArrayElementNode *CFGVisitor::VisitArrayElementNode(ArrayElementNode *node) {
  mCurrentBB->AddStatement(node);
  return node;
}

// For statement of current BB
VarListNode *CFGVisitor::VisitVarListNode(VarListNode *node) {
  mCurrentBB->AddStatement(node);
  return node;
}

// For statement of current BB
ExprListNode *CFGVisitor::VisitExprListNode(ExprListNode *node) {
  mCurrentBB->AddStatement(node);
  return node;
}

// For statement of current BB
LiteralNode *CFGVisitor::VisitLiteralNode(LiteralNode *node) {
  mCurrentBB->AddStatement(node);
  return node;
}

// For statement of current BB
UnaOperatorNode *CFGVisitor::VisitUnaOperatorNode(UnaOperatorNode *node) {
  mCurrentBB->AddStatement(node);
  return node;
}

// For statement of current BB
BinOperatorNode *CFGVisitor::VisitBinOperatorNode(BinOperatorNode *node) {
  mCurrentBB->AddStatement(node);
  return node;
}

// For statement of current BB
TerOperatorNode *CFGVisitor::VisitTerOperatorNode(TerOperatorNode *node) {
  mCurrentBB->AddStatement(node);
  return node;
}

// For statement of current BB
InstanceOfNode *CFGVisitor::VisitInstanceOfNode(InstanceOfNode *node) {
  mCurrentBB->AddStatement(node);
  return node;
}

// For statement of current BB
TypeOfNode *CFGVisitor::VisitTypeOfNode(TypeOfNode *node) {
  mCurrentBB->AddStatement(node);
  return node;
}

// For statement of current BB
NewNode *CFGVisitor::VisitNewNode(NewNode *node) {
  mCurrentBB->AddStatement(node);
  return node;
}

// For statement of current BB
DeleteNode *CFGVisitor::VisitDeleteNode(DeleteNode *node) {
  mCurrentBB->AddStatement(node);
  return node;
}

// For statement of current BB
CallNode *CFGVisitor::VisitCallNode(CallNode *node) {
  mCurrentBB->AddStatement(node);
  return node;
}

// For statement of current BB
AssertNode *CFGVisitor::VisitAssertNode(AssertNode *node) {
  mCurrentBB->AddStatement(node);
  return node;
}

// Allocate a new AST_Function node
AST_Function *CFGVisitor::NewFunction()   {
  return new(mHandler->GetMemPool()->Alloc(sizeof(AST_Function))) AST_Function;
}

// Allocate a new AST_BB node
AST_BB *CFGVisitor::NewBB(BBKind k) {
  return new(mHandler->GetMemPool()->Alloc(sizeof(AST_BB))) AST_BB(k);
}

// Helper for a node in dot graph
static std::string BBLabelStr(AST_BB *bb, const char *shape = nullptr) {
  static const char* const kBBNames[] =
  { "unknown", "uncond", "block", "branch", "loop", "switch", "case", "yield", "term", "join" };
  if(shape == nullptr)
    return kBBNames[bb->GetKind()];
  std::string str("BB" + std::to_string(bb->GetId()));
  str += " [label=\"" + str + (shape[0] == 'e' ? std::string("\\n") + kBBNames[bb->GetKind()] : "")
    + "\", shape=" + shape + "];\n";
  return str;
}

// Dump current AST_Function node
void AST_Function::Dump() {
  std::cout << "Function {" << std::endl;
  unsigned num = GetNestedFunctionsNum();
  if(num > 0) {
    std::cout << "Nested Functions: " << num << " [" << std::endl;
    for(unsigned i = 0; i < num; ++i) {
      std::cout << "Function: " << i + 1 << std::endl;
      GetNestedFunctionAtIndex(i)->Dump();
    }
    std::cout << "] // Nested Functions" << std::endl;
  }
  std::cout << "BBs: [" << std::endl;

  std::stack<AST_BB*> bb_stack;
  AST_BB *entry = GetEntryBB(), *exit = GetExitBB();
  bb_stack.push(exit);
  bb_stack.push(entry);
  std::set<AST_BB*> visited;
  visited.insert(exit);
  visited.insert(entry);
  // Dump CFG in dot format
  std::string dot("---\ndigraph CFG {\n");
  dot += BBLabelStr(entry, "box") + BBLabelStr(exit, "doubleoctagon");
  const char* scoped = " [style=dashed color=grey];";
  while(!bb_stack.empty()) {
    AST_BB *bb = bb_stack.top();
    bb_stack.pop();
    unsigned succ_num = bb->GetSuccessorsNum();
    std::cout << "BB" << bb->GetId() << ", " << BBLabelStr(bb) << (succ_num ? " ( succ: " : " ( Exit ");
    for(unsigned i = 0; i < succ_num; ++i) {
      AST_BB *curr = bb->GetSuccessorAtIndex(i);
      std::cout << "BB" << curr->GetId() << " ";
      dot += "BB" + std::to_string(bb->GetId()) + " -> BB" + std::to_string(curr->GetId())
        + (bb == entry ? (curr == exit ? scoped : ";") :
            succ_num == 1 ? ";" : i ? bb->GetKind() == BK_Block ? scoped :
            " [color=darkred];" : " [color=darkgreen];") + "\n";
      if(visited.find(curr) == visited.end()) {
        bb_stack.push(curr);
        visited.insert(curr);
        dot += BBLabelStr(curr, "ellipse");
      }
    }
    std::cout << ")" << std::endl;
    unsigned stmt_num = bb->GetStatementsNum();
    if(stmt_num) {
      for(unsigned i = 0; i < stmt_num; ++i) {
        TreeNode *stmt = bb->GetStatementAtIndex(i);
        std::cout << "  " << i + 1 << ". NodeId: " << stmt->GetNodeId() << ", "
          << AstDump::GetEnumNodeKind(stmt->GetKind()) << std::endl;
      }
    }
  }
  std::cout << dot << "} // CFG in dot format" << std::endl;
  std::cout << "] // BBs\nLastBBId" << (num ? " (Including nested functions)" : "") << ": "
    << GetLastBBId() << "\n} // Function" << std::endl;
}

void AST_CFG::Build() {
  BuildCFG();
}

void AST_CFG::BuildCFG() {
  if (mTrace) std::cout << "============== BuildCFG ==============" << std::endl;
  CFGVisitor visitor(mHandler, mTrace, true);
  // Set the init function for current module
  AST_Function *func = visitor.NewFunction();
  mHandler->SetFunction(func);
  // Start to build CFG for current module
  visitor.InitializeFunction(func);
  for(auto it: mHandler->GetASTModule()->mTrees) {
    visitor.Visit(it->mRootNode);
  }
  visitor.FinalizeFunction();
}

}