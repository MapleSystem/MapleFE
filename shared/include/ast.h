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

enum NodeKind {
  NK_Token,
  NK_Operator,
  NK_Construct,
  NK_Function,
};

class TreeNode {
public:
  NodeKind mKind;
public:
  void Dump();
};

// OperatorNode-s are coming from operator token.
class OperatorNode : public TreeNode {
public:
  OprId mOprId;
public:
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

class TokenNode : public TreeNode {
};

////////////////////////////////////////////////////////////////////////
//                  The AST Tree
////////////////////////////////////////////////////////////////////////

class ASTTree {
public:
  TreePool  mMemPool;
  TreeNode *mRootNode;
public:
  ASTTree() {mRootNode = NULL;}
  ~ASTTree(){}
};

#endif
