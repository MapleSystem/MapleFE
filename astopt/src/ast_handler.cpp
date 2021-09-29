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
#include "ast_ast.h"
#include "ast_scp.h"
#include "ast_ti.h"
#include "ast_cfa.h"
#include "ast_dfa.h"
#include "typetable.h"

namespace maplefe {

Module_Handler::~Module_Handler() {
  mNodeId2BbMap.clear();
  mModuleFuncs.clear();
  mBbId2BbMap.clear();
  mBbIdVec.clear();
  mNodeId2Decl.clear();
  mArrayDeclId2EleTypeIdMap.clear();
  delete mCfgFunc;
  delete mAST;
  delete mSCP;
  delete mTI;
  delete mTypeTable;
  delete mCFA;
  delete mDFA;
}

MemPool *Module_Handler::GetMemPool() {
  return mASTHandler->GetMemPool();
}

HandlerIndex AST_Handler::GetHandlerIndex(const char *filename) {
  if (mModuleHandlerMap.find(filename) == mModuleHandlerMap.end())
   return HandlerNotFound;
  return mModuleHandlerMap.at(filename);
}

bool AST_Handler::AddModule(ModuleNode *m) {
  const char *filename = m->GetFilename();
  if (mModuleHandlerMap.find(filename) != mModuleHandlerMap.end())
    return false;
  mModuleHandlerMap[filename] = mModuleHandlers.GetNum();

  Module_Handler *handler = new(mMemPool.Alloc(sizeof(Module_Handler))) Module_Handler(mFlags);
  handler->SetASTModule(m);
  handler->SetASTHandler(this);
  mModuleHandlers.PushBack(handler);
  return true;
}

void Module_Handler::AdjustAST() {
  if (!mAST) {
    mAST = new(GetMemPool()->Alloc(sizeof(AST_AST))) AST_AST(this, mFlags);
  }
  mAST->AdjustAST();
}

void Module_Handler::ScopeAnalysis() {
  if (!mSCP) {
    mSCP = new(GetMemPool()->Alloc(sizeof(AST_SCP))) AST_SCP(this, mFlags);
  }
  if (!mTypeTable) {
    mTypeTable = new(GetMemPool()->Alloc(sizeof(TypeTable))) TypeTable();
  }
  mSCP->ScopeAnalysis();
}

void Module_Handler::TypeInference() {
  if (!mTI) {
    mTI = new(GetMemPool()->Alloc(sizeof(TypeInfer))) TypeInfer(this, mFlags);
  }
  if (!mTypeTable) {
    mTypeTable = new(GetMemPool()->Alloc(sizeof(TypeTable))) TypeTable();
  }
  mTI->TypeInference();
}

void Module_Handler::BuildCFG() {
  CfgBuilder builder(this, mFlags);
  builder.Build();
}

void Module_Handler::ControlFlowAnalysis() {
  if (!mCFA) {
    mCFA = new(GetMemPool()->Alloc(sizeof(AST_CFA))) AST_CFA(this, mFlags);
  }
  mCFA->ControlFlowAnalysis();
}

void Module_Handler::DataFlowAnalysis() {
  if (!mDFA) {
    mDFA = new(GetMemPool()->Alloc(sizeof(AST_DFA))) AST_DFA(this, mFlags);
  }
  mDFA->DataFlowAnalysis();
}

// input an identifire ===> returen the decl node with same name
TreeNode *Module_Handler::FindDecl(IdentifierNode *node) {
  if (!node) {
    return NULL;
  }
  unsigned nid = node->GetNodeId();
  // search the map mNodeId2Decl first
  if (mNodeId2Decl.find(nid) != mNodeId2Decl.end()) {
    return mNodeId2Decl[nid];
  }

  ASTScope *scope = node->GetScope();
  MASSERT(scope && "null scope");
  TreeNode *decl = scope->FindDeclOf(node->GetStrIdx());

  if (decl) {
    unsigned did = decl->GetNodeId();
    mNodeId2Decl[nid] = decl;
  }
  return decl;
}

TreeNode *Module_Handler::FindType(IdentifierNode *node) {
  if (!node) {
    return NULL;
  }
  ASTScope *scope = node->GetScope();
  MASSERT(scope && "null scope");
  TreeNode *type = scope->FindTypeOf(node->GetStrIdx());

  return type;
}

// input a node ==> return the function node contains it
TreeNode *Module_Handler::FindFunc(TreeNode *node) {
  ASTScope *scope = node->GetScope();
  MASSERT(scope && "null scope");
  while (scope) {
    TreeNode *sn = scope->GetTree();
    if (sn->IsFunction()) {
      return sn;
    }
    scope = scope->GetParent();
  }
  return NULL;
}

void Module_Handler::Dump(char *msg) {
  std::cout << msg << " : " << std::endl;
  CfgFunc *func = GetCfgFunc();
  func->Dump();
}

}
