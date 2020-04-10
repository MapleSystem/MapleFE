/*
* Copyright (C) [2020] Futurewei Technologies, Inc. All rights reverved.
*
* OpenArkFE is licensed under the Mulan PSL v1.
* You can use this software according to the terms and conditions of the Mulan PSL v1.
* You may obtain a copy of Mulan PSL v1 at:
*
*  http://license.coscl.org.cn/MulanPSL
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR
* FIT FOR A PARTICULAR PURPOSE.
* See the Mulan PSL v1 for more details.
*/

#include "ast_scope.h"

ASTScope::ASTScope(ASTScope *parent) {
  if (parent)
    SetParent(parent);
}

void ASTScope::AddChild(ASTScope *s) {
  for (unsigned i = 0; i < mChildren.GetNum(); i++) {
    ASTScope *scope = mChildren.ValueAtIndex(i);
    if (s == scope)
      return;
  }

  mChildren.PushBack(s);
  s->SetParent(this);
}

// If it's a local declaration, add it to mDecls.
void ASTScope::TryAddDecl(TreeNode *tree) {
  if (tree->IsIdentifier()) {
    IdentifierNode *inode = (IdentifierNode*)tree;
    mDecls.PushBack(inode);
  } else if (tree->IsVarList()) {
    VarListNode *vl = (VarListNode*)tree;
    for (unsigned i = 0; i < vl->GetNum(); i++) {
      IdentifierNode *inode = vl->VarAtIndex(i);
      mDecls.PushBack(inode);
    }
  }
}

// If it's a local type declaration, add it to mTypes.
void ASTScope::TryAddType(TreeNode *tree) {
  if (tree->IsClass()) {
    LocalType lt = {TK_Class, tree};
    mTypes.PushBack(lt);
  } else if (tree->IsInterface()) {
    LocalType lt = {TK_Interface, tree};
    mTypes.PushBack(lt);
  } else if (tree->IsFunction()) {
    FunctionNode *func = (FunctionNode*)tree;
    LocalType lt = {TK_Function, func};
    mTypes.PushBack(lt);
  }
}

void ASTScope::Release() {
  mChildren.Release();
  mTrees.Release();
  mTypes.Release();
  mDecls.Release();
}

///////////////////////////////////////////////////////////////////
//               AST Scope Pool
///////////////////////////////////////////////////////////////////

ASTScopePool::~ASTScopePool() {
}

// Create a new scope under 'parent'.
ASTScope* ASTScopePool::NewScope(ASTScope *parent) {
  char *addr = mMemPool.Alloc(sizeof(ASTScope));
  ASTScope *s = NULL;
  if (parent)
    s = new (addr) ASTScope(parent);
  else
    s = new (addr) ASTScope();
  mScopes.push_back(s);
  return s;
}
