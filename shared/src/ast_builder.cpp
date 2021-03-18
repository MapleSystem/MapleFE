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

#include <cstring>

#include "token.h"
#include "ruletable.h"
#include "ast_builder.h"
#include "ast_scope.h"
#include "ast_attr.h"
#include "ast_type.h"
#include "ast_module.h"
#include "massert.h"

namespace maplefe {

ASTBuilder gASTBuilder;

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

  return tree_node;
}

TreeNode* ASTBuilder::CreateTokenTreeNode(const Token *token) {
  unsigned size = 0;
  if (token->IsIdentifier()) {
    IdentifierNode *n = (IdentifierNode*)mTreePool->NewTreeNode(sizeof(IdentifierNode));
    new (n) IdentifierNode(token->GetName());
    mLastTreeNode = n;
    return n;
  } else if (token->IsLiteral()) {
    LitData data = token->GetLitData();
    LiteralNode *n = (LiteralNode*)mTreePool->NewTreeNode(sizeof(LiteralNode));
    new (n) LiteralNode(data);
    mLastTreeNode = n;
    return n;
  } else if (token->IsKeyword()) {
    const char *keyword = token->GetName();
    // If it's an attribute
    AttrNode *n = gAttrPool.GetAttrNode(keyword);
    if (n) {
      mLastTreeNode = n;
      return n;
    }
    // If it's a type
    PrimTypeNode *type = gPrimTypePool.FindType(keyword);
    if (type) {
      mLastTreeNode = type;
      return type;
    }
    // We define special literal tree node for 'this', 'super'.
    if ((strlen(token->GetName()) == 4) && !strncmp(token->GetName(), "this", 4)) {
      LitData data;
      data.mType = LT_ThisLiteral;
      LiteralNode *n = (LiteralNode*)mTreePool->NewTreeNode(sizeof(LiteralNode));
      new (n) LiteralNode(data);
      mLastTreeNode = n;
      return n;
    } else if ((strlen(token->GetName()) == 5) && !strncmp(token->GetName(), "super", 5)) {
      LitData data;
      data.mType = LT_SuperLiteral;
      LiteralNode *n = (LiteralNode*)mTreePool->NewTreeNode(sizeof(LiteralNode));
      new (n) LiteralNode(data);
      mLastTreeNode = n;
      return n;
    }
    // Otherwise, it doesn't create any tree node.
  }

  // Other tokens shouldn't be involved in the tree creation.
  return NULL;
}

////////////////////////////////////////////////////////////////////////////////////////
//                      Static Functions help build the tree
////////////////////////////////////////////////////////////////////////////////////////

static void add_attribute_to(TreeNode *tree, TreeNode *attr) {
  MASSERT(attr->IsAttr());
  AttrNode *attr_node = (AttrNode*)attr;
  AttrId aid = attr_node->GetId();
  if (tree->IsVarList()) {
    VarListNode *vl = (VarListNode*)tree;
    for (unsigned i = 0; i < vl->GetNum(); i++) {
      IdentifierNode *inode = vl->VarAtIndex(i);
      inode->AddAttr(aid);
    }
    return;
  } else if (tree->IsBlock()){
    BlockNode *b = (BlockNode*)tree;
    if (b->IsInstInit()) {
      b->AddAttr(aid);
      return;
    }
  } else {
    ClassNode *klass = (ClassNode*)tree;
    klass->AddAttr(aid);
    return;
  }
}

// It's the caller to assure tree is valid, meaning something could carry type.
static void add_type_to(TreeNode *tree, TreeNode *type) {
  if (tree->IsIdentifier()) {
    IdentifierNode *in = (IdentifierNode*)tree;
    in->SetType(type);
  } else if (tree->IsVarList()) {
    VarListNode *vl = (VarListNode*)tree;
    for (unsigned i = 0; i < vl->GetNum(); i++)
      vl->VarAtIndex(i)->SetType(type);
  } else if (tree->IsFunction()) {
    FunctionNode *func = (FunctionNode*)tree;
    func->SetType(type);
  } else {
    MERROR("Unsupported tree node in add_type_to()");
  }
}

////////////////////////////////////////////////////////////////////////////////////////
//                      Major Functions to build the tree
////////////////////////////////////////////////////////////////////////////////////////

TreeNode* ASTBuilder::BuildPackageName() {
  MASSERT(!gModule.mPackage);
  MASSERT(mLastTreeNode->IsField() || mLastTreeNode->IsIdentifier());

  PackageNode *n = (PackageNode*)mTreePool->NewTreeNode(sizeof(PackageNode));
  new (n) PackageNode();
  n->SetPackage(mLastTreeNode);

  gModule.SetPackage(n);

  mLastTreeNode = n;
  return mLastTreeNode;
}

TreeNode* ASTBuilder::BuildSingleTypeImport() {
  ImportNode *n = (ImportNode*)mTreePool->NewTreeNode(sizeof(ImportNode));
  new (n) ImportNode();
  n->SetImportSingle();
  n->SetImportType();

  MASSERT(mParams.size() == 1);
  Param p = mParams[0];
  MASSERT(!p.mIsEmpty && p.mIsTreeNode);
  TreeNode *tree = p.mData.mTreeNode;
  MASSERT(tree->IsIdentifier() || tree->IsField());

  n->SetTarget(tree);
  mLastTreeNode = n;
  return mLastTreeNode;
}

TreeNode* ASTBuilder::BuildAllTypeImport() {
  ImportNode *n = (ImportNode*)mTreePool->NewTreeNode(sizeof(ImportNode));
  new (n) ImportNode();
  n->SetImportAll();
  n->SetImportType();

  MASSERT(mParams.size() == 1);
  Param p = mParams[0];
  MASSERT(!p.mIsEmpty && p.mIsTreeNode);
  TreeNode *tree = p.mData.mTreeNode;
  MASSERT(tree->IsIdentifier() || tree->IsField());

  n->SetTarget(tree);
  mLastTreeNode = n;
  return mLastTreeNode;
}

// It takes the mLastTreeNode as parameter
TreeNode* ASTBuilder::BuildSingleStaticImport() {
  ImportNode *n = (ImportNode*)mTreePool->NewTreeNode(sizeof(ImportNode));
  new (n) ImportNode();
  n->SetImportSingle();
  n->SetImportType();

  MASSERT(mLastTreeNode->IsIdentifier() || mLastTreeNode->IsField());
  n->SetTarget(mLastTreeNode);
  n->SetImportStatic();
  mLastTreeNode = n;
  return mLastTreeNode;
}

TreeNode* ASTBuilder::BuildAllStaticImport() {
  BuildAllTypeImport();
  ImportNode *import = (ImportNode*)mLastTreeNode;
  import->SetImportStatic();
  return mLastTreeNode;
}

TreeNode* ASTBuilder::BuildAllImport() {
}

// Takes one argument, the expression of the parenthesis
TreeNode* ASTBuilder::BuildParenthesis() {
  if (mTrace)
    std::cout << "In BuildParenthesis" << std::endl;

  TreeNode *expr = NULL;

  MASSERT(mParams.size() == 1);
  Param p = mParams[0];
  MASSERT(!p.mIsEmpty && p.mIsTreeNode);
  expr = p.mData.mTreeNode;

  ParenthesisNode *n = (ParenthesisNode*)mTreePool->NewTreeNode(sizeof(ParenthesisNode));
  new (n) ParenthesisNode();
  n->SetExpr(expr);

  mLastTreeNode = n;
  return mLastTreeNode;
}

