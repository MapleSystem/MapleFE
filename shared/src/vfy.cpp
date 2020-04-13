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

// [TODO] Java class in Global scope is special, it doesn't need a
//        forward declaration....
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

// Before entering VerifyTree(), tree has been done with VerifyScope if it
// is a scope. It's caller's duty to assure this assumption.

void Verifier::VerifyTree(TreeNode *tree) {
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
  } else {
    // Replace the temp IdentifierNode with the found Decl.
    // Sometimes inode and decl are the same, which happens for the declaration statement.
    // We will verify decl statement as the others, so its inode is the same as decl.
    if (inode != decl) {
      mTempParent->ReplaceChild(inode, decl);
      //std::cout << "Replace " << inode << " with " << decl << std::endl;
    }
  }
}

void Verifier::VerifyDimension(DimensionNode *tree){
}

// Nothing needed for Attr
void Verifier::VerifyAttr(AttrNode *tree){
  return;
}

// Nothing needed for PrimType
void Verifier::VerifyPrimType(PrimTypeNode *tree){
  return;
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

// Nothing needed for Literal
void Verifier::VerifyLiteral(LiteralNode *tree){
  return;
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

void Verifier::VerifyBlock(BlockNode *block){
  ASTScope *scope = gModule.NewScope(mCurrScope);
  mCurrScope = scope;
  scope->SetTree(block);

  for (unsigned i = 0; i < block->GetChildrenNum(); i++) {
    TreeNode *t = block->GetChildAtIndex(i);
    // Step 1. Try to add decl.
    scope->TryAddDecl(t);
    // Step 2. Try to add type.
    scope->TryAddType(t);
    // Step 3. Verify the t.
    VerifyTree(t);
  }
}

void Verifier::VerifyFunction(FunctionNode *tree){
}

/////////////////////////////////////////////////////////////////////////////////////
//                              ClassNode verification
// ClassNode is the most complicated. We decided to separate it into multiple
// functions to allow the language specific implementation more easilier.
/////////////////////////////////////////////////////////////////////////////////////

// This verification follows the most common rules of many languages.
void Verifier::VerifyClassFields(ClassNode *klass) {
  // rule 1. No duplicated fields name.
}

void Verifier::VerifyClassMethods(ClassNode *klass) {
}

void Verifier::VerifyClassSuperClasses(ClassNode *klass) {
}

void Verifier::VerifyClassSuperInterfaces(ClassNode *klass) {
}

void Verifier::VerifyClass(ClassNode *klass){
  // Step 1. Create a new scope
  ASTScope *scope = gModule.NewScope(mCurrScope);
  mCurrScope = scope;
  scope->SetTree(klass);

  // Step 2. Verifiy Fields.
  VerifyClassFields(klass);

  // Step 3. Verifiy Methods.
  VerifyClassMethods(klass);

  // Step 4. Verifiy super classes.
  VerifyClassSuperClasses(klass);

  // Step 5. Verifiy super interfaces.
  VerifyClassSuperInterfaces(klass);
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
