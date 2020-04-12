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
  mCurrScope = NULL;
  mTempParent = NULL;
}

Verifier::~Verifier() {
  mTempTrees.Release();
}

void Verifier::Do() {
  VerifyGlobalScope();
}

// Each time when we enter a new scope, we need handle possible local
// variable declarations or type declaration. So it's different than
// VerifyTree().
//
// If you have a sharp eye, you can tell the Decl list of the scope is
// built progressively with verification of sub trees. This means
// only Decl-s before a subtree can be seen by it, which is reasonable
// for most languages.

void Verifier::VerifyGlobalScope() {
  mCurrScope = gModule.mRootScope;
  std::vector<ASTTree*>::iterator tree_it = gModule.mTrees.begin();
  for (; tree_it != gModule.mTrees.end(); tree_it++) {
    ASTTree *asttree = *tree_it;
    TreeNode *tree = asttree->mRootNode;
    // Step 1. Try to add decl.
    mCurrScope->TryAddDecl(tree);
    // Step 2. Try to add type.
    mCurrScope->TryAddType(tree);
    // Step 3. Verify the tree.
    VerifyTree(tree);
  }
}

// To copy the sub trees in the corresponding tree of currect scope
// into a temp SmallVector. This provides an opportunity for each language
// to define its own treatment of PrepareTempTrees(). And provides
// a unified structure of mTempTrees for traversal.
void Verifier::PrepareTempTrees() {
  mTempTrees.Clear();

  // The default implementation only takes care of block node.
  TreeNode *tree = mCurrScope->GetTree();
  if (tree->IsBlock()) {
    BlockNode *block = (BlockNode*)tree;
    for (unsigned i = 0; i < block->GetChildrenNum(); i++) {
      TreeNode *t = block->GetChildAtIndex(i);
      mTempTrees.PushBack(t);
    }
  }
}

void Verifier::VerifyScope(ASTScope *scope) {
  mCurrScope = scope;
  TreeNode *tree = scope->GetTree();
  PrepareTempTrees();

  for (unsigned i = 0; i < mTempTrees.GetNum(); i++) {
    TreeNode *tree = mTempTrees.ValueAtIndex(i);
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
  // If a tree is also scope, it's handled differently as local
  // decls and local types need be taken care of.
  if (tree->IsScope()) {
    ASTScope *scope = gModule.NewScope(mCurrScope);
    scope->SetTree(tree);
    VerifyScope(scope);
    return;
  }
#undef  NODEKIND
#define NODEKIND(K) if (tree->Is##K()) Verify##K(tree);
#include "ast_nk.def"
}

// This function has two jobs.
// 1. It looks into the Decl's of this scope and its ancestors' scopes,
//    to find a decl with the same name. If not found, the identifier
//    is undeclaraed.
// 2. Replace this node with the decl node, since they are the same node.
//    The abandoned one was created at the first time Parser saw it. At
//    that time parser has no idea what it is, and just give it a new
//    identifier node. So After verification, they should point to the
//    real one.
void Verifier::VerifyIdentifier(IdentifierNode *inode) {
  ASTScope *scope = mCurrScope;
  IdentifierNode *decl = NULL;
  while (scope) {
    if (decl = scope->FindDeclOf(inode)) {
      break;
    }
    scope = scope->GetParent();
  }

  if (!decl) {
    std::cout << "Error: Var " << inode->GetName() << " not found decl." << std::endl;
  }
}

void Verifier::VerifyDimension(DimensionNode *tree){
}

void Verifier::VerifyAttr(AttrNode *tree){
}

void Verifier::VerifyPrimType(PrimTypeNode *tree){
}

void Verifier::VerifyVarList(VarListNode *vlnode){
  TreeNode *old_temp_parent = mTempParent;
  mTempParent = vlnode;
  for (unsigned i = 0; i < vlnode->GetNum(); i++) {
    IdentifierNode *n = vlnode->VarAtIndex(i);
    VerifyIdentifier(n);
  }
  mTempParent = old_temp_parent;
}

void Verifier::VerifyLiteral(LiteralNode *tree){
}

void Verifier::VerifyUnaOperator(UnaOperatorNode *tree){
}

void Verifier::VerifyBinOperator(BinOperatorNode *binop){
  TreeNode *old_temp_parent = mTempParent;
  mTempParent = binop;
  VerifyTree(binop->mOpndA);
  VerifyTree(binop->mOpndB);
  mTempParent = old_temp_parent;
}

void Verifier::VerifyTerOperator(TerOperatorNode *tree){
}

void Verifier::VerifyBlock(BlockNode *tree){
}

void Verifier::VerifyFunction(FunctionNode *tree){
}

void Verifier::VerifyClass(ClassNode *tree){
}

void Verifier::VerifyInterface(InterfaceNode *tree){
}

void Verifier::VerifyAnnotationType(AnnotationTypeNode *tree){
}

void Verifier::VerifyAnnotation(AnnotationNode *tree){
}

void Verifier::VerifyException(ExceptionNode *tree){
}

void Verifier::VerifyReturn(ReturnNode *tree){
}

void Verifier::VerifyCondBranch(CondBranchNode *tree){
}

void Verifier::VerifyBreak(BreakNode *tree){
}

void Verifier::VerifyForLoop(ForLoopNode *tree){
}

void Verifier::VerifyWhileLoop(WhileLoopNode *tree){
}

void Verifier::VerifyDoLoop(DoLoopNode *tree){
}

void Verifier::VerifySwitchLabel(SwitchLabelNode *tree){
}

void Verifier::VerifySwitchCase(SwitchCaseNode *tree){
}

void Verifier::VerifySwitch(SwitchNode *tree){
}

void Verifier::VerifyPass(PassNode *tree){
}
