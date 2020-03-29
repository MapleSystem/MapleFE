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
private:
  bool mTrace;

public:
  // information for a single action
  unsigned                mActionId;
  std::vector<Param>      mParams;

  TreePool               *mTreePool;

private:
  ASTScope               *mCurrScope; // current working scope.

  // The last created node. It will be referenced by the
  // following AddAttribute() or other functions.
  TreeNode               *mLastTreeNode;

  // Some pending tree nodes exist because their rule tables don't have
  // actions. These pending treenodes are supposed to transferred further
  // to their ancestors, until they are consumed by one of them.
  SmallVector<TreeNode*>  mPendingNodes;

public:
  ASTBuilder(TreePool *p, bool trace=false) : mTreePool(p), mTrace(trace) {}
  ~ASTBuilder() {}

  void SetTrace(bool b) {mTrace = b;}

  void AddParam(Param p) {mParams.push_back(p);}
  void ClearParams() {mParams.clear();}

  // Create Functions for Token
  TreeNode* CreateTokenTreeNode(const Token*);

  TreeNode* Build();

  TreeNode* BuildBlock();

  TreeNode* BuildUnaryOperation();
  TreeNode* BuildPostfixOperation();
  TreeNode* BuildBinaryOperation();

  TreeNode* BuildDecl();
  TreeNode* BuildVarList();

  TreeNode* AddInitTo();
  TreeNode* AddTypeTo();
  TreeNode* AddAttribute();
  TreeNode* AddAttributeTo();

  // Function related
  TreeNode* BuildFunction();
  TreeNode* BuildConstructor();
  TreeNode* AddFunctionBodyTo();

  TreeNode* BuildClass();
  TreeNode* AddClassBody();
  TreeNode* AddSuperClass();
  TreeNode* AddSuperInterface();
  TreeNode* BuildInstInit();

  // Annotation related
  TreeNode* BuildAnnotationType();
  TreeNode* BuildAnnotation();
  TreeNode* AddAnnotationTypeBody();

  // Dimension Related
  TreeNode* BuildDim();
  TreeNode* BuildDims();
  TreeNode* AddDims();
  TreeNode* AddDimsTo();

  // Statements, Control Flow
  TreeNode* BuildAssignment();
  TreeNode* BuildReturn();
  TreeNode* BuildCondBranch();
  TreeNode* AddCondBranchTrueStatement();
  TreeNode* AddCondBranchFalseStatement();
  TreeNode* AddLabel();
  TreeNode* BuildBreak();
  TreeNode* BuildForLoop();
};

#endif
