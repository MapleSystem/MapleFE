/*
* Copyright (C) [2021] Futurewei Technologies, Inc. All rights reverved.
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

#ifndef __AST_CFA_HEADER__
#define __AST_CFA_HEADER__

#include <stack>
#include <utility>
#include "ast_module.h"
#include "ast.h"
#include "ast_type.h"
#include "gen_astvisitor.h"

namespace maplefe {

class AST_CFA {
 private:
  Module_Handler  *mHandler;
  unsigned         mFlags;
  std::unordered_set<unsigned> mReachableBbIdx;;

 public:
  explicit AST_CFA(Module_Handler *h, unsigned f) : mHandler(h), mFlags(f) {}
  ~AST_CFA() {}

  void ControlFlowAnalysis();
  void CollectReachableBB();
  void RemoveUnreachableBB();
  void Dump();
};

}
#endif
