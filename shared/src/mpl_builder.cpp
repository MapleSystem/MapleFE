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

maple::BaseNode *A2M::ProcessNode(StmtExprKind skind, TreeNode *tnode, maple::BlockNode *block) {
  maple::BaseNode *mpl_node = nullptr;
  switch (tnode->GetKind()) {
#undef  NODEKIND
#define NODEKIND(K) case NK_##K: mpl_node = Process##K(skind, tnode, block); break;
#include "ast_nk.def"
    default:
      break;
  }
  return mpl_node;
}

maple::BaseNode *A2M::ProcessPackage(StmtExprKind skind, TreeNode *tnode, maple::BlockNode *block) {
  NOTYETIMPL("ProcessPackage()");
  PackageNode *node = static_cast<PackageNode *>(tnode);
  return nullptr;
}

maple::BaseNode *A2M::ProcessImport(StmtExprKind skind, TreeNode *tnode, maple::BlockNode *block) {
  NOTYETIMPL("ProcessImport()");
  ImportNode *node = static_cast<ImportNode *>(tnode);
  return nullptr;
}

maple::BaseNode *A2M::ProcessIdentifier(StmtExprKind skind, TreeNode *tnode, maple::BlockNode *block) {
  IdentifierNode *node = static_cast<IdentifierNode *>(tnode);
  MIRSymbol *symbol = GetSymbol(node, block);
  return mMirBuilder->CreateExprDread(symbol);
}

maple::BaseNode *A2M::ProcessField(StmtExprKind skind, TreeNode *tnode, maple::BlockNode *block) {
  if (skind == SK_Expr) {
    NOTYETIMPL("ProcessField() SK_Expr");
    return nullptr;
  }

  if (!tnode->IsIdentifier()) {
    NOTYETIMPL("ProcessField() not Identifier");
    return nullptr;
  }

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
    TyidxFieldAttrPair P0(basetype->GetTypeIndex(), genAttrs.ConvertToFieldAttrs());
    FieldPair P1(stridx, P0);
    stype->fields.push_back(P1);
  }

  return nullptr;
}

maple::BaseNode *A2M::ProcessDimension(StmtExprKind skind, TreeNode *tnode, maple::BlockNode *block) {
  NOTYETIMPL("ProcessDimension()");
  DimensionNode *node = static_cast<DimensionNode *>(tnode);
  return nullptr;
}

maple::BaseNode *A2M::ProcessAttr(StmtExprKind skind, TreeNode *tnode, maple::BlockNode *block) {
  NOTYETIMPL("ProcessAttr()");
  // AttrNode *node = static_cast<AttrNode *>(tnode);
  return nullptr;
}

maple::BaseNode *A2M::ProcessPrimType(StmtExprKind skind, TreeNode *tnode, maple::BlockNode *block) {
  NOTYETIMPL("ProcessPrimType()");
  PrimTypeNode *node = static_cast<PrimTypeNode *>(tnode);
  return nullptr;
}

maple::BaseNode *A2M::ProcessUserType(StmtExprKind skind, TreeNode *tnode, maple::BlockNode *block) {
  NOTYETIMPL("ProcessUserType()");
  UserTypeNode *node = static_cast<UserTypeNode *>(tnode);
  return nullptr;
}

maple::BaseNode *A2M::ProcessCast(StmtExprKind skind, TreeNode *tnode, maple::BlockNode *block) {
  NOTYETIMPL("ProcessCast()");
  CastNode *node = static_cast<CastNode *>(tnode);
  return nullptr;
}

maple::BaseNode *A2M::ProcessParenthesis(StmtExprKind skind, TreeNode *tnode, maple::BlockNode *block) {
  ParenthesisNode *node = static_cast<ParenthesisNode *>(tnode);
  return ProcessNode(skind, node->GetExpr(), block);
}