// For first parameter has to be an operator.
TreeNode* ASTBuilder::BuildCast() {
  if (mTrace)
    std::cout << "In BuildCast" << std::endl;

  MASSERT(mParams.size() == 2 && "BuildCast has NO 2 params?");
  Param p_a = mParams[0];
  Param p_b = mParams[1];
  MASSERT(!p_a.mIsEmpty && !p_a.mIsEmpty);
  MASSERT(p_a.mIsTreeNode && p_a.mIsTreeNode);

  TreeNode *desttype = p_a.mData.mTreeNode;
  TreeNode *expr = p_b.mData.mTreeNode;

  CastNode *n = (CastNode*)mTreePool->NewTreeNode(sizeof(CastNode));
  new (n) CastNode();

  n->SetDestType(desttype);
  n->SetExpr(expr);

  mLastTreeNode = n;
  return n;
}


// For first parameter has to be an operator.
TreeNode* ASTBuilder::BuildUnaryOperation() {
  if (mTrace)
    std::cout << "In build unary" << std::endl;

  MASSERT(mParams.size() == 2 && "Binary Operator has NO 2 params?");
  Param p_a = mParams[0];
  Param p_b = mParams[1];
  MASSERT(!p_a.mIsTreeNode && "First param of Unary Operator is not a token?");

  Token *token = p_a.mData.mToken;
  MASSERT(token->IsOperator() && "First param of Unary Operator is not an operator token?");

  // create the sub tree
  UnaOperatorNode *n = (UnaOperatorNode*)mTreePool->NewTreeNode(sizeof(UnaOperatorNode));
  new (n) UnaOperatorNode(token->GetOprId());

  // set 1st param
  if (p_b.mIsTreeNode)
    n->SetOpnd(p_b.mData.mTreeNode);
  else {
    TreeNode *tn = CreateTokenTreeNode(p_b.mData.mToken);
    n->SetOpnd(tn);
  }
  n->GetOpnd()->SetParent(n);

  mLastTreeNode = n;
  return n;
}

// This is the same as BuildUnaryOperation, except setting mIsPost to true.
TreeNode* ASTBuilder::BuildPostfixOperation() {
  if (mTrace)
    std::cout << "In BuildPostfixOperation" << std::endl;
  UnaOperatorNode * t = (UnaOperatorNode*)BuildUnaryOperation();
  t->SetIsPost(true);
  mLastTreeNode = t;
  return t;
}

// For second parameter has to be an operator.
TreeNode* ASTBuilder::BuildBinaryOperation() {
  if (mTrace)
    std::cout << "In build binary" << std::endl;

  MASSERT(mParams.size() == 3 && "Binary Operator has NO 3 params?");
  Param p_a = mParams[0];
  Param p_b = mParams[1];
  Param p_c = mParams[2];
  MASSERT(!p_b.mIsTreeNode && "Second param of Binary Operator is not a token?");

  Token *token = p_b.mData.mToken;
  MASSERT(token->IsOperator() && "Second param of Binary Operator is not an operator token?");

  // create the sub tree
  BinOperatorNode *n = (BinOperatorNode*)mTreePool->NewTreeNode(sizeof(BinOperatorNode));
  new (n) BinOperatorNode(token->GetOprId());
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
  if (mTrace)
    std::cout << "In assignment --> BuildBinary" << std::endl;
  return BuildBinaryOperation();
}

// Takes one argument, the result expression
TreeNode* ASTBuilder::BuildReturn() {
  if (mTrace)
    std::cout << "In BuildReturn" << std::endl;

  ReturnNode *result = (ReturnNode*)mTreePool->NewTreeNode(sizeof(ReturnNode));
  new (result) ReturnNode();

  Param p_result = mParams[0];
  if (!p_result.mIsEmpty) {
    if (!p_result.mIsTreeNode)
      MERROR("The return value is not a tree node.");
    TreeNode *result_value = p_result.mData.mTreeNode;
    result->SetResult(result_value);
  }

  mLastTreeNode = result;
  return mLastTreeNode;
}

// Takes one argument, the condition expression
TreeNode* ASTBuilder::BuildCondBranch() {
  if (mTrace)
    std::cout << "In BuildCondBranch" << std::endl;

  CondBranchNode *cond_branch = (CondBranchNode*)mTreePool->NewTreeNode(sizeof(CondBranchNode));
  new (cond_branch) CondBranchNode();

  Param p_cond = mParams[0];
  if (p_cond.mIsEmpty)
    MERROR("The condition expression is empty in building conditional branch.");

  if (!p_cond.mIsTreeNode)
    MERROR("The condition expr is not a tree node.");

  TreeNode *cond_expr = p_cond.mData.mTreeNode;
  cond_branch->SetCond(cond_expr);

  mLastTreeNode = cond_branch;
  return mLastTreeNode;
}

// Takes one argument, the body of true statement/block
TreeNode* ASTBuilder::AddCondBranchTrueStatement() {
  if (mTrace)
    std::cout << "In AddCondBranchTrueStatement" << std::endl;

  CondBranchNode *cond_branch = (CondBranchNode*)mLastTreeNode;
  Param p_true = mParams[0];
  if (!p_true.mIsEmpty) {
    if (!p_true.mIsTreeNode)
      MERROR("The condition expr is not a tree node.");
    TreeNode *true_expr = CvtToBlock(p_true.mData.mTreeNode);
    cond_branch->SetTrueBranch(true_expr);
  }

  return mLastTreeNode;
}

// Takes one argument, the body of false statement/block
TreeNode* ASTBuilder::AddCondBranchFalseStatement() {
  if (mTrace)
    std::cout << "In AddCondBranchFalseStatement" << std::endl;

  CondBranchNode *cond_branch = (CondBranchNode*)mLastTreeNode;
  Param p_false = mParams[0];
  if (!p_false.mIsEmpty) {
    if (!p_false.mIsTreeNode)
      MERROR("The condition expr is not a tree node.");
    TreeNode *false_expr = CvtToBlock(p_false.mData.mTreeNode);
    cond_branch->SetFalseBranch(false_expr);
  }

  return mLastTreeNode;
}

// AddLabel tabkes two arguments, target tree, and label
TreeNode* ASTBuilder::AddLabel() {
  if (mTrace)
    std::cout << "In AddLabel " << std::endl;

  MASSERT(mParams.size() == 2 && "AddLabel has NO 2 params?");
  Param p_tree = mParams[0];
  Param p_label = mParams[1];
  MASSERT(p_tree.mIsTreeNode && "Target tree in AddLabel is not a tree.");

  TreeNode *tree = p_tree.mData.mTreeNode;

  if (p_label.mIsEmpty)
    return tree;

  // Label should be an identifier node
  MASSERT(p_label.mIsTreeNode && "Label in AddLabel is not a tree.");
  TreeNode *label = p_label.mData.mTreeNode;
  MASSERT(label->IsIdentifier() && "Label in AddLabel is not an identifier.");

  tree->SetLabel(label);

  mLastTreeNode = tree;
  return tree;
}

// BuildBreak takes one argument, an identifer node of empty.
TreeNode* ASTBuilder::BuildBreak() {
  if (mTrace)
    std::cout << "In BuildBreak " << std::endl;

  BreakNode *break_node = (BreakNode*)mTreePool->NewTreeNode(sizeof(BreakNode));
  new (break_node) BreakNode();

  MASSERT(mParams.size() == 1 && "BuildBreak has NO 1 params?");
  Param p_target = mParams[0];
  if (!p_target.mIsEmpty) {
    MASSERT(p_target.mIsTreeNode && "Target in BuildBreak is not a tree.");
    TreeNode *target = p_target.mData.mTreeNode;
    MASSERT(target->IsIdentifier() && "Target in BuildBreak is not an identifier.");
    break_node->SetTarget(target);
  }

  mLastTreeNode = break_node;
  return break_node;
}

