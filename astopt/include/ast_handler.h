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
#include <unordered_map>
#include <unordered_set>
#include "ast_module.h"
#include "ast.h"
#include "ast_cfg.h"
#include "ast_type.h"
#include "gen_astvisitor.h"

namespace maplefe {

class AST_BB;
class AST_Function;
class AST_CFG;
class AST_AST;
class AST_DFA;

// Each source file is a module
class AST_Handler {
 private:
  MemPool       mMemPool;    // Memory pool for all AST_Function and AST_BB
  ModuleNode   *mASTModule;  // for an AST module
  AST_Function *mFunction;   // an init function for statements in module scope
  AST_CFG      *mCFG;
  AST_AST      *mAST;
  AST_DFA      *mDFA;
  bool          mTrace;
  std::unordered_map<unsigned, AST_BB *> mNodeId2BbMap;

 public:
  // only reachable BBs
  std::unordered_map<unsigned, AST_BB *> mBbId2BbMap;

 public:
  explicit AST_Handler(ModuleNode *module, bool trace) :
    mASTModule(module),
    mFunction(nullptr),
    mCFG(nullptr),
    mAST(nullptr),
    mDFA(nullptr),
    mTrace(trace) {}
  ~AST_Handler() {mMemPool.Release();}

  void AdjustAST();
  void BuildCFG();
  void ASTCollectAndDBRemoval();
  void BuildDFA();

  ModuleNode *GetASTModule() {return mASTModule;}
  MemPool   *GetMemPool()   {return &mMemPool;}

  void          SetFunction(AST_Function *func) {mFunction = func;}
  AST_Function *GetFunction()                   {return mFunction;}

  void SetBbFromNodeId(unsigned id, AST_BB *bb) { mNodeId2BbMap[id] = bb; }
  AST_BB *GetBbFromNodeId(unsigned id) { return mNodeId2BbMap[id]; }

  void SetBbFromBbId(unsigned id, AST_BB *bb) { mBbId2BbMap[id] = bb; }
  AST_BB *GetBbFromBbId(unsigned id) { return mBbId2BbMap[id]; }

  bool GetTrace() {return mTrace;}

  void Dump(char *msg);
};
}
#endif
