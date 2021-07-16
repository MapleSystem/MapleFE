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

  BuildScopeBaseVisitor::VisitBlockNode(node);

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
      decl = scope->FindTypeOf(id->GetStrIdx());
    }
    // add it if not found
    if (!decl) {
      scope->AddType(it);
    }
  }

  mScopeStack.push(scope);

  BuildScopeBaseVisitor::VisitFunctionNode(node);

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
  }

  mScopeStack.push(scope);

  BuildScopeBaseVisitor::VisitLambdaNode(node);

  mScopeStack.pop();
  return node;
}

ClassNode *BuildScopeVisitor::VisitClassNode(ClassNode *node) {
  ASTScope *parent = mScopeStack.top();
  // inner class is a decl
  if (parent) {
    parent->AddDecl(node);
    parent->AddType(node);
  }
  ASTScope *scope = mASTModule->NewScope(parent, node);

  // add fields as decl
  for(unsigned i = 0; i < node->GetFieldsNum(); i++) {
    TreeNode *it = node->GetField(i);
    if (it->IsIdentifier()) {
      scope->AddDecl(it);
    }
  }

  mScopeStack.push(scope);

  BuildScopeBaseVisitor::VisitClassNode(node);

  mScopeStack.pop();
  return node;
}

InterfaceNode *BuildScopeVisitor::VisitInterfaceNode(InterfaceNode *node) {
  ASTScope *parent = mScopeStack.top();
  // inner interface is a decl
  if (parent) {
    parent->AddDecl(node);
    parent->AddType(node);
  }

  ASTScope *scope = mASTModule->NewScope(parent, node);

  // add fields as decl
  for(unsigned i = 0; i < node->GetFieldsNum(); i++) {
    TreeNode *it = node->GetFieldAtIndex(i);
    if (it->IsIdentifier()) {
      scope->AddDecl(it);
    }
  }
  mScopeStack.push(scope);

  BuildScopeBaseVisitor::VisitInterfaceNode(node);

  mScopeStack.pop();
  return node;
}

StructNode *BuildScopeVisitor::VisitStructNode(StructNode *node) {
  ASTScope *parent = mScopeStack.top();
  // struct is a decl
  if (parent) {
    parent->AddDecl(node);
    parent->AddType(node);
  }

  ASTScope *scope = mASTModule->NewScope(parent, node);

  // add fields as decl
  for(unsigned i = 0; i < node->GetFieldsNum(); i++) {
    TreeNode *it = node->GetField(i);
    if (it->IsIdentifier()) {
      scope->AddDecl(it);
    }
  }
  mScopeStack.push(scope);

  BuildScopeBaseVisitor::VisitStructNode(node);

  mScopeStack.pop();
  return node;
}

DeclNode *BuildScopeVisitor::VisitDeclNode(DeclNode *node) {
  BuildScopeBaseVisitor::VisitDeclNode(node);
  ASTScope *scope = mScopeStack.top();
  scope->AddDecl(node);
  return node;
}

UserTypeNode *BuildScopeVisitor::VisitUserTypeNode(UserTypeNode *node) {
  BuildScopeBaseVisitor::VisitUserTypeNode(node);
  ASTScope *scope = mScopeStack.top();
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
        scope->AddType(node);
        return node;
      }
    }

    if (p->IsScope()) {
      // normal type decl
      scope->AddType(node);
    }
  }
  return node;
}

TypeAliasNode *BuildScopeVisitor::VisitTypeAliasNode(TypeAliasNode *node) {
  ASTScope *scope = mScopeStack.top();
  BuildScopeBaseVisitor::VisitTypeAliasNode(node);
  UserTypeNode *ut = node->GetId();
  if (ut) {
    scope->AddType(ut);
  }
  return node;
}

ForLoopNode *BuildScopeVisitor::VisitForLoopNode(ForLoopNode *node) {
  if (node->GetProp() == FLP_JSIn) {
    ASTScope *parent = mScopeStack.top();
    ASTScope *scope = mASTModule->NewScope(parent, node);
    TreeNode *var = node->GetVariable();
    if (var) {
      if (var->IsDecl()) {
        TreeNode *id = (static_cast<DeclNode *>(var))->GetVar();
        scope->AddDecl(id);
      }
    }
  }
  BuildScopeBaseVisitor::VisitForLoopNode(node);
  return node;
}

void AST_SCP::RenameVar() {
  if (mTrace) std::cout << "============== RenameVar ==============" << std::endl;
  RenameVarVisitor visitor(mHandler, mTrace, true);
  ModuleNode *module = mHandler->GetASTModule();
  visitor.mPass = 0;
  visitor.Visit(module);

  visitor.mPass = 1;
  for (auto it: visitor.mStridx2DeclIdMap) {
    unsigned stridx = it.first;
    unsigned size = visitor.mStridx2DeclIdMap[stridx].size();
    if (size > 1) {
      const char *name = gStringPool.GetStringFromStrIdx(stridx);

      if (mTrace) {
        std::cout << "\nstridx: " << stridx << " " << name << std::endl;
        std::cout << "decl nid : ";
        for (auto i : visitor.mStridx2DeclIdMap[stridx]) {
          std::cout << " " << i;
        }
        std::cout << std::endl;
      }

      std::deque<unsigned>::reverse_iterator rit = visitor.mStridx2DeclIdMap[stridx].rbegin();
      size--;
      for (; size && rit!= visitor.mStridx2DeclIdMap[stridx].rend(); --size, ++rit) {
        unsigned nid = *rit;
        std::string str(name);
        str += "__v";
        str += std::to_string(size);
        visitor.mOldStrIdx = stridx;
        visitor.mNewStrIdx = gStringPool.GetStrIdx(str);
        TreeNode *tn = visitor.mNodeId2NodeMap[nid];
        ASTScope *scope = tn->GetScope();
        tn = scope->GetTree();
        if (mTrace) {
          std::cout << "update name : "
                    << gStringPool.GetStringFromStrIdx(visitor.mOldStrIdx)
                    << " --> "
                    << gStringPool.GetStringFromStrIdx(visitor.mNewStrIdx)
                    << std::endl;
        }
        visitor.Visit(tn);
      }
    }
  }
}

// fields are not renamed
bool RenameVarVisitor::SkipRename(IdentifierNode *node) {
  TreeNode *parent = node->GetParent();
  if (parent) {
    switch (parent->GetKind()) {
      case NK_Struct:
      case NK_Class:
      case NK_Interface:
      case NK_Field:
        return true;
      default:
        return false;
    }
  }
  return true;
}

IdentifierNode *RenameVarVisitor::VisitIdentifierNode(IdentifierNode *node) {
  AstVisitor::VisitIdentifierNode(node);
  // fields are not renamed
  if (SkipRename(node)) {
    return node;
  }
  if (mPass == 0) {
    unsigned id = node->GetNodeId();
    unsigned stridx = node->GetStrIdx();
    mNodeId2NodeMap[id] = node;
    if (stridx) {
      TreeNode *parent = node->GetParent();
      if (parent) {
        if (parent->IsDecl() || parent->IsFunction()) {
          mStridx2DeclIdMap[stridx].push_back(id);
        }
      }
    } else {
      if (mTrace) std::cout << "Unexpected - decl without name stridx" << std::endl;
    }
  } else {
    if (node->GetStrIdx() == mOldStrIdx) {
      node->SetStrIdx(mNewStrIdx);
      if (mTrace) std::cout << "   name updated" << std::endl;
      TreeNode *parent = node->GetParent();
      if (parent && parent->IsDecl()) {
        parent->SetStrIdx(mNewStrIdx);
      }
    }
  }
  return node;
}

}