// BuildForLoop takes four arguments.
//  1. init statement, could be a list
//  2. cond expression, should be a boolean expresion.
//  3. update statement, could be a list
//  4. body.
TreeNode* ASTBuilder::BuildForLoop() {
  if (mTrace)
    std::cout << "In BuildForLoop " << std::endl;

  ForLoopNode *for_loop = (ForLoopNode*)mTreePool->NewTreeNode(sizeof(ForLoopNode));
  new (for_loop) ForLoopNode();

  MASSERT(mParams.size() == 4 && "BuildForLoop has NO 4 params?");

  Param p_init = mParams[0];
  if (!p_init.mIsEmpty) {
    MASSERT(p_init.mIsTreeNode && "ForLoop init is not a treenode.");
    TreeNode *init = p_init.mData.mTreeNode;
    if (init->IsPass()) {
      PassNode *pass_node = (PassNode*)init;
      for (unsigned i = 0; i < pass_node->GetChildrenNum(); i++)
        for_loop->AddInit(pass_node->GetChild(i));
    } else {
      for_loop->AddInit(init);
    }
  }

  Param p_cond = mParams[1];
  if (!p_cond.mIsEmpty) {
    MASSERT(p_cond.mIsTreeNode && "ForLoop init is not a treenode.");
    TreeNode *cond = p_cond.mData.mTreeNode;
    for_loop->SetCond(cond);
  }

  Param p_update = mParams[2];
  if (!p_update.mIsEmpty) {
    MASSERT(p_update.mIsTreeNode && "ForLoop update is not a treenode.");
    TreeNode *update = p_update.mData.mTreeNode;
    if (update->IsPass()) {
      PassNode *pass_node = (PassNode*)update;
      for (unsigned i = 0; i < pass_node->GetChildrenNum(); i++)
        for_loop->AddUpdate(pass_node->GetChild(i));
    } else {
      for_loop->AddUpdate(update);
    }
  }

  Param p_body = mParams[3];
  if (!p_body.mIsEmpty) {
    MASSERT(p_body.mIsTreeNode && "ForLoop body is not a treenode.");
    TreeNode *body = CvtToBlock(p_body.mData.mTreeNode);
    for_loop->SetBody(body);
  }

  mLastTreeNode = for_loop;
  return mLastTreeNode;
}

TreeNode* ASTBuilder::BuildWhileLoop() {
  if (mTrace)
    std::cout << "In BuildWhileLoop " << std::endl;

  WhileLoopNode *while_loop = (WhileLoopNode*)mTreePool->NewTreeNode(sizeof(WhileLoopNode));
  new (while_loop) WhileLoopNode();

  MASSERT(mParams.size() == 2 && "BuildWhileLoop has NO 2 params?");

  Param p_cond = mParams[0];
  if (!p_cond.mIsEmpty) {
    MASSERT(p_cond.mIsTreeNode && "WhileLoop condition is not a treenode.");
    TreeNode *cond = p_cond.mData.mTreeNode;
    while_loop->SetCond(cond);
  }

  Param p_body = mParams[1];
  if (!p_body.mIsEmpty) {
    MASSERT(p_body.mIsTreeNode && "WhileLoop body is not a treenode.");
    TreeNode *body = CvtToBlock(p_body.mData.mTreeNode);
    while_loop->SetBody(body);
  }

  mLastTreeNode = while_loop;
  return mLastTreeNode;
}

TreeNode* ASTBuilder::BuildDoLoop() {
  if (mTrace)
    std::cout << "In BuildDoLoop " << std::endl;

  DoLoopNode *do_loop = (DoLoopNode*)mTreePool->NewTreeNode(sizeof(DoLoopNode));
  new (do_loop) DoLoopNode();

  MASSERT(mParams.size() == 2 && "BuildDoLoop has NO 2 params?");

  Param p_cond = mParams[0];
  if (!p_cond.mIsEmpty) {
    MASSERT(p_cond.mIsTreeNode && "DoLoop condition is not a treenode.");
    TreeNode *cond = p_cond.mData.mTreeNode;
    do_loop->SetCond(cond);
  }

  Param p_body = mParams[1];
  if (!p_body.mIsEmpty) {
    MASSERT(p_body.mIsTreeNode && "DoLoop body is not a treenode.");
    TreeNode *body = CvtToBlock(p_body.mData.mTreeNode);
    do_loop->SetBody(body);
  }

  mLastTreeNode = do_loop;
  return mLastTreeNode;
}

// BuildSwitchLabel takes one argument, the expression telling the value of label.
TreeNode* ASTBuilder::BuildSwitchLabel() {
  if (mTrace)
    std::cout << "In BuildSwitchLabel " << std::endl;

  SwitchLabelNode *label =
    (SwitchLabelNode*)mTreePool->NewTreeNode(sizeof(SwitchLabelNode));
  new (label) SwitchLabelNode();

  MASSERT(mParams.size() == 1 && "BuildSwitchLabel has NO 1 params?");
  Param p_value = mParams[0];
  MASSERT(!p_value.mIsEmpty);
  MASSERT(p_value.mIsTreeNode && "Label in BuildSwitchLabel is not a tree.");

  TreeNode *value = p_value.mData.mTreeNode;
  label->SetValue(value);

  mLastTreeNode = label;
  return label;
}

// BuildDefaultSwitchLabel takes NO argument.
TreeNode* ASTBuilder::BuildDefaultSwitchLabel() {
  if (mTrace)
    std::cout << "In BuildDefaultSwitchLabel " << std::endl;
  SwitchLabelNode *label =
    (SwitchLabelNode*)mTreePool->NewTreeNode(sizeof(SwitchLabelNode));
  new (label) SwitchLabelNode();
  label->SetIsDefault(true);
  mLastTreeNode = label;
  return label;
}

// BuildOneCase takes two arguments, the expression of a label and a
// and the statements under the label. Both the label and the statements
// could be a PassNode, and I need look into it. We don't want to carry
// PassNode into the SwitchCaseNode.

TreeNode* ASTBuilder::BuildOneCase() {
  if (mTrace)
    std::cout << "In BuildOneCase " << std::endl;

  SwitchCaseNode *case_node =
    (SwitchCaseNode*)mTreePool->NewTreeNode(sizeof(SwitchCaseNode));
  new (case_node) SwitchCaseNode();

  MASSERT(mParams.size() == 2 && "BuildOneCase has NO 1 params?");

  Param p_label = mParams[0];
  MASSERT(!p_label.mIsEmpty);
  MASSERT(p_label.mIsTreeNode && "Labels in BuildOneCase is not a tree.");
  TreeNode *label = p_label.mData.mTreeNode;
  case_node->AddLabel(label);

  Param p_stmt = mParams[1];
  MASSERT(!p_stmt.mIsEmpty);
  MASSERT(p_stmt.mIsTreeNode && "Stmts in BuildOneCase is not a tree.");
  TreeNode *stmt = p_stmt.mData.mTreeNode;
  case_node->AddStmt(stmt);

  mLastTreeNode = case_node;
  return case_node;
}

SwitchCaseNode* ASTBuilder::SwitchLabelToCase(SwitchLabelNode *label) {
  SwitchCaseNode *case_node =
    (SwitchCaseNode*)mTreePool->NewTreeNode(sizeof(SwitchCaseNode));
  new (case_node) SwitchCaseNode();
  case_node->AddLabel(label);
  return case_node;
}

