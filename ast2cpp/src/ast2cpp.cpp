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

// starting point of AST
void A2C::ProcessAST() {
  // loop through modules
  unsigned size = mASTHandler->mASTModules.GetNum();
  for (int i = 0; i < size; i++) {
    // set gModule
    gModule = mASTHandler->mASTModules.ValueAtIndex(i);
    mASTHandler->SetASTModule(gModule);

    if (mTraceA2C) {
      std::cout << "============= in ProcessAST ===========" << std::endl;
      std::cout << "srcLang : " << gModule->GetSrcLangString() << std::endl;
    }

    for(unsigned i = 0; i < gModule->GetTreesNum(); i++) {
      TreeNode *tnode = gModule->GetTree(i);
      if (mTraceA2C) {
        tnode->Dump(0);
        std::cout << std::endl;
      }
    }

    if (mTraceA2C) {
      std::cout << "============= AstGraph ===========" << std::endl;
      AstGraph graph(gModule);
      graph.DumpGraph("After LoadFromAstBuf()", &std::cout);
    }

    mASTHandler->AdjustAST();
    if (mTraceA2C) {
      std::cout << "============= After AdjustAST ===========" << std::endl;
      for(unsigned i = 0; i < gModule->GetTreesNum(); i++) {
        TreeNode *tnode = gModule->GetTree(i);
        if (mTraceA2C) {
          tnode->Dump(0);
          std::cout << std::endl;
        }
      }
      AstGraph graph(gModule);
      graph.DumpGraph("After AdjustAST()", &std::cout);
    }

    mASTHandler->BuildCFG();
    if (mTraceA2C) {
      mASTHandler->Dump("After mASTHandler->BuildCFG()");
    }

    mASTHandler->ASTCollectAndDBRemoval();
    if (mTraceA2C) {
      mASTHandler->Dump("After mASTHandler->ASTCollectAndDBRemoval()");
    }

    if (mTraceA2C) {
      std::cout << "============= AstGraph ===========" << std::endl;
      AstGraph graph(gModule);
      graph.DumpGraph("After BuildCFG()", &std::cout);
    }

    if (mTraceA2C) {
      std::cout << "============= AstDump ===========" << std::endl;
      AstDump astdump(gModule);
      astdump.Dump("After BuildCFG()", &std::cout);
    }

    mASTHandler->BuildDFA();
    if (mTraceA2C) {
      // mASTHandler->Dump("After mASTHandler->BuildDFA()");
    }

    if (mTraceA2C) {
      std::cout << "============= AstStore ===========" << std::endl;
      AstStore saveAst(gModule);
      saveAst.StoreInAstBuf();
      //AstBuffer &ast_buf = saveAst.GetAstBuf();
    }
  }
}
}

