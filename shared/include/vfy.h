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

/////////////////////////////////////////////////////////////////////////////////
// This is the portal of verifying. We decided to have a standalone verification
// which takes an ASTModule recently generated. At this point we have a complete
// module with AST trees created by ASTBuilder.
//
// The verification is a top-down traversal on the AST trees. 
//
// It carries on more than one jobs.
// Verification : Checks the validity of semanteme
// Updating     : This is an additional work besides verification. The trees at
//                this point are incomplete because a lot of information is missing
//                and some tree nodes are temporary. For example, a variable was
//                give a new IdentifierNode each time it appears. It doesn't point
//                to the one which was declared before. So this type nodes need
//                be updated.
// Locating     : Locate where in source code is wrong.
// Logging      : Record the verification result into a log.
//
// As each language has different semantic spec, most of the functions below
// will be virtual, allowing to be overidden.
/////////////////////////////////////////////////////////////////////////////////

#ifndef __VFY_HEADER__
#define __VFY_HEADER__

#include "ast.h"
#include "ast_attr.h"
#include "ast_type.h"

class ASTScope;
class TreeNode;

class Verifier {
private:
  ASTScope *mCurrScope;

public:
  Verifier();
  ~Verifier();

  void Do();

  virtual void VerifyScope(ASTScope*);
  virtual void VerifyTree(TreeNode*);

  // Verify of each type of node
#undef  NODEKIND
#define NODEKIND(K) virtual void Verify##K(K##Node*);
#include "ast_nk.def"

};

#endif
