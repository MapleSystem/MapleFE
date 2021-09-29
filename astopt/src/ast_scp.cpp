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
#include "typetable.h"

namespace maplefe {

void AST_SCP::ScopeAnalysis() {
  MSGNOLOC0("============== ScopeAnalysis ==============");
  BuildScope();
  RenameVar();
  AdjustASTWithScope();
}

void AST_SCP::BuildScope() {
  MSGNOLOC0("============== BuildScope ==============");
  BuildScopeVisitor visitor(mHandler, mFlags, true);
  while(!visitor.mScopeStack.empty()) {
    visitor.mScopeStack.pop();
  }
  ModuleNode *module = mHandler->GetASTModule();
  ASTScope *scope = module->GetRootScope();
  visitor.mScopeStack.push(scope);
  visitor.mUserScopeStack.push(scope);

  visitor.InitInternalTypes();

  module->SetScope(scope);
  visitor.Visit(module);

  if (mFlags & FLG_trace_3) {
    mHandler->GetTypeTable()->Dump();
  }
}

void BuildScopeVisitor::InitInternalTypes() {
  // add prim and builtin typesto root scope
  ModuleNode *module = mHandler->GetASTModule();
  ASTScope *scope = module->GetRootScope();
  for (unsigned i = 0; i <= (unsigned)TY_Max; i++) {
    TreeNode *node = mHandler->GetTypeTable()->GetTypeFromTypeIdx(i);
    node->SetScope(scope);
    if (node->IsUserType()) {
      static_cast<UserTypeNode *>(node)->GetId()->SetScope(scope);
    }
    AddTypeAndDecl(scope, node);
  }

  // add dummpy console.log()
  ClassNode *console = AddClass("console");
  FunctionNode *log = AddFunction("log");
  console->AddMethod(log);
}

ClassNode *BuildScopeVisitor::AddClass(std::string name, unsigned tyidx) {
  ClassNode *node = (ClassNode*)gTreePool.NewTreeNode(sizeof(ClassNode));
  new (node) ClassNode();
  unsigned idx = gStringPool.GetStrIdx(name);
  node->SetStrIdx(idx);
  node->SetTypeIdx(tyidx);

  ModuleNode *module = mHandler->GetASTModule();
  ASTScope *scope = module->GetRootScope();
  AddTypeAndDecl(scope, node);
  return node;
}

FunctionNode *BuildScopeVisitor::AddFunction(std::string name) {
  FunctionNode *func = (FunctionNode*)gTreePool.NewTreeNode(sizeof(FunctionNode));
  new (func) FunctionNode();
  unsigned idx = gStringPool.GetStrIdx(name);
  func->SetStrIdx(idx);

  IdentifierNode *id = (IdentifierNode*)gTreePool.NewTreeNode(sizeof(IdentifierNode));
  new (id) IdentifierNode(idx);
  func->SetFuncName(id);

  // add console and func to root scope
  ModuleNode *module = mHandler->GetASTModule();
  ASTScope *scope = module->GetRootScope();
  scope->AddDecl(func);
  id->SetScope(scope);
  return func;
}

void BuildScopeVisitor::AddType(ASTScope *scope, TreeNode *node) {
  scope->AddType(node);
  mHandler->GetTypeTable()->AddType(node);
}

void BuildScopeVisitor::AddTypeAndDecl(ASTScope *scope, TreeNode *node) {
  scope->AddType(node);
  scope->AddDecl(node);
  mHandler->GetTypeTable()->AddType(node);
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
      AddType(scope, it);
    }
  }
  mScopeStack.push(scope);
  mUserScopeStack.push(scope);
  BuildScopeBaseVisitor::VisitFunctionNode(node);
  mScopeStack.pop();
  mUserScopeStack.pop();
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
  mUserScopeStack.push(scope);
  BuildScopeBaseVisitor::VisitLambdaNode(node);
  mScopeStack.pop();
  mUserScopeStack.pop();
  return node;
}

ClassNode *BuildScopeVisitor::VisitClassNode(ClassNode *node) {
  ASTScope *parent = mScopeStack.top();
  // inner class is a decl
  if (parent) {
    parent->AddDecl(node);
    AddType(parent, node);
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
    AddType(parent, node);
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
  BuildScopeBaseVisitor::VisitInterfaceNode(node);
  mScopeStack.pop();
  return node;
}

StructNode *BuildScopeVisitor::VisitStructNode(StructNode *node) {
  ASTScope *parent = mScopeStack.top();
  // struct is a decl
  if (parent) {
    parent->AddDecl(node);
    AddType(parent, node);
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
  ASTScope *scope = NULL;
  if (node->GetProp() == JS_Var) {
    // promote to use function or module scope
    scope = mUserScopeStack.top();

    // update scope
    node->SetScope(scope);
    if (node->GetVar()) {
      node->GetVar()->SetScope(scope);
    }
  } else {
    // use current scope
    scope = mScopeStack.top();
  }
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
      // handled by typealias node
      return node;
    }

    if (p->IsScope()) {
      // normal type decl
      AddType(scope, node);
    }
  }
  return node;
}

TypeAliasNode *BuildScopeVisitor::VisitTypeAliasNode(TypeAliasNode *node) {
  ASTScope *scope = mScopeStack.top();
  BuildScopeBaseVisitor::VisitTypeAliasNode(node);
  scope->AddDecl(node);
  AddType(scope, node);
  return node;
}

