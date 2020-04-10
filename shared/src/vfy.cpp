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

///////////////////////////////////////////////////////////////////////////////
// The verification is done in the following rules.
// 1. Verifier works scope by scope.
// 2. It first works on the top scope.
// 3. In a scope, it works on the children trees sequentially.
// 4. when it verifies a child tree, it goes further into the child. It returns
//    until the verification of current child is done. So this is a
//    resursive process.
///////////////////////////////////////////////////////////////////////////////

Verifier::Verifier() {
}

Verifier::~Verifier() {
}

void Verifier::Do() {
  gModule.PreVerify();
  VerifyScope(gModule.mRootScope);
}

// Each time when we enter a new scope, we need handle possible local
// variable declarations or type declaration. So it's different than
// VerifyTree().

void Verifier::VerifyScope(ASTScope *scope) {
  mCurrScope = scope;
  for (unsigned i = 0; i < scope->GetTreeNum(); i++) {
    TreeNode *tree = scope->GetTree(i);

    // Step 1. Try to add decl.
    scope->TryAddDecl(tree);

    // Step 2. Try to add type.
    scope->TryAddType(tree);

    // Step 3. Verify the tree.
    VerifyTree(tree);
  }
}

// Before entering VerifyTree(), tree has been done with VerifyScope if it
// is a scope. It's caller's duty to assure this assumption.

void Verifier::VerifyTree(TreeNode *tree) {
#undef  NODEKIND
#define NODEKIND(K) if (tree->Is##K()) Verify##K(tree);
#include "ast_nk.def"
}

void Verifier::VerifyIdentifier(TreeNode *tree) {
}

void Verifier::VerifyDimension(TreeNode *tree){
}

void Verifier::VerifyAttr(TreeNode *tree){
}

void Verifier::VerifyPrimType(TreeNode *tree){
}

void Verifier::VerifyVarList(TreeNode *tree){
}

void Verifier::VerifyLiteral(TreeNode *tree){
}

void Verifier::VerifyUnaOperator(TreeNode *tree){
}

void Verifier::VerifyBinOperator(TreeNode *tree){
}

void Verifier::VerifyTerOperator(TreeNode *tree){
}

void Verifier::VerifyBlock(TreeNode *tree){
}

void Verifier::VerifyFunction(TreeNode *tree){
}

void Verifier::VerifyClass(TreeNode *tree){
}

void Verifier::VerifyInterface(TreeNode *tree){
}

void Verifier::VerifyAnnotationType(TreeNode *tree){
}

void Verifier::VerifyAnnotation(TreeNode *tree){
}

void Verifier::VerifyException(TreeNode *tree){
}

void Verifier::VerifyReturn(TreeNode *tree){
}

void Verifier::VerifyCondBranch(TreeNode *tree){
}

void Verifier::VerifyBreak(TreeNode *tree){
}

void Verifier::VerifyForLoop(TreeNode *tree){
}

void Verifier::VerifyWhileLoop(TreeNode *tree){
}

void Verifier::VerifyDoLoop(TreeNode *tree){
}

void Verifier::VerifySwitchLabel(TreeNode *tree){
}

void Verifier::VerifySwitchCase(TreeNode *tree){
}

void Verifier::VerifySwitch(TreeNode *tree){
}

void Verifier::VerifyPass(TreeNode *tree){
}
