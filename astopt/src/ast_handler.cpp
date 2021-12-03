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
#include "ast_info.h"
#include "ast_adj.h"
#include "ast_scp.h"
#include "ast_ti.h"
#include "ast_cfa.h"
#include "ast_dfa.h"
#include "ast_util.h"
#include "ast_xxport.h"
#include "astopt.h"
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
  delete mINFO;
  delete mADJ;
  delete mSCP;
  delete mTI;
  delete mCFA;
  delete mDFA;
  delete mUtil;
}

MemPool *Module_Handler::GetMemPool() {
  return mASTHandler->GetMemPool();
}

Module_Handler *AST_Handler::GetModuleHandler(unsigned i) {
  return mModuleHandlers.ValueAtIndex(i);
}

Module_Handler *AST_Handler::GetModuleHandler(ModuleNode *module) {
  for (unsigned i = 0; i < mModuleHandlers.GetNum(); i++) {
    Module_Handler *h = mModuleHandlers.ValueAtIndex(i);
    if (h->GetASTModule() == module) {
      return h;
    }
  }
  return NULL;
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
  mModuleHandlerMap[filename] = mSize;

  Module_Handler *handler = new(mMemPool.Alloc(sizeof(Module_Handler))) Module_Handler(mFlags);
  handler->SetASTModule(m);
  handler->SetASTHandler(this);
  mModuleHandlers.PushBack(handler);
  mSize++;
  return true;
}

void Module_Handler::CollectInfo() {
  if (!mUtil) {
    mUtil = new(GetMemPool()->Alloc(sizeof(AST_Util))) AST_Util(this, mFlags);
  }
  if (!mINFO) {
    mINFO = new(GetMemPool()->Alloc(sizeof(AST_INFO))) AST_INFO(this, mFlags);
  }
  mINFO->CollectInfo();
}

void Module_Handler::AdjustAST() {
  if (!mADJ) {
    mADJ = new(GetMemPool()->Alloc(sizeof(AST_ADJ))) AST_ADJ(this, mFlags);
  }
  mADJ->AdjustAST();
}

void Module_Handler::ScopeAnalysis() {
  if (!mSCP) {
    mSCP = new(GetMemPool()->Alloc(sizeof(AST_SCP))) AST_SCP(this, mFlags);
  }
  mSCP->ScopeAnalysis();
}

void Module_Handler::TypeInference() {
  if (!mTI) {
    mTI = new(GetMemPool()->Alloc(sizeof(TypeInfer))) TypeInfer(this, mFlags);
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

AstOpt *Module_Handler::GetAstOpt() {
  return mASTHandler->GetAstOpt();
}

AST_XXport *Module_Handler::GetASTXXport() {
  return mASTHandler->GetAstOpt()->GetASTXXport();
}

// input an identifire ===> returen the decl node with same name
TreeNode *Module_Handler::FindDecl(IdentifierNode *node, bool deep) {
  TreeNode *decl = NULL;
  if (!node) {
    return decl;
  }
  unsigned nid = node->GetNodeId();

  // search the existing map mNodeId2Decl first
  if (mNodeId2Decl.find(nid) != mNodeId2Decl.end()) {
    decl = mNodeId2Decl[nid];
    // deep and decl is imported, chase down
    if (deep && decl && !decl->IsTypeIdModule() &&
        GetASTXXport()->IsImportedDeclId(mHidx, decl->GetNodeId())) {
      decl = GetASTXXport()->GetExportedNodeFromImportedNode(mHidx, decl->GetNodeId());
    }
    return decl;
  }

  ASTScope *scope = node->GetScope();
  MASSERT(scope && "null scope");

  TreeNode *tree = scope->GetTree();
  unsigned stridx = node->GetStrIdx();

  decl = mINFO->GetField(tree->GetNodeId(), stridx);

  if (!decl) {
    decl = scope->FindDeclOf(stridx);
  }

  if (decl) {
    unsigned did = decl->GetNodeId();
    mNodeId2Decl[nid] = decl;
  }

  if (deep && decl && !decl->IsTypeIdModule() &&
      GetASTXXport()->IsImportedDeclId(mHidx, decl->GetNodeId())) {
    decl = GetASTXXport()->GetExportedNodeFromImportedNode(mHidx, decl->GetNodeId());
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

void Module_Handler::AddDirectField(TreeNode *node) {
  mDirectFieldSet.insert(node->GetNodeId());
}

bool Module_Handler::IsDirectField(TreeNode *node) {
  return mDirectFieldSet.find(node->GetNodeId()) != mDirectFieldSet.end();
}

bool Module_Handler::IsFromLambda(TreeNode *node) {
  if (!node) {
    return false;
  }
  unsigned nid = node->GetNodeId();
  return mINFO->IsFromLambda(nid);
}

bool Module_Handler::IsDef(TreeNode *node) {
  return mDFA->IsDef(node->GetNodeId());
}

bool Module_Handler::IsCppField(TreeNode *node) {
  return mUtil->IsCppField(node);
}

void Module_Handler::Dump(char *msg) {
  std::cout << msg << " : " << std::endl;
  CfgFunc *func = GetCfgFunc();
  func->Dump();
}

}
