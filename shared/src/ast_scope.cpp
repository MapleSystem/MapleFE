/*
* Copyright (C) [2020] Futurewei Technologies, Inc. All rights reverved.
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

#include "ast_scope.h"
#include "gen_astdump.h"

namespace maplefe {

void ASTScope::AddChild(ASTScope *s) {
  for (unsigned i = 0; i < mChildren.GetNum(); i++) {
    if (s == GetChild(i)) {
      return;
    }
  }

  mChildren.PushBack(s);
  s->SetParent(this);
}

// This is to find the decl having the name as stridx
// starting from local scope
TreeNode* ASTScope::FindDeclOf(unsigned stridx) {
  ASTScope *scope = this;
  while (scope) {
    for (unsigned i = 0; i < scope->GetDeclNum(); i++) {
      TreeNode *tree = scope->GetDecl(i);
      if (tree->GetStrIdx() == stridx) {
        return tree;
      }
    }
    // search parent scope
    scope = scope->mParent;
  }
  return NULL;
}

// This is to find the type having the name as stridx.
//
// starting from local scope
TreeNode* ASTScope::FindTypeOf(unsigned stridx) {
  ASTScope *scope = this;
  while (scope) {
    for (unsigned i = 0; i < scope->GetTypeNum(); i++) {
      TreeNode *tree = scope->GetType(i);
      if (tree->GetStrIdx() == stridx) {
        return tree;
      }
    }
    // search parent scope
    scope = scope->mParent;
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
    for (unsigned i = 0; i < vl->GetVarsNum(); i++) {
      IdentifierNode *inode = vl->GetVarAtIndex(i);
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

bool ASTScope::IsAncestor(ASTScope *ancestor) {
  ASTScope *p = this;
  while (p) {
    if (p == ancestor) {
      return true;
    }
    p = p->GetParent();
  }
  return false;
}

void ASTScope::Dump(unsigned indent) {
  mTree->DumpIndentation(indent);
  std::cout << "scope: " << AstDump::GetEnumNodeKind(mTree->GetKind()) << " " << mTree->GetName() << " " << mTree->GetNodeId() << std::endl;
  for (unsigned i = 0; i < GetDeclNum(); i++) {
    TreeNode *node = GetDecl(i);
    std::string str = "";
    switch (node->GetKind()) {
      case NK_Identifier: str = "    arg: "; break;
      case NK_Decl:       str = "   decl: "; break;
      case NK_Function:   str = "   func: "; break;
      case NK_Struct:     str = " struct: "; break;
      case NK_Class:      str = "  class: "; break;
    }
    if (str.length()) {
      node->DumpIndentation(indent);
      std::string name = node->GetStrIdx() ? node->GetName() : "-";
      std::cout << str << name << " " << node->GetNodeId() << std::endl;
    }
  }

  for (unsigned i = 0; i < GetTypeNum(); i++) {
    TreeNode *node = GetType(i);
    node->DumpIndentation(indent);
    std::string name = node->GetStrIdx() ? node->GetName() : "-";
    std::cout << "   type: " << name << " " << node->GetNodeId() << std::endl;
  }

  for (unsigned i = 0; i < GetChildrenNum(); i++) {
    ASTScope *scope = GetChild(i);
    scope->Dump(indent + 2);
  }
}

}
