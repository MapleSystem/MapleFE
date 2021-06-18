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
/////////////////////////////////////////////////////////////////////////////////
//                   Java Specific Verification                                //
/////////////////////////////////////////////////////////////////////////////////

#include "vfy_java.h"
#include "ast_module.h"
#include "ast_scope.h"

namespace maplefe {

// Collect all types, decls of global scope all at once.
void VerifierJava::VerifyGlobalScope() {
  mCurrScope = mASTModule->mRootScope;
  for (unsigned i = 0; i < mASTModule->GetTreesNum(); i++) {
    TreeNode *tree = mASTModule->GetTree(i);
    mCurrScope->TryAddDecl(tree);
    mCurrScope->TryAddType(tree);
  }

  for (unsigned i = 0; i < mASTModule->GetTreesNum(); i++) {
    TreeNode *tree = mASTModule->GetTree(i);
    VerifyTree(tree);
  }
}

void VerifierJava::VerifyClassMethods(ClassNode *klass) {
  for (unsigned i = 0; i < klass->GetMethodsNum(); i++) {
    FunctionNode *method = klass->GetMethod(i);
    // step 1. verify the duplication
    for (unsigned j = 0; j < klass->GetMethodsNum(); j++) {
      if (j == i)
        continue;
      FunctionNode *method_other = klass->GetMethod(j);
      if (method->OverrideEquivalent(method_other)) {
        mLog.Duplicate("ClassMethod Duplication! ", method, method_other);
      }
    }

    // step 2. verify functioin.
    VerifyFunction(method);
  }
}
}
