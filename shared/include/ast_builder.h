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
  bool mIsTreeNode;
  union {
    TreeNode   *mTreeNode;
    Token      *mToken;
  }mData;
};

class ASTBuilder {
public:
  unsigned                mActionId;
  std::vector<Param>      mParams;
  TreePool               *mTreePool;
public:
  ASTBuilder(TreePool *p) : mTreePool(p) {}
  ~ASTBuilder() {}

  void AddParam(Param p) {mParams.push_back(p);}
  void ClearParams() {mParams.clear();}

  TreeNode* CreateTokenTreeNode(const Token*);

  TreeNode* Build();
  TreeNode* BuildUnaryOperation();
  TreeNode* BuildBinaryOperation();
  TreeNode* BuildAssignment();
  TreeNode* BuildReturn();
  TreeNode* BuildDecl();
  TreeNode* AddAttribute();

};

#endif
