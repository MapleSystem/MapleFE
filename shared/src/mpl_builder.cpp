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

namespace maplefe {

void A2M::ProcessNode(TreeNode *tnode) {
  switch (tnode->GetKind()) {
#undef  NODEKIND
#define NODEKIND(K) case NK_##K: Process##K(tnode); break;
#include "ast_nk.def"
    default:
      break;
  }
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
  MIRType *ptype = mNodeTypeMap[parent->GetName()];
  MIRStructType *stype = static_cast<MIRStructType *>(ptype);
  MASSERT(stype && "struct type not valid");

  IdentifierNode *inode = static_cast<IdentifierNode *>(tnode);
  const char    *name = inode->GetName();
  TreeNode      *type = inode->GetType(); // PrimTypeNode or UserTypeNode
  TreeNode      *init = inode->GetInit(); // Init value

  GenericAttrs genAttrs;
  MapAttr(genAttrs, inode);
  
  GStrIdx stridx = GlobalTables::GetStrTable().GetOrCreateStrIdxFromName(name);
  MIRType *basetype = MapType(type);
  if (basetype) {
    TyidxFieldAttrPair P0(basetype->tyIdx, genAttrs.ConvertToFieldAttrs());
    FieldPair P1(stridx, P0);
    stype->fields.push_back(P1);
  }

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
  // NOTYETIMPL("ProcessFunction()");
  MASSERT(tnode->IsFunction() && "it is not an FunctionNode");
  FunctionNode *ast_func = static_cast<FunctionNode *>(tnode);
  const char                  *ast_name = ast_func->GetName();
  // SmallVector<AttrId>          mAttrs;
  // SmallVector<AnnotationNode*> mAnnotations; //annotation or pragma
  // SmallVector<ExceptionNode*>  mThrows;      // exceptions it can throw
  TreeNode                    *ast_rettype = ast_func->GetType();        // return type
  // SmallVector<TreeNode*>       mParams;      //
  BlockNode                   *ast_body = ast_func->GetBody();
  // DimensionNode               *mDims;
  // bool                         mIsConstructor;

  MIRType *rettype = MapType(ast_rettype);
  TyIdx tyidx = rettype ? rettype->GetTypeIndex() : TyIdx(0);
  MIRFunction *func = mMirBuilder->GetOrCreateFunction(ast_name, tyidx);

  // init function fields
  func->body = func->codeMemPool->New<maple::BlockNode>();

  func->symTab = func->dataMemPool->New<maple::MIRSymbolTable>(&func->dataMPAllocator);
  func->pregTab = func->dataMemPool->New<maple::MIRPregTable>(&func->dataMPAllocator);
  func->typeNameTab = func->dataMemPool->New<maple::MIRTypeNameTable>(&func->dataMPAllocator);
  func->labelTab = func->dataMemPool->New<maple::MIRLabelTable>(&func->dataMPAllocator);

  // process function arguments for function type and formal parameters
  std::vector<TyIdx> funcvectype;
  std::vector<TypeAttrs> funcvecattr;
  for (int i = 0; i < ast_func->GetParamsNum(); i++) {
    TreeNode *param = ast_func->GetParam(i);
    MIRType *type = mDefaultType;
    if (param->IsIdentifier()) {
      IdentifierNode *inode = static_cast<IdentifierNode *>(param);
      type = MapType(inode->GetType());
    } else {
      NOTYETIMPL("ProcessFunction arg type()");
    }

    GStrIdx stridx = GlobalTables::GetStrTable().GetOrCreateStrIdxFromName(param->GetName());
    FormalDef formalDef(stridx, nullptr, type->tyIdx, TypeAttrs());
    func->formalDefVec.push_back(formalDef);
    funcvectype.push_back(type->tyIdx);
    funcvecattr.push_back(TypeAttrs());
  }

  // create function type
  MIRFuncType *functype = GlobalTables::GetTypeTable().GetOrCreateFunctionType(mMirModule, rettype->tyIdx, funcvectype, funcvecattr, /*isvarg*/ false, true);
  func->funcType = functype;

  // update function symbol's type
  MIRSymbol *funcst = GlobalTables::GetGsymTable().GetSymbolFromStIdx(func->stIdx.Idx());
  funcst->SetTyIdx(functype->tyIdx);

  // set up function body
  if (ast_body) {
    // update mBlockNodeMap
    mBlockNodeMap[ast_body] = func->body;

    for (int i = 0; i < ast_body->GetChildrenNum(); i++) {
      TreeNode *child = ast_body->GetChildAtIndex(i);
      ProcessNode(child);
    }
  }

  // insert into class/interface methods list
  TreeNode *parent = tnode->GetParent();
  if (parent->IsClass() || parent->IsInterface()) {
    MIRType *ptype = mNodeTypeMap[parent->GetName()];
    MIRStructType *stype = static_cast<MIRStructType *>(ptype);
    MASSERT(stype && "struct type not valid");

    StIdx stidx = func->stIdx;
    MIRSymbol *fn_st = GlobalTables::GetGsymTable().GetSymbolFromStIdx(stidx.Idx());
    FuncAttrs funcattrs(func->GetAttrs());
    TyidxFuncAttrPair P0(fn_st->tyIdx, funcattrs);
    MethodPair P1(stidx, P0);
    stype->methods.push_back(P1);
  }

  return;
}

void A2M::ProcessClass(TreeNode *tnode) {
  ClassNode *classnode = static_cast<ClassNode *>(tnode);
  const char *name = classnode->GetName();
  MIRType *type = GlobalTables::GetTypeTable().GetOrCreateClassType(name, mMirModule);
  mNodeTypeMap[name] = type;

  for (int i=0; i < classnode->GetMethodsNum(); i++) {
    ProcessFunction(classnode->GetMethod(i));
  }

  for (int i=0; i < classnode->GetFieldsNum(); i++) {
    ProcessField(classnode->GetField(i));
  }

  // set kind to kTypeClass from kTypeClassIncomplete
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
}

