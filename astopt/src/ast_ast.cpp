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
#include <algorithm>
#include "ast_handler.h"
#include "ast_ast.h"

namespace maplefe {

void AST_AST::AdjustAST() {
  MSGNOLOC0("============== AdjustAST ==============");
  ModuleNode *module = mHandler->GetASTModule();
  for(unsigned i = 0; i < module->GetTreesNum(); i++) {
    TreeNode *it = module->GetTree(i);
    it->SetParent(module);
  }

  // sort fields according to the field name stridx
  SortFieldsVisitor sort_visitor(mHandler, mFlags, true);
  sort_visitor.Visit(module);

  // adjust ast
  AdjustASTVisitor adjust_visitor(mHandler, mFlags, true);
  adjust_visitor.Visit(module);
}

template <typename T1, typename T2>
static void SortFields(T1 *node) {
  std::vector<std::pair<unsigned, T2 *>> vec;
  for (unsigned i = 0; i < node->GetFieldsNum(); i++) {
    T2 *fld = node->GetField(i);
    unsigned stridx = fld->GetStrIdx();
    std::pair<unsigned, T2*> p(stridx, fld);
    vec.push_back(p);
  }
  std::sort(vec.begin(), vec.end());
  for (unsigned i = 0; i < node->GetFieldsNum(); i++) {
    node->SetField(i, vec[i].second);
  }
}


StructNode *SortFieldsVisitor::VisitStructNode(StructNode *node) {
  (void) AstVisitor::VisitStructNode(node);
  // sort fields
  SortFields<StructNode, TreeNode>(node);
  return node;
}

StructLiteralNode *SortFieldsVisitor::VisitStructLiteralNode(StructLiteralNode *node) {
  (void) AstVisitor::VisitStructLiteralNode(node);
  // sort fields
  SortFields<StructLiteralNode, FieldLiteralNode>(node);
  return node;
}

ClassNode *SortFieldsVisitor::VisitClassNode(ClassNode *node) {
  (void) AstVisitor::VisitClassNode(node);
  // sort fields
  SortFields<ClassNode, TreeNode>(node);
  return node;
}

InterfaceNode *SortFieldsVisitor::VisitInterfaceNode(InterfaceNode *node) {
  (void) AstVisitor::VisitInterfaceNode(node);
  // sort fields
  SortFields<InterfaceNode, IdentifierNode>(node);
  return node;
}

// set parent for some identifier's type
IdentifierNode *AdjustASTVisitor::VisitIdentifierNode(IdentifierNode *node) {
  (void) AstVisitor::VisitIdentifierNode(node);
  // MASSERT(node->GetParent() && "identifier with NULL parent");
  TreeNode *type = node->GetType();
  if (type && type->IsUserType()) {
    type->SetParent(node);
  }
  // rename return() function to return__fixed()
  unsigned stridx = node->GetStrIdx();
  if (stridx == gStringPool.GetStrIdx("return")) {
    unsigned idx = gStringPool.GetStrIdx("return__fixed");
    node->SetStrIdx(idx);
    TreeNode *parent = node->GetParent();
    if (parent && parent->IsFunction()) {
      FunctionNode *func = static_cast<FunctionNode *>(parent);
      MASSERT(func->GetFuncName() == node && "return not function name");
      func->SetStrIdx(idx);
    }
  }
  // rename throw() function to throw__fixed()
  else if (stridx == gStringPool.GetStrIdx("throw")) {
    unsigned idx = gStringPool.GetStrIdx("throw__fixed");
    node->SetStrIdx(idx);
    TreeNode *parent = node->GetParent();
    if (parent && parent->IsFunction()) {
      FunctionNode *func = static_cast<FunctionNode *>(parent);
      MASSERT(func->GetFuncName() == node && "throw not function name");
      func->SetStrIdx(idx);
    }
  }
  return node;
}

unsigned AdjustASTVisitor::GetFieldSize(TreeNode *node) {
  unsigned size = 0;
  switch (node->GetKind()) {
    case NK_Struct:
      size = static_cast<StructNode *>(node)->GetFieldsNum();
      break;
    case NK_Class:
      size = static_cast<ClassNode *>(node)->GetFieldsNum();
      break;
    case NK_Interface:
      size = static_cast<InterfaceNode *>(node)->GetFieldsNum();
      break;
    default:
      break;
  }
  return size;
}

TreeNode *AdjustASTVisitor::GetField(TreeNode *node, unsigned i) {
  TreeNode *fld = NULL;
  switch (node->GetKind()) {
    case NK_Struct:
      fld = static_cast<StructNode *>(node)->GetField(i);
      break;
    case NK_Class:
      fld = static_cast<ClassNode *>(node)->GetField(i);
      break;
    case NK_Interface:
      fld = static_cast<InterfaceNode *>(node)->GetField(i);
      break;
    default:
      break;
  }
  return fld;
}

static bool IsInterface(TreeNode *node) {
  bool isI = node->IsInterface();
  if (node->IsStruct()) {
    isI = isI || (static_cast<StructNode *>(node)->GetProp() == SProp_TSInterface);
  }
  return isI;
}

TreeNode *AdjustASTVisitor::GetCanonicStructNode(TreeNode *node) {
  unsigned size = GetFieldSize(node);
  bool isI0 = IsInterface(node);

  for (auto s: mFieldNum2StructNodeIdMap[size]) {
    bool isI = IsInterface(s);
    // skip if one is interface but other is not
    if ((isI0 && !isI) || (!isI0 && isI)) {
      continue;
    }
    bool match = true;;
    // check fields
    for (unsigned fid = 0; fid < size; fid++) {
      TreeNode *f0 = GetField(node, fid);
      TreeNode *f1 = GetField(s, fid);
      if (f0->IsIdentifier() && f1->IsIdentifier()) {
        IdentifierNode *i0 = static_cast<IdentifierNode *>(f0);
        IdentifierNode *i1 = static_cast<IdentifierNode *>(f1);
        if (i0->GetStrIdx() != i1->GetStrIdx() ||
            i0->GetType() != i1->GetType()) {
          match = false;
          break;
        }
      } else {
        match = false;
        break;
      }
    }

    if (match) {
      return s;
    }
  }

  // not match
  unsigned stridx = node->GetStrIdx();
  TreeNode *parent_orig = node->GetParent();

  // anonymous struct will be added to module scope
  if (stridx == 0) {
    // parent could be modified
    AddAnonymousStruct(node);
  }
  mFieldNum2StructNodeIdMap[size].insert(node);

  // for non root tree node, replace it with a UserType node
  // node: use the node's oritinal parent_orig
  if (!parent_orig || !parent_orig->IsModule()) {
    IdentifierNode *newid = (IdentifierNode*)gTreePool.NewTreeNode(sizeof(IdentifierNode));
    new (newid) IdentifierNode(node->GetStrIdx());

    UserTypeNode *utype = (UserTypeNode*)gTreePool.NewTreeNode(sizeof(UserTypeNode));
    new (utype) UserTypeNode(newid);
    node = utype;
  }
  return node;
}

void AdjustASTVisitor::AddAnonymousStruct(TreeNode *node) {
  std::string str("AnonymousStruct_");
  str += std::to_string(mNum++);
  unsigned stridx = gStringPool.GetStrIdx(str);
  node->SetStrIdx(stridx);

  // for struct node, set structid
  if (node->IsStruct()) {
    StructNode *snode = static_cast<StructNode *>(node);
    IdentifierNode *id = snode->GetStructId();
    if (!id) {
      id = (IdentifierNode*)gTreePool.NewTreeNode(sizeof(IdentifierNode));
      new (id) IdentifierNode(0);
      snode->SetStructId(id);
    }

    // set stridx for structid
    id->SetStrIdx(stridx);
  }

  ModuleNode *module = mHandler->GetASTModule();
  module->AddTreeFront(node);
}

TypeAliasNode *AdjustASTVisitor::CreateTypeAlias(TreeNode *to, TreeNode *from) {
  IdentifierNode *id1 = (IdentifierNode*)gTreePool.NewTreeNode(sizeof(IdentifierNode));
  new (id1) IdentifierNode(from->GetStrIdx());

  UserTypeNode *utype1 = (UserTypeNode*)gTreePool.NewTreeNode(sizeof(UserTypeNode));
  new (utype1) UserTypeNode(id1);

  IdentifierNode *id2 = (IdentifierNode*)gTreePool.NewTreeNode(sizeof(IdentifierNode));
  new (id2) IdentifierNode(to->GetStrIdx());

  UserTypeNode *utype2 = (UserTypeNode*)gTreePool.NewTreeNode(sizeof(UserTypeNode));
  new (utype2) UserTypeNode(id2);

  TypeAliasNode *alias = (TypeAliasNode*)gTreePool.NewTreeNode(sizeof(TypeAliasNode));
  new (alias) TypeAliasNode();
  alias->SetId(utype1);
  alias->SetAlias(utype2);

  return alias;
}

ClassNode *AdjustASTVisitor::VisitClassNode(ClassNode *node) {
  (void) AstVisitor::VisitClassNode(node);
  // skip getting canonical type if not only fields
  if (node->GetMethodsNum() || node->GetSuperClassesNum() || node->GetSuperInterfacesNum() ||
      node->GetSuperClassesNum() || node->GetTypeParametersNum()) {
    return node;
  }

  TreeNode *newnode = GetCanonicStructNode(node);

  // create a TypeAlias for duplicated type if top level
  if (newnode != node && node->GetParent() && node->GetParent()->IsModule()) {
    node = (ClassNode*)CreateTypeAlias(newnode, node);;
  } else {
    node = (ClassNode*)newnode;
  }

  return node;
}

InterfaceNode *AdjustASTVisitor::VisitInterfaceNode(InterfaceNode *node) {
  (void) AstVisitor::VisitInterfaceNode(node);
  // skip getting canonical type if not only fields
  if (node->GetMethodsNum() || node->GetSuperInterfacesNum()) {
    return node;
  }

  TreeNode *newnode = GetCanonicStructNode(node);

  // create a TypeAlias for duplicated type if top level
  if (newnode != node && node->GetParent() && node->GetParent()->IsModule()) {
    node = (InterfaceNode*)CreateTypeAlias(newnode, node);;
  } else {
    node = (InterfaceNode*)newnode;
  }

  return node;
}

StructNode *AdjustASTVisitor::VisitStructNode(StructNode *node) {
  (void) AstVisitor::VisitStructNode(node);
  IdentifierNode *id = node->GetStructId();
  if (id && node->GetStrIdx() == 0) {
    node->SetStrIdx(id->GetStrIdx());
  }

  // skip getting canonical type for TypeAlias
  TreeNode *parent = node->GetParent();
  while (parent) {
    if (parent->IsTypeAlias()) {
      return node;
    }
    parent = parent->GetParent();
  }

  // skip getting canonical type if not only fields
  if (node->GetMethodsNum() || node->GetSupersNum() || node->GetTypeParametersNum()) {
    return node;
  }

  TreeNode *newnode = GetCanonicStructNode(node);

  // create a TypeAlias for duplicated type if top level
  if (newnode != node && node->GetParent() && node->GetParent()->IsModule()) {
    node = (StructNode*)CreateTypeAlias(newnode, node);;
  } else {
    node = (StructNode*)newnode;
  }

  return node;
}

TreeNode *AdjustASTVisitor::CreateTypeNodeFromName(IdentifierNode *node) {
  UserTypeNode *ut = (UserTypeNode*)gTreePool.NewTreeNode(sizeof(UserTypeNode));
  new (ut) UserTypeNode(node);
  return ut;
}

// set UserTypeNode's mStrIdx to be its mId's if exist
UserTypeNode *AdjustASTVisitor::VisitUserTypeNode(UserTypeNode *node) {
  (void) AstVisitor::VisitUserTypeNode(node);
  TreeNode *id = node->GetId();
  if (id && id->IsIdentifier()) {
    node->SetStrIdx(id->GetStrIdx());
  }
  return node;
}

FunctionNode *AdjustASTVisitor::VisitFunctionNode(FunctionNode *node) {
  (void) AstVisitor::VisitFunctionNode(node);
  TreeNode *type = node->GetType();
  if (type && type->IsUserType()) {
    type->SetParent(node);
  }
  // Refine function TypeParames
  for(unsigned i = 0; i < node->GetTypeParamsNum(); i++) {
    type = node->GetTypeParamAtIndex(i);
    if (type->IsIdentifier()) {
      IdentifierNode *inode = static_cast<IdentifierNode *>(type);
      TreeNode *newtype = CreateTypeNodeFromName(inode);
      node->SetTypeParamAtIndex(i, newtype);
      newtype->SetParent(node);
      newtype->SetStrIdx(inode->GetStrIdx());
    }
  }
  return node;
}

// move init from identifier to decl
// copy stridx to decl
DeclNode *AdjustASTVisitor::VisitDeclNode(DeclNode *node) {
  (void) AstVisitor::VisitDeclNode(node);
  TreeNode *var = node->GetVar();
  if (var->IsIdentifier()) {
    IdentifierNode *inode = static_cast<IdentifierNode *>(var);

    // copy stridx from Identifier to Decl
    unsigned stridx = inode->GetStrIdx();
    if (stridx) {
      node->SetStrIdx(stridx);
      mUpdated = true;
    }

    // move init from Identifier to Decl
    TreeNode *init = inode->GetInit();
    if (init) {
      node->SetInit(init);
      inode->ClearInit();
      mUpdated = true;
    }
  } else if (var->IsBindingPattern()) {
    BindingPatternNode *bind = static_cast<BindingPatternNode *>(var);
    VisitBindingPatternNode(bind);
  } else {
    NOTYETIMPL("decl not idenfier or bind pattern");
  }
  return node;
}

// split export decl and body
// export {func add(y)}  ==> export {add}; func add(y)
ExportNode *AdjustASTVisitor::VisitExportNode(ExportNode *node) {
  TreeNode *parent = node->GetParent();
  if (!parent || parent->IsNamespace()) {
    // Export declarations are not permitted in a namespace
    return node;
  }
  if (node->GetPairsNum() == 1) {
    XXportAsPairNode *p = node->GetPair(0);
    TreeNode *bfnode = p->GetBefore();
    TreeNode *after = p->GetAfter();
    if (bfnode) {
      if (!bfnode->IsIdentifier()) {
        switch (bfnode->GetKind()) {
          case NK_Function: {
            FunctionNode *func = static_cast<FunctionNode *>(bfnode);
            IdentifierNode *n = (IdentifierNode*)gTreePool.NewTreeNode(sizeof(IdentifierNode));
            new (n) IdentifierNode(func->GetStrIdx());
            // update p
            p->SetBefore(n);
            n->SetParent(p);
            mUpdated = true;
            // insert func into AST after node
            if (parent->IsBlock()) {
              static_cast<BlockNode *>(parent)->InsertStmtAfter(func, node);
            } else if (parent->IsModule()) {
              static_cast<ModuleNode *>(parent)->InsertAfter(func, node);
            }
            // cp annotation from node to func
            for (unsigned i = 0; i < node->GetAnnotationsNum(); i++) {
              func->AddAnnotation(node->GetAnnotationAtIndex(i));
            }
            node->ClearAnnotation();
            break;
          }
          case NK_Class: {
            ClassNode *classnode = static_cast<ClassNode *>(bfnode);
            IdentifierNode *n = (IdentifierNode*)gTreePool.NewTreeNode(sizeof(IdentifierNode));
            new (n) IdentifierNode(classnode->GetStrIdx());
            // update p
            p->SetBefore(n);
            n->SetParent(p);
            mUpdated = true;
            // insert classnode into AST after node
            if (parent->IsBlock()) {
              static_cast<BlockNode *>(parent)->InsertStmtAfter(classnode, node);
            } else if (parent->IsModule()) {
              static_cast<ModuleNode *>(parent)->InsertAfter(classnode, node);
            }
            // cp annotation from node to classnode
            for (unsigned i = 0; i < node->GetAnnotationsNum(); i++) {
              classnode->AddAnnotation(node->GetAnnotationAtIndex(i));
            }
            node->ClearAnnotation();
            break;
          }
        }
      }
    }
  }
  return node;
}

// if-then-else : use BlockNode for then and else bodies
CondBranchNode *AdjustASTVisitor::VisitCondBranchNode(CondBranchNode *node) {
  TreeNode *tn = VisitTreeNode(node->GetCond());
  tn = VisitTreeNode(node->GetTrueBranch());
  if (tn && !tn->IsBlock()) {
    BlockNode *blk = (BlockNode*)gTreePool.NewTreeNode(sizeof(BlockNode));
    new (blk) BlockNode();
    blk->AddChild(tn);
    node->SetTrueBranch(blk);
    mUpdated = true;
  }
  tn = VisitTreeNode(node->GetFalseBranch());
  if (tn && !tn->IsBlock()) {
    BlockNode *blk = (BlockNode*)gTreePool.NewTreeNode(sizeof(BlockNode));
    new (blk) BlockNode();
    blk->AddChild(tn);
    node->SetFalseBranch(blk);
    mUpdated = true;
  }
  return node;
}

// for : use BlockNode for body
ForLoopNode *AdjustASTVisitor::VisitForLoopNode(ForLoopNode *node) {
  (void) AstVisitor::VisitForLoopNode(node);
  TreeNode *tn = node->GetBody();
  if (tn && !tn->IsBlock()) {
    BlockNode *blk = (BlockNode*)gTreePool.NewTreeNode(sizeof(BlockNode));
    new (blk) BlockNode();
    blk->AddChild(tn);
    node->SetBody(blk);
    mUpdated = true;
  }
  return node;
}

static unsigned uniq_number = 1;

// lamda : create a FunctionNode for it
//         use BlockNode for body, add a ReturnNode
LambdaNode *AdjustASTVisitor::VisitLambdaNode(LambdaNode *node) {
  FunctionNode *func = (FunctionNode*)gTreePool.NewTreeNode(sizeof(FunctionNode));
  new (func) FunctionNode();

  // func name
  std::string str("__lambda_");
  str += std::to_string(uniq_number++);
  str += "__";
  IdentifierNode *name = (IdentifierNode*)gTreePool.NewTreeNode(sizeof(IdentifierNode));
  unsigned stridx = gStringPool.GetStrIdx(str);
  new (name) IdentifierNode(stridx);
  func->SetStrIdx(stridx);
  func->SetFuncName(name);

  // func parameters
  for (int i = 0; i < node->GetParamsNum(); i++) {
    func->AddParam(node->GetParam(i));
  }

  // func body
  TreeNode *tn = VisitTreeNode(node->GetBody());
  if (tn) {
    if (tn->IsBlock()) {
      func->SetBody(static_cast<BlockNode*>(tn));
      func->SetType(node->GetType());
    } else {
      BlockNode *blk = (BlockNode*)gTreePool.NewTreeNode(sizeof(BlockNode));
      new (blk) BlockNode();

      ReturnNode *ret = (ReturnNode*)gTreePool.NewTreeNode(sizeof(ReturnNode));
      new (ret) ReturnNode();
      ret->SetResult(tn);
      blk->AddChild(ret);

      func->SetBody(blk);
    }
  }

  // func return type
  if (node->GetType()) {
    func->SetType(node->GetType());
  }

  mUpdated = true;
  // note: the following conversion is only for the visitor to notice the node is updated
  return (LambdaNode*)func;
}

}