ForLoopNode *BuildScopeVisitor::VisitForLoopNode(ForLoopNode *node) {
  ASTScope *parent = mScopeStack.top();
  ASTScope *scope = parent;
  if (node->GetProp() == FLP_JSIn) {
    scope = mASTModule->NewScope(parent, node);
    TreeNode *var = node->GetVariable();
    if (var) {
      if (var->IsDecl()) {
        scope->AddDecl(var);
      } else {
        NOTYETIMPL("VisitForLoopNode() FLP_JSIn var not decl");
      }
    }
    mScopeStack.push(scope);
  }

  BuildScopeBaseVisitor::VisitForLoopNode(node);

  if (scope != parent) {
    mScopeStack.pop();
  }
  return node;
}

// rename var with same name, i --> i__vN where N is 1, 2, 3 ...
void AST_SCP::RenameVar() {
  MSGNOLOC0("============== RenameVar ==============");
  RenameVarVisitor visitor(mHandler, mFlags, true);
  ModuleNode *module = mHandler->GetASTModule();
  visitor.mPass = 0;
  visitor.Visit(module);

  visitor.mPass = 1;
  for (auto it: visitor.mStridx2DeclIdMap) {
    unsigned stridx = it.first;
    unsigned size = it.second.size();
    if (size > 1) {
      const char *name = gStringPool.GetStringFromStrIdx(stridx);

      if (mFlags & FLG_trace_3) {
        std::cout << "\nstridx: " << stridx << " " << name << std::endl;
        std::cout << "decl nid : ";
        for (auto i : visitor.mStridx2DeclIdMap[stridx]) {
          std::cout << " " << i;
        }
        std::cout << std::endl;
      }

      // variable renaming is in reverse order starting from smaller scope variables
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
        if (mFlags & FLG_trace_3) {
          std::cout << "\nupdate name : "
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
        return true;
      case NK_Field: {
        FieldNode *f = static_cast<FieldNode *>(parent);
        // skip for mField
        return node == f->GetField();;
      }
      default:
        return false;
    }
  }
  return true;
}

// check if node is of same name as a parameter of func
bool RenameVarVisitor::IsFuncArg(FunctionNode *func, IdentifierNode *node) {
  for (unsigned i = 0; i < func->GetParamsNum(); i++) {
    if (func->GetParam(i)->GetStrIdx() == node->GetStrIdx()) {
      return true;
    }
  }
  return false;
}

// insert decl in lattice order of scopes hierachy to ensure proper name versions.
//
// the entries of node id in mStridx2DeclIdMap are inserted to a list from larger scopes to smaller scopes
// later variable renaming is performed in reverse from bottom up of AST within the scope of the variable
//
void RenameVarVisitor::InsertToStridx2DeclIdMap(unsigned stridx, IdentifierNode *node) {
  ASTScope *s0 = node->GetScope();
  std::deque<unsigned>::iterator it;
  unsigned i = 0;
  for (it = mStridx2DeclIdMap[stridx].begin(); it != mStridx2DeclIdMap[stridx].end(); ++it) {
    i = *it;
    TreeNode *node1 = mNodeId2NodeMap[i];
    ASTScope *s1 = node1->GetScope();
    // decl at same scope already exist
    if (s1 == s0) {
      return;
    }
    // do not insert node after node1 if node's scope s0 is an ancestor of node1's scope s1
    if (s1->IsAncestor(s0)) {
      mStridx2DeclIdMap[stridx].insert(it, node->GetNodeId());
      return;
    }
  }
  mStridx2DeclIdMap[stridx].push_back(node->GetNodeId());
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
        // insert in order according to scopes hierachy to ensure proper name version
        if ((parent->IsDecl() && parent->GetStrIdx() == stridx)) {
          // decl
          InsertToStridx2DeclIdMap(stridx, node);
        } else if (parent->IsFunction() && IsFuncArg(static_cast<FunctionNode *>(parent), node)) {
          // func parameters
          InsertToStridx2DeclIdMap(stridx, node);
        }
      }
    } else {
      NOTYETIMPL("Unexpected - decl without name stridx");
    }
  } else if (mPass == 1) {
    if (node->GetStrIdx() == mOldStrIdx) {
      node->SetStrIdx(mNewStrIdx);
      MSGNOLOC0("   name updated");
      TreeNode *parent = node->GetParent();
      if (parent && parent->IsDecl()) {
        parent->SetStrIdx(mNewStrIdx);
      }
    }
  }
  return node;
}

void AST_SCP::AdjustASTWithScope() {
  MSGNOLOC0("============== AdjustASTWithScope ==============");
  AdjustASTWithScopeVisitor visitor(mHandler, mFlags, true);
  ModuleNode *module = mHandler->GetASTModule();
  visitor.Visit(module);
}

IdentifierNode *AdjustASTWithScopeVisitor::VisitIdentifierNode(IdentifierNode *node) {
  TreeNode *decl = mHandler->FindDecl(node);
  if (!decl) {
    LitData data;
    bool change = false;
    // handle literals true false
    unsigned stridx = node->GetStrIdx();
    if (stridx == gStringPool.GetStrIdx("true")) {
      data.mType = LT_BooleanLiteral;
      data.mData.mBool = true;
      change = true;
    } else if (stridx == gStringPool.GetStrIdx("false")) {
      data.mType = LT_BooleanLiteral;
      data.mData.mBool = false;
      change = true;
    }

    if (change) {
      LiteralNode *lit = (LiteralNode*)gTreePool.NewTreeNode(sizeof(LiteralNode));
      new (lit) LiteralNode(data);
      return (IdentifierNode*)(lit);
    }
  }
  return node;
}

}
