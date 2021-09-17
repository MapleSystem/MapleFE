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
  // it also include first visit to named types to reduce
  // creation of unnecessary anonymous struct
  SortFieldsVisitor sort_visitor(mHandler, mFlags, true);
  sort_visitor.Visit(module);

  // collect class/interface/struct decl first to reduce
  // introducing of unnecessary anonymous structs
  CollectClassStructVisitor base_visitor(mHandler, mFlags, true);
  base_visitor.Visit(module);

  // adjust ast
  AdjustASTVisitor adjust_visitor(mHandler, mFlags, true);
  adjust_visitor.Visit(module);
}

TypeId AST_AST::GetTypeId(TreeNode *node) {
  TypeId tid = TY_None;
  switch (node->GetKind()) {
    case NK_Literal: {
      LiteralNode *lit = static_cast<LiteralNode *>(node);
      LitData data = lit->GetData();
      LitId id = data.mType;
      switch (id) {
        case LT_IntegerLiteral:
        case LT_FPLiteral:
        case LT_DoubleLiteral:
          tid = TY_Number;
          break;
        case LT_BooleanLiteral:
          tid = TY_Boolean;
          break;
        case LT_CharacterLiteral:
        case LT_StringLiteral:
          tid = TY_String;
          break;
        case LT_NullLiteral:
          tid = TY_Null;
          break;
        case LT_ThisLiteral:
        case LT_SuperLiteral:
          tid = TY_Symbol;
          break;
        case LT_VoidLiteral:
          tid = TY_Void;
          break;
        default:
          break;
      }
      break;
    }
    case NK_PrimType: {
      PrimTypeNode *ptn = static_cast<PrimTypeNode *>(node);
      tid = ptn->GetPrimType();
      break;
    }
    case NK_UserType: {
      UserTypeNode *utn = static_cast<UserTypeNode *>(node);
      TreeNode *id = utn->GetId();
      if (id->GetTypeId() != TY_None) {
        tid = id->GetTypeId();
        break;
      }
      unsigned idx = gStringPool.GetStrIdx("String");
      if (id->GetStrIdx() == idx) {
        tid = TY_String;
        id->SetStrIdx(idx);
      }
      break;
    }
    default:
      break;
  }
  return tid;
}

