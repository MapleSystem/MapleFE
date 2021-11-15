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
#include "astopt.h"
#include "ast_cfg.h"
#include "ast_scp.h"
#include "ast_xxport.h"
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

  // add all module themselves to the stridx to scope map
  AST_Handler *asthandler = mHandler->GetASTHandler();
  for (int i = 0; i < asthandler->GetSize(); i++) {
    Module_Handler *handler = asthandler->GetModuleHandler(i);
    ModuleNode *module = handler->GetASTModule();
    ASTScope *scope = module->GetRootScope();
    module->SetScope(scope);
    visitor.AddScopeMap(module->GetStrIdx(), scope);
  }

  ModuleNode *module = mHandler->GetASTModule();
  ASTScope *scope = module->GetRootScope();

  visitor.InitInternalTypes();

  visitor.SetRunIt(true);
  unsigned count = 0;
  // run twice if necessary in case struct's scope is used before set
  while (visitor.GetRunIt() && count  < 2) {
    visitor.SetRunIt(false);
    while(!visitor.mScopeStack.empty()) {
      visitor.mScopeStack.pop();
    }
    visitor.mScopeStack.push(scope);

    while(!visitor.mUserScopeStack.empty()) {
      visitor.mUserScopeStack.pop();
    }
    visitor.mUserScopeStack.push(scope);

    visitor.Visit(module);
    count++;
  }

  if (mFlags & FLG_trace_3) {
    gTypeTable.Dump();
  }
}

void BuildScopeVisitor::AddDecl(ASTScope *scope, TreeNode *node) {
  unsigned sid = scope->GetTree()->GetNodeId();
  if (mScope2DeclsMap[sid].find(node->GetNodeId()) ==
      mScope2DeclsMap[sid].end()) {
    scope->AddDecl(node);
    mScope2DeclsMap[sid].insert(node->GetNodeId());
  }
}

void BuildScopeVisitor::AddImportedDecl(ASTScope *scope, TreeNode *node) {
  unsigned sid = scope->GetTree()->GetNodeId();
  if (mScope2ImportedDeclsMap[sid].find(node->GetNodeId()) ==
      mScope2ImportedDeclsMap[sid].end()) {
    scope->AddImportDecl(node);
    mScope2ImportedDeclsMap[sid].insert(node->GetNodeId());
  }
}

void BuildScopeVisitor::AddType(ASTScope *scope, TreeNode *node) {
  unsigned sid = scope->GetTree()->GetNodeId();
  if (mScope2TypesMap[sid].find(node->GetNodeId()) ==
      mScope2TypesMap[sid].end()) {
    scope->AddType(node);
    gTypeTable.AddType(node);
    mScope2TypesMap[sid].insert(node->GetNodeId());
  }
}

void BuildScopeVisitor::AddTypeAndDecl(ASTScope *scope, TreeNode *node) {
  AddType(scope, node);
  AddDecl(scope, node);
}

void BuildScopeVisitor::InitInternalTypes() {
  // add primitive and builtin types to root scope
  ModuleNode *module = mHandler->GetASTModule();
  ASTScope *scope = module->GetRootScope();
  for (unsigned i = 1; i < gTypeTable.GetPreBuildSize(); i++) {
    TreeNode *node = gTypeTable.GetTypeFromTypeIdx(i);
    node->SetScope(scope);
    if (node->IsUserType()) {
      static_cast<UserTypeNode *>(node)->GetId()->SetScope(scope);
      AddType(scope, node);
      AddDecl(scope, node);
    } else {
      AddType(scope, node);
    }
  }

  // add dummpy console.log()
  ClassNode *console = AddClass("console");
  FunctionNode *log = AddFunction("log");
  log->SetTypeIdx(TY_Void);
  console->AddMethod(log);
}

ClassNode *BuildScopeVisitor::AddClass(std::string name, unsigned tyidx) {
  ClassNode *node = mHandler->NewTreeNode<ClassNode>();
  unsigned idx = gStringPool.GetStrIdx(name);
  node->SetStrIdx(idx);
  node->SetTypeIdx(tyidx);

  ModuleNode *module = mHandler->GetASTModule();
  ASTScope *scope = module->GetRootScope();
  AddTypeAndDecl(scope, node);
  return node;
}

FunctionNode *BuildScopeVisitor::AddFunction(std::string name) {
  FunctionNode *func = mHandler->NewTreeNode<FunctionNode>();
  unsigned idx = gStringPool.GetStrIdx(name);
  func->SetStrIdx(idx);

  IdentifierNode *id = mHandler->NewTreeNode<IdentifierNode>();
  id->SetStrIdx(idx);
  func->SetFuncName(id);

  // add func to module scope
  ModuleNode *module = mHandler->GetASTModule();
  ASTScope *scope = module->GetRootScope();
  AddDecl(scope, func);
  id->SetScope(scope);
  return func;
}

