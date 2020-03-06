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
  mRootScope = mScopePool.NewScope(NULL);
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
}

void ASTModule::Dump() {
  std::cout << "============= Module ===========" << std::endl;

  // step 1. Dump global decls.
  std::cout << "[Global Decls]" << std::endl;
  std::vector<TreeNode*>::iterator it = mRootScope->mDecls.begin();
  for (; it != mRootScope->mDecls.end(); it++) {
    TreeNode *n = *it;
    MASSERT(n->IsIdentifier() && "Decl is not an IdentifierNode.");
    IdentifierNode *in = (IdentifierNode*) n;
    n->Dump(0);
  }

  std::cout << std::endl;

  // step 2. Dump the top trees.
  std::vector<ASTTree*>::iterator tree_it = mTrees.begin();
  for (; tree_it != mTrees.end(); tree_it++) {
    ASTTree *tree = *tree_it;
    tree->Dump(0);
  }
}