unsigned AST_AST::GetFieldSize(TreeNode *node) {
  unsigned size = 0;
  switch (node->GetKind()) {
    case NK_StructLiteral:
      size = static_cast<StructLiteralNode *>(node)->GetFieldsNum();
      break;
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

TreeNode *AST_AST::GetField(TreeNode *node, unsigned i) {
  TreeNode *fld = NULL;
  switch (node->GetKind()) {
    case NK_StructLiteral:
      fld = static_cast<StructLiteralNode *>(node)->GetField(i);
      break;
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

bool AST_AST::IsInterface(TreeNode *node) {
  bool isI = node->IsInterface();
  if (node->IsStruct()) {
    isI = isI || (static_cast<StructNode *>(node)->GetProp() == SProp_TSInterface);
  }
  return isI;
}

bool AST_AST::IsTypeCompatible(TreeNode *node1, TreeNode *node2) {
  if (node1 == node2) {
    return true;
  }
  // only one is NULL
  if ((!node1 && node2) || (node1 && !node2)) {
    return false;
  }
  // at least one is prim
  if (node1->GetKind() == NK_PrimType || node2->GetKind() == NK_PrimType) {
    TypeId tid_field = GetTypeId(node2);
    TypeId tid_target = GetTypeId(node1);
    return (tid_field == tid_target);
  }
  // not same kind
  if (node1->GetKind() != node2->GetKind()) {
    return false;
  }
  bool result = false;
  // same kind
  NodeKind nk = node1->GetKind();
  switch (nk) {
    case NK_UserType: {
      UserTypeNode *ut1 = static_cast<UserTypeNode *>(node1);
      UserTypeNode *ut2 = static_cast<UserTypeNode *>(node2);
      result = IsTypeCompatible(ut1->GetId(), ut2->GetId());
    }
    case NK_PrimType: {
      PrimTypeNode *pt1 = static_cast<PrimTypeNode *>(node1);
      PrimTypeNode *pt2 = static_cast<PrimTypeNode *>(node2);
      result = (pt1->GetPrimType() == pt2->GetPrimType());
    }
    case NK_PrimArrayType: {
      PrimArrayTypeNode *pat1 = static_cast<PrimArrayTypeNode *>(node1);
      PrimArrayTypeNode *pat2 = static_cast<PrimArrayTypeNode *>(node2);
      result = IsTypeCompatible(pat1->GetPrim(), pat2->GetPrim());
      if (!result) {
        break;
      }
      result = IsTypeCompatible(pat1->GetDims(), pat2->GetDims());
    }
    case NK_Dimension: {
      DimensionNode *dim1 = static_cast<DimensionNode *>(node1);
      DimensionNode *dim2 = static_cast<DimensionNode *>(node2);
      result = (dim1->GetDimensionsNum() == dim2->GetDimensionsNum());
      if (!result) {
        break;
      }
      for (unsigned i = 0; i < dim1->GetDimensionsNum(); i++) {
        result = result && (dim1->GetDimension(i) == dim2->GetDimension(i));
      }
      break;
    }
    default: {
      break;
    }
  }
  return result;
}

// check if "field" is compatible to "target"
bool AST_AST::IsFieldCompatibleTo(TreeNode *field, TreeNode *target) {
  if (!target->IsIdentifier()) {
    return false;
  }
  unsigned stridx_target = target->GetStrIdx();
  IdentifierNode *id_target = static_cast<IdentifierNode *>(target);

  unsigned stridx_field = 0;
  bool result = false;

  // is identifier
  if (field->IsIdentifier()) {
    stridx_field = field->GetStrIdx();
    if (stridx_field != stridx_target) {
      return false;
    }
    IdentifierNode *id_field = static_cast<IdentifierNode *>(field);
    TreeNode *type_target = id_target->GetType();
    TreeNode *type_field = id_field->GetType();
    result = IsTypeCompatible(type_field, type_target);
  }
  // field literal
  else if (field->IsFieldLiteral()) {
    FieldLiteralNode *fln = static_cast<FieldLiteralNode *>(field);
    TreeNode *name = fln->GetFieldName();
    if (name->IsIdentifier()) {
      stridx_field = name->GetStrIdx();
    }
    if (stridx_field != stridx_target) {
      return false;
    }
    TreeNode *lit = fln->GetLiteral();
    if (!lit->IsLiteral()) {
      return false;
    }
    LiteralNode *ln = static_cast<LiteralNode *>(lit);
    TypeId tid_field = GetTypeId(ln);
    TypeId tid_target = GetTypeId(id_target->GetType());
    result = (tid_field == tid_target);
  }

  return result;
}

TreeNode *AST_AST::GetCanonicStructNode(TreeNode *node) {
  unsigned size = GetFieldSize(node);
  bool isI0 = IsInterface(node);

  for (auto s: mFieldNum2StructNodeMap[size]) {
    // found itself
    if (node == s) {
      return s;
    }
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
      if (!IsFieldCompatibleTo(f0, f1)) {
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

  // node as anonymous struct will be added to module scope
  if (GetNameAnonyStruct() && stridx == 0) {
    TreeNode *anony = GetAnonymousStruct(node);
    if (!anony) {
      return node;
    } else {
      node = anony;
    }
  }
  mFieldNum2StructNodeMap[size].insert(node);

  return node;
}


IdentifierNode *AST_AST::CreateIdentifierNode(unsigned stridx) {
  IdentifierNode *node = (IdentifierNode*)gTreePool.NewTreeNode(sizeof(IdentifierNode));
  new (node) IdentifierNode(stridx);
  return node;
}

UserTypeNode *AST_AST::CreateUserTypeNode(unsigned stridx, ASTScope *scope) {
  IdentifierNode *node = CreateIdentifierNode(stridx);
  if (scope) {
    node->SetScope(scope);
  }

  UserTypeNode *utype = (UserTypeNode*)gTreePool.NewTreeNode(sizeof(UserTypeNode));
  new (utype) UserTypeNode(node);
  utype->SetStrIdx(stridx);
  node->SetParent(utype);
  return utype;
}

UserTypeNode *AST_AST::CreateUserTypeNode(IdentifierNode *node) {
  UserTypeNode *utype = (UserTypeNode*)gTreePool.NewTreeNode(sizeof(UserTypeNode));
  new (utype) UserTypeNode(node);
  utype->SetStrIdx(node->GetStrIdx());
  node->SetParent(utype);
  return utype;
}

TypeAliasNode *AST_AST::CreateTypeAliasNode(TreeNode *to, TreeNode *from) {
  UserTypeNode *utype1 = CreateUserTypeNode(from->GetStrIdx());
  UserTypeNode *utype2 = CreateUserTypeNode(to->GetStrIdx());

  TypeAliasNode *alias = (TypeAliasNode*)gTreePool.NewTreeNode(sizeof(TypeAliasNode));
  new (alias) TypeAliasNode();
  alias->SetId(utype1);
  alias->SetStrIdx(from->GetStrIdx());
  alias->SetAlias(utype2);

  return alias;
}

StructNode *AST_AST::CreateStructFromStructLiteral(StructLiteralNode *node) {
  StructNode *newnode = (StructNode*)gTreePool.NewTreeNode(sizeof(StructNode));
  new (newnode) StructNode(0);

  for (unsigned i = 0; i < node->GetFieldsNum(); i++) {
    FieldLiteralNode *fl = node->GetField(i);
    TreeNode *name = fl->GetFieldName();
    if (!name || !name->IsIdentifier()) {
      newnode->Release();
      return NULL;
    }

    IdentifierNode *fid = CreateIdentifierNode(name->GetStrIdx());
    newnode->AddField(fid);

    TreeNode *lit = fl->GetLiteral();
    if (lit && lit->IsLiteral()) {
      TypeId tid = GetTypeId(lit);
      PrimTypeNode *type = (PrimTypeNode*)gTreePool.NewTreeNode(sizeof(PrimTypeNode));
      new (type) PrimTypeNode();
      type->SetPrimType(tid);
      fid->SetType(type);
    } else {
      NOTYETIMPL("StructLiteralNode literal field kind");
    }
  }
  return newnode;
}

TreeNode *AST_AST::GetAnonymousStruct(TreeNode *node) {
  std::string str("AnonymousStruct_");
  str += std::to_string(mNum++);
  unsigned stridx = gStringPool.GetStrIdx(str);
  TreeNode *newnode = node;
  if (newnode->IsStructLiteral()) {
    StructLiteralNode *sl = static_cast<StructLiteralNode*>(node);
    newnode = CreateStructFromStructLiteral(sl);
    if (!newnode) {
      return NULL;
    }
  }
  newnode->SetStrIdx(stridx);

  // for struct node, set structid
  if (newnode->IsStruct()) {
    StructNode *snode = static_cast<StructNode *>(newnode);
    IdentifierNode *id = snode->GetStructId();
    if (!id) {
      id = CreateIdentifierNode(0);
      snode->SetStructId(id);
    }

    // set stridx for structid
    id->SetStrIdx(stridx);
  }

  ModuleNode *module = mHandler->GetASTModule();
  module->AddTreeFront(newnode);
  return newnode;
}

StructNode *CollectClassStructVisitor::VisitStructNode(StructNode *node) {
  // skip getting canonical type if not only fields
  if (node->GetMethodsNum() || node->GetSupersNum() || node->GetStrIdx() == 0) {
    return node;
  }

  mAst->GetCanonicStructNode(node);
  return node;
}

ClassNode *CollectClassStructVisitor::VisitClassNode(ClassNode *node) {
  // skip getting canonical type if not only fields
  if (node->GetMethodsNum() || node->GetSuperClassesNum() || node->GetSuperInterfacesNum() ||
      node->GetSuperClassesNum() || node->GetTypeParametersNum()) {
    return node;
  }

  mAst->GetCanonicStructNode(node);
  return node;
}

InterfaceNode *CollectClassStructVisitor::VisitInterfaceNode(InterfaceNode *node) {
  // skip getting canonical type if not only fields
  if (node->GetMethodsNum() || node->GetSuperInterfacesNum()) {
    return node;
  }

  mAst->GetCanonicStructNode(node);
  return node;
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

ClassNode *AdjustASTVisitor::VisitClassNode(ClassNode *node) {
  (void) AstVisitor::VisitClassNode(node);
  // skip getting canonical type if not only fields
  if (node->GetMethodsNum() || node->GetSuperClassesNum() || node->GetSuperInterfacesNum() ||
      node->GetSuperClassesNum() || node->GetTypeParametersNum()) {
    return node;
  }

  TreeNode *parent = node->GetParent();
  TreeNode *newnode = mAst->GetCanonicStructNode(node);

  // create a TypeAlias for duplicated type if top level
  if (newnode != node && parent && parent->IsModule()) {
    newnode = (ClassNode*)(mAst->CreateTypeAliasNode(newnode, node));
  }

  return (ClassNode*)newnode;
}

InterfaceNode *AdjustASTVisitor::VisitInterfaceNode(InterfaceNode *node) {
  (void) AstVisitor::VisitInterfaceNode(node);
  // skip getting canonical type if not only fields
  if (node->GetMethodsNum() || node->GetSuperInterfacesNum()) {
    return node;
  }

  TreeNode *parent = node->GetParent();
  TreeNode *newnode = mAst->GetCanonicStructNode(node);

  // create a TypeAlias for duplicated type if top level
  if (newnode != node && parent && parent->IsModule()) {
    newnode = (InterfaceNode*)(mAst->CreateTypeAliasNode(newnode, node));
  }
  return (InterfaceNode*)newnode;
}

StructLiteralNode *AdjustASTVisitor::VisitStructLiteralNode(StructLiteralNode *node) {
  (void) AstVisitor::VisitStructLiteralNode(node);

  unsigned size = node->GetFieldsNum();
  for (unsigned fid = 0; fid < size; fid++) {
    FieldLiteralNode *field = node->GetField(fid);
    TreeNode *name = field->GetFieldName();
    TreeNode *lit = field->GetLiteral();
    if (!name || !lit || lit->GetKind() != NK_Literal) {
      return node;
    }
  }

  TreeNode *parent = node->GetParent();

  // create anonymous struct for structliteral in decl init
  if (parent && parent->IsDecl()) {
    DeclNode *decl = static_cast<DeclNode *>(parent);
    TreeNode *var = decl->GetVar();
    if (var->IsIdentifier()) {
      IdentifierNode *id = static_cast<IdentifierNode *>(var);
      if (!id->GetType()) {
        TreeNode *newnode = mAst->GetCanonicStructNode(node);
        if (newnode && newnode != node) {
          UserTypeNode *utype = mAst->CreateUserTypeNode(newnode->GetStrIdx());
          static_cast<IdentifierNode *>(var)->SetType(utype);
        }
      }
    }
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
  TreeNode *parent_orig = node->GetParent();
  TreeNode *p = parent_orig;
  while (p) {
    if (p->IsTypeAlias()) {
      return node;
    }
    p = p->GetParent();
  }

  // skip getting canonical type if not only fields
  if (node->GetMethodsNum() || node->GetSupersNum() || node->GetTypeParametersNum()) {
    return node;
  }

  TreeNode *newnode = mAst->GetCanonicStructNode(node);

  // create a TypeAlias for duplicated type if top level
  // except newly added anonymous type which has updated parent
  TreeNode *parent = node->GetParent();
  if (newnode != node && parent_orig && parent && parent == parent_orig && parent->IsModule()) {
    newnode = mAst->CreateTypeAliasNode(newnode, node);
  }

  // for non top level tree node, replace it with a UserType node
  if (!parent_orig || !parent_orig->IsModule()) {
    newnode = mAst->CreateUserTypeNode(newnode->GetStrIdx());
  }

  return (StructNode*)newnode;
}

// set UserTypeNode's mStrIdx to be its mId's
UserTypeNode *AdjustASTVisitor::VisitUserTypeNode(UserTypeNode *node) {
  (void) AstVisitor::VisitUserTypeNode(node);
  TreeNode *id = node->GetId();
  if (id) {
    node->SetStrIdx(id->GetStrIdx());
  }
  return node;
}

// set TypeAliasNode's mStrIdx to be its mId's
TypeAliasNode *AdjustASTVisitor::VisitTypeAliasNode(TypeAliasNode *node) {
  (void) AstVisitor::VisitTypeAliasNode(node);
  UserTypeNode *id = node->GetId();
  if (id && id->GetId()) {
    node->SetStrIdx(id->GetId()->GetStrIdx());
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
      TreeNode *newtype = mAst->CreateUserTypeNode(inode);
      node->SetTypeParamAtIndex(i, newtype);
      newtype->SetParent(node);
    }
  }
  return node;
}

// move init from identifier to decl
// copy stridx to decl
DeclNode *AdjustASTVisitor::VisitDeclNode(DeclNode *node) {
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

  // reorg before processing init
  (void) AstVisitor::VisitDeclNode(node);
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
            IdentifierNode *n = mAst->CreateIdentifierNode(func->GetStrIdx());
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
            IdentifierNode *n = mAst->CreateIdentifierNode(classnode->GetStrIdx());
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
  unsigned stridx = gStringPool.GetStrIdx(str);
  IdentifierNode *name = mAst->CreateIdentifierNode(stridx);
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