maple::BaseNode *A2M::ProcessVarList(StmtExprKind skind, TreeNode *tnode, maple::BlockNode *block) {
  VarListNode *node = static_cast<VarListNode *>(tnode);
  for (int i = 0; i < node->GetNum(); i++) {
    TreeNode *n = node->VarAtIndex(i);
    IdentifierNode *idn = static_cast<IdentifierNode *>(n);
    MIRSymbol *symbol = GetSymbol(idn, block);
  }
  return nullptr;
}

maple::BaseNode *A2M::ProcessExprList(StmtExprKind skind, TreeNode *tnode, maple::BlockNode *block) {
  NOTYETIMPL("ProcessExprList()");
  ExprListNode *node = static_cast<ExprListNode *>(tnode);
  return nullptr;
}

maple::BaseNode *A2M::ProcessLiteral(StmtExprKind skind, TreeNode *tnode, maple::BlockNode *block) {
  NOTYETIMPL("ProcessLiteral()");
  LiteralNode *node = static_cast<LiteralNode *>(tnode);
  return nullptr;
}

maple::BaseNode *A2M::ProcessUnaOperator(StmtExprKind skind, TreeNode *tnode, maple::BlockNode *block) {
  NOTYETIMPL("ProcessUnaOperator()");
  UnaOperatorNode *node = static_cast<UnaOperatorNode *>(tnode);
  return nullptr;
}

maple::BaseNode *A2M::ProcessBinOperator(StmtExprKind skind, TreeNode *tnode, maple::BlockNode *block) {
  BinOperatorNode *bon = static_cast<BinOperatorNode *>(tnode);
  OprId ast_op = bon->mOprId;
  TreeNode *ast_lhs = bon->mOpndA;
  TreeNode *ast_rhs = bon->mOpndB;
  maple::BaseNode *lhs = ProcessNode(SK_Expr, ast_lhs, block);
  maple::BaseNode *rhs = ProcessNode(SK_Expr, ast_rhs, block);
  maple::BaseNode *mpl_node = nullptr;
  maple::Opcode op = maple::kOpUndef;

  op = MapOpcode(ast_op);
  if (op != kOpUndef) {
    mpl_node = ProcessBinOperatorMpl(SK_Expr, op, lhs, rhs, block);
    return mpl_node;
  }

  if (ast_op == OPR_Assign) {
    mpl_node = ProcessBinOperatorMplAssign(SK_Stmt, lhs, rhs, block);
    return mpl_node;
  }

  op = MapComboOpcode(ast_op);
  if (op != kOpUndef) {
    mpl_node = ProcessBinOperatorMplComboAssign(SK_Stmt, op, lhs, rhs, block);
    return mpl_node;
  }

  if (ast_op == OPR_Arrow) {
    mpl_node = ProcessBinOperatorMplArror(SK_Expr, lhs, rhs, block);
    return mpl_node;
  }

  return mpl_node;
}

maple::BaseNode *A2M::ProcessTerOperator(StmtExprKind skind, TreeNode *tnode, maple::BlockNode *block) {
  NOTYETIMPL("ProcessTerOperator()");
  TerOperatorNode *node = static_cast<TerOperatorNode *>(tnode);
  return nullptr;
}

maple::BaseNode *A2M::ProcessLambda(StmtExprKind skind, TreeNode *tnode, maple::BlockNode *block) {
  NOTYETIMPL("ProcessLambda()");
  LambdaNode *node = static_cast<LambdaNode *>(tnode);
  return nullptr;
}

maple::BaseNode *A2M::ProcessBlock(StmtExprKind skind, TreeNode *tnode, maple::BlockNode *block) {
  BlockNode *ast_block = static_cast<BlockNode *>(tnode);
  for (int i = 0; i < ast_block->GetChildrenNum(); i++) {
    TreeNode *child = ast_block->GetChildAtIndex(i);
    BaseNode *stmt = ProcessNode(skind, child, block);
    if (stmt) {
      block->AddStatement(stmt);
    }
  }
  return nullptr;
}

