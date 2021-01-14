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

A2M::A2M(const char *filename) : mFileName(filename) {
  // maple::MIRModule mod(mFileName);
  // mMirModule = &mod;
}

void A2M::ProcessAST() {
  //std::cout << "============= Dump AST in ProcessAST ===========" << std::endl;
  for(auto it: gModule.mTrees) {
  //  it->Dump(0);
  }
}
