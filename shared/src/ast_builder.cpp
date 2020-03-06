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
    return n;
  } else if (token->IsLiteral()) {
    LiteralToken *lt = (LiteralToken*)token;
    LitData data = lt->GetLitData();
    LiteralNode *n = (LiteralNode*)mTreePool->NewTreeNode(sizeof(LiteralNode));
    new (n) LiteralNode(data);
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

  IdentifierNode *n = p_name.mData.mTreeNode;
  if (!n->IsIdentifier())
    MERROR("Variable is not an identifier.");
  n->SetType(type);

  // Step 3. Save this decl
  mPendingDecls.push_back((TreeNode*)n);
  mLastDecl = (TreeNode*)n;

  return n;
}

TreeNode* ASTBuilder::AddAttribute() {
  std::cout << "In AddAttribute" << std::endl;
  Param p_attr = mParams[0];
  // we simply return NULL for now.
  return NULL;
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