maple::BaseNode *A2M::ProcessFunction(StmtExprKind skind, TreeNode *tnode, maple::BlockNode *block) {
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

  TreeNode *parent = tnode->GetParent();
  MIRStructType *stype = nullptr;
  if (parent->IsClass() || parent->IsInterface()) {
    MIRType *ptype = mNodeTypeMap[parent->GetName()];
    stype = static_cast<MIRStructType *>(ptype);
    MASSERT(stype && "struct type not valid");
  }

  MIRType *rettype = MapType(ast_rettype);
  TyIdx tyidx = rettype ? rettype->GetTypeIndex() : TyIdx(0);
  MIRFunction *func = mMirBuilder->GetOrCreateFunction(ast_name, tyidx);

  mMirModule->AddFunction(func);
  mMirModule->SetCurFunction(func);

  // init function fields
  func->body = func->codeMemPool->New<maple::BlockNode>();
  func->symTab = func->dataMemPool->New<maple::MIRSymbolTable>(&func->dataMPAllocator);
  func->pregTab = func->dataMemPool->New<maple::MIRPregTable>(&func->dataMPAllocator);
  func->typeNameTab = func->dataMemPool->New<maple::MIRTypeNameTable>(&func->dataMPAllocator);
  func->labelTab = func->dataMemPool->New<maple::MIRLabelTable>(&func->dataMPAllocator);

  // set class/interface type
  if (stype) {
    func->SetClassTyIdx(stype->GetTypeIndex());
  }

  // process function arguments for function type and formal parameters
  std::vector<TyIdx> funcvectype;
  std::vector<TypeAttrs> funcvecattr;

  // insert this as first parameter
  if (stype) {
    GStrIdx stridx = GlobalTables::GetStrTable().GetOrCreateStrIdxFromName("this");
    TypeAttrs attr = TypeAttrs();
    MIRType *sptype = GlobalTables::GetTypeTable().GetOrCreatePointerType(stype);
    FormalDef formalDef(stridx, nullptr, sptype->GetTypeIndex(), attr);
    func->formalDefVec.push_back(formalDef);
    funcvectype.push_back(sptype->GetTypeIndex());
    funcvecattr.push_back(attr);
  }

  // process remaining parameters
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
    TypeAttrs attr = TypeAttrs();
    FormalDef formalDef(stridx, nullptr, type->GetTypeIndex(), attr);
    func->formalDefVec.push_back(formalDef);
    funcvectype.push_back(type->GetTypeIndex());
    funcvecattr.push_back(attr);
  }

  // use className|funcName|_argTypes_retType as function name
  UpdateFuncName(func);

  // create function type
  MIRFuncType *functype = GlobalTables::GetTypeTable().GetOrCreateFunctionType(mMirModule, rettype->GetTypeIndex(), funcvectype, funcvecattr, /*isvarg*/ false, true);
  func->funcType = functype;

  // update function symbol's type
  MIRSymbol *funcst = GlobalTables::GetGsymTable().GetSymbolFromStIdx(func->stIdx.Idx());
  funcst->SetTyIdx(functype->GetTypeIndex());

  // set up function body
  if (ast_body) {
    // update mBlockNodeMap
    mBlockNodeMap[ast_body] = func->body;
    mBlockFuncMap[func->body] = func;
    ProcessNode(skind, ast_body, func->body);
  }

  // add method with updated funcname to parent stype
  if (stype) {
    StIdx stIdx = func->stIdx;
    FuncAttrs funcattrs(func->GetAttrs());
    TyidxFuncAttrPair P0(funcst->tyIdx, funcattrs);
    MethodPair P1(stIdx, P0);
    stype->methods.push_back(P1);
  }

  return nullptr;
}

