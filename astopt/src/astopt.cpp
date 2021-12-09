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
#include "typetable.h"
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
  mASTXXport = new AST_XXport(this, f);
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

  // build dependency of modules 
  PreprocessModules();

  for (auto handler: mHandlersInOrder) {
    // basic analysis
    handler->BasicAnalysis();

    // build CFG
    handler->BuildCFG();

    // control flow analysis
    handler->ControlFlowAnalysis();

    // type inference
    handler->TypeInference();

    // data flow analysis
    handler->DataFlowAnalysis();
  }

  for (auto handler: mHandlersInOrder) {
    ModuleNode *module = handler->GetASTModule();

    AstStore saveAst(module);
    saveAst.StoreInAstBuf();
  }

  return;
}

void AstOpt::PreprocessModules() {
  // initialize gTypeTable with builtin types
  gTypeTable.AddPrimAndBuiltinTypes();

  // scan through modules to setup mNodeId2NodeMap
  BuildNodeIdToNodeVisitor visitor(this, mFlags);

  for (int i = 0; i < GetModuleNum(); i++) {
    Module_Handler *handler = mASTHandler->GetModuleHandler(i);
    // fill handler index
    handler->SetHidx(i);

    ModuleNode *module = handler->GetASTModule();
    // add module as type for import/export purpose
    module->SetTypeId(TY_Module);
    gTypeTable.AddType(module);

    visitor.SetHandler(handler);
    visitor.Visit(module);
  }

  // list modules according to dependency
  mASTXXport->BuildModuleOrder();
  // collect import/export info
  mASTXXport->CollectXXportInfo();
}

}
