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

#include "ast2cpp.h"
#include "ast_handler.h"
#include "gen_astdump.h"
#include "gen_astgraph.h"
#include "gen_aststore.h"
#include "gen_astload.h"
#include "emitter.h"
#include "cpp_emitter.h"

namespace maplefe {

#define NOTYETIMPL(K)      { if (mTraceA2C) { MNYI(K);      }}
#define AST2CPPMSG0(K)     { if (mTraceA2C) { MMSG0(K);     }}
#define AST2CPPMSG(K,v)    { if (mTraceA2C) { MMSG(K,v);    }}
#define AST2CPPMSG2(K,v,w) { if (mTraceA2C) { MMSG2(K,v,w); }}

void A2C::EmitTS() {
  unsigned size = mASTHandler->mModuleHandlers.GetNum();
  for (int i = 0; i < size; i++) {
    Module_Handler *handler = mASTHandler->mModuleHandlers.ValueAtIndex(i);
    ModuleNode *module = handler->GetASTModule();
    // build CFG
    handler->BuildCFG();
  }

  std::cout << "============= Emitter ===========" << std::endl;
  for (int i = 0; i < size; i++) {
    Module_Handler *handler = mASTHandler->mModuleHandlers.ValueAtIndex(i);
    ModuleNode *module = handler->GetASTModule();
    maplefe::Emitter emitter(module);
    std::string code = emitter.Emit("Convert AST to TypeScript code");
    std::cout << code;
  }
}

// starting point of AST
void A2C::ProcessAST() {
  // loop through module handlers

  if (mEmitTSOnly) {
    EmitTS();
    return;
  }

  unsigned size = mASTHandler->mModuleHandlers.GetNum();
  for (int i = 0; i < size; i++) {
    Module_Handler *handler = mASTHandler->mModuleHandlers.ValueAtIndex(i);
    ModuleNode *module = handler->GetASTModule();

    if (mTraceA2C) {
      std::cout << "============= in ProcessAST ===========" << std::endl;
      std::cout << "srcLang : " << module->GetSrcLangString() << std::endl;
    }

    for(unsigned i = 0; i < module->GetTreesNum(); i++) {
      TreeNode *tnode = module->GetTree(i);
      if (mTraceA2C) {
        tnode->Dump(0);
        std::cout << std::endl;
      }
    }

    if (mTraceA2C) {
      std::cout << "============= AstGraph ===========" << std::endl;
      AstGraph graph(module);
      graph.DumpGraph("After LoadFromAstBuf()", &std::cout);
    }

    // rewirte some AST nodes
    handler->AdjustAST();

    // build scope info
    handler->BuildScope();

    // rename var with same name, i --> i__vN where N is 1, 2, 3 ...
    handler->RenameVar();

    if (mTraceA2C) {
      std::cout << "============= After AdjustAST ===========" << std::endl;
      for(unsigned i = 0; i < module->GetTreesNum(); i++) {
        TreeNode *tnode = module->GetTree(i);
        if (mTraceA2C) {
          tnode->Dump(0);
          std::cout << std::endl;
        }
      }
      AstGraph graph(module);
      graph.DumpGraph("After AdjustAST()", &std::cout);
    }

    // build CFG
    handler->BuildCFG();
    if (mTraceA2C) {
      handler->Dump("After BuildCFG()");
    }

    // remove dead blocks
    handler->RemoveDeadBlocks();

    // type inference
    handler->TypeInference();

    if (mTraceA2C) {
      std::cout << "============= AstGraph ===========" << std::endl;
      AstGraph graph(module);
      graph.DumpGraph("After BuildCFG()", &std::cout);
    }

    if (mTraceA2C) {
      std::cout << "============= AstDump ===========" << std::endl;
      AstDump astdump(module);
      astdump.Dump("After BuildCFG()", &std::cout);
    }

    // data flow analysis for the module
    handler->DataFlowAnalysis();
    if (mTraceA2C) {
      handler->Dump("After DataFlowAnalysis()");
    }

    if (mTraceA2C) {
      std::cout << "============== Dump Scope ==============" << std::endl;
      module->GetRootScope()->Dump(0);
    }
  }

  if (mTraceA2C) {
    std::cout << "============= Emitter ===========" << std::endl;
    unsigned size = mASTHandler->mModuleHandlers.GetNum();
    for (int i = 0; i < size; i++) {
      Module_Handler *handler = mASTHandler->mModuleHandlers.ValueAtIndex(i);
      ModuleNode *module = handler->GetASTModule();
      maplefe::Emitter emitter(module);
      std::string code = emitter.Emit("Convert AST to TypeScript code");
      std::cout << code;
    }
  }

  maplefe::CppEmitter cppemitter(mASTHandler);
  cppemitter.EmitCxxFiles();
}
}

