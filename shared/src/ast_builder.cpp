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

#include "token.h"
#include "ruletable.h"
#include "ast_builder.h"
#include "ast_scope.h"
#include "ast_attr.h"
#include "ast_type.h"

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
  } else if (token->IsKeyword()) {
    KeywordToken *kt = (KeywordToken*)token;
    const char *keyword = kt->GetName();

    // Check if it's an attribute
    AttrNode *n = gAttrPool.GetAttrNode(keyword);
    if (n) {
      mLastTreeNode = n;
      return n;
    }

    // Check if it's a type
    PrimTypeNode *type = gPrimTypePool.FindType(keyword);
    if (type) {
      mLastTreeNode = type;
      return type;
    }
  }

  // Other tokens shouldn't be involved in the tree creation.
  return NULL;
}

////////////////////////////////////////////////////////////////////////////////////////
//                      Static Functions help build the tree
////////////////////////////////////////////////////////////////////////////////////////

// It's the caller to assure both arguments are valid.
static void add_attribute_to_kernel(TreeNode *tree, AttrNode *attr) {
  AttrId aid = attr->GetId();
  if (tree->IsVarList()) {
    VarListNode *vl = (VarListNode*)tree;
    for (unsigned i = 0; i < vl->GetNum(); i++) {
      IdentifierNode *inode = vl->VarAtIndex(i);
      inode->AddAttr(aid);
    }
    return;
  } else if (tree->IsFunction()) {
    FunctionNode *func = (FunctionNode*)tree;
    func->AddAttr(aid);
    return;
  } else if (tree->IsIdentifier()) {
    IdentifierNode *iden = (IdentifierNode*)tree;
    iden->AddAttr(aid);
    return;
  } else if (tree->IsBlock()){
    BlockNode *b = (BlockNode*)b;
    if (b->IsInstInit()) {
      b->AddAttr(aid);
      return;
    }
  }
  MERROR("Unsupported tree, wanting to add attribute.");
}

static void add_attribute_to(TreeNode *tree_node, TreeNode *attr) {
  if (attr->IsAttribute()) {
    AttrNode *attr_node = (AttrNode*)attr;
    add_attribute_to_kernel(tree_node, attr_node);
  } else if (attr->IsPass()) {
    PassNode *pass = (PassNode*)attr;
    for (unsigned i = 0; i < pass->GetChildrenNum(); i++) {
      TreeNode *child = pass->GetChild(i);
      if (child->IsAttribute()) {
        AttrNode *attr_node = (AttrNode*)child;
        add_attribute_to_kernel(tree_node, attr_node);
      } else {
        MERROR("The to-be-added node is not an attribute?");
      }
    }
  } else {
    MERROR("The to-be-added node is not an attribute?");
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
  if (mTrace)
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

// Attach the attributes to mLastTreeNode.
// We only take the AttrId from the tree node.
TreeNode* ASTBuilder::AddAttribute() {
  if (mTrace)
    std::cout << "In AddAttribute" << std::endl;

  Param p_attr = mParams[0];
  if (p_attr.mIsEmpty) {
    if (mTrace)
      std::cout << " do nothing." << std::endl;
    return mLastTreeNode;
  }

  if (!p_attr.mIsTreeNode)
    MERROR("The attribue is not a treenode");
  TreeNode *attr= p_attr.mData.mTreeNode;
  add_attribute_to(mLastTreeNode, attr);

  return mLastTreeNode;
}

// Takes two arguments. 1) target tree node; 2) attribute
TreeNode* ASTBuilder::AddAttributeTo() {
  if (mTrace)
    std::cout << "In AddAttributeTo " << std::endl;

  Param p_tree = mParams[0];
  MASSERT(!p_tree.mIsEmpty && "Treenode cannot be empty in AddAttributeTo");
  MASSERT(p_tree.mIsTreeNode && "The tree node is not a treenode in AddAttributeTo()");
  TreeNode *tree = p_tree.mData.mTreeNode;

  Param p_attr = mParams[1];
  if(p_attr.mIsEmpty)
    return tree;

  MASSERT(p_attr.mIsTreeNode && "The Attr is not a treenode in AddAttributeTo()");
  TreeNode *attr = p_attr.mData.mTreeNode;

  add_attribute_to(tree, attr);

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

  // set last tree node
  mLastTreeNode = block;
  return mLastTreeNode;
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
    b = BuildBlock();
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
  annon_type->SetName(node_name);

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
  Param p_attr = mParams[0];
  return mLastTreeNode;
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
    IdentifierNode *inode = (IdentifierNode*)node_a;
    inode->SetDims(node_b);
  }

  mLastTreeNode = node_a;
  return node_a;
}

// AddDims() takes one parameters, the dims.
// Add to mLastTreeNode
TreeNode* ASTBuilder::AddDims() {
  if (mTrace)
    std::cout << "In AddDims " << std::endl;

  Param p_dims = mParams[0];

  if (p_dims.mIsEmpty)
    return mLastTreeNode;

  TreeNode *dims = p_dims.mData.mTreeNode;

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
//                   Other Functions
////////////////////////////////////////////////////////////////////////////////

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
    func->Construct();
  }

  mLastTreeNode = func;
  return mLastTreeNode;
}
