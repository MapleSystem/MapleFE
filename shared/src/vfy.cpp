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

#include "vfy.h"
#include "ast.h"
#include "ast_module.h"
#include "ast_scope.h"
#include "massert.h"

namespace maplefe {

///////////////////////////////////////////////////////////////////////////////
// The verification is done in the following rules.
// 1. Verifier works scope by scope.
// 2. It first works on the top scope.
// 3. In a scope, it works on the children trees sequentially.
// 4. when it verifies a child tree, it goes further into the child. It returns
//    until the verification of current child is done. So this is a
//    resursive process.
///////////////////////////////////////////////////////////////////////////////

Verifier::Verifier(ModuleNode *m) : mASTModule(m) {
  mCurrScope = NULL;
  mTempParent = NULL;
}

Verifier::~Verifier() {
}

void Verifier::Do() {
  VerifyGlobalScope();
  mLog.Dump();
}

// In this implementation, the decls and types are recognized while
// verifying a tree. So this implies only Decl-s before a subtree can
// be seen by it, which is reasonable for most languages like C/C++.
//
// Java has a different story, so it has its own implementation. Please
// see java/vfy_java.cpp.

void Verifier::VerifyGlobalScope() {
  mCurrScope = mASTModule->mRootScope;
  for (unsigned i = 0; i < mASTModule->GetTreesNum(); i++) {
    TreeNode *tree = mASTModule->GetTree(i);
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
#define NODEKIND(K) if (tree->Is##K()) Verify##K((K##Node *)tree);
#include "ast_nk.def"
}

// This collect all decls and types in a whole scope. This is useful for
// some languages like C++/Java where a field or function has scope of
// the whole class body. Even the code before the field decl can still
// reference it.
void Verifier::CollectAllDeclsTypes(ASTScope *scope) {
  TreeNode *t = scope->GetTree();
  MASSERT(t->IsClass() && "right now we only support class node here");
  ClassNode *klass = (ClassNode*)t;

  // All fields, methods, local classes, local interfaces are decls.
  // All local classes/interfaces are types.
  for (unsigned i = 0; i < klass->GetFieldsNum(); i++) {
    TreeNode *in = klass->GetField(i);
    scope->AddDecl(in);
  }
  for (unsigned i = 0; i < klass->GetMethodsNum(); i++) {
    FunctionNode *n = klass->GetMethod(i);
    scope->AddDecl(n);
  }
  for (unsigned i = 0; i < klass->GetLocalClassesNum(); i++) {
    ClassNode *n = klass->GetLocalClass(i);
    scope->AddDecl(n);
    scope->AddType(n);
  }
  for (unsigned i = 0; i < klass->GetLocalInterfacesNum(); i++) {
    InterfaceNode *n = klass->GetLocalInterface(i);
    scope->AddDecl(n);
    scope->AddType(n);
  }
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
    if (decl = (IdentifierNode*) scope->FindDeclOf(inode->GetStrIdx()))
      break;
    scope = scope->GetParent();
  }

  if (!decl) {
    mLog.MissDecl(inode);
  } else {
    // Replace the temp IdentifierNode with the found Decl.
    // Sometimes inode and decl are the same, which happens for the declaration statement.
    // We will verify decl statement as the others, so its inode is the same as decl.
    if (inode != decl) {
      // TODO : There are many complicated cases which we will handle in the furture.
      //        Right now I just put a simple check of mTempParent.
      if (mTempParent)
        mTempParent->ReplaceChild(inode, decl);
    }
  }
}

void Verifier::VerifyDecl(DeclNode *tree){
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

// Nothing needed for PrimArrayType
void Verifier::VerifyPrimArrayType(PrimArrayTypeNode *tree){
  return;
}

void Verifier::VerifyVarList(VarListNode *vlnode){
  TreeNode *old_temp_parent = mTempParent;
  mTempParent = vlnode;
  for (unsigned i = 0; i < vlnode->GetVarsNum(); i++) {
    IdentifierNode *n = vlnode->GetVarAtIndex(i);
    VerifyIdentifier(n);
  }
  mTempParent = old_temp_parent;
}

// Nothing needed for Literal
void Verifier::VerifyLiteral(LiteralNode *tree){
  return;
}

void Verifier::VerifyTemplateLiteral(TemplateLiteralNode *tree){
  return;
}

void Verifier::VerifyRegExpr(RegExprNode *tree){
  return;
}

void Verifier::VerifyUnaOperator(UnaOperatorNode *tree){
}

void Verifier::VerifyBinOperator(BinOperatorNode *binop){
  TreeNode *old_temp_parent = mTempParent;
  mTempParent = binop;
  VerifyTree(binop->GetOpndA());
  VerifyTree(binop->GetOpndB());
  mTempParent = old_temp_parent;
}

void Verifier::VerifyTerOperator(TerOperatorNode *tree){
}

void Verifier::VerifyBlock(BlockNode *block){
  mCurrScope = mASTModule->NewScope(mCurrScope);
  mCurrScope->SetTree(block);

  for (unsigned i = 0; i < block->GetChildrenNum(); i++) {
    TreeNode *t = block->GetChildAtIndex(i);
    // Step 1. Try to add decl.
    mCurrScope->TryAddDecl(t);
    // Step 2. Try to add type.
    mCurrScope->TryAddType(t);
    // Step 3. Verify the t.
    VerifyTree(t);
  }
}

// Function body's block is different than a pure BlockNode.
void Verifier::VerifyFunction(FunctionNode *func){
  ASTScope *old_scope = mCurrScope;
  mCurrScope = mASTModule->NewScope(mCurrScope);
  mCurrScope->SetTree(func);

  // Add the parameters to the decl. Since we search for the decl of a var from
  // nearest scope to the farest, so it will shadown the
  // decl with same name in the ancestors' scope.
  for (unsigned i = 0; i < func->GetParamsNum(); i++) {
    IdentifierNode *inode = (IdentifierNode*)func->GetParam(i);
    mCurrScope->TryAddDecl(inode);
  }

  BlockNode *block = func->GetBody();
  for (unsigned i = 0; block && i < block->GetChildrenNum(); i++) {
    TreeNode *t = block->GetChildAtIndex(i);
    // Step 1. Try to add decl.
    mCurrScope->TryAddDecl(t);
    // Step 2. Try to add type.
    mCurrScope->TryAddType(t);
    // Step 3. Verify the t.
    VerifyTree(t);
  }

  mCurrScope = old_scope;
}

/////////////////////////////////////////////////////////////////////////////////////
//                              ClassNode verification
// ClassNode is the most complicated. We decided to separate it into multiple
// functions to allow the language specific implementation more easilier.
/////////////////////////////////////////////////////////////////////////////////////

// This verification follows the most common rules of many languages.
// Each language can have its own implementation, and maybe reuse this implementation
// together with its own specific ones.
void Verifier::VerifyClassFields(ClassNode *klass) {
  // rule 1. No duplicated fields name with another decls.
  for (unsigned i = 0; i < klass->GetFieldsNum(); i++) {
    TreeNode *na = klass->GetField(i);
    bool hit_self = false;
    for (unsigned j = 0; j < mCurrScope->GetDeclNum(); j++) {
      TreeNode *nb = mCurrScope->GetDecl(j);
      if (na->GetStrIdx() == nb->GetStrIdx()) {
        if (nb->IsIdentifier()) {
          if (!hit_self)
            hit_self = true;
          else
            mLog.Duplicate("Field Decl Duplication! ", na, nb);
        } else {
          mLog.Duplicate("Field Decl Duplication! ", na, nb);
        }
      }
    }
  }
}

void Verifier::VerifyClassMethods(ClassNode *klass) {}
void Verifier::VerifyClassSuperClasses(ClassNode *klass) { }
void Verifier::VerifyClassSuperInterfaces(ClassNode *klass) {}

void Verifier::VerifyClass(ClassNode *klass){
  // Step 1. Create a new scope
  ASTScope *scope = mASTModule->NewScope(mCurrScope);
  mCurrScope = scope;
  scope->SetTree(klass);

  CollectAllDeclsTypes(mCurrScope);

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

void Verifier::VerifyThrow(ThrowNode *tree){
}

void Verifier::VerifyTry(TryNode *tree){
}

void Verifier::VerifyCatch(CatchNode *tree){
}

void Verifier::VerifyFinally(FinallyNode *tree){
}

void Verifier::VerifyException(ExceptionNode *tree){
}

void Verifier::VerifyReturn(ReturnNode *tree){
}

void Verifier::VerifyCondBranch(CondBranchNode *tree){
}

void Verifier::VerifyBreak(BreakNode *tree){
}

void Verifier::VerifyContinue(ContinueNode *tree){
}

void Verifier::VerifyForLoop(ForLoopNode *tree){
}

void Verifier::VerifyWhileLoop(WhileLoopNode *tree){
}

void Verifier::VerifyDoLoop(DoLoopNode *tree){
}

// This verifies if identifier is a type, like class. If yes, replace
// it with the original type decl node.
void Verifier::VerifyType(IdentifierNode *inode) {
  ASTScope *scope = mCurrScope;
  TreeNode *type = NULL;
  while (scope) {
    if (type = scope->FindTypeOf(inode->GetStrIdx()))
      break;
    scope = scope->GetParent();
  }

  if (!type) {
    mLog.MissDecl(inode);
  } else {
    // Replace the temp IdentifierNode with the found Decl.
    // Sometimes inode and type are the same, which happens for the declaration statement.
    // We will verify type statement as the others, so its inode is the same as type.
    if (inode != type) {
      mTempParent->ReplaceChild(inode, type);
      //std::cout << "Replace " << inode << " with " << type << std::endl;
    }
  }
}

// Verify if the class name and parameter are available. Replace it with
// the original class node or decl node.
// Class is not about decl, it's about type since we put class declaration as a type.
// Param is about decl.
void Verifier::VerifyNew(NewNode *new_node){
  TreeNode *tree = new_node->GetId();
  MASSERT(tree
          && (tree->IsIdentifier() || tree->IsUserType())
          && "We only support single identifier in NewNode right now");
  IdentifierNode *inode = (IdentifierNode*)tree;

  TreeNode *old_temp_parent = mTempParent;
  mTempParent = new_node;

  // verify class type
  VerifyType(inode);
  // verify parameters.
  // A parameter could be any type. We have to verify type by type.
  for (unsigned i = 0; i < new_node->GetArgsNum(); i++) {
    TreeNode *p = new_node->GetArg(i);
    if(p->IsIdentifier()) {
      IdentifierNode *inode = (IdentifierNode*)p;
      VerifyIdentifier(inode);
    }
  }

  mTempParent = old_temp_parent;
}

void Verifier::VerifyDelete(DeleteNode *tree){
}

void Verifier::VerifySwitchLabel(SwitchLabelNode *tree){
}

void Verifier::VerifySwitchCase(SwitchCaseNode *tree){
}

void Verifier::VerifySwitch(SwitchNode *tree){
}

void Verifier::VerifyPass(PassNode *tree){
}

void Verifier::VerifyExprList(ExprListNode *tree){
  return;
}

void Verifier::VerifyNamespace(NamespaceNode *tree){
  return;
}

void Verifier::VerifyCall(CallNode *tree){
  return;
}

void Verifier::VerifyAssert(AssertNode *tree){
  return;
}

void Verifier::VerifyField(FieldNode *tree){
  return;
}

void Verifier::VerifyCast(CastNode *tree){
  return;
}

void Verifier::VerifyParenthesis(ParenthesisNode *tree){
  return;
}

void Verifier::VerifyModule(ModuleNode *tree){
  return;
}

void Verifier::VerifyPackage(PackageNode *tree){
  return;
}

void Verifier::VerifyDeclare(DeclareNode *tree){
  return;
}

void Verifier::VerifyImport(ImportNode *tree){
  return;
}

void Verifier::VerifyExport(ExportNode *tree){
  return;
}

void Verifier::VerifyXXportAsPair(XXportAsPairNode *tree){
  return;
}

void Verifier::VerifyUserType(UserTypeNode *tree){
  return;
}

void Verifier::VerifyLambda(LambdaNode *tree){
  return;
}

void Verifier::VerifyInstanceOf(InstanceOfNode *tree){
  return;
}

void Verifier::VerifyIn(InNode *tree){
  return;
}

void Verifier::VerifyComputedName(ComputedNameNode *tree){
  return;
}

void Verifier::VerifyIs(IsNode *tree){
  return;
}

void Verifier::VerifyAwait(AwaitNode *tree){
  return;
}

void Verifier::VerifyTypeOf(TypeOfNode *tree){
  return;
}

void Verifier::VerifyTypeAlias(TypeAliasNode *tree){
  return;
}

void Verifier::VerifyAsType(AsTypeNode *tree){
  return;
}

void Verifier::VerifyConditionalType(ConditionalTypeNode *tree){
  return;
}

void Verifier::VerifyTypeParameter(TypeParameterNode *tree){
  return;
}

void Verifier::VerifyKeyOf(KeyOfNode *tree){
  return;
}

void Verifier::VerifyInfer(InferNode *tree){
  return;
}

void Verifier::VerifyArrayElement(ArrayElementNode *tree){
  return;
}

void Verifier::VerifyArrayLiteral(ArrayLiteralNode *tree){
  return;
}

void Verifier::VerifyNumIndexSig(NumIndexSigNode *tree){
  return;
}

void Verifier::VerifyStrIndexSig(StrIndexSigNode *tree){
  return;
}

void Verifier::VerifyStruct(StructNode *tree){
  return;
}

void Verifier::VerifyNameTypePair(NameTypePairNode *tree){
  return;
}

void Verifier::VerifyTupleType(TupleTypeNode *tree){
  return;
}

void Verifier::VerifyBindingElement(BindingElementNode *tree){
  return;
}

void Verifier::VerifyBindingPattern(BindingPatternNode *tree){
  return;
}

void Verifier::VerifyStructLiteral(StructLiteralNode *tree){
  return;
}

void Verifier::VerifyFieldLiteral(FieldLiteralNode *tree){
  return;
}
}
