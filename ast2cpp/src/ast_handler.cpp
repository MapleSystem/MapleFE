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

#include <stack>
#include <set>
#include "ast_handler.h"
#include "ast_cfg.h"
#include "ast_dfa.h"

namespace maplefe {

void AST_Handler::BuildCFG() {
  AST_CFG cfg(this, mTrace);
  cfg.Build();
}

void AST_Handler::BuildDFA() {
  AST_DFA dfa(this, mTrace);
  dfa.Build();
}

void AST_Handler::Dump(char *msg) {
  std::cout << std::endl << msg << ":" << std::endl;
  AST_Function *func = GetFunction();
  func->Dump();
}

}
