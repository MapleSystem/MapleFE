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

#include <stack>
#include <set>
#include <tuple>
#include "stringpool.h"
#include "ast_cfg.h"
#include "ast_scp.h"

namespace maplefe {

void AST_SCP::BuildScope() {
  if (mTrace) std::cout << "============== BuildScope ==============" << std::endl;
  BuildScopeVisitor visitor(mHandler, mTrace, true);
  while(!visitor.mScopeStack.empty()) {
    visitor.mScopeStack.pop();
  }
  ModuleNode *module = mHandler->GetASTModule();
  visitor.mScopeStack.push(module->GetRootScope());
  module->SetScope(module->GetRootScope());

  for(unsigned i = 0; i < module->GetTreesNum(); i++) {
    TreeNode *it = module->GetTree(i);
    visitor.Visit(it);
  }
}

BlockNode *BuildScopeVisitor::VisitBlockNode(BlockNode *node) {
  ASTScope *parent = mScopeStack.top();
  ASTScope *scope = mASTModule->NewScope(parent, node);

  mScopeStack.push(scope);

  AstVisitor::VisitBlockNode(node);

  mScopeStack.pop();
  return node;
}

FunctionNode *BuildScopeVisitor::VisitFunctionNode(FunctionNode *node) {
  ASTScope *parent = mScopeStack.top();
  // function is a decl
  parent->AddDecl(node);
  ASTScope *scope = mASTModule->NewScope(parent, node);

  // add parameters as decl
  for(unsigned i = 0; i < node->GetParamsNum(); i++) {
    TreeNode *it = node->GetParam(i);
    scope->AddDecl(it);
    it->SetScope(scope);
  }

  for(unsigned i = 0; i < node->GetTypeParamsNum(); i++) {
    TreeNode *it = node->GetTypeParamAtIndex(i);

    // add it to scope's mTypes only if it is a new type
    TreeNode *tn = it;
    if (it->IsUserType()) {
      UserTypeNode *ut = static_cast<UserTypeNode *>(it);
      tn = ut->GetId();
    }
    TreeNode *decl = NULL;
    if (tn->IsIdentifier()) {
      IdentifierNode *id = static_cast<IdentifierNode *>(tn);
      // check if it is a known type
      decl = scope->FindTypeOf(id);
    }
    // add it if not found
    if (!decl) {
      scope->AddType(it);
    }
  }

  mScopeStack.push(scope);

  AstVisitor::VisitFunctionNode(node);

  mScopeStack.pop();
  return node;
}

LambdaNode *BuildScopeVisitor::VisitLambdaNode(LambdaNode *node) {
  ASTScope *parent = mScopeStack.top();
  ASTScope *scope = mASTModule->NewScope(parent, node);

  // add parameters as decl
  for(unsigned i = 0; i < node->GetParamsNum(); i++) {
    TreeNode *it = node->GetParam(i);
    scope->AddDecl(it);
    it->SetScope(scope);
  }

  mScopeStack.push(scope);

  AstVisitor::VisitLambdaNode(node);

  mScopeStack.pop();
  return node;
}

ClassNode *BuildScopeVisitor::VisitClassNode(ClassNode *node) {
  ASTScope *parent = mScopeStack.top();
  // inner class is a decl
  if (parent) {
    parent->AddDecl(node);
    parent->AddType(node);
    node->SetScope(parent);
  }
  ASTScope *scope = mASTModule->NewScope(parent, node);

  // add fields as decl
  for(unsigned i = 0; i < node->GetFieldsNum(); i++) {
    TreeNode *it = node->GetField(i);
    if (it->IsIdentifier()) {
      scope->AddDecl(it);
      it->SetScope(scope);
    }
  }

  mScopeStack.push(scope);

  AstVisitor::VisitClassNode(node);

  mScopeStack.pop();
  return node;
}

InterfaceNode *BuildScopeVisitor::VisitInterfaceNode(InterfaceNode *node) {
  ASTScope *parent = mScopeStack.top();
  // inner interface is a decl
  if (parent) {
    parent->AddDecl(node);
    parent->AddType(node);
    node->SetScope(parent);
  }

  ASTScope *scope = mASTModule->NewScope(parent, node);

  // add fields as decl
  for(unsigned i = 0; i < node->GetFieldsNum(); i++) {
    TreeNode *it = node->GetFieldAtIndex(i);
    if (it->IsIdentifier()) {
      scope->AddDecl(it);
      it->SetScope(scope);
    }
  }
  mScopeStack.push(scope);

  AstVisitor::VisitInterfaceNode(node);

  mScopeStack.pop();
  return node;
}

DeclNode *BuildScopeVisitor::VisitDeclNode(DeclNode *node) {
  ASTScope *parent = mScopeStack.top();
  AstVisitor::VisitDeclNode(node);
  parent->AddDecl(node);
  node->SetScope(parent);
  return node;
}

UserTypeNode *BuildScopeVisitor::VisitUserTypeNode(UserTypeNode *node) {
  ASTScope *parent = mScopeStack.top();
  AstVisitor::VisitUserTypeNode(node);
  TreeNode *p = node->GetParent();
  if (p) {
    if (p->IsFunction()) {
      // exclude function return type
      FunctionNode *f = static_cast<FunctionNode *>(p);
      if (f->GetType() == node) {
        return node;
      }
    } else if (p->IsTypeAlias()) {
      // typalias id
      TypeAliasNode *ta = static_cast<TypeAliasNode *>(p);
      if (ta->GetId() == node) {
        parent->AddType(node);
        return node;
      }
    } else if (p->IsScope()) {
      // normal type decl
      parent->AddType(node);
    }
  }
  return node;
}

TypeAliasNode *BuildScopeVisitor::VisitTypeAliasNode(TypeAliasNode *node) {
  ASTScope *parent = mScopeStack.top();
  AstVisitor::VisitTypeAliasNode(node);
  UserTypeNode *ut = node->GetId();
  if (ut) {
    parent->AddType(ut);
  }
  return node;
}

}
