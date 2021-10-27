/*
* Copyright (C) [2020] Futurewei Technologies, Inc. All rights reverved.
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

#include "astopt.h"
#include "ast_handler.h"
#include "gen_astdump.h"
#include "gen_astgraph.h"
#include "gen_aststore.h"
#include "gen_astload.h"

namespace maplefe {

// starting point of AST
void AstOpt::ProcessAST(unsigned flags) {
  // loop through module handlers
  unsigned size = mASTHandler->mModuleHandlers.GetNum();
  for (int i = 0; i < size; i++) {
    Module_Handler *handler = mASTHandler->mModuleHandlers.ValueAtIndex(i);
    ModuleNode *module = handler->GetASTModule();

    mFlags = flags;
    if (mFlags & FLG_trace_3) {
      std::cout << "============= in ProcessAST ===========" << std::endl;
      std::cout << "srcLang : " << module->GetSrcLangString() << std::endl;

      for(unsigned i = 0; i < module->GetTreesNum(); i++) {
        TreeNode *tnode = module->GetTree(i);
        tnode->Dump(0);
        std::cout << std::endl;
      }
    }
  }

  // collect AST info
  CollectInfo();

  // rewirte some AST nodes
  AdjustAST();

  // scope analysis
  ScopeAnalysis();

  // build CFG
  BuildCFG();

  // control flow analysis
  ControlFlowAnalysis();

  // type inference
  TypeInference();

  // data flow analysis
  DataFlowAnalysis();

  for (int i = 0; i < size; i++) {
    Module_Handler *handler = mASTHandler->mModuleHandlers.ValueAtIndex(i);
    ModuleNode *module = handler->GetASTModule();

    AstStore saveAst(module);
    saveAst.StoreInAstBuf();
  }

  return;
}

void AstOpt::CollectInfo() {
  unsigned size = mASTHandler->mModuleHandlers.GetNum();
  for (int i = 0; i < size; i++) {
    Module_Handler *handler = mASTHandler->mModuleHandlers.ValueAtIndex(i);
    handler->CollectInfo();
  }
}

void AstOpt::AdjustAST() {
  unsigned size = mASTHandler->mModuleHandlers.GetNum();
  for (int i = 0; i < size; i++) {
    Module_Handler *handler = mASTHandler->mModuleHandlers.ValueAtIndex(i);
    handler->AdjustAST();
  }
}

void AstOpt::ScopeAnalysis() {
  unsigned size = mASTHandler->mModuleHandlers.GetNum();
  for (int i = 0; i < size; i++) {
    Module_Handler *handler = mASTHandler->mModuleHandlers.ValueAtIndex(i);
    handler->ScopeAnalysis();

    if (mFlags & FLG_trace_2) {
      std::cout << "============== Dump Scope ==============" << std::endl;
      ModuleNode *module = handler->GetASTModule();
      module->GetRootScope()->Dump(0);
    }
  }
}

void AstOpt::BuildCFG() {
  unsigned size = mASTHandler->mModuleHandlers.GetNum();
  for (int i = 0; i < size; i++) {
    Module_Handler *handler = mASTHandler->mModuleHandlers.ValueAtIndex(i);
    handler->BuildCFG();
  }
}

void AstOpt::ControlFlowAnalysis() {
  unsigned size = mASTHandler->mModuleHandlers.GetNum();
  for (int i = 0; i < size; i++) {
    Module_Handler *handler = mASTHandler->mModuleHandlers.ValueAtIndex(i);
    handler->ControlFlowAnalysis();
  }
}

void AstOpt::TypeInference() {
  unsigned size = mASTHandler->mModuleHandlers.GetNum();
  for (int i = 0; i < size; i++) {
    Module_Handler *handler = mASTHandler->mModuleHandlers.ValueAtIndex(i);
    handler->TypeInference();
  }
}

void AstOpt::DataFlowAnalysis() {
  unsigned size = mASTHandler->mModuleHandlers.GetNum();
  for (int i = 0; i < size; i++) {
    Module_Handler *handler = mASTHandler->mModuleHandlers.ValueAtIndex(i);
    handler->DataFlowAnalysis();
  }
}

}
