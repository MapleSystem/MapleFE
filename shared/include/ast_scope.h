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

#ifndef __AST_SCOPE_H__
#define __AST_SCOPE_H__

#include <vector>

#include "ast.h"
#include "mempool.h"

////////////////////////////////////////////////////////////////////////////
//                         AST Scope
// Scope in a file are arranged as a tree. The root of each tree
// is a top level of scope in the module. However, the module has a topmost
// scope which could contain the file level variables.
////////////////////////////////////////////////////////////////////////////

class ASTScope {
public:
  ASTScope              *mParent;
  std::vector<ASTScope*> mChildren;
  TreeNode              *mTree;   // corresponding TreeNode
  std::vector<TreeNode*> mDecls;  // Local Decls
public:
  ASTScope(){}
  ASTScope(ASTScope *p);
  ~ASTScope() {}

  // It's the caller's duty to make sure p is not NULL
  void SetParent(ASTScope *p) {mParent = p; p->AddChild(this);}

  void AddChild(ASTScope *s);
  void AddDecl(TreeNode *n) {mDecls.push_back(n);}
};

////////////////////////////////////////////////////////////////////////////
//                         AST Scope Pool
// The size of ASTScope is fixed, good to use MemPool to allocate the memory.
// The ASTScopePool is one for each of ASTModule.
////////////////////////////////////////////////////////////////////////////

class ASTScopePool {
private:
  MemPool                mMemPool;
  std::vector<ASTScope*> mScopes;
public:
  ASTScopePool() {}
  ~ASTScopePool();
  
  ASTScope* NewScope(ASTScope *parent);
};

#endif
