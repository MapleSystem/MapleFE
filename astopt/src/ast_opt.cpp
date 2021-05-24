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

#include "ast_opt.h"
#include "ast_handler.h"
#include "gen_astdump.h"
#include "gen_astgraph.h"
#include "gen_aststore.h"
#include "gen_astload.h"

namespace maplefe {

// starting point of AST
void AstOpt::ProcessAST(bool trace_a2c) {
  mTraceAstOpt = trace_a2c;
  if (mTraceAstOpt) {
    std::cout << "============= in ProcessAST ===========" << std::endl;
    std::cout << "srcLang : " << gModule->GetSrcLangString() << std::endl;
  }
  for(unsigned i = 0; i < gModule->GetTreesNum(); i++) {
    TreeNode *tnode = gModule->GetTree(i);
    if (mTraceAstOpt) {
      tnode->Dump(0);
      std::cout << std::endl;
    }
  }

  if (mTraceAstOpt) {
    std::cout << "============= AstGraph ===========" << std::endl;
    AstGraph graph(gModule);
    graph.DumpGraph("Initial AST", &std::cout);
  }

  AST_Handler handler(gModule, mTraceAstOpt);

  handler.BuildCFG();
  if (mTraceAstOpt) {
    handler.Dump("After handler.BuildCFG()");
  }

  handler.AdjustAST();
  if (mTraceAstOpt) {
    // handler.Dump("After handler.AdjustAST()");
  }

  // rebuild CFG if necessary
  // handler.BuildCFG();

  if (mTraceAstOpt) {
    std::cout << "============= AstGraph ===========" << std::endl;
    AstGraph graph(gModule);
    graph.DumpGraph("After BuildCFG()", &std::cout);
  }

  if (mTraceAstOpt) {
    std::cout << "============= AstDump ===========" << std::endl;
    AstDump astdump(gModule);
    astdump.Dump("After BuildCFG()", &std::cout);
  }

  handler.BuildDFA();
  if (mTraceAstOpt) {
    // handler.Dump("After handler.BuildDFA()");
  }
}
}

