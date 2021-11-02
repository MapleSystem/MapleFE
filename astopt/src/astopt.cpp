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
#include "ast_xxport.h"
#include "gen_astgraph.h"
#include "gen_aststore.h"
#include "gen_astload.h"

namespace maplefe {

class ImportedFiles;

AstOpt::AstOpt(AST_Handler *h, unsigned f) {
  mASTHandler = h;
  h->SetAstOpt(this);
  mASTXxport = new AST_Xxport(this, f);
  mFlags = f;
}

unsigned AstOpt::GetModuleNum() {
  return mASTHandler->GetSize();
}

// starting point of AST
void AstOpt::ProcessAST(unsigned flags) {
  // loop through module handlers
  for (int i = 0; i < GetModuleNum(); i++) {
    Module_Handler *handler = mASTHandler->GetModuleHandler(i);
    handler->SetHidx(i);
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

  // basic analysis
  BasicAnalysis();

  // build CFG
  BuildCFG();

  // control flow analysis
  ControlFlowAnalysis();

  // type inference
  TypeInference();

  // data flow analysis
  DataFlowAnalysis();

  for (auto it: mHandlersIdxInOrder) {
    ModuleNode *module = it->GetASTModule();

    AstStore saveAst(module);
    saveAst.StoreInAstBuf();
  }

  return;
}

void AstOpt::BasicAnalysis() {
  // list modules according to dependency
  mASTXxport->BuildModuleOrder();

  // collect AST info
  CollectInfo();

  // rewirte some AST nodes
  AdjustAST();

  // scope analysis
  ScopeAnalysis();
}

void AstOpt::CollectInfo() {
  for (auto it: mHandlersIdxInOrder) {
    it->CollectInfo();
  }
}

void AstOpt::AdjustAST() {
  for (auto it: mHandlersIdxInOrder) {
    it->AdjustAST();
  }
}

void AstOpt::ScopeAnalysis() {
  for (auto it: mHandlersIdxInOrder) {
    it->ScopeAnalysis();

    if (mFlags & FLG_trace_2) {
      std::cout << "============== Dump Scope ==============" << std::endl;
      ModuleNode *module = it->GetASTModule();
      module->GetRootScope()->Dump(0);
    }
  }
}

void AstOpt::BuildCFG() {
  for (auto it: mHandlersIdxInOrder) {
    it->BuildCFG();
  }
}

void AstOpt::ControlFlowAnalysis() {
  for (auto it: mHandlersIdxInOrder) {
    it->ControlFlowAnalysis();
  }
}

void AstOpt::TypeInference() {
  for (auto it: mHandlersIdxInOrder) {
    it->TypeInference();
  }
}

void AstOpt::DataFlowAnalysis() {
  for (auto it: mHandlersIdxInOrder) {
    it->DataFlowAnalysis();
  }
}

}