maple::BaseNode *A2M::ProcessClass(StmtExprKind skind, TreeNode *tnode, maple::BlockNode *block) {
  ClassNode *classnode = static_cast<ClassNode *>(tnode);
  const char *name = classnode->GetName();
  MIRType *type = GlobalTables::GetTypeTable().GetOrCreateClassType(name, mMirModule);
  mNodeTypeMap[name] = type;

  for (int i=0; i < classnode->GetMethodsNum(); i++) {
    ProcessFunction(skind, classnode->GetMethod(i), block);
  }

  for (int i=0; i < classnode->GetFieldsNum(); i++) {
    ProcessField(skind, classnode->GetField(i), block);
  }

  // set kind to kTypeClass from kTypeClassIncomplete
  type->typeKind = maple::MIRTypeKind::kTypeClass;
  return nullptr;
}

maple::BaseNode *A2M::ProcessInterface(StmtExprKind skind, TreeNode *tnode, maple::BlockNode *block) {
  NOTYETIMPL("ProcessInterface()");
  InterfaceNode *node = static_cast<InterfaceNode *>(tnode);
  return nullptr;
}

maple::BaseNode *A2M::ProcessAnnotationType(StmtExprKind skind, TreeNode *tnode, maple::BlockNode *block) {
  NOTYETIMPL("ProcessAnnotationType()");
  AnnotationTypeNode *node = static_cast<AnnotationTypeNode *>(tnode);
  return nullptr;
}

maple::BaseNode *A2M::ProcessAnnotation(StmtExprKind skind, TreeNode *tnode, maple::BlockNode *block) {
  NOTYETIMPL("ProcessAnnotation()");
  AnnotationNode *node = static_cast<AnnotationNode *>(tnode);
  return nullptr;
}

maple::BaseNode *A2M::ProcessException(StmtExprKind skind, TreeNode *tnode, maple::BlockNode *block) {
  NOTYETIMPL("ProcessException()");
  ExceptionNode *node = static_cast<ExceptionNode *>(tnode);
  return nullptr;
}

maple::BaseNode *A2M::ProcessReturn(StmtExprKind skind, TreeNode *tnode, maple::BlockNode *block) {
  ReturnNode *node = static_cast<ReturnNode *>(tnode);
  BaseNode *val = ProcessNode(skind, node->GetResult(), block);
  NaryStmtNode *stmt = mMirBuilder->CreateStmtReturn(val);
  return stmt;
}

maple::BaseNode *A2M::ProcessCondBranch(StmtExprKind skind, TreeNode *tnode, maple::BlockNode *block) {
  NOTYETIMPL("ProcessCondBranch()");
  CondBranchNode *node = static_cast<CondBranchNode *>(tnode);
  return nullptr;
}

maple::BaseNode *A2M::ProcessBreak(StmtExprKind skind, TreeNode *tnode, maple::BlockNode *block) {
  NOTYETIMPL("ProcessBreak()");
  BreakNode *node = static_cast<BreakNode *>(tnode);
  return nullptr;
}

maple::BaseNode *A2M::ProcessForLoop(StmtExprKind skind, TreeNode *tnode, maple::BlockNode *block) {
  NOTYETIMPL("ProcessForLoop()");
  ForLoopNode *node = static_cast<ForLoopNode *>(tnode);
  return nullptr;
}

maple::BaseNode *A2M::ProcessWhileLoop(StmtExprKind skind, TreeNode *tnode, maple::BlockNode *block) {
  NOTYETIMPL("ProcessWhileLoop()");
  WhileLoopNode *node = static_cast<WhileLoopNode *>(tnode);
  return nullptr;
}

maple::BaseNode *A2M::ProcessDoLoop(StmtExprKind skind, TreeNode *tnode, maple::BlockNode *block) {
  NOTYETIMPL("ProcessDoLoop()");
  DoLoopNode *node = static_cast<DoLoopNode *>(tnode);
  return nullptr;
}

maple::BaseNode *A2M::ProcessNew(StmtExprKind skind, TreeNode *tnode, maple::BlockNode *block) {
  NOTYETIMPL("ProcessNew()");
  NewNode *node = static_cast<NewNode *>(tnode);
  return nullptr;
}

