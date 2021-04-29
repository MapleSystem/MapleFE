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
#include "a2c_astdump.h"

namespace maplefe {

A2C::A2C(const char *filename) : mFileName(filename) {
}

A2C::~A2C() {
}

// starting point of AST
void A2C::ProcessAST(bool trace_a2m) {
  mTraceA2C = trace_a2m;
  if (mTraceA2C) std::cout << "============= in ProcessAST ===========" << std::endl;
  for(auto it: gModule.mTrees) {
    TreeNode *tnode = it->mRootNode;
    if (mTraceA2C) {
      tnode->Dump(0);
      std::cout << std::endl;
    }
  }

  if (mTraceA2C) {
      std::cout << "============= A2C_AstDump ===========" << std::endl;
      A2C_AstDump astdump;
      for(auto it: gModule.mTrees)
          astdump.dump(it->mRootNode);

  }
}
}

