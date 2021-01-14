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

#include "ast2mpl.h"
#include "mir_function.h"
#include "constant_fold.h"

bool ConstantFoldModule(maple::MIRModule *module) {
  maple::ConstantFold cf(module);
  MapleVector<maple::MIRFunction *> &funcList = module->functionList;
  for (MapleVector<maple::MIRFunction *>::iterator it = funcList.begin(); it != funcList.end(); it++) {
    maple::MIRFunction *curfun = *it;
    maple::BlockNode *block = curfun->body;
    module->SetCurFunction(curfun);
    if (!block) {
      continue;
    }
    cf.Simplify(block);
  }
  return true;
}

A2M::A2M(const char *filename) : mFileName(filename) {
  maple::MIRModule mod(mFileName);
  mMirModule = &mod;
}

void A2M::ProcessAST(bool verbose) {
  mVerbose = verbose;
  if (mVerbose) std::cout << "============= in ProcessAST ===========" << std::endl;
  for(auto it: gModule.mTrees) {
    if (mVerbose) it->Dump(0);
    TreeNode *tnode = it->mRootNode;
    switch (tnode->GetKind()) {
#undef  NODEKIND
#define NODEKIND(K) case NK_##K: Process##K(tnode); break;
#include "ast_nk.def"
      default:
       break;
    }
  }
}
