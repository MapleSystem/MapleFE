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
//////////////////////////////////////////////////////////////////////////////////////////////
// I decided to build AST tree after the parsing, instead of generating expr, stmt and etc.
//
// A module (compilation unit) could have multiple trees, depending on how many top level
// syntax constructs in it. Take Java for example, the top level constructs have Class, Interface.
// For C, it has only functions.
//
// AST is generated from the Appeal tree after SortOut. We walk through the appeal tree, and on
// each node, we apply the 'action' of the rule table which help build the tree node.
//////////////////////////////////////////////////////////////////////////////////////////////

#ifndef __AST_HEADER__
#define __AST_HEADER__

// A node in AST could be one of the following types.
//
// 1. Token
//    This is the leaf node in an AST. It could be a variable, a literal.
//
// 2. Operator
//    This is one of the operators defined in supported_operators.def
//    As you may know, operators, literals and variables (identifiers) are all preprocessed
//    to be a token. But we also define 'Token' as a NodeKind. So please keep in mind, we
//    catagorize operator to a dedicated NodeKind.
//
// 3. Construct
//    This is syntax construct, such as for(..) loop construct. This is language specific,
//    and most of these nodes are defined under each language, such as java/ directory.
//    However, I do define some popular construct in shared/ directory.
//
// 4. Function
//    A Function node have its arguments as children node. The return value is not counted.
//

#include "ast_type.h"
#include "ast_mempool.h"
#include "ruletable.h"
#include "token.h"

enum NodeKind {
  NK_Identifier,
  NK_Literal,
  NK_Operator,
  NK_Construct,
  NK_Function,
  NK_Null,
};

// TreeNode has a VIRTUAL destructor. Why?
//
// Derived TreeNodes are allocated in a mempool and are referenced identically as TreeNode*,
// no matter what derived class they are. So when free the mempool, we call the destructor
// of each node through a TreeNode pointer. In this way, a virtual destructor of TreeNode
// if needed in order to invoke the derived class destructor.

class TreeNode {
public:
  NodeKind mKind;
public:
  TreeNode() {mKind = NK_Null;}
  virtual ~TreeNode() {}

  bool IsIdentifier() {return mKind == NK_Identifier;}
  bool IsLiteral()    {return mKind == NK_Literal;}
  bool IsOperator()   {return mKind == NK_Operator;}
  bool IsConstruct()  {return mKind == NK_Construct;}
  bool IsFunction()   {return mKind == NK_Function;}
  void Dump();
};

// OperatorNode-s are coming from operator token.
class OperatorNode : public TreeNode {
public:
  OprId mOprId;
public:
  OperatorNode(OprId id) : mOprId(id) {mKind = NK_Operator;}
  ~OperatorNode() {}

  void Dump();
};

class TreeSymbol;

class FunctionNode : public TreeNode {
public:
  TreeType mRetType;
  std::vector<TreeSymbol*> mParams;
public:
  void Dump();
};

class IdentifierNode : public TreeNode {
public:
  const char *mName; // In the lexer's StringPool
public:
  IdentifierNode(const char *s) : mName(s) {mKind = NK_Identifier;}
  ~IdentifierNode(){}

  void Dump();
};

class LiteralNode : public TreeNode {
public:
  LitData mData;
public:
  LiteralNode(LitData d) : mData(d) {mKind = NK_Literal;}
  ~LiteralNode(){}

  void Dump();
};

////////////////////////////////////////////////////////////////////////////
//                  The AST Builder
// ASTBuilder takes the action Id and parameter list, to create a sub tree.
// Its main body contains huge amount of building functions.
//
// The treenode mempool is part of ASTTree since the memory goes with the
// tree not the builder. The Builder is purely a collect of functions to build
// sub trees.
////////////////////////////////////////////////////////////////////////////

class ASTBuilder {
public:
  unsigned                mActionId;
  std::vector<TreeNode *> mParams;
  TreePool               *mTreePool;
public:
  TreeNode* Build();
  void AddParam(TreeNode *n) {mParams.push_back(n);}

  TreeNode* BuildBinaryOperator(TreeNode *op1, TreeNode *opr, TreeNode *op2);
};

////////////////////////////////////////////////////////////////////////////
//                  The AST Tree
////////////////////////////////////////////////////////////////////////////

class AppealNode;

class ASTTree {
public:
  TreePool   mTreePool;
  TreeNode  *mRootNode;
  ASTBuilder mBuilder;
public:
  ASTTree() {mRootNode = NULL; mBuilder.mTreePool = &mTreePool;}
  ~ASTTree(){}

  TreeNode* NewTokenTreeNode(const AppealNode *);
  TreeNode* NewActionTreeNode(const AppealNode *, std::map<AppealNode*, TreeNode*> &);
};

#endif
