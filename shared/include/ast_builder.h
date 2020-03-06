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

#ifndef __AST_BUILDER_HEADER__
#define __AST_BUILDER_HEADER__

#include "ast.h"
#include "ast_mempool.h"

////////////////////////////////////////////////////////////////////////////
//                  The AST Builder
// ASTBuilder takes the action Id and parameter list, to create a sub tree.
// Its main body contains huge amount of building functions.
//
// The treenode mempool is part of ASTTree since the memory goes with the
// tree not the builder. The Builder is purely a collect of functions to build
// sub trees from AppealNode.
//
// This is different than those BuildXXX() functions in AST, which creates
// trees from existing children trees.
////////////////////////////////////////////////////////////////////////////

class Token;

struct Param {
  bool mIsEmpty;    // some parameters could be missing
  bool mIsTreeNode;
  union {
    TreeNode   *mTreeNode;
    Token      *mToken;
  }mData;
};

class ASTScope;

class ASTBuilder {
public:
  // information for a single action
  unsigned                mActionId;
  std::vector<Param>      mParams;

  TreePool               *mTreePool;

private:
  // A few information to locate each decl into their scope
  // 1. Pending declarations before their scope is created
  // 2. Since the scopes are created as a tree, children-first
  //    parent-last order. Some parent's Decls could be created
  //    before children's, so they have to wait in the stack
  //    untile children are done.
  // 3. As the tree could be multiple-layer, we need to record
  //    which Decls are for which scope, so comes the mScopeStartDecls.
  //    Each number in mScopeStartDecls represents the index of
  //    Decl for that layer of scope.
  std::vector<TreeNode*>  mPendingDecls;
  std::vector<unsigned>   mScopeStartDecls;
  ASTScope               *mCurrScope; // current working scope.

  // The last created Decl node. It will be referenced by the
  // following AddAttribute() or other functions.
  TreeNode               *mLastDecl;

public:
  ASTBuilder(TreePool *p) : mTreePool(p) {}
  ~ASTBuilder() {}

  void AddParam(Param p) {mParams.push_back(p);}
  void ClearParams() {mParams.clear();}

  // Create Functions for Token
  TreeNode* CreateTokenTreeNode(const Token*);

  // Create Functions for AppealNode Tree
  TreeNode* Build();
  TreeNode* BuildUnaryOperation();
  TreeNode* BuildBinaryOperation();
  TreeNode* BuildAssignment();
  TreeNode* BuildReturn();
  TreeNode* BuildDecl();
  TreeNode* AddAttribute();

  // Move the remaining Pending Decls into the ASTScope, which is mostly
  // the module's root scope. The remaining Decls are usually the global
  // Decls. This is usually called when BuildAST() is done, which creates
  // the top level ASTTree.
  void AssignRemainingDecls(ASTScope*);
};

#endif
