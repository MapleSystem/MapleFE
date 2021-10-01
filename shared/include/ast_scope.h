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

#ifndef __AST_SCOPE_H__
#define __AST_SCOPE_H__

#include <vector>

#include "ast.h"
#include "mempool.h"
#include "container.h"

namespace maplefe {

////////////////////////////////////////////////////////////////////////////
//                         AST Scope
// Scope in a file are arranged as a tree. The root of each tree
// is a top level of scope in the module. However, the module has a topmost
// scope which could contain the file level variables.
////////////////////////////////////////////////////////////////////////////

class ASTScope {
private:
  ASTScope *mParent;

  // TreeNode corresponding to this scope. It's a different story of
  // global scope and other local scope. Global scope has no corresponding
  // tree with it. It actually related to the module. Local scope always
  // has a local TreeNode with it.
  TreeNode *mTree;

  SmallVector<ASTScope*> mChildren;

  // Local User types like local class,etc.
  SmallVector<TreeNode*> mTypes;

  // The decls in a scope could have many variaties, it could be a variable, a
  // class, a field of a class, a method of a class, an interface, a lambda, etc.
  // So TreeNode is the only choice for it.
  SmallVector<TreeNode*> mDecls;

public:
  ASTScope() : mParent(NULL), mTree(NULL) {}
  ASTScope(ASTScope *p) : mTree(NULL) { SetParent(p); }
  ASTScope(ASTScope *p, TreeNode *t)  { SetParent(p); SetTree(t); }
  ~ASTScope() {Release();}

  // It's the caller's duty to make sure p is not NULL
  void SetParent(ASTScope *p) {mParent = p; if(p) p->AddChild(this);}
  ASTScope* GetParent() {return mParent;}

  TreeNode* GetTree() {return mTree;}
  void SetTree(TreeNode* t) {mTree = t; if(t) t->SetScope(this);}

  void AddChild(ASTScope*);

  unsigned GetChildrenNum() {return mChildren.GetNum();}
  unsigned GetDeclNum() {return mDecls.GetNum();}
  unsigned GetTypeNum() {return mTypes.GetNum();}
  ASTScope* GetChild(unsigned i) {return mChildren.ValueAtIndex(i);}
  TreeNode* GetDecl(unsigned i) {return mDecls.ValueAtIndex(i);}
  TreeNode* GetType(unsigned i) {return mTypes.ValueAtIndex(i);}

  TreeNode* FindDeclOf(unsigned stridx);
  TreeNode* FindTypeOf(unsigned stridx);

  void AddDecl(TreeNode *n) {mDecls.PushBack(n);}
  void AddType(TreeNode *n) {mTypes.PushBack(n);}
  void TryAddDecl(TreeNode *n);
  void TryAddType(TreeNode *n);

  bool IsAncestor(ASTScope *ancestor);

  void Dump(unsigned indent = 0);

  virtual void Release();
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

}
#endif
