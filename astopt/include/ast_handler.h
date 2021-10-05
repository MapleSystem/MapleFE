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
#include "ast_cfg.h"
#include "ast_type.h"
#include "gen_astvisitor.h"

namespace maplefe {

#define NOTYETIMPL(M) { if (mFlags & FLG_trace) { MNYI(M);        }}
#define MSGNOLOC0(M)  { if (mFlags & FLG_trace_3) { MMSGNOLOC0(M);  }}
#define MSGNOLOC(M,v) { if (mFlags & FLG_trace_3) { MMSGNOLOC(M,v); }}

enum AST_Flags {
  FLG_trace_1      = 0x00000001,
  FLG_trace_2      = 0x00000002,
  FLG_trace_3      = 0x00000004,
  FLG_trace_4      = 0x00000008,
  FLG_trace        = 0x0000000f,

  FLG_emit_ts      = 0x00000010,
  FLG_emit_ts_only = 0x00000020,
  FLG_format_cpp   = 0x00000040,
  FLG_no_imported  = 0x00000080,
};

class CfgBB;
class CfgFunc;
class AST_AST;
class AST_CFA;
class AST_DFA;
class AST_SCP;
class AST_Handler;
class TypeInfer;
class TypeTable;

// Each source file is a module
class Module_Handler {
 private:
  AST_Handler  *mASTHandler;
  ModuleNode   *mASTModule;  // for an AST module
  CfgFunc      *mCfgFunc;   // initial CfgFunc in module scope
  AST_AST      *mAST;
  AST_SCP      *mSCP;
  TypeInfer    *mTI;
  AST_CFA      *mCFA;
  AST_DFA      *mDFA;
  const char   *mOutputFilename;
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

 public:
  explicit Module_Handler(unsigned f) :
    mCfgFunc(nullptr),
    mAST(nullptr),
    mSCP(nullptr),
    mTI(nullptr),
    mCFA(nullptr),
    mDFA(nullptr),
    mFlags(f) {}
  ~Module_Handler();

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
  AST_AST *GetAST() {return mAST;}
  AST_CFA *GetCFA() {return mCFA;}
  AST_DFA *GetDFA() {return mDFA;}
  AST_SCP *GetSCP() {return mSCP;}
  TypeInfer *GetTI() {return mTI;}
  void SetAST(AST_AST *p) {mAST = p;}
  void SetCFA(AST_CFA *p) {mCFA = p;}
  void SetDFA(AST_DFA *p) {mDFA = p;}
  void SetSCP(AST_SCP *p) {mSCP = p;}
  void SetTI(TypeInfer *p) {mTI = p;}

  DeclNode *GetDeclOf(IdentifierNode *inode);

  TreeNode *FindDecl(IdentifierNode *node);
  TreeNode *FindType(IdentifierNode *node);
  TreeNode *FindFunc(TreeNode *node);

  void Dump(char *msg);
};

struct StrLess {
  bool operator()(const char *p, const char *q) const {
    return std::strcmp(p, q) < 0;
  }
};

using HandlerIndex = unsigned;
const HandlerIndex HandlerNotFound = UINT_MAX;

class AST_Handler {
 private:
  MemPool  mMemPool;    // Memory pool for all CfgFunc, CfgBB, etc.
  unsigned mFlags;
 public:
  // vector of all AST modules
  SmallVector<Module_Handler *> mModuleHandlers;
  // mapping of mModuleHandlers index with its corresponding filename as its key
  std::map<const char*, HandlerIndex, StrLess> mModuleHandlerMap;

  explicit AST_Handler(unsigned f) : mFlags(f) {}
  ~AST_Handler() {mMemPool.Release();}

  MemPool *GetMemPool() {return &mMemPool;}

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

}
#endif
