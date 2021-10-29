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

//////////////////////////////////////////////////////////////////////////////////////////////
//                This is the interface to translate AST to C++
//////////////////////////////////////////////////////////////////////////////////////////////

#ifndef __ASTOPT_HEADER__
#define __ASTOPT_HEADER__

#include <vector>
#include "ast_module.h"
#include "ast.h"

namespace maplefe {

class AST_Handler;
class Module_Handler;
class AST_Xxport;

class AstOpt {
private:
  AST_Handler *mASTHandler;
  AST_Xxport  *mASTXxport;
  unsigned     mFlags;

  // module handlers in mASTHandler sorted by import/export dependency
  std::vector<Module_Handler *> mHandlersIdxInOrder;

public:
  explicit AstOpt(AST_Handler *h, unsigned f);
  ~AstOpt() = default;

  AST_Handler *GetASTHandler() {return mASTHandler;}
  unsigned GetModuleNum();
  Module_Handler *GetModuleHandler(unsigned i) { return mHandlersIdxInOrder[i]; }
  void AddModuleHandler(Module_Handler *h) { mHandlersIdxInOrder.push_back(h); }

  virtual void ProcessAST(unsigned trace);

  void CollectInfo();
  void AdjustAST();
  void ScopeAnalysis();

  void BasicAnalysis();
  void BuildCFG();
  void ControlFlowAnalysis();
  void TypeInference();
  void DataFlowAnalysis();
};

}
#endif
