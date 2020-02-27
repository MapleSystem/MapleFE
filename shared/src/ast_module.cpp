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

#include "ast_module.h"
#include "ast.h"

ASTModule::ASTModule() {
  mRootScope = new ASTScope();
}

ASTModule::~ASTModule() {
  // free trees
  std::vector<ASTTree*>::iterator it = mTrees.begin();
  for (; it != mTrees.end(); it++) {
    ASTTree *tree = *it;
    if (tree)
      delete tree;
  }
  mTrees.clear();

  // free scopes
  std::vector<ASTScope*>::iterator scope_it = mScopes.begin();
  for (; scope_it != mScopes.end(); scope_it++) {
    ASTScope *scope = *scope_it;
    if (scope)
      delete scope;
  }
  mScopes.clear();
}
