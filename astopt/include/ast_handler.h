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

class CfgBB;
class CfgFunc;
class AST_AST;
class AST_DFA;
class AST_SCP;
class AST_Handler;
class TypeInfer;

// Each source file is a module
class Module_Handler {
 private:
  AST_Handler  *mASTHandler;
  ModuleNode   *mASTModule;  // for an AST module
  CfgFunc      *mCfgFunc;   // initial CfgFunc in module scope
  AST_AST      *mAST;
  AST_DFA      *mDFA;
  AST_SCP      *mSCP;
  TypeInfer    *mTI;
  const char   *mOutputFileName;
  bool          mTrace;
  std::unordered_map<unsigned, CfgBB *> mNodeId2BbMap;

 public:
  // module node id to its ast function vector
  std::unordered_map<unsigned, std::vector<CfgFunc *>> mModuleFuncsMap;
  // only reachable BBs
  std::unordered_map<unsigned, CfgBB *> mBbId2BbMap;
  // identifier node id to decl
  std::unordered_map<unsigned, TreeNode *> mNodeId2Decl;
  // array's element type: decl node id to typeid
  std::unordered_map<unsigned, TypeId> mArrayDeclId2EleTypeIdMap;

 public:
  explicit Module_Handler(bool trace) :
    mCfgFunc(nullptr),
    mAST(nullptr),
    mDFA(nullptr),
    mSCP(nullptr),
    mTI(nullptr),
    mTrace(trace) {}
  ~Module_Handler() {}

  void AdjustAST();
  void BuildScope();
  void RenameVar();
  void BuildCFG();
  void ASTCollectAndDBRemoval(CfgFunc *func);
  void BuildDFA(CfgFunc *func);
  void TypeInference();

  const char *GetOutputFileName()          {return mOutputFileName;}
  void SetOutputFileName(const char *name) {mOutputFileName = name;}

  void SetASTHandler(AST_Handler *h) {mASTHandler = h;}
  AST_Handler *GetASTHandler()       {return mASTHandler;}

  void SetASTModule(ModuleNode *mod) {mASTModule = mod;}
  ModuleNode *GetASTModule()         {return mASTModule;}

  MemPool *GetMemPool();

  void     SetCfgFunc(CfgFunc *func)  {mCfgFunc = func;}
  CfgFunc *GetCfgFunc()               {return mCfgFunc;}

  void SetBbFromNodeId(unsigned id, CfgBB *bb) { mNodeId2BbMap[id] = bb; }
  CfgBB *GetBbFromNodeId(unsigned id)          { return mNodeId2BbMap[id]; }

  void SetBbFromBbId(unsigned id, CfgBB *bb) { mBbId2BbMap[id] = bb; }
  CfgBB *GetBbFromBbId(unsigned id)          { return mBbId2BbMap[id]; }

  bool GetTrace() {return mTrace;}
  AST_AST *GetAST() {return mAST;}
  AST_DFA *GetDFA() {return mDFA;}
  AST_SCP *GetSCP() {return mSCP;}
  TypeInfer *GetTI() {return mTI;}
  void SetAST(AST_AST *p) {mAST = p;}
  void SetDFA(AST_DFA *p) {mDFA = p;}
  void SetSCP(AST_SCP *p) {mSCP = p;}
  void SetTI(TypeInfer *p) {mTI = p;}

  DeclNode *GetDeclOf(IdentifierNode *inode);

  TreeNode *FindDecl(IdentifierNode *node);
  TreeNode *FindType(IdentifierNode *node);
  TreeNode *FindFunc(TreeNode *node);

  void Dump(char *msg);
};

class AST_Handler {
 private:
  MemPool mMemPool;    // Memory pool for all CfgFunc, CfgBB, etc.
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
