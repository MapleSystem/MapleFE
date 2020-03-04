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
  std::vector<ASTScope*>::iterator it = mChildren.begin();
  for (; it != mChildren.end(); it++) {
    if (*it == s)
      return;
  }

  mChildren.push_back(s);
  s->SetParent(this);
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
