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
#include "container.h"

////////////////////////////////////////////////////////////////////////////
//                         AST Scope
// Scope in a file are arranged as a tree. The root of each tree
// is a top level of scope in the module. However, the module has a topmost
// scope which could contain the file level variables.
////////////////////////////////////////////////////////////////////////////

// Local Types involved in the scope.
enum TypeKind {
  TK_Prim,
  TK_Class,
  TK_Interface,
  TK_Function,
  TK_Struct,
  TK_Null
};

struct LocalType {
  TypeKind  mTypeId;
  TreeNode *mTree;
};

class ASTScope {
private:
  ASTScope *mParent;

  // TreeNode corresponding to this scope. It's a different story of
  // global scope and other local scope. Global scope has no corresponding
  // tree with it. It actually related to the module. Local scope always
  // has a local TreeNode with it.
  TreeNode *mTree;
 
  SmallVector<ASTScope*> mChildren;
  SmallVector<LocalType> mTypes;   // Local types. Could be primitive or
                                   // user type include class, struct, etc.

  // Local Variable Decls. We use IdentifierNode here. The type info is
  // inside the IdentifierNode class.
  SmallVector<IdentifierNode*> mDecls;

public:
  ASTScope() : mParent(NULL), mTree(NULL) {}
  ASTScope(ASTScope *p);
  ~ASTScope() {Release();}

  // It's the caller's duty to make sure p is not NULL
  void SetParent(ASTScope *p) {mParent = p; p->AddChild(this);}

  TreeNode* GetTree() {return mTree;}
  void SetTree(TreeNode* t) {mTree = t;}

  void AddChild(ASTScope*);

  unsigned GetDeclNum() {return mDecls.GetNum();}
  unsigned GetTypeNum() {return mTypes.GetNum();}
  IdentifierNode* GetDecl(unsigned i) {return mDecls.ValueAtIndex(i);}
  LocalType       GetType(unsigned i) {return mTypes.ValueAtIndex(i);}

  // Each language could have different specification for declaration,
  // types, or else. So we put them as virtual functions. So is the
  // Release(). We provide the common implementation which most languages
  // adopt.

  virtual void TryAddDecl(TreeNode *n);
  virtual void TryAddType(TreeNode *n);

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

#endif
