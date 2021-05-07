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

#ifndef __AST_HANDLER_HEADER__
#define __AST_HANDLER_HEADER__

#include <stack>
#include <utility>
#include "ast_module.h"
#include "ast.h"
#include "ast_cfg.h"
#include "ast_type.h"
#include "gen_astvisitor.h"

namespace maplefe {

class AST_BB;
class AST_Function;
class AST_CFG;;
class AST_DFA;;

// Each source file is a module
class AST_Handler {
 private:
  MemPool       mMemPool;     // Memory pool for all AST_Function and AST_BB
  ASTModule    *mASTModule;   // for an AST module
  AST_Function *mFunction;    // an init function for statements in module scope
  AST_CFG      *mCFG;
  AST_DFA      *mDFA;
  bool          mTrace;

 public:
  explicit AST_Handler(ASTModule *module, bool trace) :
    mASTModule(module),
    mFunction(nullptr),
    mTrace(trace) {}
  ~AST_Handler() {mMemPool.Release();}

  void BuildCFG();
  void BuildDFA();

  ASTModule *GetASTModule() {return mASTModule;}
  MemPool   *GetMemPool()   {return &mMemPool;}

  void          SetFunction(AST_Function *func) {mFunction = func;}
  AST_Function *GetFunction()                   {return mFunction;}

  bool GetTrace() {return mTrace;}

  void Dump(char *msg);
};
}
#endif