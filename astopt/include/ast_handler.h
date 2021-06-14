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

class AstBasicBlock;
class AstFunction;
class AST_CFG;
class AST_AST;
class AST_DFA;
class AST_Handler;

// Each source file is a module
class Module_Handler {
 private:
  AST_Handler  *mASTHandler;
  ModuleNode   *mASTModule;  // for an AST module
  AstFunction  *mFunction;   // an init function for statements in module scope
  AST_CFG      *mCFG;
  AST_AST      *mAST;
  AST_DFA      *mDFA;
  const char   *mOutputFileName;
  bool          mTrace;
  std::unordered_map<unsigned, AstBasicBlock *> mNodeId2BbMap;

 public:
  // module node id to its ast function vector
  std::unordered_map<unsigned, std::vector<AstFunction *>> mModuleFuncsMap;
  // only reachable BBs
  std::unordered_map<unsigned, AstBasicBlock *> mBbId2BbMap;
  // scope node id to scope
  std::unordered_map<unsigned, ASTScope *> mNodeId2Scope;

 public:
  explicit Module_Handler(bool trace) :
    mFunction(nullptr),
    mCFG(nullptr),
    mAST(nullptr),
    mDFA(nullptr),
    mTrace(trace) {}
  ~Module_Handler() {}

  void AdjustAST();
  void BuildScope(ModuleNode *mod);
  void BuildCFG();
  void ASTCollectAndDBRemoval(AstFunction *func);
  void BuildDFA(AstFunction *func);

  const char *GetOutputFileName() {return mOutputFileName;}
  void SetOutputFileName(const char *name) {mOutputFileName = name;}

  void SetASTHandler(AST_Handler *h) {mASTHandler = h;}
  AST_Handler *GetASTHandler() {return mASTHandler;}

  void SetASTModule(ModuleNode *mod) {mASTModule = mod;}
  ModuleNode *GetASTModule() {return mASTModule;}

  MemPool *GetMemPool();

  void         SetFunction(AstFunction *func)  {mFunction = func;}
  AstFunction *GetFunction()                   {return mFunction;}

  void SetBbFromNodeId(unsigned id, AstBasicBlock *bb) { mNodeId2BbMap[id] = bb; }
  AstBasicBlock *GetBbFromNodeId(unsigned id) { return mNodeId2BbMap[id]; }

  void SetBbFromBbId(unsigned id, AstBasicBlock *bb) { mBbId2BbMap[id] = bb; }
  AstBasicBlock *GetBbFromBbId(unsigned id) { return mBbId2BbMap[id]; }

  bool GetTrace() {return mTrace;}
  AST_CFG *GetCFG() {return mCFG;}
  AST_AST *GetAST() {return mAST;}
  AST_DFA *GetDFA() {return mDFA;}
  void SetCFG(AST_CFG *p) {mCFG = p;}
  void SetAST(AST_AST *p) {mAST = p;}
  void SetDFA(AST_DFA *p) {mDFA = p;}

  TreeNode *FindDecl(IdentifierNode *inode);
  TreeNode *FindFunc(IdentifierNode *inode);

  void Dump(char *msg);
};

class AST_Handler {
 private:
  MemPool mMemPool;    // Memory pool for all AstFunction, AstBasicBlock, etc.
  bool    mTrace;
 public:
  // vector of all AST modules
  SmallVector<Module_Handler *> mModuleHandlers;

  explicit AST_Handler(bool trace) : mTrace(trace) {}
  ~AST_Handler() {mMemPool.Release();}

  MemPool *GetMemPool() {return &mMemPool;}
  // Create an object of Module_Handler and set it for module m
  void AddModule(ModuleNode *m);
};

}
#endif
