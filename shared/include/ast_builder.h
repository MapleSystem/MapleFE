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

#ifndef __AST_BUILDER_HEADER__
#define __AST_BUILDER_HEADER__

#include "ast.h"
#include "ast_module.h"
#include "ast_mempool.h"

namespace maplefe {

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
  bool        mTrace;
  ModuleNode *mASTModule;

  // The last created node. It will be referenced by the
  // following AddModifier() or other functions.
  TreeNode   *mLastTreeNode;

  // Sometimes we call BuildIdentifier() to build an identifier node
  // from keywords or any reserved words. This is allowed in Typescript.
  // We save the Name of the keyword for BuildIdentifier().
  const char *mNameForBuildIdentifier;

public:
  // information for a single action
  unsigned                mActionId;
  std::vector<Param>      mParams;

public:
  ASTBuilder(ModuleNode *m) : mASTModule(m), mTrace(false), mLastTreeNode(NULL), mNameForBuildIdentifier(NULL) {}
  ~ASTBuilder() {}

  void SetTrace(bool b) {mTrace = b;}
  void SetModule(ModuleNode *m) {mASTModule = m;}

  void AddParam(Param p) {mParams.push_back(p);}
  void ClearParams() {mParams.clear();}
  bool ParamsEmpty() {return mParams.empty();}

  // Create Functions for Token
  TreeNode* CreateTokenTreeNode(const Token*);
  TreeNode* BuildIdentifier(const Token*);
  TreeNode* BuildIdentifier(const TreeNode*);

  TreeNode* Build();

#undef  ACTION
#define ACTION(K) TreeNode* K();
#include "supported_actions.def"

  void AddArguments(TreeNode *call, TreeNode *args);
  void AddParams(TreeNode *func, TreeNode *params);
};

}
#endif
