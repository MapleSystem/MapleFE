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
//                to the one which was declared before. So these nodes need
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
#include "ast_module.h"
#include "container.h"
#include "vfy_log.h"

namespace maplefe {

class ASTScope;
class TreeNode;

class Verifier {
protected:
  VfyLog      mLog;
  ModuleNode *mASTModule;

  ASTScope *mCurrScope;

  // I need a temporary place to save the parent node when we
  // handle a child node, since sometimes we need replace the child
  // node with a new one.
  TreeNode *mTempParent;

protected:
  // collect decls and types in the whole scope.
  virtual void CollectAllDeclsTypes(ASTScope*);

  // A list of class node verification.
  virtual void VerifyClassFields(ClassNode*);
  virtual void VerifyClassMethods(ClassNode*);
  virtual void VerifyClassSuperClasses(ClassNode*);
  virtual void VerifyClassSuperInterfaces(ClassNode*);

  virtual void VerifyType(IdentifierNode*);
public:
  Verifier(ModuleNode *m);
  ~Verifier();

  void Do();

  virtual void VerifyGlobalScope();
  virtual void VerifyTree(TreeNode*);

  // Verify of each type of node
#undef  NODEKIND
#define NODEKIND(K) virtual void Verify##K(K##Node*);
#include "ast_nk.def"

};

}
#endif
