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

#include "ast.h"
#include "ast_builder.h"
#include "token.h"
#include "ruletable.h"
#include "ast_scope.h"

#include "massert.h"

////////////////////////////////////////////////////////////////////////////////////////
// For the time being, we simply use a big switch-case. Later on we could use a more
// flexible solution.
////////////////////////////////////////////////////////////////////////////////////////

// Return the sub-tree.
TreeNode* ASTBuilder::Build() {
  TreeNode *tree_node = NULL;
#define ACTION(A) \
  case (ACT_##A): \
    tree_node = A(); \
    break;

  switch (mActionId) {
#include "supported_actions.def"
  }
}

TreeNode* ASTBuilder::CreateTokenTreeNode(const Token *token) {
  unsigned size = 0;
  if (token->IsIdentifier()) {
    IdentifierToken *t = (IdentifierToken*)token;
    IdentifierNode *n = (IdentifierNode*)mTreePool->NewTreeNode(sizeof(IdentifierNode));
    new (n) IdentifierNode(t->mName);
    mLastTreeNode = n;
    return n;
  } else if (token->IsLiteral()) {
    LiteralToken *lt = (LiteralToken*)token;
    LitData data = lt->GetLitData();
    LiteralNode *n = (LiteralNode*)mTreePool->NewTreeNode(sizeof(LiteralNode));
    new (n) LiteralNode(data);
    mLastTreeNode = n;
    return n;
  } else {
    // Other tokens shouldn't be involved in the tree creation.
    return NULL;
  }
}

// For first parameter has to be an operator.
TreeNode* ASTBuilder::BuildUnaryOperation() {
  std::cout << "In build unary" << std::endl;

  MASSERT(mParams.size() == 2 && "Binary Operator has NO 2 params?");
  Param p_a = mParams[0];
  Param p_b = mParams[1];
  MASSERT(!p_a.mIsTreeNode && "First param of Unary Operator is not a token?");

  Token *token = p_a.mData.mToken;
  MASSERT(token->IsOperator() && "First param of Unary Operator is not an operator token?");

  // create the sub tree
  UnaryOperatorNode *n = (UnaryOperatorNode*)mTreePool->NewTreeNode(sizeof(UnaryOperatorNode));
  new (n) UnaryOperatorNode(((OperatorToken*)token)->mOprId);
  mLastTreeNode = n;

  // set 1st param
  if (p_b.mIsTreeNode)
    n->mOpnd = p_b.mData.mTreeNode;
  else {
    TreeNode *tn = CreateTokenTreeNode(p_b.mData.mToken);
    n->mOpnd = tn;
  }
  n->mOpnd->SetParent(n);

  return n;
}

// For second parameter has to be an operator.
TreeNode* ASTBuilder::BuildBinaryOperation() {
  std::cout << "In build binary" << std::endl;

  MASSERT(mParams.size() == 3 && "Binary Operator has NO 3 params?");
  Param p_a = mParams[0];
  Param p_b = mParams[1];
  Param p_c = mParams[2];
  MASSERT(!p_b.mIsTreeNode && "Second param of Binary Operator is not a token?");

  Token *token = p_b.mData.mToken;
  MASSERT(token->IsOperator() && "Second param of Binary Operator is not an operator token?");

  // create the sub tree
  BinaryOperatorNode *n = (BinaryOperatorNode*)mTreePool->NewTreeNode(sizeof(BinaryOperatorNode));
  new (n) BinaryOperatorNode(((OperatorToken*)token)->mOprId);
  mLastTreeNode = n;

  // set 1st param
  if (p_a.mIsTreeNode)
    n->mOpndA = p_a.mData.mTreeNode;
  else {
    TreeNode *tn = CreateTokenTreeNode(p_a.mData.mToken);
    n->mOpndA = tn;
  }
  n->mOpndA->SetParent(n);

  // set 2nd param
  if (p_c.mIsTreeNode)
    n->mOpndB = p_c.mData.mTreeNode;
  else {
    TreeNode *tn = CreateTokenTreeNode(p_c.mData.mToken);
    n->mOpndB = tn;
  }
  n->mOpndB->SetParent(n);

  return n;
}

// Assignment is actually a binary operator.
TreeNode* ASTBuilder::BuildAssignment() {
  std::cout << "In assignment --> BuildBinary" << std::endl;
  return BuildBinaryOperation();
}

TreeNode* ASTBuilder::BuildReturn() {
  return NULL;
}

////////////////////////////////////////////////////////////////////////////////
// Issues in building declarations.
// 1) First we are going to create an IdentifierNode, which should be attached
//    to a ASTScope.
// 2) The tree is created in order of children-first, so the scope is not there yet.
//    We need have a list of pending declarations until the scope is created.
////////////////////////////////////////////////////////////////////////////////

// BuildDecl takes two parameters, type and name
TreeNode* ASTBuilder::BuildDecl() {
  std::cout << "In build Decl" << std::endl;

  MASSERT(mParams.size() == 2 && "BinaryDecl has NO 2 params?");
  Param p_type = mParams[0];
  Param p_name = mParams[1];

  // Step 1. Get the Type
  //         We only deal with keyword type right now.
  if (p_type.mIsTreeNode) {
    MERROR("We only handle keyword type now. To Be Implemented.");
  }

  Token *token = p_type.mData.mToken;
  if (!token->IsKeyword())
    MERROR("Type is not a keyword.");

  KeywordToken *kw_token = (KeywordToken*)token;
  ASTType *type = gASTTypePool.FindPrimType(kw_token->GetName());

  // Step 2. Get the name, which is already a IdentifierNode.
  if (!p_name.mIsTreeNode)
    MERROR("The variable name should be a IdentifierNode already, but actually NOT?");

  TreeNode *n = p_name.mData.mTreeNode;
  if (!n->IsIdentifier() && !n->IsVarList())
    MERROR("Variable is not an identifier or VarList.");

  if (n->IsIdentifier()) {
    ((IdentifierNode *)n)->SetType(type);
  } else if (n->IsVarList()) {
    VarListNode *vl = (VarListNode*)n;
    for (unsigned i = 0; i < vl->mVars.GetNum(); i++)
      vl->mVars.AtIndex(i)->SetType(type);
  }

  // Step 3. Save this decl
  mPendingDecls.push_back((TreeNode*)n);
  mLastTreeNode = (TreeNode*)n;

  return n;
}

// BuildVariableList takes two parameters, var 1 and var 2
TreeNode* ASTBuilder::BuildVarList() {
  std::cout << "In build Variable List" << std::endl;

  MASSERT(mParams.size() == 2 && "BuildVarList has NO 2 params?");
  Param p_var_a = mParams[0];
  Param p_var_b = mParams[1];

  // Both variable should have been created as tree node.
  if (!p_var_a.mIsTreeNode || !p_var_b.mIsTreeNode) {
    MERROR("The var in BuildVarList is not a treenode");
  }

  TreeNode *node_a = p_var_a.mIsEmpty ? NULL : p_var_a.mData.mTreeNode;
  TreeNode *node_b = p_var_b.mIsEmpty ? NULL : p_var_b.mData.mTreeNode;

  // There are a few different scenarios.
  // (1) node_a is a VarListNode, and we dont care about node_b
  // (2) node_a is an IdentifierNode, node_b is a VarListNode
  // (4) both are IdentifierNode
  // The solution is simple, pick an existing varListNode as the result
  // or create a new one. Merge the remaining node(s) to the result node.

  VarListNode *node_ret = NULL;
  if (node_a && node_a->IsVarList()) {
    node_ret = (VarListNode*)node_a;
    node_ret->Merge(node_b);
  } else if (node_b && node_b->IsVarList()) {
    node_ret = (VarListNode*)node_b;
    node_ret->Merge(node_a);
  } else {
    // both nodes are not VarListNode
    node_ret = (VarListNode*)mTreePool->NewTreeNode(sizeof(VarListNode));
    new (node_ret) VarListNode();
    if (node_a)
      node_ret->Merge(node_a);
    if (node_b)
      node_ret->Merge(node_b);
  }

  // Set last tree node
  mLastTreeNode = node_ret;

  return node_ret;
}

TreeNode* ASTBuilder::AddAttribute() {
  std::cout << "In AddAttribute" << std::endl;
  Param p_attr = mParams[0];
  return mLastTreeNode;
}

TreeNode* ASTBuilder::AddAttributeTo() {
  std::cout << "In AddAttributeTo" << std::endl;
  Param p_attr = mParams[0];
  return mLastTreeNode;
}

TreeNode* ASTBuilder::AddInitTo() {
  std::cout << "In AddInitTo" << std::endl;
  Param p_decl = mParams[0];
  Param p_init;

  // If there is no init value, return NULL.
  if (mParams.size() == 1)
    return NULL;

  p_init = mParams[1];
  if (p_init.mIsEmpty)
    return NULL;

  // Both variable should have been created as tree node.
  if (!p_decl.mIsTreeNode || !p_init.mIsTreeNode)
    MERROR("The decl or init is not a treenode in AddInitTo()");

  TreeNode *node_decl = p_decl.mData.mTreeNode;
  TreeNode *node_init = p_init.mData.mTreeNode;

  if (!node_decl->IsIdentifier())
    MERROR("The target of AddInitTo should be an indentifier node. Not?");

  IdentifierNode *in = (IdentifierNode*)node_decl;
  in->SetInit(node_init);

  return in;
}

// This takes just one argument which is the class name.
TreeNode* ASTBuilder::BuildClass() {
  std::cout << "In BuildClass" << std::endl;

  Param p_name = mParams[0];

  if (!p_name.mIsTreeNode)
    MERROR("The class name is not a treenode in BuildClass()");
  TreeNode *node_name = p_name.mData.mTreeNode;

  if (!node_name->IsIdentifier())
    MERROR("The class name should be an indentifier node. Not?");
  IdentifierNode *in = (IdentifierNode*)node_name;

  ClassNode *node_class = (ClassNode*)mTreePool->NewTreeNode(sizeof(ClassNode));
  new (node_class) ClassNode();
  node_class->SetName(node_name);

  // set last tree node
  mLastTreeNode = node_class;

  return mLastTreeNode;
}

TreeNode* ASTBuilder::AddSuperClass() {
  std::cout << "In AddSuperClass" << std::endl;
  Param p_attr = mParams[0];
  return mLastTreeNode;
}

TreeNode* ASTBuilder::AddSuperInterface() {
  std::cout << "In AddSuperInterface" << std::endl;
  Param p_attr = mParams[0];
  return mLastTreeNode;
}

TreeNode* ASTBuilder::AddClassBody() {
  std::cout << "In AddClassBody" << std::endl;
  Param p_attr = mParams[0];
  return mLastTreeNode;
}

////////////////////////////////////////////////////////////////////////////////
//                   Other Functions
////////////////////////////////////////////////////////////////////////////////

void ASTBuilder::AssignRemainingDecls(ASTScope *scope) {
  std::vector<TreeNode *>::iterator it = mPendingDecls.begin();
  for (; it != mPendingDecls.end(); it++) {
    TreeNode *n = *it;
    scope->AddDecl(n);
  }
}
