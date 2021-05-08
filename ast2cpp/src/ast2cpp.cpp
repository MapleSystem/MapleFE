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

namespace maplefe {

// starting point of AST
void A2C::ProcessAST(bool trace_a2c) {
  mTraceA2C = trace_a2c;
  if (mTraceA2C) {
    std::cout << "============= in ProcessAST ===========" << std::endl;
    std::cout << "srcLang : " << gModule.GetSrcLangString() << std::endl;
  }
  for(auto it: gModule.mTrees) {
    TreeNode *tnode = it->mRootNode;
    if (mTraceA2C) {
      tnode->Dump(0);
      std::cout << std::endl;
    }
  }

  AST_Handler handler(&gModule, mTraceA2C);

  handler.BuildCFG();
  if (mTraceA2C) {
    handler.Dump("After handler.BuildCFG()");
  }

  if (mTraceA2C) {
    std::cout << "============= AstDump ===========" << std::endl;
    AstDump astdump(&gModule);
    astdump.Dump("After_Building_CFG", &std::cout);
  }

  if (mTraceA2C) {
    std::cout << "============= AstGraph ===========" << std::endl;
    AstGraph graph(&gModule);
    graph.DumpGraph("After_Building_CFG", &std::cout);
  }

  handler.BuildDFA();
  if (mTraceA2C) {
    handler.Dump("After handler.BuildDFA()");
  }
}
}