maple::BaseNode *A2M::ProcessDelete(StmtExprKind skind, TreeNode *tnode, maple::BlockNode *block) {
  NOTYETIMPL("ProcessDelete()");
  DeleteNode *node = static_cast<DeleteNode *>(tnode);
  return nullptr;
}

maple::BaseNode *A2M::ProcessCall(StmtExprKind skind, TreeNode *tnode, maple::BlockNode *block) {
  NOTYETIMPL("ProcessCall()");
  CallNode *node = static_cast<CallNode *>(tnode);
  return nullptr;
}

maple::BaseNode *A2M::ProcessSwitchLabel(StmtExprKind skind, TreeNode *tnode, maple::BlockNode *block) {
  NOTYETIMPL("ProcessSwitchLabel()");
  SwitchLabelNode *node = static_cast<SwitchLabelNode *>(tnode);
  return nullptr;
}

maple::BaseNode *A2M::ProcessSwitchCase(StmtExprKind skind, TreeNode *tnode, maple::BlockNode *block) {
  NOTYETIMPL("ProcessSwitchCase()");
  SwitchCaseNode *node = static_cast<SwitchCaseNode *>(tnode);
  return nullptr;
}

maple::BaseNode *A2M::ProcessSwitch(StmtExprKind skind, TreeNode *tnode, maple::BlockNode *block) {
  NOTYETIMPL("ProcessSwitch()");
  SwitchNode *node = static_cast<SwitchNode *>(tnode);
  return nullptr;
}

maple::BaseNode *A2M::ProcessPass(StmtExprKind skind, TreeNode *tnode, maple::BlockNode *block) {
  NOTYETIMPL("ProcessPass()");
  PassNode *node = static_cast<PassNode *>(tnode);
  return nullptr;
}

maple::BaseNode *A2M::ProcessBinOperatorMpl(StmtExprKind skind,
                                       maple::Opcode op,
                                       maple::BaseNode *lhs,
                                       maple::BaseNode *rhs,
                                       maple::BlockNode *block) {
  maple::BaseNode *val = mMirBuilder->CreateExprBinary(op, mDefaultType, lhs, rhs);
  return val;
}

maple::BaseNode *A2M::ProcessBinOperatorMplAssign(StmtExprKind skind,
                                             maple::BaseNode *lhs,
                                             maple::BaseNode *rhs,
                                             maple::BlockNode *block) {
  DreadNode *dr = static_cast<DreadNode *>(lhs);
  if (!dr) {
    NOTYETIMPL("ProcessBinOperatorMplAssign() null dr");
    return nullptr;
  }
  MIRFunction *func = mBlockFuncMap[block];
  MIRSymbol *symbol;
  if (func) {
    symbol = func->symTab->GetSymbolFromStIdx(dr->stIdx.Idx());
  } else {
    symbol = GlobalTables::GetGsymTable().GetSymbolFromStIdx(dr->stIdx.Idx());
  }
  if (!symbol) {
    NOTYETIMPL("ProcessBinOperatorMplAssign()");
  }
  DassignNode *node = mMirBuilder->CreateStmtDassign(dr->stIdx, 0, rhs);
  return node;
}

maple::BaseNode *A2M::ProcessBinOperatorMplComboAssign(StmtExprKind skind,
                                                  maple::Opcode op,
                                                  maple::BaseNode *lhs,
                                                  maple::BaseNode *rhs,
                                                  maple::BlockNode *block) {
  NOTYETIMPL("ProcessBinOperatorMplComboAssign()");
  maple::BaseNode *comb = ProcessBinOperatorMpl(SK_Expr, op, lhs, rhs, block);
  maple::BaseNode *assign = ProcessBinOperatorMplAssign(SK_Stmt, lhs, comb, block);
  return assign;
}

maple::BaseNode *A2M::ProcessBinOperatorMplArror(StmtExprKind skind,
                                            maple::BaseNode *lhs,
                                            maple::BaseNode *rhs,
                                            maple::BlockNode *block) {
  NOTYETIMPL("ProcessBinOperatorMplArror()");
  return nullptr;
}


}

