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

ASTModule gModule;

ASTModule::ASTModule() {
  mRootScope = mScopePool.NewScope(NULL);
  mPackage = NULL;
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

  mImports.Release();
}

// AFAIK, all languages allow only one package name if it allows.
void ASTModule::SetPackage(const char *p) {
  MASSERT(!mPackage);
  mPackage = p;
}

// Return a new scope newly created.
// Set the parent<->child relation between it and p.
ASTScope* ASTModule::NewScope(ASTScope *p) {
  ASTScope *newscope = mScopePool.NewScope(p);
  return newscope;
}

void ASTModule::Dump() {
  std::cout << "============= Module ===========" << std::endl;
  std::vector<ASTTree*>::iterator tree_it = mTrees.begin();
  for (; tree_it != mTrees.end(); tree_it++) {
    ASTTree *tree = *tree_it;
    tree->Dump(0);
  }
}
