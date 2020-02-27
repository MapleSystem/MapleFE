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

class ASTTree;
class ASTScope;

class ASTModule {
public:
  std::vector<ASTTree*>  mTrees;  // All trees in the module. There is no root tree
                                  // which covers all the others.
                                  // Everything else will be treated as a TreeNode, not a tree,
                                  // even if it's a local class.
                                  // Memory is released in ~ASTModule();
  std::vector<ASTScope*> mScopes; // All TOP scopes. The inside scopes can be traversed
                                  // through the scope tree.
                                  // Memory is released in ~ASTModule();
  ASTScope              *mRootScope; // the scope corresponding to a module. All other scopes
                                     // are children of mRootScope.
public:
  ASTModule();
  ~ASTModule();

public:
  void AddTree(ASTTree* t) { mTrees.push_back(t); }
};

#endif
