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
#include "typetable.h"
#include "ast_util.h"
#include "ast_info.h"

namespace maplefe {

void AST_INFO::CollectInfo() {
  MSGNOLOC0("============== ASTInfo ==============");
  ModuleNode *module = mHandler->GetASTModule();
  for(unsigned i = 0; i < module->GetTreesNum(); i++) {
    TreeNode *it = module->GetTree(i);
    it->SetParent(module);
  }

  // initialize gTypeTable with builtin types
  gTypeTable.AddPrimAndBuiltinTypes();

  // collect type parameters
  MSGNOLOC0("============== Type Parameter ==============");
  mStrIdxVisitor = new FindStrIdxVisitor(mHandler, mFlags, true);

  MSGNOLOC0("============== Fill node info ==============");
  FillNodeInfoVisitor visitor_node(mHandler, mFlags, true);
  visitor_node.Visit(module);

  // collect class/interface/struct decl
  mPass = 0;
  MSGNOLOC0("============== Collect class/interface/struct ==============");
  ClassStructVisitor visitor(mHandler, mFlags, true);
  visitor.Visit(module);

  // sort fields according to the field name stridx
  // it also include first visit to named types to reduce
  // creation of unnecessary anonymous struct
  mPass = 1;
  MSGNOLOC0("============== super fields and Sort fields ==============");
  visitor.Visit(module);

  // merge class/interface/struct decl first to reduce
  // introducing of unnecessary anonymous structs
  mPass = 2;
  MSGNOLOC0("============== merge class/interface/struct ==============");
  visitor.Visit(module);

  delete mStrIdxVisitor;
}

TypeId AST_INFO::GetTypeId(TreeNode *node) {
  return node->GetTypeId();
}

unsigned AST_INFO::GetFieldsSize(TreeNode *node, bool native) {
  unsigned size = 0;
  unsigned nid = node->GetNodeId();
  if (!native && mStructId2FieldsMap.find(nid) != mStructId2FieldsMap.end()) {
    size = mStructId2FieldsMap[nid].GetNum();
    return size;
  }
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

TreeNode *AST_INFO::GetField(TreeNode *node, unsigned i, bool native) {
  TreeNode *fld = NULL;
  unsigned nid = node->GetNodeId();
  if (!native && mStructId2FieldsMap.find(nid) != mStructId2FieldsMap.end()) {
    fld = mStructId2FieldsMap[nid].ValueAtIndex(i);
    return fld;
  }
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

void AST_INFO::AddField(TreeNode *container, TreeNode *node) {
  unsigned nid = container->GetNodeId();
  if (mStructId2FieldsMap.find(nid) == mStructId2FieldsMap.end()) {
    for (unsigned i = 0; i < GetFieldsSize(container, true); i++) {
      TreeNode *fld = GetField(container, i, true);
      mStructId2FieldsMap[nid].PushBack(fld);
    }
  }
  AddField(nid, node);
}

void AST_INFO::AddField(unsigned nid, TreeNode *node) {
  mStructId2FieldsMap[nid].PushBack(node);
}

// get the filed in nid, including its super, with name stridx
// return NULL if not found
TreeNode *AST_INFO::GetField(unsigned nid, unsigned stridx) {
  if (mStructId2FieldsMap.find(nid) != mStructId2FieldsMap.end()) {
    unsigned size = mStructId2FieldsMap[nid].GetNum();
    for (unsigned i = 0; i < size; i++) {
      TreeNode *fld = mStructId2FieldsMap[nid].ValueAtIndex(i);
      if (fld->GetStrIdx() == stridx) {
        return fld;
      }
    }
  }
  return NULL;
}

unsigned AST_INFO::GetSuperSize(TreeNode *node, unsigned idx) {
  unsigned size1 = 0;
  unsigned size2 = 0;
  switch (node->GetKind()) {
    case NK_Struct: {
      size1 = static_cast<StructNode *>(node)->GetSupersNum();
      size2 = 0;
      break;
    }
    case NK_Class: {
      size1 = static_cast<ClassNode *>(node)->GetSuperClassesNum();
      size2 = static_cast<ClassNode *>(node)->GetSuperInterfacesNum();
      break;
    }
    case NK_Interface: {
      size1 = static_cast<InterfaceNode *>(node)->GetSuperInterfacesNum();
      size2 = 0;
      break;
    }
    default:
      break;
  }
  return (idx == 1) ? size1 : size2;
}

TreeNode *AST_INFO::GetSuper(TreeNode *node, unsigned i, unsigned idx) {
  TreeNode *fld1 = NULL;
  TreeNode *fld2 = NULL;
  switch (node->GetKind()) {
    case NK_Struct:
      fld1 = static_cast<StructNode *>(node)->GetSuper(i);
      fld2 = NULL;
      break;
    case NK_Class: {
      fld1 = (idx == 1) ? static_cast<ClassNode *>(node)->GetSuperClass(i) : NULL;
      fld2 = (idx == 2) ? static_cast<ClassNode *>(node)->GetSuperInterface(i) : NULL;
      break;
    }
    case NK_Interface:
      fld1 = static_cast<InterfaceNode *>(node)->GetSuperInterfaceAtIndex(i);
      fld2 = NULL;
      break;
    default:
      break;
  }
  return (idx == 1) ? fld1 : fld2;
}

bool AST_INFO::IsInterface(TreeNode *node) {
  bool isI = node->IsInterface();
  if (node->IsStruct()) {
    isI = isI || (static_cast<StructNode *>(node)->GetProp() == SProp_TSInterface);
  }
  return isI;
}

bool AST_INFO::IsTypeCompatible(TreeNode *node1, TreeNode *node2) {
  if (node1 == node2) {
    return true;
  }
  // only one is NULL
  if ((!node1 && node2) || (node1 && !node2)) {
    return false;
  }
  // at least one is prim
  if (node1->IsPrimType() || node2->IsPrimType()) {
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

bool AST_INFO::IsTypeIdCompatibleTo(TypeId field, TypeId target) {
  if (target == TY_None) {
    return true;
  }  else if (target == TY_Number) {
    return field == TY_Int || field == TY_Long || field == TY_Float || field == TY_Double;
  } else {
    return field == target;
  }
  return false;
}

// check if "field" is compatible to "target"
bool AST_INFO::IsFieldCompatibleTo(TreeNode *field, TreeNode *target) {
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
    result = IsTypeIdCompatibleTo(tid_field, tid_target);
  }

  return result;
}

TreeNode *AST_INFO::GetCanonicStructNode(TreeNode *node) {
  unsigned size = GetFieldsSize(node);
  bool isI0 = IsInterface(node);

  for (auto s: mFieldNum2StructNodeMap[size]) {
    // found itself
    if (node == s) {
      return s;
    }
    bool isI = IsInterface(s);
    if (!node->IsStructLiteral()) {
      // skip if one is interface but other is not
      if ((isI0 && !isI) || (!isI0 && isI)) {
        continue;
      }
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

  // do not proceed if node contains type parameter
  if (WithTypeParamFast(node)) {
    return node;
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

IdentifierNode *AST_INFO::CreateIdentifierNode(unsigned stridx) {
  IdentifierNode *node = (IdentifierNode*)gTreePool.NewTreeNode(sizeof(IdentifierNode));
  new (node) IdentifierNode(stridx);
  return node;
}

UserTypeNode *AST_INFO::CreateUserTypeNode(unsigned stridx, ASTScope *scope) {
  IdentifierNode *node = CreateIdentifierNode(stridx);
  SetTypeId(node, TY_Class);
  if (scope) {
    node->SetScope(scope);
  }

  UserTypeNode *utype = (UserTypeNode*)gTreePool.NewTreeNode(sizeof(UserTypeNode));
  new (utype) UserTypeNode(node);
  utype->SetStrIdx(stridx);
  SetTypeId(utype, TY_Class);
  node->SetParent(utype);
  return utype;
}

UserTypeNode *AST_INFO::CreateUserTypeNode(IdentifierNode *node) {
  SetTypeId(node, TY_Class);
  UserTypeNode *utype = (UserTypeNode*)gTreePool.NewTreeNode(sizeof(UserTypeNode));
  new (utype) UserTypeNode(node);
  utype->SetStrIdx(node->GetStrIdx());
  SetTypeId(utype, TY_Class);
  node->SetParent(utype);
  return utype;
}

TypeAliasNode *AST_INFO::CreateTypeAliasNode(TreeNode *to, TreeNode *from) {
  UserTypeNode *utype1 = CreateUserTypeNode(from->GetStrIdx());
  UserTypeNode *utype2 = CreateUserTypeNode(to->GetStrIdx());

  TypeAliasNode *alias = (TypeAliasNode*)gTreePool.NewTreeNode(sizeof(TypeAliasNode));
  new (alias) TypeAliasNode();
  alias->SetId(utype1);
  alias->SetStrIdx(from->GetStrIdx());
  alias->SetAlias(utype2);

  return alias;
}

StructNode *AST_INFO::CreateStructFromStructLiteral(StructLiteralNode *node) {
  StructNode *newnode = (StructNode*)gTreePool.NewTreeNode(sizeof(StructNode));
  new (newnode) StructNode(0);
  SetTypeId(newnode, TY_Class);

  for (unsigned i = 0; i < node->GetFieldsNum(); i++) {
    FieldLiteralNode *fl = node->GetField(i);
    TreeNode *name = fl->GetFieldName();
    if (!name || !name->IsIdentifier()) {
      newnode->Release();
      return NULL;
    }

    IdentifierNode *fid = CreateIdentifierNode(name->GetStrIdx());
    newnode->AddField(fid);
    mHandler->AddDirectField(fid);

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

TreeNode *AST_INFO::GetAnonymousStruct(TreeNode *node) {
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

bool AST_INFO::WithStrIdx(TreeNode *node, unsigned stridx) {
  mStrIdxVisitor->Init(stridx);
  mStrIdxVisitor->Visit(node);
  return mStrIdxVisitor->GetFound();
}

bool AST_INFO::WithTypeParam(TreeNode *node) {
  for (auto idx: mTypeParamStrIdxSet) {
    if (WithStrIdx(node, idx)) {
      return true;
    }
  }
  return false;
}

bool AST_INFO::WithTypeParamFast(TreeNode *node) {
  return (mWithTypeParamNodeSet.find(node->GetNodeId()) != mWithTypeParamNodeSet.end());
}

template <typename T1>
void AST_INFO::ExtendFields(T1 *node, TreeNode *sup) {
  if (sup == NULL) {
    // let sup be node itself
    sup = node;
  }
  unsigned nid = node->GetNodeId();
  if (sup && sup->IsUserType()) {
    sup = static_cast<UserTypeNode *>(sup)->GetId();
  }
  if (sup && sup->IsIdentifier()) {
    sup = GetStructFromStrIdx(sup->GetStrIdx());
  }
  if (!sup) {
    return;
  }
  for (unsigned i = 0; i < GetFieldsSize(sup, true); i++) {
    TreeNode *fld = GetField(sup, i, true);
    AddField(nid, fld);
  }
  for (unsigned i = 0; i < GetSuperSize(sup, 1); i++) {
    TreeNode *s = GetSuper(sup, i, 1);
    ExtendFields<T1>(node, s);
  }
  for (unsigned i = 0; i < GetSuperSize(sup, 2); i++) {
    TreeNode *s = GetSuper(sup, i, 2);
    ExtendFields<T1>(node, s);
  }
}

template <typename T1>
static void DumpVec(std::vector<std::pair<unsigned, T1 *>> vec) {
  unsigned size = vec.size();
  std::cout << "================ Dump Vec ================" << std::endl;
  for (unsigned i = 0; i < size; i++) {
    std::cout << "item #" << i
              << " nodeid " << vec[i].second->GetNodeId()
              << " stridx " << vec[i].second->GetStrIdx()
              << std::endl;
  }
}

template <typename T1, typename T2>
void AST_INFO::SortFields(T1 *node) {
  std::vector<std::pair<unsigned, T2 *>> vec;
  unsigned size = GetFieldsSize(node, true);
  if (size) {
    for (unsigned i = 0; i < size; i++) {
      T2 *fld = node->GetField(i);
      unsigned stridx = fld->GetStrIdx();
      std::pair<unsigned, T2*> p(stridx, fld);
      vec.push_back(p);
    }
    std::sort(vec.begin(), vec.end());
    for (unsigned i = 0; i < size; i++) {
      node->SetField(i, vec[i].second);
    }
  }

  unsigned nid = node->GetNodeId();
  if (mStructId2FieldsMap.find(nid) != mStructId2FieldsMap.end()) {
    std::vector<std::pair<unsigned, TreeNode *>> vec;
    unsigned size = mStructId2FieldsMap[nid].GetNum();
    for (unsigned i = 0; i < size; i++) {
      TreeNode *fld = mStructId2FieldsMap[nid].ValueAtIndex(i);
      unsigned stridx = fld->GetStrIdx();
      std::pair<unsigned, TreeNode*> p(stridx, fld);
      vec.push_back(p);
    }
    std::sort(vec.begin(), vec.end());
    for (unsigned i = 0; i < size; i++) {
      *(mStructId2FieldsMap[nid].RefAtIndex(i)) = vec[i].second;
    }
  }
}

void AST_INFO::SetTypeId(TreeNode *node, TypeId tid) {
  mHandler->GetUtil()->SetTypeId(node, tid);
}

void AST_INFO::SetTypeIdx(TreeNode *node, unsigned tidx) {
  mHandler->GetUtil()->SetTypeIdx(node, tidx);
}

IdentifierNode *FillNodeInfoVisitor::VisitIdentifierNode(IdentifierNode *node) {
  (void) AstVisitor::VisitIdentifierNode(node);
  TreeNode *type = node->GetType();
  if (type) {
    mInfo->SetTypeId(node, type->GetTypeId());
    mInfo->SetTypeIdx(node, type->GetTypeIdx());
  }
  return node;
}

FunctionNode *FillNodeInfoVisitor::VisitFunctionNode(FunctionNode *node) {
  (void) AstVisitor::VisitFunctionNode(node);
  TreeNode *type = node->GetType();
  if (type) {
    mInfo->SetTypeId(node, type->GetTypeId());
    mInfo->SetTypeIdx(node, type->GetTypeIdx());
  }
  return node;
}

LiteralNode *FillNodeInfoVisitor::VisitLiteralNode(LiteralNode *node) {
  (void) AstVisitor::VisitLiteralNode(node);
  LitData data = node->GetData();
  LitId id = data.mType;
  switch (id) {
    case LT_IntegerLiteral:
      mInfo->SetTypeId(node, TY_Int);
      mInfo->SetTypeIdx(node, TY_Int);
      break;
    case LT_FPLiteral:
      mInfo->SetTypeId(node, TY_Float);
      mInfo->SetTypeIdx(node, TY_Float);
      break;
    case LT_DoubleLiteral:
      mInfo->SetTypeId(node, TY_Double);
      mInfo->SetTypeIdx(node, TY_Double);
      break;
    case LT_BooleanLiteral:
      mInfo->SetTypeId(node, TY_Boolean);
      mInfo->SetTypeIdx(node, TY_Boolean);
      break;
    case LT_CharacterLiteral:
      mInfo->SetTypeId(node, TY_Char);
      mInfo->SetTypeIdx(node, TY_Char);
      break;
    case LT_StringLiteral:
      mInfo->SetTypeId(node, TY_String);
      mInfo->SetTypeIdx(node, TY_String);
      node->SetStrIdx(data.mData.mStrIdx);
      break;
    case LT_NullLiteral:
      mInfo->SetTypeId(node, TY_Null);
      break;
    case LT_ThisLiteral:
    case LT_SuperLiteral:
      mInfo->SetTypeId(node, TY_Object);
      break;
    case LT_VoidLiteral:
      mInfo->SetTypeId(node, TY_Void);
      break;
    default:
      break;
  }
  return node;
}

PrimTypeNode *FillNodeInfoVisitor::VisitPrimTypeNode(PrimTypeNode *node) {
  (void) AstVisitor::VisitPrimTypeNode(node);
  mInfo->SetTypeIdx(node, node->GetPrimType());
  return node;
}

UserTypeNode *FillNodeInfoVisitor::VisitUserTypeNode(UserTypeNode *node) {
  (void) AstVisitor::VisitUserTypeNode(node);
  TreeNode *id = node->GetId();
  if (id && !id->IsTypeIdNone()) {
    mInfo->SetTypeId(node, id->GetTypeId());
  }
  return node;
}

StructLiteralNode *ClassStructVisitor::VisitStructLiteralNode(StructLiteralNode *node) {
  mInfo->SetTypeId(node, TY_Class);
  (void) AstVisitor::VisitStructLiteralNode(node);
  if (mInfo->GetPass() == 0) {
    // field literal stridx to its ids'
    for (unsigned i = 0; i < node->GetFieldsNum(); i++) {
      FieldLiteralNode *fln = node->GetField(i);
      TreeNode *name = fln->GetFieldName();
      if (name) {
        fln->SetStrIdx(name->GetStrIdx());
      }
    }
  } else if (mInfo->GetPass() == 1) {
    if (mInfo->WithTypeParam(node)) {
      mInfo->InsertWithTypeParamNode(node);
    }
    // sort fields
    mInfo->SortFields<StructLiteralNode, FieldLiteralNode>(node);
  }
  return node;
}

StructNode *ClassStructVisitor::VisitStructNode(StructNode *node) {
  if (node->GetProp() != SProp_TSEnum) {
    mInfo->SetTypeId(node, TY_Class);
    if (node->GetStructId()) {
      mInfo->SetTypeId(node->GetStructId(), TY_Class);
    }
  }
  (void) AstVisitor::VisitStructNode(node);
  if (mInfo->GetPass() == 0) {
    IdentifierNode *id = node->GetStructId();
    if (id && node->GetStrIdx() == 0) {
      node->SetStrIdx(id->GetStrIdx());
    }
    mInfo->SetStrIdx2Struct(node->GetStrIdx(), node);
    for (unsigned i = 0; i < node->GetFieldsNum(); ++i) {
      if (auto t = node->GetField(i)) {
        mHandler->AddDirectField(t);
      }
    }
  } else if (mInfo->GetPass() == 1) {
    if (mInfo->WithTypeParam(node)) {
      mInfo->InsertWithTypeParamNode(node);
    }
    // extends
    mInfo->ExtendFields<StructNode>(node, NULL);
    // sort fields
    mInfo->SortFields<StructNode, TreeNode>(node);
  } else if (mInfo->GetPass() == 2) {
    // skip getting canonical type if not only fields
    if (node->GetMethodsNum() || node->GetStrIdx() == 0) {
      return node;
    }
    mInfo->GetCanonicStructNode(node);
  }
  return node;
}

ClassNode *ClassStructVisitor::VisitClassNode(ClassNode *node) {
  mInfo->SetTypeId(node, TY_Class);
  (void) AstVisitor::VisitClassNode(node);
  if (mInfo->GetPass() == 0) {
    mInfo->SetStrIdx2Struct(node->GetStrIdx(), node);
    for (unsigned i = 0; i < node->GetFieldsNum(); ++i) {
      if (auto t = node->GetField(i)) {
        mHandler->AddDirectField(t);
      }
    }
  } else if (mInfo->GetPass() == 1) {
    if (mInfo->WithTypeParam(node)) {
      mInfo->InsertWithTypeParamNode(node);
    }
    // include super class fields
    mInfo->ExtendFields<ClassNode>(node, NULL);
    // sort fields
    mInfo->SortFields<ClassNode, TreeNode>(node);
  } else if (mInfo->GetPass() == 2) {
    // skip getting canonical type if not only fields
    if (node->GetMethodsNum() || node->GetTypeParamsNum()) {
      return node;
    }
    mInfo->GetCanonicStructNode(node);
  }
  return node;
}

InterfaceNode *ClassStructVisitor::VisitInterfaceNode(InterfaceNode *node) {
  mInfo->SetTypeId(node, TY_Class);
  (void) AstVisitor::VisitInterfaceNode(node);
  if (mInfo->GetPass() == 0) {
    mInfo->SetStrIdx2Struct(node->GetStrIdx(), node);
    for (unsigned i = 0; i < node->GetFieldsNum(); ++i) {
      if (auto t = node->GetField(i)) {
        mHandler->AddDirectField(t);
      }
    }
  } else if (mInfo->GetPass() == 1) {
    if (mInfo->WithTypeParam(node)) {
      mInfo->InsertWithTypeParamNode(node);
    }
    // sort fields
    mInfo->SortFields<InterfaceNode, IdentifierNode>(node);
  } else if (mInfo->GetPass() == 2) {
    // skip getting canonical type if not only fields
    if (node->GetMethodsNum() || node->GetSuperInterfacesNum()) {
      return node;
    }
    mInfo->GetCanonicStructNode(node);
  }
  return node;
}

TypeParameterNode *ClassStructVisitor::VisitTypeParameterNode(TypeParameterNode *node) {
  (void) AstVisitor::VisitTypeParameterNode(node);
  if (mInfo->GetPass() == 0) {
    TreeNode *id = node->GetId();
    if (id) {
      unsigned stridx = id->GetStrIdx();
      mInfo->InsertTypeParamStrIdx(stridx);
      node->SetStrIdx(stridx);
    }
  }
  return node;
}

IdentifierNode *FindStrIdxVisitor::VisitIdentifierNode(IdentifierNode *node) {
  (void) AstVisitor::VisitIdentifierNode(node);
  if (node->GetStrIdx() == mStrIdx) {
    mFound = true;
  }
  return node;
}

}