// This takes one argument, which is all SwitchCaseNode-s. It could be
// a PassNode. We don't handle it at all. We simply forward this tree node
// up to the parent, which should be final Switch Node.
TreeNode* ASTBuilder::BuildAllCases() {
  if (mTrace)
    std::cout << "In BuildAllCases " << std::endl;

  MASSERT(mParams.size() == 1 && "BuildAllCases has NO 1 params?");
  Param p_cases = mParams[0];
  MASSERT(!p_cases.mIsEmpty);
  MASSERT(p_cases.mIsTreeNode && "Cases in BuildAllCases is not a tree.");
  TreeNode *cases = p_cases.mData.mTreeNode;

  // We want to make sure every tree node after BuildAllCases() is
  // a SwitchCaseNode. This will ease the handling in future AST process.
  // So for those SwitchLabelNode which only have no statement, will be
  // converted to a SwitchCaseNode.

  if (cases->IsPass()) {
    PassNode *pass = (PassNode*)cases;
    for (unsigned i = 0; i < pass->GetChildrenNum(); i++) {
      TreeNode *child = pass->GetChild(i);
      if (child->IsSwitchLabel()) {
        SwitchCaseNode *newcase = SwitchLabelToCase((SwitchLabelNode*)child);
        pass->SetChild(i, newcase);
      }
    }
  } else if (cases->IsSwitchLabel()) {
    SwitchLabelNode *label = (SwitchLabelNode*)cases;
    SwitchCaseNode *newcase = SwitchLabelToCase(label);
    cases = newcase;
  }

  mLastTreeNode = cases;
  return cases;
}

TreeNode* ASTBuilder::BuildSwitch() {
  if (mTrace)
    std::cout << "In BuildSwitch " << std::endl;

  SwitchNode *switch_node =
    (SwitchNode*)mTreePool->NewTreeNode(sizeof(SwitchNode));
  new (switch_node) SwitchNode();

  MASSERT(mParams.size() == 2 && "BuildSwitch has NO 1 params?");

  Param p_cond = mParams[0];
  MASSERT(!p_cond.mIsEmpty);
  MASSERT(p_cond.mIsTreeNode && "Condition in BuildSwitch is not a tree.");
  TreeNode *cond = p_cond.mData.mTreeNode;
  switch_node->SetCond(cond);

  Param p_cases = mParams[1];
  MASSERT(!p_cases.mIsEmpty);
  MASSERT(p_cases.mIsTreeNode && "Cases in BuildSwitch is not a tree.");
  TreeNode *cases = p_cases.mData.mTreeNode;
  switch_node->AddCase(cases);

  mLastTreeNode = switch_node;
  return switch_node;
}

////////////////////////////////////////////////////////////////////////////////
// Issues in building declarations.
// 1) First we are going to create an IdentifierNode, which should be attached
//    to a ASTScope.
// 2) The tree is created in order of children-first, so the scope is not there yet.
//    We need have a list of pending declarations until the scope is created.
////////////////////////////////////////////////////////////////////////////////

// AddTypeTo takes two parameters, 1) tree; 2) type
TreeNode* ASTBuilder::AddTypeTo() {
  if (mTrace)
    std::cout << "In AddTypeTo " << std::endl;

  MASSERT(mParams.size() == 2 && "BinaryDecl has NO 2 params?");
  Param p_type = mParams[1];
  Param p_name = mParams[0];

  MASSERT(!p_type.mIsEmpty && p_type.mIsTreeNode
          && "Not appropriate type node in AddTypeTo()");
  TreeNode *tree_type = p_type.mData.mTreeNode;

  if (!p_name.mIsTreeNode)
    MERROR("The variable name should be a IdentifierNode already, but actually NOT?");
  TreeNode *node = p_name.mData.mTreeNode;

  add_type_to(node, tree_type);

  return node;
}

// BuildDecl takes two parameters, 1) type; 2) name
TreeNode* ASTBuilder::BuildDecl() {
  if (mTrace)
    std::cout << "In BuildDecl" << std::endl;

  MASSERT(mParams.size() == 2 && "BinaryDecl has NO 2 params?");
  Param p_type = mParams[0];
  Param p_name = mParams[1];

  MASSERT(!p_type.mIsEmpty && p_type.mIsTreeNode
          && "Not appropriate type node in BuildDecl()");
  TreeNode *tree_type = p_type.mData.mTreeNode;

  if (!p_name.mIsTreeNode)
    MERROR("The variable name should be a IdentifierNode already, but actually NOT?");
  TreeNode *node = p_name.mData.mTreeNode;

  add_type_to(node, tree_type);

  mLastTreeNode = node;
  return node;
}

// BuildField takes two parameters,
// 1) upper enclosing node, could be another field.
// 2) name of this field.
TreeNode* ASTBuilder::BuildField() {
  if (mTrace)
    std::cout << "In BuildField" << std::endl;

  MASSERT(mParams.size() == 2 && "BuildField has NO 2 params?");
  Param p_var_a = mParams[0];
  Param p_var_b = mParams[1];

  // Both variable should have been created as tree node.
  if (!p_var_a.mIsTreeNode || !p_var_b.mIsTreeNode) {
    MERROR("The param in BuildField is not a treenode");
  }

  // The second param should be an IdentifierNode
  TreeNode *node_a = p_var_a.mIsEmpty ? NULL : p_var_a.mData.mTreeNode;
  TreeNode *node_b = p_var_b.mIsEmpty ? NULL : p_var_b.mData.mTreeNode;

  FieldNode *field = NULL;

  if (node_b->IsPass()) {
    TreeNode *upper = node_a;
    PassNode *pass = (PassNode*)node_b;
    for (unsigned i = 0; i < pass->GetChildrenNum(); i++) {
      TreeNode *child = pass->GetChild(i);
      MASSERT(child->IsIdentifier());

      field = (FieldNode*)mTreePool->NewTreeNode(sizeof(FieldNode));
      new (field) FieldNode();
      field->SetUpper(upper);
      field->SetField((IdentifierNode*)child);

      upper = field;
    }
  } else {
    MASSERT(node_b->IsIdentifier());
    field = (FieldNode*)mTreePool->NewTreeNode(sizeof(FieldNode));
    new (field) FieldNode();
    field->SetUpper(node_a);
    field->SetField((IdentifierNode*)node_b);
  }

  mLastTreeNode = field;
  return mLastTreeNode;
}

