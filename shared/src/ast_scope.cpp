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
  mParent = NULL;
  mTree = NULL;
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

// We are using name address to decide if two names are equal, since we have a
// string pool with any two equal strings will be at the same address.
TreeNode* ASTScope::FindDeclOf(IdentifierNode *inode) {
  for (unsigned i = 0; i < GetDeclNum(); i++) {
    TreeNode *tree = GetDecl(i);
    if (tree->IsIdentifier()) {
      IdentifierNode *id = (IdentifierNode*)tree;
      MASSERT(id->GetType() && "Identifier has no type?");
    }
    if (tree->GetName() == inode->GetName())
      return tree;
  }
  return NULL;
}

// If it's a local declaration, add it to mDecls.
// This is a general common implementaiton, it assumes
// the correct declaration is IdentifierNode with type.
// So, following is not a decl,
//     a;
// It's legal in c/c++ as an expression statement, but not a
// legal decl.

void ASTScope::TryAddDecl(TreeNode *tree) {
  if (tree->IsIdentifier()) {
    IdentifierNode *inode = (IdentifierNode*)tree;
    if (inode->GetType())
      mDecls.PushBack(inode);
  } else if (tree->IsVarList()) {
    VarListNode *vl = (VarListNode*)tree;
    for (unsigned i = 0; i < vl->GetNum(); i++) {
      IdentifierNode *inode = vl->VarAtIndex(i);
      if (inode->GetType())
        mDecls.PushBack(inode);
    }
  }
}

// If it's a local type declaration, add it to mTypes.
void ASTScope::TryAddType(TreeNode *tree) {
  if (tree->IsClass() || tree->IsInterface() || tree->IsFunction()) {
    mTypes.PushBack(tree);
  }
}

void ASTScope::Release() {
  mChildren.Release();
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
