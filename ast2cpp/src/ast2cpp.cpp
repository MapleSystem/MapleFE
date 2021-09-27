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

void A2C::EmitTS() {
  HandlerIndex size = mASTHandler->mModuleHandlers.GetNum();
  for (HandlerIndex i = 0; i < size; i++) {
    Module_Handler *handler = mASTHandler->mModuleHandlers.ValueAtIndex(i);
    ModuleNode *module = handler->GetASTModule();
    // build CFG
    handler->BuildCFG();
  }

  for (HandlerIndex i = 0; i < size; i++) {
    Module_Handler *handler = mASTHandler->mModuleHandlers.ValueAtIndex(i);
    ModuleNode *module = handler->GetASTModule();
    std::cout << "============= AstDump ===========" << std::endl;
    AstDump astdump(module);
    astdump.Dump("After BuildCFG()", &std::cout);
    std::cout << "============= AstGraph ===========" << std::endl;
    AstGraph graph(module);
    graph.DumpGraph("After BuildCFG()", &std::cout);
    std::cout << "============= Emitter ===========" << std::endl;
    maplefe::Emitter emitter(module);
    std::string code = emitter.Emit("Convert AST to TypeScript code");
    std::cout << code;
  }
}

// starting point of AST
void A2C::ProcessAST() {
  // used for FE verification
  if (mFlags & FLG_emit_ts_only) {
    EmitTS();
    return;
  }

  // loop through module handlers
  HandlerIndex size = mASTHandler->mModuleHandlers.GetNum();
  for (HandlerIndex i = 0; i < size; i++) {
    Module_Handler *handler = mASTHandler->mModuleHandlers.ValueAtIndex(i);
    ModuleNode *module = handler->GetASTModule();

    if (mFlags & FLG_trace_1) {
      std::cout << "============= in ProcessAST ===========" << std::endl;
      std::cout << "srcLang : " << module->GetSrcLangString() << std::endl;

      for(unsigned i = 0; i < module->GetTreesNum(); i++) {
        TreeNode *tnode = module->GetTree(i);
        if (mFlags & FLG_trace_1) {
          tnode->Dump(0);
          std::cout << std::endl;
        }
      }
    }

    if (mFlags & FLG_trace_2) {
      std::cout << "============= AstGraph ===========" << std::endl;
      AstGraph graph(module);
      graph.DumpGraph("After LoadFromAstBuf()", &std::cout);
    }

    // rewirte some AST nodes
    handler->AdjustAST();

    // scope analysis
    handler->ScopeAnalysis();

    if (mFlags & FLG_trace_2) {
      std::cout << "============= After AdjustAST ===========" << std::endl;
      for(unsigned i = 0; i < module->GetTreesNum(); i++) {
        TreeNode *tnode = module->GetTree(i);
        if (mFlags & FLG_trace_1) {
          tnode->Dump(0);
          std::cout << std::endl;
        }
      }
      AstGraph graph(module);
      graph.DumpGraph("After AdjustAST()", &std::cout);
    }

    // build CFG
    handler->BuildCFG();
    if (mFlags & FLG_trace_2) {
      handler->Dump("After BuildCFG()");
    }

    // control flow analysis
    handler->ControlFlowAnalysis();

    // type inference
    handler->TypeInference();

    if (mFlags & FLG_trace_2) {
      std::cout << "============= AstGraph ===========" << std::endl;
      AstGraph graph(module);
      graph.DumpGraph("After BuildCFG()", &std::cout);
    }

    if (mFlags & FLG_trace_2) {
      std::cout << "============= AstDump ===========" << std::endl;
      AstDump astdump(module);
      astdump.Dump("After BuildCFG()", &std::cout);
    }

    // data flow analysis
    handler->DataFlowAnalysis();
    if (mFlags & FLG_trace_2) {
      handler->Dump("After DataFlowAnalysis()");
    }

    if (mFlags & FLG_trace_2) {
      std::cout << "============== Dump Scope ==============" << std::endl;
      module->GetRootScope()->Dump(0);
    }
  }

  if (mFlags & FLG_emit_ts) {
    std::cout << "============= Emitter ===========" << std::endl;
    HandlerIndex size = mASTHandler->mModuleHandlers.GetNum();
    for (HandlerIndex i = 0; i < size; i++) {
      Module_Handler *handler = mASTHandler->mModuleHandlers.ValueAtIndex(i);
      ModuleNode *module = handler->GetASTModule();
      maplefe::Emitter emitter(module);
      std::string code = emitter.Emit("Convert AST to TypeScript code");
      std::cout << code;
    }
  }

  if (mFlags & FLG_trace_2) {
    std::cout << "============= CppEmitter ===========" << std::endl;
  }
  maplefe::CppEmitter cppemitter(mASTHandler, mFlags);
  cppemitter.EmitCxxFiles();
}
}

