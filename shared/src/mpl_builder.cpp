/*
* Copyright (CtNode) [2020] Futurewei Technologies, Inc. All rights reverved.
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

#include "ast2mpl.h"

#define NOTYETIMPL(K) { if (mTraceA2m) { MNYI(K); }}

MIRType *A2M::MapType(TreeNode *type) {
  if (mNodeTypeMap.find(type) != mNodeTypeMap.end()) {
    return mNodeTypeMap[type];
  }

  MIRType *mir_type = nullptr;
  if (type->IsPrimType()) {
    PrimTypeNode *ptnode = static_cast<PrimTypeNode *>(type);
    PrimType prim;
    switch (ptnode->GetPrimType()) {
      case TY_Boolean: prim = PTY_u1; break;
      case TY_Byte:    prim = PTY_u8; break;
      case TY_Short:   prim = PTY_i16; break;
      case TY_Int:     prim = PTY_i32; break;
      case TY_Long:    prim = PTY_i64; break;
      case TY_Char:    prim = PTY_i8; break;
      case TY_Float:   prim = PTY_f32; break;
      case TY_Double:  prim = PTY_f64; break;
      case TY_Void:    prim = PTY_void; break;
      case TY_Null:    prim = PTY_void; break;
      default: MASSERT("Unsupported PrimType"); break;
    }
    TyIdx tid(prim);
    mir_type = GlobalTables::GetTypeTable().GetTypeFromTyIdx(tid);
    mNodeTypeMap[type] = mir_type;
  }

  if (type->IsPrimType()) {
    MASSERT("type not set");
  }
  return mir_type;
}

void A2M::MapAttr(GenericAttrs &attr, const IdentifierNode *inode) {
}

void A2M::ProcessPackage(TreeNode *tnode) {
  NOTYETIMPL("ProcessPackage()");
  return;
}

void A2M::ProcessImport(TreeNode *tnode) {
  NOTYETIMPL("ProcessImport()");
  return;
}

void A2M::ProcessIdentifier(TreeNode *tnode) {
  NOTYETIMPL("ProcessIdentifier()");
  return;
}

void A2M::ProcessField(TreeNode *tnode) {
  // NOTYETIMPL("ProcessField()");
  MASSERT(tnode->IsIdentifier() && "field is not an IdentifierNode");
  TreeNode *parent = tnode->GetParent();
  MASSERT((parent->IsClass() || parent->IsInterface()) && "Not class or interface");
  MIRType *ptype = mNodeTypeMap[parent];
  MIRStructType *stype = static_cast<MIRStructType *>(ptype);
  MASSERT(stype && "struct type not valid");

  IdentifierNode *inode = static_cast<IdentifierNode *>(tnode);
  const char    *name = inode->GetName();
  TreeNode      *type = inode->GetType(); // PrimTypeNode or UserTypeNode
  TreeNode      *init = inode->GetInit(); // Init value
  // DimensionNode *mDims
  // unsigned dnum = inode->GetDimsNum();
  // SmallVector<AttrId> mAttrs
  GenericAttrs genAttrs;
  MapAttr(genAttrs, inode);
  //unsigned anum = inode->GetAttrsNum();
  //for (int i = 0; i < anum; i++) {
  //}
  
  GStrIdx stridx = GlobalTables::GetStrTable().GetOrCreateStrIdxFromName(name);
  MIRType *basetype = MapType(type);
  TyidxFieldAttrPair P0(basetype->tyIdx, genAttrs.ConvertToFieldAttrs());
  FieldPair P1(stridx, P0);
  stype->fields.push_back(P1);

  return;
}

void A2M::ProcessDimension(TreeNode *tnode) {
  NOTYETIMPL("ProcessDimension()");
  return;
}

void A2M::ProcessAttr(TreeNode *tnode) {
  NOTYETIMPL("ProcessAttr()");
  return;
}

void A2M::ProcessPrimType(TreeNode *tnode) {
  NOTYETIMPL("ProcessPrimType()");
  return;
}

void A2M::ProcessUserType(TreeNode *tnode) {
  NOTYETIMPL("ProcessUserType()");
  return;
}

void A2M::ProcessCast(TreeNode *tnode) {
  NOTYETIMPL("ProcessCast()");
  return;
}

void A2M::ProcessParenthesis(TreeNode *tnode) {
  NOTYETIMPL("ProcessParenthesis()");
  return;
}

void A2M::ProcessVarList(TreeNode *tnode) {
  NOTYETIMPL("ProcessVarList()");
  return;
}

void A2M::ProcessExprList(TreeNode *tnode) {
  NOTYETIMPL("ProcessExprList()");
  return;
}

void A2M::ProcessLiteral(TreeNode *tnode) {
  NOTYETIMPL("ProcessLiteral()");
  return;
}

void A2M::ProcessUnaOperator(TreeNode *tnode) {
  NOTYETIMPL("ProcessUnaOperator()");
  return;
}

void A2M::ProcessBinOperator(TreeNode *tnode) {
  NOTYETIMPL("ProcessBinOperator()");
  return;
}

void A2M::ProcessTerOperator(TreeNode *tnode) {
  NOTYETIMPL("ProcessTerOperator()");
  return;
}

void A2M::ProcessLambda(TreeNode *tnode) {
  NOTYETIMPL("ProcessLambda()");
  return;
}

void A2M::ProcessBlock(TreeNode *tnode) {
  NOTYETIMPL("ProcessBlock()");
  return;
}

void A2M::ProcessFunction(TreeNode *tnode) {
  NOTYETIMPL("ProcessFunction()");
  return;
}

void A2M::ProcessClass(TreeNode *tnode) {
  ClassNode *classnode = static_cast<ClassNode *>(tnode);
  const char *name = classnode->GetName();
  GStrIdx stridx = GlobalTables::GetStrTable().GetOrCreateStrIdxFromName(name);
  MIRType *type = GlobalTables::GetTypeTable().GetOrCreateClassType(name, mMirModule);
  mNodeTypeMap[tnode] = type;

  for (int i=0; i < classnode->GetMethodsNum(); i++) {
    NOTYETIMPL("ProcessClass() - ProcessFunction");
    ProcessFunction(classnode->GetMethod(i));
  }

  for (int i=0; i < classnode->GetFieldsNum(); i++) {
    ProcessField(classnode->GetField(i));
  }

  type->typeKind = maple::MIRTypeKind::kTypeClass;
  return;
}

void A2M::ProcessInterface(TreeNode *tnode) {
  NOTYETIMPL("ProcessInterface()");
  return;
}

void A2M::ProcessAnnotationType(TreeNode *tnode) {
  NOTYETIMPL("ProcessAnnotationType()");
  return;
}

void A2M::ProcessAnnotation(TreeNode *tnode) {
  NOTYETIMPL("ProcessAnnotation()");
  return;
}

void A2M::ProcessException(TreeNode *tnode) {
  NOTYETIMPL("ProcessException()");
  return;
}

void A2M::ProcessReturn(TreeNode *tnode) {
  NOTYETIMPL("ProcessReturn()");
  return;
}

void A2M::ProcessCondBranch(TreeNode *tnode) {
  NOTYETIMPL("ProcessCondBranch()");
  return;
}

void A2M::ProcessBreak(TreeNode *tnode) {
  NOTYETIMPL("ProcessBreak()");
  return;
}

void A2M::ProcessForLoop(TreeNode *tnode) {
  NOTYETIMPL("ProcessForLoop()");
  return;
}

void A2M::ProcessWhileLoop(TreeNode *tnode) {
  NOTYETIMPL("ProcessWhileLoop()");
  return;
}

void A2M::ProcessDoLoop(TreeNode *tnode) {
  NOTYETIMPL("ProcessDoLoop()");
  return;
}

void A2M::ProcessNew(TreeNode *tnode) {
  NOTYETIMPL("ProcessNew()");
  return;
}

void A2M::ProcessDelete(TreeNode *tnode) {
  NOTYETIMPL("ProcessDelete()");
  return;
}

void A2M::ProcessCall(TreeNode *tnode) {
  NOTYETIMPL("ProcessCall()");
  return;
}

void A2M::ProcessSwitchLabel(TreeNode *tnode) {
  NOTYETIMPL("ProcessSwitchLabel()");
  return;
}

void A2M::ProcessSwitchCase(TreeNode *tnode) {
  NOTYETIMPL("ProcessSwitchCase()");
  return;
}

void A2M::ProcessSwitch(TreeNode *tnode) {
  NOTYETIMPL("ProcessSwitch()");
  return;
}

void A2M::ProcessPass(TreeNode *tnode) {
  NOTYETIMPL("ProcessPass()");
  return;
}