ASTScope *BuildScopeVisitor::NewScope(ASTScope *parent, TreeNode *node) {
  MASSERT(parent && "parent scope NULL");
  ASTScope *scope = node->GetScope();
  if (!scope || (scope && scope->GetTree() != node)) {
    scope = mASTModule->NewScope(parent, node);
  }
  return scope;
}

BlockNode *BuildScopeVisitor::VisitBlockNode(BlockNode *node) {
  ASTScope *parent = mScopeStack.top();
  ASTScope *scope = NewScope(parent, node);
  mScopeStack.push(scope);
  BuildScopeBaseVisitor::VisitBlockNode(node);
  mScopeStack.pop();
  return node;
}

FunctionNode *BuildScopeVisitor::VisitFunctionNode(FunctionNode *node) {
  ASTScope *parent = mScopeStack.top();
  // function is a decl
  AddDecl(parent, node);
  ASTScope *scope = NewScope(parent, node);

  // add parameters as decl
  for(unsigned i = 0; i < node->GetParamsNum(); i++) {
    TreeNode *it = node->GetParam(i);
    AddDecl(scope, it);
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
  mUserScopeStack.pop();
  mScopeStack.pop();
  return node;
}

LambdaNode *BuildScopeVisitor::VisitLambdaNode(LambdaNode *node) {
  ASTScope *parent = mScopeStack.top();
  ASTScope *scope = NewScope(parent, node);

  // add parameters as decl
  for(unsigned i = 0; i < node->GetParamsNum(); i++) {
    TreeNode *it = node->GetParam(i);
    AddDecl(scope, it);
  }
  mScopeStack.push(scope);
  mUserScopeStack.push(scope);
  BuildScopeBaseVisitor::VisitLambdaNode(node);
  mUserScopeStack.pop();
  mScopeStack.pop();
  return node;
}

ClassNode *BuildScopeVisitor::VisitClassNode(ClassNode *node) {
  ASTScope *parent = mScopeStack.top();
  // inner class is a decl
  if (parent) {
    AddDecl(parent, node);
    AddType(parent, node);
  }

  ASTScope *scope = NewScope(parent, node);
  if (node->GetStrIdx()) {
    mStrIdx2ScopeMap[node->GetStrIdx()] = scope;
  }

  // add fields as decl
  for(unsigned i = 0; i < node->GetFieldsNum(); i++) {
    TreeNode *it = node->GetField(i);
    if (it->IsIdentifier()) {
      AddDecl(scope, it);
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
    AddDecl(parent, node);
    AddType(parent, node);
  }

  ASTScope *scope = NewScope(parent, node);
  if (node->GetStrIdx()) {
    mStrIdx2ScopeMap[node->GetStrIdx()] = scope;
  }

  // add fields as decl
  for(unsigned i = 0; i < node->GetFieldsNum(); i++) {
    TreeNode *it = node->GetField(i);
    if (it->IsIdentifier()) {
      AddDecl(scope, it);
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
    AddDecl(parent, node);
    AddType(parent, node);
  }

  ASTScope *scope = NewScope(parent, node);
  if (node->GetStructId() && node->GetStructId()->GetStrIdx()) {
    mStrIdx2ScopeMap[node->GetStructId()->GetStrIdx()] = scope;
  }

  // add fields as decl
  for(unsigned i = 0; i < node->GetFieldsNum(); i++) {
    TreeNode *it = node->GetField(i);
    if (it->IsIdentifier()) {
      AddDecl(scope, it);
    }
  }
  mScopeStack.push(scope);
  BuildScopeBaseVisitor::VisitStructNode(node);
  mScopeStack.pop();
  return node;
}

NamespaceNode *BuildScopeVisitor::VisitNamespaceNode(NamespaceNode *node) {
  node->SetTypeId(TY_Namespace);
  TreeNode *id = node->GetId();
  unsigned stridx = 0;
  if (id && id->GetStrIdx()) {
    stridx = id->GetStrIdx();
    node->SetStrIdx(stridx);
  }

  ASTScope *parent = mScopeStack.top();
  // inner namespace is a decl
  if (parent) {
    AddTypeAndDecl(parent, node);
  }

  ASTScope *scope = NewScope(parent, node);
  if (stridx != 0) {
    mStrIdx2ScopeMap[stridx] = scope;
  }

  mScopeStack.push(scope);
  mUserScopeStack.push(scope);
  BuildScopeBaseVisitor::VisitNamespaceNode(node);
  mUserScopeStack.pop();
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
  AddDecl(scope, node);
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
  AddDecl(scope, node);
  AddType(scope, node);
  return node;
}

ForLoopNode *BuildScopeVisitor::VisitForLoopNode(ForLoopNode *node) {
  ASTScope *parent = mScopeStack.top();
  ASTScope *scope = parent;
  if (node->GetProp() == FLP_JSIn) {
    scope = NewScope(parent, node);
    TreeNode *var = node->GetVariable();
    if (var) {
      if (var->IsDecl()) {
        AddDecl(scope, var);
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

FieldNode *BuildScopeVisitor::VisitFieldNode(FieldNode *node) {
  BuildScopeBaseVisitor::VisitFieldNode(node);

  TreeNode *upper = node->GetUpper();
  TreeNode *field = node->GetField();

  if (upper && upper->GetStrIdx()) {
    ASTScope *scope = mStrIdx2ScopeMap[upper->GetStrIdx()];
    if (!scope && upper->IsIdentifier()) {
      TreeNode *decl = mHandler->FindDecl(static_cast<IdentifierNode *>(upper));
      if (decl && decl->IsIdentifier()) {
        IdentifierNode *id = static_cast<IdentifierNode *>(decl);
        TreeNode *type = id->GetType();
        if (type && type->IsUserType()) {
          TreeNode *id = static_cast<UserTypeNode *>(type)->GetId();
          if (id) {
            scope = mStrIdx2ScopeMap[id->GetStrIdx()];
          }
        } else if (id->GetParent() && id->GetParent()->IsXXportAsPair()) {
          scope = id->GetScope();
        }
      }
    }
    if (scope) {
      mScopeStack.push(scope);
      BuildScopeBaseVisitor::Visit(field);
      mScopeStack.pop();
    } else {
      mRunIt = true;
    }
  }

  return node;
}

ImportNode *BuildScopeVisitor::VisitImportNode(ImportNode *node) {
  (void) AstVisitor::VisitImportNode(node);
  ASTScope *scope = mScopeStack.top();

  Module_Handler *targetHandler = NULL;
  TreeNode *target = node->GetTarget();
  if (target) {
    unsigned hstridx = target->GetStrIdx();
    unsigned hidx = mXXport->GetHandleIdxFromStrIdx(hstridx);
    targetHandler = mHandler->GetASTHandler()->GetModuleHandler(hidx);
  }

  for (unsigned i = 0; i < node->GetPairsNum(); i++) {
    XXportAsPairNode *p = node->GetPair(i);
    TreeNode *bfnode = p->GetBefore();
    TreeNode *afnode = p->GetAfter();
    if (p->IsDefault()) {
      if (bfnode) {
        bfnode->SetScope(scope);
        AddImportedDecl(scope, bfnode);
      }
    } else if (afnode) {
      afnode->SetScope(scope);
      AddImportedDecl(scope, afnode);

      if (bfnode && targetHandler) {
        ModuleNode *mod = targetHandler->GetASTModule();
        ASTScope *modscp = mod->GetScope();
        bfnode->SetScope(modscp);
      }
    }
  }

  return node;
}

ExportNode *BuildScopeVisitor::VisitExportNode(ExportNode *node) {
  (void) AstVisitor::VisitExportNode(node);
  ASTScope *scope = mScopeStack.top();

  Module_Handler *targetHandler = NULL;
  TreeNode *target = node->GetTarget();
  // re-export
  if (target) {
    unsigned hstridx = target->GetStrIdx();
    unsigned hidx = mXXport->GetHandleIdxFromStrIdx(hstridx);
    targetHandler = mHandler->GetASTHandler()->GetModuleHandler(hidx);

    for (unsigned i = 0; i < node->GetPairsNum(); i++) {
      XXportAsPairNode *p = node->GetPair(i);
      TreeNode *bfnode = p->GetBefore();
      TreeNode *afnode = p->GetAfter();
      if (bfnode && targetHandler) {
        ModuleNode *mod = targetHandler->GetASTModule();
        ASTScope *modscp = mod->GetScope();
        bfnode->SetScope(modscp);
      }
    }
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
        TreeNode *tn = mHandler->GetAstOpt()->GetNodeFromNodeId(nid);
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
    TreeNode *node1 = mHandler->GetAstOpt()->GetNodeFromNodeId(i);
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
      LiteralNode *lit = mHandler->NewTreeNode<LiteralNode>();
      lit->SetData(data);
      return (IdentifierNode*)(lit);
    }
  }
  return node;
}

}
