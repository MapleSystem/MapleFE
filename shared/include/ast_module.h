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

#ifndef __AST_MODULE_H__
#define __AST_MODULE_H__

#include <vector>

#include "ast_scope.h"

class ASTTree;
class ASTScope;

// The module is a member of class Parser.
class ASTModule {
private:
  const char *mFileName;
public:
  std::vector<ASTTree*>  mTrees;  // All trees in the module. There is no root tree
                                  // which covers all the others.
                                  // Everything else will be treated as a TreeNode, not a tree,
                                  // even if it's a local class.
                                  // Memory is released in ~ASTModule();
  ASTScope              *mRootScope; // the scope corresponding to a module. All other scopes
                                     // are children of mRootScope.
  ASTScopePool           mScopePool; // All the scopes are store in this pool. It also contains
                                     // a vector of ASTScope pointer for traversal. 
public:
  ASTModule();
  ~ASTModule();

  void SetFileName(const char *f) {mFileName = f;}
  void AddTree(ASTTree* t) { mTrees.push_back(t); }

  void Dump();
};

#endif