// BuildVariableList takes two parameters, var 1 and var 2
TreeNode* ASTBuilder::BuildVarList() {
  if (mTrace)
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

// Attach the modifier(s) to mLastTreeNode.
TreeNode* ASTBuilder::AddModifier() {
  if (mTrace)
    std::cout << "In AddModifier" << std::endl;

  Param p_mod = mParams[0];
  if (p_mod.mIsEmpty) {
    if (mTrace)
      std::cout << " do nothing." << std::endl;
    return mLastTreeNode;
  }

  if (!p_mod.mIsTreeNode)
    MERROR("The modifier is not a treenode");
  TreeNode *mod= p_mod.mData.mTreeNode;

  if (mod->IsPass()) {
    PassNode *pass = (PassNode*)mod;
    for (unsigned i = 0; i < pass->GetChildrenNum(); i++) {
      TreeNode *child = pass->GetChild(i);
      if (child->IsAnnotation()) {
        AnnotationNode *a = (AnnotationNode*)child;
        mLastTreeNode->AddAnnotation(a);
      } else {
        add_attribute_to(mLastTreeNode, child);
      }
    }
  } else if (mod->IsAnnotation()) {
    AnnotationNode *a = (AnnotationNode*)mod;
    mLastTreeNode->AddAnnotation(a);
  } else {
    add_attribute_to(mLastTreeNode, mod);
  }

  return mLastTreeNode;
}

// Takes two arguments. 1) target tree node; 2) modifier, which could be
// attr, annotation/pragma.
//
// This function doesn't update mLastTreeNode.
TreeNode* ASTBuilder::AddModifierTo() {
  if (mTrace)
    std::cout << "In AddModifierTo " << std::endl;

  Param p_tree = mParams[0];
  MASSERT(!p_tree.mIsEmpty && "Treenode cannot be empty in AddModifierTo");
  MASSERT(p_tree.mIsTreeNode && "The tree node is not a treenode in AddModifierTo()");
  TreeNode *tree = p_tree.mData.mTreeNode;

  Param p_mod = mParams[1];
  if(p_mod.mIsEmpty)
    return tree;

  MASSERT(p_mod.mIsTreeNode && "The Attr is not a treenode in AddModifierTo()");
  TreeNode *mod = p_mod.mData.mTreeNode;

  if (mod->IsPass()) {
    PassNode *pass = (PassNode*)mod;
    for (unsigned i = 0; i < pass->GetChildrenNum(); i++) {
      TreeNode *child = pass->GetChild(i);
      if (child->IsAnnotation()) {
        AnnotationNode *a = (AnnotationNode*)child;
        tree->AddAnnotation(a);
      } else {
        add_attribute_to(tree, child);
      }
    }
  } else if (mod->IsAnnotation()) {
    AnnotationNode *a = (AnnotationNode*)mod;
    tree->AddAnnotation(a);
  } else {
    add_attribute_to(tree, mod);
  }

  return tree;
}

TreeNode* ASTBuilder::AddInitTo() {
  if (mTrace)
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
  if (mTrace)
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
  node_class->SetName(in->GetName());

  mLastTreeNode = node_class;
  return mLastTreeNode;
}

TreeNode* ASTBuilder::SetClassIsJavaEnum() {
  ClassNode *klass = (ClassNode*)mLastTreeNode;
  klass->SetJavaEnum();
  return mLastTreeNode;
}

// This takes just one argument which is the root of sub tree
TreeNode* ASTBuilder::BuildBlock() {
  if (mTrace)
    std::cout << "In BuildBlock" << std::endl;

  BlockNode *block = (BlockNode*)mTreePool->NewTreeNode(sizeof(BlockNode));
  new (block) BlockNode();

  Param p_subtree = mParams[0];
  if (!p_subtree.mIsEmpty) {
    if (!p_subtree.mIsTreeNode)
      MERROR("The subtree is not a treenode in BuildBlock()");

    // If the subtree is PassNode, we need add all children to block
    // If else, simply assign subtree as child.
    TreeNode *subtree = p_subtree.mData.mTreeNode;
    if (subtree->IsPass()) {
      PassNode *pass_node = (PassNode*)subtree;
      for (unsigned i = 0; i < pass_node->GetChildrenNum(); i++)
        block->AddChild(pass_node->GetChild(i));
    } else {
      block->AddChild(subtree);
    }
  }

  mLastTreeNode = block;
  return mLastTreeNode;
}

// This takes just one argument which is the root of tree to be added.
TreeNode* ASTBuilder::AddToBlock() {
  if (mTrace)
    std::cout << "In AddToBlock" << std::endl;

  MASSERT(mLastTreeNode->IsBlock());
  BlockNode *block = (BlockNode*)mLastTreeNode;

  Param p_subtree = mParams[0];
  if (!p_subtree.mIsEmpty) {
    if (!p_subtree.mIsTreeNode)
      MERROR("The subtree is not a treenode in BuildBlock()");

    // If the subtree is PassNode, we need add all children to block
    // If else, simply assign subtree as child.
    TreeNode *subtree = p_subtree.mData.mTreeNode;
    if (subtree->IsPass()) {
      PassNode *pass_node = (PassNode*)subtree;
      for (unsigned i = 0; i < pass_node->GetChildrenNum(); i++)
        block->AddChild(pass_node->GetChild(i));
    } else {
      block->AddChild(subtree);
    }
  }

  // set last tree node
  mLastTreeNode = block;
  return mLastTreeNode;
}

// if tnode is not a BlockNode, wrap it into a BlockNode
TreeNode* ASTBuilder::CvtToBlock(TreeNode *tnode) {
  if (mTrace)
    std::cout << "In CvtToBlock" << std::endl;

  if (tnode->IsBlock()) {
    return tnode;
  }

  BlockNode *block = (BlockNode*)mTreePool->NewTreeNode(sizeof(BlockNode));
  new (block) BlockNode();
  block->AddChild(tnode);
  return block;
}

// This takes just one argument which either a block node, or the root of sub tree
TreeNode* ASTBuilder::BuildInstInit() {
  if (mTrace)
    std::cout << "In BuildInstInit" << std::endl;

  BlockNode *b = NULL;

  Param p_subtree = mParams[0];
  if (!p_subtree.mIsEmpty) {
    if (!p_subtree.mIsTreeNode)
      MERROR("The subtree is not a treenode in BuildInstInit()");

    TreeNode *subtree = p_subtree.mData.mTreeNode;
    if (subtree->IsBlock()) {
      b = (BlockNode*)subtree;
      b->SetIsInstInit();
    }
  }

  if (!b) {
    b = (BlockNode*)BuildBlock();
    b->SetIsInstInit();
  }

  // set last tree node
  mLastTreeNode = b;
  return mLastTreeNode;
}

TreeNode* ASTBuilder::AddSuperClass() {
  if (mTrace)
    std::cout << "In AddSuperClass" << std::endl;
  Param p_attr = mParams[0];
  return mLastTreeNode;
}

TreeNode* ASTBuilder::AddSuperInterface() {
  if (mTrace)
    std::cout << "In AddSuperInterface" << std::endl;
  Param p_attr = mParams[0];
  return mLastTreeNode;
}

// Takes one parameter which is the tree of class body.
TreeNode* ASTBuilder::AddClassBody() {
  if (mTrace)
    std::cout << "In AddClassBody" << std::endl;

  Param p_body = mParams[0];
  if (!p_body.mIsTreeNode)
    MERROR("The class body is not a tree node.");
  TreeNode *tree_node = p_body.mData.mTreeNode;
  MASSERT(tree_node->IsBlock() && "Class body is not a BlockNode?");
  BlockNode *block = (BlockNode*)tree_node;

  MASSERT(mLastTreeNode->IsClass() && "Class is not a ClassNode?");
  ClassNode *klass = (ClassNode*)mLastTreeNode;
  klass->AddBody(block);
  klass->Construct();

  return mLastTreeNode;
}

// This takes just one argument which is the annotation type name.
TreeNode* ASTBuilder::BuildAnnotationType() {
  if (mTrace)
    std::cout << "In BuildAnnotationType" << std::endl;

  Param p_name = mParams[0];

  if (!p_name.mIsTreeNode)
    MERROR("The annotationtype name is not a treenode in BuildAnnotationtType()");
  TreeNode *node_name = p_name.mData.mTreeNode;

  if (!node_name->IsIdentifier())
    MERROR("The annotation type name should be an indentifier node. Not?");
  IdentifierNode *in = (IdentifierNode*)node_name;

  AnnotationTypeNode *annon_type = (AnnotationTypeNode*)mTreePool->NewTreeNode(sizeof(AnnotationTypeNode));
  new (annon_type) AnnotationTypeNode();
  annon_type->SetId(in);

  // set last tree node and return it.
  mLastTreeNode = annon_type;
  return mLastTreeNode;
}

TreeNode* ASTBuilder::AddAnnotationTypeBody() {
  if (mTrace)
    std::cout << "In AddAnnotationTypeBody" << std::endl;
  Param p_attr = mParams[0];
  return mLastTreeNode;
}

TreeNode* ASTBuilder::BuildAnnotation() {
  if (mTrace)
    std::cout << "In BuildAnnotation" << std::endl;
  Param p_name = mParams[0];

  if (!p_name.mIsTreeNode)
    MERROR("The annotationtype name is not a treenode in BuildAnnotation()");
  TreeNode *iden = p_name.mData.mTreeNode;

  if (!iden->IsIdentifier())
    MERROR("The annotation name is NOT an indentifier node.");

  AnnotationNode *annot = (AnnotationNode*)mTreePool->NewTreeNode(sizeof(AnnotationNode));
  new (annot) AnnotationNode();
  annot->SetId((IdentifierNode*)iden);

  // set last tree node and return it.
  mLastTreeNode = annot;
  return mLastTreeNode;
}

TreeNode* ASTBuilder::BuildInterface() {
  if (mTrace)
    std::cout << "In BuildInterface" << std::endl;
  Param p_name = mParams[0];

  if (!p_name.mIsTreeNode)
    MERROR("The name is not a treenode in BuildInterface()");
  TreeNode *node_name = p_name.mData.mTreeNode;

  if (!node_name->IsIdentifier())
    MERROR("The name is NOT an indentifier node.");
  IdentifierNode *in = (IdentifierNode*)node_name;

  InterfaceNode *interf = (InterfaceNode*)mTreePool->NewTreeNode(sizeof(InterfaceNode));
  new (interf) InterfaceNode();
  interf->SetName(in->GetName());

  // set last tree node and return it.
  mLastTreeNode = interf;
  return mLastTreeNode;
}

// Takes one parameter which is the tree of interface body.
TreeNode* ASTBuilder::AddInterfaceBody() {
  if (mTrace)
    std::cout << "In AddInterfaceBody" << std::endl;

  Param p_body = mParams[0];
  if (!p_body.mIsTreeNode)
    MERROR("The interface body is not a tree node.");
  TreeNode *tree_node = p_body.mData.mTreeNode;
  MASSERT(tree_node->IsBlock() && "Interface body is not a BlockNode?");
  BlockNode *block = (BlockNode*)tree_node;

  MASSERT(mLastTreeNode->IsInterface() && "Interface is not a InterfaceNode?");
  InterfaceNode *interf = (InterfaceNode*)mLastTreeNode;
  interf->Construct(block);
  return interf;
}

// This takes just one argument which is the length of this dimension
// [TODO] Don't support yet.
TreeNode* ASTBuilder::BuildDim() {
  if (mTrace)
    std::cout << "In BuildDim" << std::endl;

  DimensionNode *dim = (DimensionNode*)mTreePool->NewTreeNode(sizeof(DimensionNode));
  new (dim) DimensionNode();
  dim->AddDim();

  // set last tree node and return it.
  mLastTreeNode = dim;
  return mLastTreeNode;
}

// BuildDims() takes two parameters. Each contains a set of dimension info.
// Each is a DimensionNode.
TreeNode* ASTBuilder::BuildDims() {
  if (mTrace)
    std::cout << "In build dimension List" << std::endl;

  MASSERT(mParams.size() == 2 && "BuildDims has NO 2 params?");
  Param p_dims_a = mParams[0];
  Param p_dims_b = mParams[1];

  // Both variable should have been created as tree node.
  if (!p_dims_a.mIsTreeNode || !p_dims_b.mIsTreeNode) {
    MERROR("The var in BuildVarList is not a treenode");
  }

  TreeNode *node_a = p_dims_a.mIsEmpty ? NULL : p_dims_a.mData.mTreeNode;
  TreeNode *node_b = p_dims_b.mIsEmpty ? NULL : p_dims_b.mData.mTreeNode;

  // Pick an existing node, merge the other into to.
  DimensionNode *node_ret = NULL;
  if (node_a) {
    node_ret = (DimensionNode*)node_a;
    node_ret->Merge(node_b);
  } else if (node_b) {
    node_ret = (DimensionNode*)node_b;
    node_ret->Merge(node_a);
  } else {
    // both nodes are NULL
    MERROR("BuildDims() has two NULL parameters?");
  }

  // Set last tree node
  mLastTreeNode = node_ret;
  return node_ret;
}

// AddDimsTo() takes two parameters. The first is the variable,
// the second is the dims.
TreeNode* ASTBuilder::AddDimsTo() {
  if (mTrace)
    std::cout << "In AddDimsTo " << std::endl;

  MASSERT(mParams.size() == 2 && "AddDimsTo has NO 2 params?");
  Param p_dims_a = mParams[0];
  Param p_dims_b = mParams[1];

  // Both variable should have been created as tree node.
  if (p_dims_a.mIsEmpty)
    MERROR("The var in AddDimsTo() is empty?");

  TreeNode *node_a = p_dims_a.mData.mTreeNode;
  TreeNode *node_b = p_dims_b.mIsEmpty ? NULL : p_dims_b.mData.mTreeNode;

  if (node_b) {
    MASSERT(node_b->IsDimension() && "Expected a DimensionNode.");
    DimensionNode *dim = (DimensionNode*)node_b;
    if (node_a->IsIdentifier()) {
      IdentifierNode *inode = (IdentifierNode*)node_a;
      inode->SetDims(dim);
      mLastTreeNode = node_a;
    } else if (node_a->IsPrimType()) {
      PrimTypeNode *pt = (PrimTypeNode*)node_a;
      PrimArrayTypeNode *pat = (PrimArrayTypeNode*)mTreePool->NewTreeNode(sizeof(PrimArrayTypeNode));
      new (pat) PrimArrayTypeNode();
      pat->SetPrim(pt);
      pat->SetDims(dim);
      mLastTreeNode = pat;
    }
  }

  return mLastTreeNode;
}

// AddDims() takes one parameters, the dims.
// Add to mLastTreeNode
TreeNode* ASTBuilder::AddDims() {
  if (mTrace)
    std::cout << "In AddDims " << std::endl;

  Param p_dims = mParams[0];

  if (p_dims.mIsEmpty)
    return mLastTreeNode;

  TreeNode *param_tree = p_dims.mData.mTreeNode;
  DimensionNode *dims = NULL;
  if (param_tree) {
    MASSERT(param_tree->IsDimension() && "Expected a DimensionNode.");
    dims = (DimensionNode*)param_tree;
  }

  if (mLastTreeNode->IsIdentifier()) {
    IdentifierNode *node = (IdentifierNode*)mLastTreeNode;
    node->SetDims(dims);
  } else if (mLastTreeNode->IsFunction()) {
    FunctionNode *node = (FunctionNode*)mLastTreeNode;
    node->SetDims(dims);
  }

  return mLastTreeNode;
}

////////////////////////////////////////////////////////////////////////////////
//                    New & Delete operation related
////////////////////////////////////////////////////////////////////////////////

// This is a help function which adds parameters to a function decl.
// It's the caller's duty to assure 'func' and 'params' are non null.
void ASTBuilder::AddParams(TreeNode *func, TreeNode *params) {
  if (params->IsIdentifier()) {
    // one single parameter at call site
    IdentifierNode *inode = (IdentifierNode*)params;
    if (func->IsFunction())
      ((FunctionNode*)func)->AddParam(inode);
    else if (func->IsNew())
      ((NewNode*)func)->AddParam(inode);
    else
      MERROR("Unsupported yet.");
  } else if (params->IsVarList()) {
    // a list of decls at function declaration
    VarListNode *vl = (VarListNode*)params;
    for (unsigned i = 0; i < vl->GetNum(); i++) {
      IdentifierNode *inode = vl->VarAtIndex(i);
      if (func->IsFunction())
        ((FunctionNode*)func)->AddParam(inode);
      else if (func->IsNew())
        ((NewNode*)func)->AddParam(inode);
      else
        MERROR("Unsupported yet.");
    }
  } else if (params->IsPass()) {
    // a list of identifiers at call site.
    PassNode *pass = (PassNode*)params;
    for (unsigned i = 0; i < pass->GetChildrenNum(); i++) {
      TreeNode *child = pass->GetChild(i);
      if (func->IsFunction())
        ((FunctionNode*)func)->AddParam(child);
      else if (func->IsNew())
        ((NewNode*)func)->AddParam(child);
      else
        MERROR("Unsupported yet.");
    }
  }
}

TreeNode* ASTBuilder::BuildNewOperation() {
  if (mTrace)
    std::cout << "In BuildNewOperation " << std::endl;

  NewNode *new_node = (NewNode*)mTreePool->NewTreeNode(sizeof(NewNode));
  new (new_node) NewNode();

  MASSERT(mParams.size() == 3 && "BuildNewOperation has NO 3 params?");
  Param p_a = mParams[0];
  Param p_b = mParams[1];
  Param p_c = mParams[2];

  // Name could not be empty
  if (p_a.mIsEmpty)
    MERROR("The name in BuildNewOperation() is empty?");
  MASSERT(p_a.mIsTreeNode && "Name of new expression is not a tree?");
  TreeNode *name = p_a.mData.mTreeNode;
  new_node->SetId(name);
  
  TreeNode *node_b = p_b.mIsEmpty ? NULL : p_b.mData.mTreeNode;
  if (node_b)
    AddParams(new_node, node_b);

  TreeNode *node_c = p_c.mIsEmpty ? NULL : p_c.mData.mTreeNode;
  if (node_c) {
    MASSERT(node_c->IsBlock() && "ClassBody is not a block?");
    BlockNode *b = (BlockNode*)node_c;
    new_node->SetBody(b);
  }

  mLastTreeNode = new_node;
  return new_node;
}

TreeNode* ASTBuilder::BuildDeleteOperation() {
}

////////////////////////////////////////////////////////////////////////////////
//                    Building AssertNode related
// The node could take two parameters, one expression and one message.
// It also could take only one parameter, the expression.
////////////////////////////////////////////////////////////////////////////////

TreeNode* ASTBuilder::BuildAssert() {
  if (mTrace)
    std::cout << "In BuildAssert " << std::endl;

  AssertNode *assert_node = (AssertNode*)mTreePool->NewTreeNode(sizeof(AssertNode));
  new (assert_node) AssertNode();

  MASSERT(mParams.size() >= 1 && "BuildAssert has NO expression?");
  Param p_a, p_b;

  p_a = mParams[0];
  if (p_a.mIsEmpty)
    MERROR("The expression in BuildAssert() is empty?");
  MASSERT(p_a.mIsTreeNode && "Expression is not a tree?");
  TreeNode *expr = p_a.mData.mTreeNode;
  assert_node->SetExpr(expr);

  if (mParams.size() == 2) {
    p_b = mParams[1];
    if (!p_b.mIsEmpty) {
      MASSERT(p_b.mIsTreeNode && "Messge of assert is not a tree?");
      TreeNode *node_b = p_b.mData.mTreeNode;
      if (node_b)
        assert_node->SetMsg(node_b);
    }
  }

  mLastTreeNode = assert_node;
  return assert_node;
}

////////////////////////////////////////////////////////////////////////////////
//                    CallSite related
////////////////////////////////////////////////////////////////////////////////

// There are two formats of BuildCall, one with one param, the other no param.
TreeNode* ASTBuilder::BuildCall() {
  if (mTrace)
    std::cout << "In BuildCall" << std::endl;

  CallNode *call = (CallNode*)mTreePool->NewTreeNode(sizeof(CallNode));
  new (call) CallNode();

  // The default is having no param.
  TreeNode *method = mLastTreeNode;

  if (!ParamsEmpty()) {
    Param p_method = mParams[0];
    if (!p_method.mIsTreeNode)
      MERROR("The function name is not a treenode in BuildFunction()");
    method = p_method.mData.mTreeNode;
  }

  call->SetMethod(method);

  mLastTreeNode = call;
  return mLastTreeNode;
}

// The argument could be any kind of expression, like arithmetic expression,
// identifier, call, or any valid expression.
//
// This AddArguments can be used for CallNode, NewNode, etc.
// Right now I just support CallNode. NewNode will be moved from AddParams()
// to here.

TreeNode* ASTBuilder::AddArguments() {
  if (mTrace)
    std::cout << "In AddArguments" << std::endl;

  Param p_params = mParams[0];
  TreeNode *params = NULL;
  if (!p_params.mIsEmpty) {
    if (!p_params.mIsTreeNode)
      MERROR("The parameters is not a treenode in AddArguments()");
    params = p_params.mData.mTreeNode;
  }

  if (!params)
    return mLastTreeNode;

  CallNode *call = (CallNode*)mLastTreeNode;

  if (params->IsVarList()) {
    VarListNode *vl = (VarListNode*)params;
    for (unsigned i = 0; i < vl->GetNum(); i++) {
      IdentifierNode *inode = vl->VarAtIndex(i);
      call->AddArg(inode);
    }
  } else if (params->IsPass()) {
    PassNode *pass = (PassNode*)params;
    for (unsigned i = 0; i < pass->GetChildrenNum(); i++) {
      TreeNode *child = pass->GetChild(i);
      call->AddArg(child);
    }
  } else {
    call->AddArg(params);
  }

  return mLastTreeNode;
}

// BuildVariableList takes two parameters, var 1 and var 2
TreeNode* ASTBuilder::BuildExprList() {
  if (mTrace)
    std::cout << "In build Expr List" << std::endl;

  MASSERT(mParams.size() == 2 && "BuildExprList has NO 2 params?");
  Param p_var_a = mParams[0];
  Param p_var_b = mParams[1];

  // Both variable should have been created as tree node.
  if (!p_var_a.mIsTreeNode || !p_var_b.mIsTreeNode) {
    MERROR("The expr in BuildExprList is not a treenode");
  }

  TreeNode *node_a = p_var_a.mIsEmpty ? NULL : p_var_a.mData.mTreeNode;
  TreeNode *node_b = p_var_b.mIsEmpty ? NULL : p_var_b.mData.mTreeNode;

  ExprListNode *node_ret = NULL;
  if (node_a && node_a->IsExprList()) {
    node_ret = (ExprListNode*)node_a;
    node_ret->Merge(node_b);
  } else if (node_b && node_b->IsExprList()) {
    node_ret = (ExprListNode*)node_b;
    node_ret->Merge(node_a);
  } else {
    // both nodes are not ExprListNode
    node_ret = (ExprListNode*)mTreePool->NewTreeNode(sizeof(ExprListNode));
    new (node_ret) ExprListNode();
    if (node_a)
      node_ret->Merge(node_a);
    if (node_b)
      node_ret->Merge(node_b);
  }

  // Set last tree node
  mLastTreeNode = node_ret;

  return node_ret;
}

////////////////////////////////////////////////////////////////////////////////
//                    FunctionNode related
////////////////////////////////////////////////////////////////////////////////

TreeNode* ASTBuilder::AddParams() {
  if (mTrace)
    std::cout << "In AddParams" << std::endl;

  Param p_params = mParams[0];
  if (!p_params.mIsEmpty) {
    if (!p_params.mIsTreeNode)
      MERROR("The parameters is not a treenode in AddParams()");
    TreeNode *params = p_params.mData.mTreeNode;
    AddParams(mLastTreeNode, params);
  }

  return mLastTreeNode;
}

// This takes just one argument which is the function name.
TreeNode* ASTBuilder::BuildFunction() {
  if (mTrace)
    std::cout << "In BuildFunction" << std::endl;

  Param p_name = mParams[0];

  if (!p_name.mIsTreeNode)
    MERROR("The function name is not a treenode in BuildFunction()");
  TreeNode *node_name = p_name.mData.mTreeNode;

  if (!node_name->IsIdentifier())
    MERROR("The function name should be an indentifier node. Not?");
  IdentifierNode *in = (IdentifierNode*)node_name;

  FunctionNode *function = (FunctionNode*)mTreePool->NewTreeNode(sizeof(FunctionNode));
  new (function) FunctionNode();
  function->SetName(node_name->GetName());

  mLastTreeNode = function;
  return mLastTreeNode;
}

// This takes just one argument which is the function name.
TreeNode* ASTBuilder::BuildConstructor() {
  TreeNode *t = BuildFunction();
  FunctionNode *cons = (FunctionNode*)t;
  cons->SetIsConstructor();

  mLastTreeNode = cons;
  return cons;
}

// Takes func_body as argument, to mLastTreeNode which is a function.
TreeNode* ASTBuilder::AddFunctionBody() {
  if (mTrace)
    std::cout << "In AddFunctionBody" << std::endl;

  FunctionNode *func = (FunctionNode*)mLastTreeNode;

  // It's possible that the func body is empty, such as in the
  // function header declaration. Usually it's just a token ';'.
  Param p_body = mParams[0];
  if (p_body.mIsTreeNode) {
    TreeNode *tree_node = p_body.mData.mTreeNode;
    MASSERT(tree_node->IsBlock() && "Class body is not a BlockNode?");
    BlockNode *block = (BlockNode*)tree_node;
    func->AddBody(block);
  }

  mLastTreeNode = func;
  return mLastTreeNode;
}

// Takes two arguments.
// 1st: Function
// 2nd: body
TreeNode* ASTBuilder::AddFunctionBodyTo() {
  if (mTrace)
    std::cout << "In AddFunctionBodyTo" << std::endl;

  Param p_func = mParams[0];
  if (!p_func.mIsTreeNode)
    MERROR("The Function is not a tree node.");
  TreeNode *func_node = p_func.mData.mTreeNode;
  MASSERT(func_node->IsFunction() && "Function is not a FunctionNode?");
  FunctionNode *func = (FunctionNode*)func_node;

  // It's possible that the func body is empty, such as in the
  // function header declaration. Usually it's just a token ';'.
  Param p_body = mParams[1];
  if (p_body.mIsTreeNode) {
    TreeNode *tree_node = p_body.mData.mTreeNode;
    MASSERT(tree_node->IsBlock() && "Class body is not a BlockNode?");
    BlockNode *block = (BlockNode*)tree_node;
    func->AddBody(block);
  }

  mLastTreeNode = func;
  return mLastTreeNode;
}

////////////////////////////////////////////////////////////////////////////////
//                   Other Functions
////////////////////////////////////////////////////////////////////////////////

// This takes just one argument which is the tree passed from the
// children. It could be single IdentifierNode, or a PassNode with
// more than one tree nodes.
TreeNode* ASTBuilder::BuildThrows() {
  if (mTrace)
    std::cout << "In BuildThrows" << std::endl;

  Param p_throws = mParams[0];

  if (!p_throws.mIsTreeNode)
    MERROR("The exceptions is not a treenode in BuildThrows()");
  TreeNode *node_throws = p_throws.mData.mTreeNode;

  if (!node_throws->IsIdentifier() && !node_throws->IsPass())
    MERROR("The throws should be an indentifier node or pass node. Not?");

  mLastTreeNode = node_throws;
  return mLastTreeNode;
}

// Takes two arguments.
// 1st: Function
// 2nd: throws
TreeNode* ASTBuilder::AddThrowsTo() {
  if (mTrace)
    std::cout << "In AddThrowsTo" << std::endl;

  Param p_func = mParams[0];
  if (!p_func.mIsTreeNode)
    MERROR("The Function is not a tree node.");
  TreeNode *func_node = p_func.mData.mTreeNode;
  MASSERT(func_node->IsFunction() && "Function is not a FunctionNode?");
  FunctionNode *func = (FunctionNode*)func_node;

  // It's possible that the throws is a single identifier node,
  // or a pass node.
  Param p_body = mParams[1];
  if (p_body.mIsTreeNode) {
    TreeNode *tree_node = p_body.mData.mTreeNode;
    if (tree_node->IsIdentifier()) {
      IdentifierNode *id = (IdentifierNode*)tree_node;
      ExceptionNode *exception = (ExceptionNode*)mTreePool->NewTreeNode(sizeof(ExceptionNode));
      new (exception) ExceptionNode(id);
      func->AddThrow(exception);
    } else if (tree_node->IsPass()) {
      PassNode *pass = (PassNode*)tree_node;
      for (unsigned i = 0; i < pass->GetChildrenNum(); i++) {
        TreeNode *child = pass->GetChild(i);
        if (child->IsIdentifier()) {
          IdentifierNode *id = (IdentifierNode*)child;
          ExceptionNode *exception = (ExceptionNode*)mTreePool->NewTreeNode(sizeof(ExceptionNode));
          new (exception) ExceptionNode(id);
          func->AddThrow(exception);
        } else {
          MERROR("The to-be-added exception is not an identifier?");
        }
      }
    }
  }

  mLastTreeNode = func;
  return mLastTreeNode;
}

////////////////////////////////////////////////////////////////////////////////
//                   User Type Functions
////////////////////////////////////////////////////////////////////////////////

TreeNode* ASTBuilder::BuildUserType() {
  if (mTrace)
    std::cout << "In BuildUserType" << std::endl;

  Param p_id = mParams[0];
  if (!p_id.mIsTreeNode)
    MERROR("The Identifier of user type is not a treenode.");

  TreeNode *node = p_id.mData.mTreeNode;
  if (!node->IsIdentifier())
    MERROR("The Identifier of user type is not an identifier.");
  IdentifierNode *id = (IdentifierNode*)node;

  UserTypeNode *user_type = (UserTypeNode*)mTreePool->NewTreeNode(sizeof(UserTypeNode));
  new (user_type) UserTypeNode(id);
  mLastTreeNode = user_type;
  return mLastTreeNode;
}

TreeNode* ASTBuilder::AddTypeArgument() {
  if (mTrace)
    std::cout << "In AddTypeArgument" << std::endl;

  if (mParams.size() == 0)
    return mLastTreeNode;

  Param p_args = mParams[0];

  // Some language allows special syntax as type arguments, like <> in Java.
  // It's just a token.
  if (!p_args.mIsTreeNode)
    return mLastTreeNode;

  TreeNode *args = p_args.mData.mTreeNode;
  MASSERT(args);

  UserTypeNode *type_node = (UserTypeNode*)mLastTreeNode;
  type_node->AddTypeArgs(args);

  return mLastTreeNode;
}

////////////////////////////////////////////////////////////////////////////////
//                       Lambda Expression
////////////////////////////////////////////////////////////////////////////////

TreeNode* ASTBuilder::BuildLambda() {
  if (mTrace)
    std::cout << "In BuildLambda" << std::endl;

  Param p_params = mParams[0];
  TreeNode *params_node = NULL;
  if (!p_params.mIsEmpty) {
    if (!p_params.mIsTreeNode)
      MERROR("Lambda params is not a tree node.");
    else
      params_node = p_params.mData.mTreeNode;
  }

  Param p_body = mParams[1];
  TreeNode *body_node = NULL;
  if (!p_body.mIsEmpty) {
    if (!p_body.mIsTreeNode)
      MERROR("Lambda Body is not a tree node.");
    else
      body_node = CvtToBlock(p_body.mData.mTreeNode);
  }

  LambdaNode *lambda = (LambdaNode*)mTreePool->NewTreeNode(sizeof(LambdaNode));
  new (lambda) LambdaNode();

  if (params_node) {
    if (params_node->IsIdentifier())
      lambda->AddParam((IdentifierNode*)params_node);
  }

  lambda->SetBody(body_node);

  mLastTreeNode = lambda;
  return mLastTreeNode;
}
}
