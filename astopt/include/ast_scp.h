/*
* Copyright (C) [2021] Futurewei Technologies, Inc. All rights reverved.
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

#ifndef __AST_SCP_HEADER__
#define __AST_SCP_HEADER__

#include <stack>
#include <utility>

#include "stringpool.h"
#include "ast_module.h"
#include "ast.h"
#include "ast_type.h"
#include "ast_handler.h"
#include "gen_astvisitor.h"

namespace maplefe {

// DefPosition of Def: <stridx, nodeid>
typedef std::pair<unsigned, unsigned> DefPosition;
// map <bbid, BitVector>
typedef std::unordered_map<unsigned, BitVector*> BVMap;

class AST_SCP {
 private:
  Module_Handler  *mHandler;
  bool          mTrace;

 public:
  explicit AST_SCP(Module_Handler *h, bool t) : mHandler(h), mTrace(t) {}
  ~AST_SCP();

  void BuildScope();
};

class BuildScopeVisitor : public AstVisitor {
 private:
  Module_Handler *mHandler;
  ModuleNode     *mASTModule;
  bool            mTrace;

 public:
  std::stack<ASTScope *> mScopeStack;

 public:
  explicit BuildScopeVisitor(Module_Handler *h, bool t, bool base = false)
    : mHandler(h), mTrace(t), AstVisitor(t && base) {
      mASTModule = mHandler->GetASTModule();
    }
  ~BuildScopeVisitor() = default;

  TreeNode *VisitTreeNode(TreeNode *node);

  // scope nodes
  BlockNode *VisitBlockNode(BlockNode *node);
  FunctionNode *VisitFunctionNode(FunctionNode *node);
  LambdaNode *VisitLambdaNode(LambdaNode *node);
  ClassNode *VisitClassNode(ClassNode *node);
  InterfaceNode *VisitInterfaceNode(InterfaceNode *node);

  // related node with scope : decl, type
  DeclNode *VisitDeclNode(DeclNode *node);
  UserTypeNode *VisitUserTypeNode(UserTypeNode *node);
  TypeAliasNode *VisitTypeAliasNode(TypeAliasNode *node);
};

}
#endif
