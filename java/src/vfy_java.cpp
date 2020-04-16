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
/////////////////////////////////////////////////////////////////////////////////
//                   Java Specific Verification                                //
/////////////////////////////////////////////////////////////////////////////////

#include "vfy_java.h"
#include "ast_module.h"
#include "ast_scope.h"

// Collect all types, decls of global scope all at once.
void VerifierJava::VerifyGlobalScope() {
  mCurrScope = gModule.mRootScope;
  std::vector<ASTTree*>::iterator tree_it = gModule.mTrees.begin();
  for (; tree_it != gModule.mTrees.end(); tree_it++) {
    ASTTree *asttree = *tree_it;
    TreeNode *tree = asttree->mRootNode;
    mCurrScope->TryAddDecl(tree);
    mCurrScope->TryAddType(tree);
  } 

  tree_it = gModule.mTrees.begin();
  for (; tree_it != gModule.mTrees.end(); tree_it++) {
    ASTTree *asttree = *tree_it;
    TreeNode *tree = asttree->mRootNode;
    VerifyTree(tree);
  }
}
