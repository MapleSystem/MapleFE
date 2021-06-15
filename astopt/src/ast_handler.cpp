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
#include "ast_dfa.h"
#include "ast_ti.h"

namespace maplefe {

MemPool *Module_Handler::GetMemPool() {
  return mASTHandler->GetMemPool();
}

void AST_Handler::AddModule(ModuleNode *m) {
  Module_Handler *handler = new(mMemPool.Alloc(sizeof(Module_Handler))) Module_Handler(mTrace);
  handler->SetASTModule(m);
  handler->SetASTHandler(this);
  mModuleHandlers.PushBack(handler);
}

void Module_Handler::BuildCFG() {
  if (!mCFG) {
    mCFG = new(GetMemPool()->Alloc(sizeof(AST_CFG))) AST_CFG(this, mTrace);
  }
  mCFG->Build();
}

void Module_Handler::AdjustAST() {
  if (!mAST) {
    mAST = new(GetMemPool()->Alloc(sizeof(AST_AST))) AST_AST(this, mTrace);
  }
  mAST->AdjustAST();
}

void Module_Handler::ASTCollectAndDBRemoval(AstFunction *func) {
  if (!mAST) {
    mAST = new(GetMemPool()->Alloc(sizeof(AST_AST))) AST_AST(this, mTrace);
  }
  mAST->ASTCollectAndDBRemoval(func);
}

void Module_Handler::BuildDFA(AstFunction *func) {
  if (!mDFA) {
    mDFA = new(GetMemPool()->Alloc(sizeof(AST_DFA))) AST_DFA(this, mTrace);
  }
  mDFA->Build(func);
}

void Module_Handler::BuildScope(ModuleNode *mod) {
  if (!mDFA) {
    mDFA = new(GetMemPool()->Alloc(sizeof(AST_DFA))) AST_DFA(this, mTrace);
  }
  mDFA->BuildScope(mod);
}

// input an identifire ===> returen the decl node with same name
TreeNode *Module_Handler::FindDecl(IdentifierNode *inode) {
  TreeNode *decl = NULL;
  TreeNode *p = inode->GetParent();
  while (p) {
    if (p->IsScope()) {
      ASTScope *scope = mNodeId2Scope[p->GetNodeId()];
      // it will trace back to top level
      decl = scope->FindDeclOf(inode);
      break;
    }
    p = p->GetParent();
  }
  return decl;
}

// input a decl node ==> return the function node contains it
TreeNode *Module_Handler::FindFunc(IdentifierNode *inode) {
  TreeNode *p = inode->GetParent();
  while (p) {
    if (p->IsFunction()) {
      return p;
    }
    p = p->GetParent();
  }
  return NULL;
}

void Module_Handler::TypeInference(AstFunction *func) {
  if (!mTI) {
    mTI = new(GetMemPool()->Alloc(sizeof(TypeInfer))) TypeInfer(this, mTrace);
  }
  mTI->TypeInference(func);
}

void Module_Handler::Dump(char *msg) {
  std::cout << std::endl << msg << ":" << std::endl;
  AstFunction *func = GetFunction();
  func->Dump();
}

}
