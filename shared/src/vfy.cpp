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

#include "vfy.h"
#include "ast.h"
#include "ast_module.h"
#include "ast_scope.h"
#include "massert.h"

Verifier::Verifier() {
}

Verifier::~Verifier() {
}

// The verification is done in the following rules.
// 1. Verifier works scope by scope.
// 2. It first works on the top scope.
// 3. In a scope, it works on the children trees sequentially.
// 4. when it verifies a child tree, it goes into the child. It returns
//    until the verification of current child is done. So this is a
//    resursive process.

void Verifier::Do() {
  gModule.PreVerify();
  Verify(gModule.mRootScope);
}

void Verifier::Verify(ASTScope *scope) {
  mCurrScope = scope;
  for (unsigned i = 0; i < scope->GetTreeNum(); i++) {
    TreeNode *tree = scope->GetTree(i);
    Verify(tree);
  }
}

void Verifier::Verify(TreeNode *tree) {
  // step 1. try to add decl
}
