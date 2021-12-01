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
#include <cstring>
#include <map>
#include <climits>
#include "ast_module.h"
#include "ast.h"
#include "astopt.h"
#include "ast_cfg.h"
#include "ast_type.h"
#include "ast_common.h"
#include "gen_astvisitor.h"

#define DEFAULTVALUE 0xdeadbeef

namespace maplefe {

class CfgBB;
class CfgFunc;
class AST_INFO;
class AST_ADJ;
class AST_CFA;
class AST_DFA;
class AST_SCP;
class AST_Util;
class AST_XXport;;
class Module_Handler;
class TypeInfer;
class TypeTable;
class AstOpt;

using HandlerIndex = unsigned;
const HandlerIndex HandlerNotFound = UINT_MAX;

struct StrLess {
  bool operator()(const char *p, const char *q) const {
    return std::strcmp(p, q) < 0;
  }
};

class AST_Handler {
 private:
  MemPool  mMemPool;    // Memory pool for all CfgFunc, CfgBB, etc.
  AstOpt  *mAstOpt;
  unsigned mSize;
  unsigned mFlags;

  // vector of all AST modules
  SmallVector<Module_Handler *> mModuleHandlers;

 public:
  // mapping of mModuleHandlers index with its corresponding filename as its key
  std::map<const char*, HandlerIndex, StrLess> mModuleHandlerMap;

  explicit AST_Handler(unsigned f) : mSize(0), mFlags(f) {}
  ~AST_Handler() {mMemPool.Release();}

  MemPool *GetMemPool() {return &mMemPool;}

  AstOpt *GetAstOpt() {return mAstOpt;}
  void SetAstOpt(AstOpt *opt) {mAstOpt = opt;}

  Module_Handler *GetModuleHandler(unsigned i);
  Module_Handler *GetModuleHandler(ModuleNode *module);

  unsigned GetSize() {return mSize;}

  // If m does not exist in mModuleHandlerMap,
  //    create an object of Module_Handler for module m
  //    add this object to mModuleHandlers
  //    map its corresponding filename and the index of this object in mModuleHandlers in mModuleHandlerMap
  //    return true
  // Otherwise,
  //    return false
  bool AddModule(ModuleNode *m);

  // Return an index of mModuleHandlers if filename exists in mModuleHandlerMap, otherwise return HandlerNotFound
  HandlerIndex GetHandlerIndex(const char *filename);
};

// Each source file is a module
class Module_Handler {
 private:
  AST_Handler  *mASTHandler;
  ModuleNode   *mASTModule;  // for an AST module
  CfgFunc      *mCfgFunc;    // initial CfgFunc in module scope
  AST_INFO     *mINFO;
  AST_ADJ      *mADJ;
  AST_SCP      *mSCP;
  TypeInfer    *mTI;
  AST_CFA      *mCFA;
  AST_DFA      *mDFA;
  AST_Util     *mUtil;
  const char   *mOutputFilename;
  unsigned      mHidx;       // handler index in AST_Handler

  unsigned      mFlags;

  std::unordered_map<unsigned, CfgBB *> mNodeId2BbMap;

 public:
  // module's ast function vector
  std::vector<CfgFunc *> mModuleFuncs;
  // all BBs
  std::unordered_map<unsigned, CfgBB *> mBbId2BbMap;
  // bbid vec - only reachable BBs
  std::vector<unsigned> mBbIdVec;
  // identifier node id to decl
  std::unordered_map<unsigned, TreeNode *> mNodeId2Decl;
  // array's element type: decl node id to typeid
  std::unordered_map<unsigned, TypeId> mArrayDeclId2EleTypeIdMap;
  // fields' nodeid set
  std::unordered_set<unsigned> mDirectFieldSet;

 public:
  explicit Module_Handler(unsigned f) :
    mCfgFunc(nullptr),
    mINFO(nullptr),
    mADJ(nullptr),
    mSCP(nullptr),
    mTI(nullptr),
    mCFA(nullptr),
    mDFA(nullptr),
    mUtil(nullptr),
    mFlags(f) {}
  ~Module_Handler();

  void CollectInfo();
  void AdjustAST();
  void ScopeAnalysis();
  void TypeInference();
  void BuildCFG();
  void ControlFlowAnalysis();
  void DataFlowAnalysis();

  const char *GetOutputFilename()          {return mOutputFilename;}
  void SetOutputFilename(const char *name) {mOutputFilename = name;}

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

  unsigned GetFlags() {return mFlags;}
  unsigned GetHidx() {return mHidx;}
  AST_INFO *GetINFO() {return mINFO;}
  AST_ADJ *GetADJ() {return mADJ;}
  AST_CFA *GetCFA() {return mCFA;}
  AST_DFA *GetDFA() {return mDFA;}
  AST_SCP *GetSCP() {return mSCP;}
  TypeInfer *GetTI() {return mTI;}
  AST_Util *GetUtil() {return mUtil;}
  AstOpt *GetAstOpt();
  AST_XXport *GetASTXXport();

  void SetHidx(unsigned idx) {mHidx = idx;}
  void SetINFO(AST_INFO *p) {mINFO = p;}
  void SetADJ(AST_ADJ *p) {mADJ = p;}
  void SetCFA(AST_CFA *p) {mCFA = p;}
  void SetDFA(AST_DFA *p) {mDFA = p;}
  void SetSCP(AST_SCP *p) {mSCP = p;}
  void SetTI(TypeInfer *p) {mTI = p;}
  void SetUtil(AST_Util *p) {mUtil = p;}

  DeclNode *GetDeclOf(IdentifierNode *inode);

  // deep true  : find Decl in imported module as well
  //      false : find Decl in current module only
  TreeNode *FindDecl(IdentifierNode *node, bool deep = false);

  TreeNode *FindType(IdentifierNode *node);
  TreeNode *FindFunc(TreeNode *node);

  void AddDirectField(TreeNode *node);
  bool IsDirectField(TreeNode *node);

  bool IsFromLambda(TreeNode *node);

  void AddNodeId2DeclMap(unsigned nid, TreeNode *node) {
    mNodeId2Decl[nid] = node;
  }

  template <typename T>
  T *NewTreeNode() {
    T *node = (T*)gTreePool.NewTreeNode(sizeof(T));
    new (node) T();
    AstOpt *opt = mASTHandler->GetAstOpt();
    opt->AddNodeId2NodeMap(node);
    return node;
  }

  // API to check a node is c++ field which satisfy both:
  // 1. direct field
  // 2. its name is valid in c++
  bool IsCppField(TreeNode *node);

  void Dump(char *msg);
};

}
#endif
