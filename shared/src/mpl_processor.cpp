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

maple::BaseNode *A2M::ProcessNodeDecl(StmtExprKind skind, TreeNode *tnode, BlockNode *block) {
  if (!tnode) {
    return nullptr;
  }

  maple::BaseNode *mpl_node = nullptr;
  switch (tnode->GetKind()) {
    case NK_Class: {
      mpl_node =ProcessClassDecl(SK_Stmt, tnode, nullptr);
      break;
    }
    case NK_Interface: {
      mpl_node = ProcessInterfaceDecl(SK_Stmt, tnode, nullptr);
      break;
    }
    case NK_Function: {
      mpl_node = ProcessFuncDecl(SK_Stmt, tnode, nullptr);
      break;
    }
    case NK_Block: {
      mpl_node = ProcessBlockDecl(SK_Stmt, tnode, nullptr);
      break;
    }
    default: {
      break;
    }
  }
  return mpl_node;
}

maple::BaseNode *A2M::ProcessNode(StmtExprKind skind, TreeNode *tnode, BlockNode *block) {
  if (!tnode) {
    return nullptr;
  }

  maple::BaseNode *mpl_node = nullptr;
  if (mTraceA2m) { tnode->Dump(0); MMSGNOLOC0(" "); }
  switch (tnode->GetKind()) {
#undef  NODEKIND
#define NODEKIND(K) case NK_##K: mpl_node = Process##K(skind, tnode, block); break;
#include "ast_nk.def"
    default:
      break;
  }
  return mpl_node;
}

maple::BaseNode *A2M::ProcessPackage(StmtExprKind skind, TreeNode *tnode, BlockNode *block) {
  NOTYETIMPL("ProcessPackage()");
  PackageNode *node = static_cast<PackageNode *>(tnode);
  return nullptr;
}

maple::BaseNode *A2M::ProcessImport(StmtExprKind skind, TreeNode *tnode, BlockNode *block) {
  NOTYETIMPL("ProcessImport()");
  ImportNode *node = static_cast<ImportNode *>(tnode);
  return nullptr;
}

maple::BaseNode *A2M::ProcessIdentifier(StmtExprKind skind, TreeNode *tnode, BlockNode *block) {
  IdentifierNode *node = static_cast<IdentifierNode *>(tnode);
  const char *name = node->GetName();

  if (skind == SK_Stmt) {
    AST2MPLMSG("ProcessIdentifier() is a decl", name);
    maple::MIRSymbol *symbol = CreateSymbol(node, block);
    return nullptr;
  }

  // check local var
  maple::MIRSymbol *symbol = GetSymbol(node, block);
  if (symbol) {
    AST2MPLMSG("ProcessIdentifier() found local symbol", name);
    return mMirBuilder->CreateExprDread(symbol);
  }

  maple::GStrIdx stridx = maple::GlobalTables::GetStrTable().GetOrCreateStrIdxFromName(name);
  maple::MIRFunction *func = GetFunc(block);

  // check parameters
  if (func->IsAFormalName(stridx)) {
    maple::FormalDef def = func->GetFormalFromName(stridx);
    AST2MPLMSG("ProcessIdentifier() found parameter", name);
    return mMirBuilder->CreateExprDread(def.formalSym);
  }

  // check class fields
  maple::TyIdx tyidx = func->GetFormalDefAt(0).formalTyIdx;
  maple::MIRType *ctype = maple::GlobalTables::GetTypeTable().GetTypeFromTyIdx(tyidx);
  if (ctype->GetPrimType() == maple::PTY_ptr || ctype->GetPrimType() == maple::PTY_ref) {
    maple::MIRPtrType *ptype = static_cast<maple::MIRPtrType *>(ctype);
    ctype = ptype->GetPointedType();
  }
  mFieldData->ResetStrIdx(stridx);
  maple::uint32 fid = 0;
  bool status = mMirBuilder->TraverseToNamedField((maple::MIRStructType*)ctype, fid, mFieldData);
  if (status) {
    maple::MIRSymbol *sym = func->GetFormal(0); // this
    maple::BaseNode *bn = mMirBuilder->CreateExprDread(sym);
    maple::MIRType *ftype = maple::GlobalTables::GetTypeTable().GetTypeFromTyIdx(mFieldData->GetTyIdx());
    AST2MPLMSG("ProcessIdentifier() found match field", name);
    return new maple::IreadNode(maple::OP_iread, ftype->GetPrimType(), sym->GetTyIdx(), maple::FieldID(fid), bn);
  }

  // check global var
  symbol = GetSymbol(node, nullptr);
  if (symbol) {
    AST2MPLMSG("ProcessIdentifier() found global symbol", name);
    return mMirBuilder->CreateExprDread(symbol);
  }

  AST2MPLMSG("ProcessIdentifier() unknown identifier", name);
  // create a dummy var with name and mDefaultType
  symbol = mMirBuilder->GetOrCreateLocalDecl(name, *mDefaultType);
  return mMirBuilder->CreateExprDread(symbol);
}

maple::BaseNode *A2M::ProcessField(StmtExprKind skind, TreeNode *tnode, BlockNode *block) {
  FieldNode *node = static_cast<FieldNode *>(tnode);
  maple::BaseNode *bn = nullptr;

  if (skind != SK_Expr) {
    NOTYETIMPL("ProcessField() not SK_Expr");
    return bn;
  }

  // this.x  a.x
  TreeNode *upper = node->GetUpper();
  TreeNode *field = node->GetField();

  maple::MIRType *ctype = nullptr;
  maple::TyIdx cptyidx(0);
  maple::BaseNode *dr = nullptr;
  if (upper->IsLiteral()) {
    LiteralNode *lt = static_cast<LiteralNode *>(upper);
    if (lt->GetData().mType == LT_ThisLiteral) {
      maple::MIRFunction *func = GetFunc(block);
      maple::MIRSymbol *sym = func->GetFormal(0); // this
      cptyidx = sym->GetTyIdx();
      ctype = GetClass(block);
      dr = new maple::DreadNode(maple::OP_dread, maple::PTY_ptr, sym->GetStIdx(), 0);
    } else {
      NOTYETIMPL("ProcessField() not this literal");
    }
  } else {
    NOTYETIMPL("ProcessField() upper not literal");
  }

  if (!ctype) {
    NOTYETIMPL("ProcessField() null class type");
    return bn;
  }

  const char *fname = field->GetName();
  maple::GStrIdx stridx = maple::GlobalTables::GetStrTable().GetOrCreateStrIdxFromName(fname);
  mFieldData->ResetStrIdx(stridx);
  maple::uint32 fid = 0;
  bool status = mMirBuilder->TraverseToNamedField((maple::MIRStructType*)ctype, fid, mFieldData);
  maple::MIRType *ftype = maple::GlobalTables::GetTypeTable().GetTypeFromTyIdx(mFieldData->GetTyIdx());
  if (status) {
    bn = new maple::IreadNode(maple::OP_iread, ftype->GetPrimType(), cptyidx, fid, dr);
  } else {
    NOTYETIMPL("ProcessField() can not find field");
    // insert a dummy field with fname and mDefaultType
    const maple::FieldAttrs attr;
    maple::TyIdxFieldAttrPair P0(mDefaultType->GetTypeIndex(), attr);
    maple::FieldPair P1(stridx, P0);
    maple::MIRStructType *stype = static_cast<maple::MIRStructType *>(ctype);
    stype->GetFields().push_back(P1);
    bn = new maple::IreadNode(maple::OP_iread, maple::PTY_begin, cptyidx, 888, dr);
  }
  return bn;
}

maple::BaseNode *A2M::ProcessFieldDecl(StmtExprKind skind, TreeNode *tnode, BlockNode *block) {
  FieldNode *node = static_cast<FieldNode *>(tnode);
  maple::BaseNode *bn = nullptr;

  if (!tnode->IsIdentifier()) {
    NOTYETIMPL("ProcessFieldSetup() not Identifier");
    return bn;
  }

  TreeNode *parent = tnode->GetParent();
  MASSERT((parent->IsClass() || parent->IsInterface()) && "Not class or interface");
  maple::MIRType *ptype = mNodeTypeMap[parent->GetName()];
  if (ptype->IsMIRPtrType()) {
    maple::MIRPtrType * ptrtype = static_cast<maple::MIRPtrType *>(ptype);
    ptype = ptrtype->GetPointedType();
  }
  maple::MIRStructType *stype = static_cast<maple::MIRStructType *>(ptype);
  MASSERT(stype && "struct type not valid");

  IdentifierNode *inode = static_cast<IdentifierNode *>(tnode);
  const char    *name = inode->GetName();
  TreeNode      *type = inode->GetType(); // PrimTypeNode or UserTypeNode
  TreeNode      *init = inode->GetInit(); // Init value

  maple::GenericAttrs genAttrs;
  MapAttr(genAttrs, inode);

  maple::GStrIdx stridx = maple::GlobalTables::GetStrTable().GetOrCreateStrIdxFromName(name);
  maple::MIRType *mir_type = MapType(type);
  if (mir_type) {
    maple::TyIdxFieldAttrPair P0(mir_type->GetTypeIndex(), genAttrs.ConvertToFieldAttrs());
    maple::FieldPair P1(stridx, P0);
    stype->GetFields().push_back(P1);
  }

  return bn;
}

maple::BaseNode *A2M::ProcessAssert(StmtExprKind skind, TreeNode *tnode, BlockNode *block) {
  NOTYETIMPL("ProcessAssert()");
  AssertNode *node = static_cast<AssertNode *>(tnode);
  return nullptr;
}

maple::BaseNode *A2M::ProcessDimension(StmtExprKind skind, TreeNode *tnode, BlockNode *block) {
  NOTYETIMPL("ProcessDimension()");
  DimensionNode *node = static_cast<DimensionNode *>(tnode);
  return nullptr;
}

maple::BaseNode *A2M::ProcessAttr(StmtExprKind skind, TreeNode *tnode, BlockNode *block) {
  NOTYETIMPL("ProcessAttr()");
  // AttrNode *node = static_cast<AttrNode *>(tnode);
  return nullptr;
}

maple::BaseNode *A2M::ProcessPrimType(StmtExprKind skind, TreeNode *tnode, BlockNode *block) {
  NOTYETIMPL("ProcessPrimType()");
  PrimTypeNode *node = static_cast<PrimTypeNode *>(tnode);
  return nullptr;
}

maple::BaseNode *A2M::ProcessPrimArrayType(StmtExprKind skind, TreeNode *tnode, BlockNode *block) {
  NOTYETIMPL("ProcessPrimArrayType()");
  PrimArrayTypeNode *node = static_cast<PrimArrayTypeNode *>(tnode);
  return nullptr;
}

maple::BaseNode *A2M::ProcessUserType(StmtExprKind skind, TreeNode *tnode, BlockNode *block) {
  NOTYETIMPL("ProcessUserType()");
  UserTypeNode *node = static_cast<UserTypeNode *>(tnode);
  return nullptr;
}

maple::BaseNode *A2M::ProcessCast(StmtExprKind skind, TreeNode *tnode, BlockNode *block) {
  NOTYETIMPL("ProcessCast()");
  CastNode *node = static_cast<CastNode *>(tnode);
  return nullptr;
}

maple::BaseNode *A2M::ProcessParenthesis(StmtExprKind skind, TreeNode *tnode, BlockNode *block) {
  ParenthesisNode *node = static_cast<ParenthesisNode *>(tnode);
  return ProcessNode(skind, node->GetExpr(), block);
}

maple::BaseNode *A2M::ProcessVarList(StmtExprKind skind, TreeNode *tnode, BlockNode *block) {
  VarListNode *node = static_cast<VarListNode *>(tnode);
  for (int i = 0; i < node->GetNum(); i++) {
    TreeNode *n = node->VarAtIndex(i);
    IdentifierNode *inode = static_cast<IdentifierNode *>(n);
    AST2MPLMSG("ProcessVarList() decl", inode->GetName());
    maple::MIRSymbol *symbol = CreateSymbol(inode, block);
    TreeNode *init = inode->GetInit(); // Init value
    if (init) {
      maple::BaseNode *bn = ProcessNode(SK_Expr, init, block);
      maple::MIRType *mir_type = MapType(inode->GetType());
      bn = new maple::DassignNode(mir_type->GetPrimType(), bn, symbol->GetStIdx(), 0);
      if (bn) {
        maple::BlockNode *blk = mBlockNodeMap[block];
        blk->AddStatement((maple::StmtNode*)bn);
      }
    }
  }
  return nullptr;
}

maple::BaseNode *A2M::ProcessExprList(StmtExprKind skind, TreeNode *tnode, BlockNode *block) {
  NOTYETIMPL("ProcessExprList()");
  ExprListNode *node = static_cast<ExprListNode *>(tnode);
  return nullptr;
}

maple::BaseNode *A2M::ProcessLiteral(StmtExprKind skind, TreeNode *tnode, BlockNode *block) {
  LiteralNode *node = static_cast<LiteralNode *>(tnode);
  LitData data = node->GetData();
  maple::BaseNode *bn = nullptr;
  switch (data.mType) {
    case LT_IntegerLiteral: {
      maple::MIRType *typeI32 = maple::GlobalTables::GetTypeTable().GetInt32();
      maple::MIRIntConst *cst = new maple::MIRIntConst(data.mData.mInt, *typeI32);
      bn =  new maple::ConstvalNode(maple::PTY_i32, cst);
      break;
    }
    case LT_BooleanLiteral: {
      int val = (data.mData.mBool == true) ? 1 : 0;
      maple::MIRType *typeU1 = maple::GlobalTables::GetTypeTable().GetUInt1();
      bn = new maple::ConstvalNode(maple::PTY_u1, new maple::MIRIntConst(val, *typeU1));
      break;
    }
    case LT_FPLiteral:
    case LT_DoubleLiteral:
    case LT_CharacterLiteral:
    case LT_StringLiteral:
    case LT_NullLiteral:
    case LT_ThisLiteral:
    default: {
      NOTYETIMPL("ProcessLiteral() need support");
      break;
    }
  }
  return bn;
}

maple::BaseNode *A2M::ProcessUnaOperator(StmtExprKind skind, TreeNode *tnode, BlockNode *block) {
  UnaOperatorNode *node = static_cast<UnaOperatorNode *>(tnode);
  OprId ast_op = node->GetOprId();
  TreeNode *ast_rhs = node->GetOpnd();
  maple::BaseNode *bn = ProcessNode(SK_Expr, ast_rhs, block);
  maple::BaseNode *mpl_node = nullptr;

  if (!bn) {
    NOTYETIMPL("ProcessUnaOperator() null bn");
    return mpl_node;
  }

  maple::Opcode op = maple::OP_undef;

  op = MapUnaOpcode(ast_op);
  if (op != maple::OP_undef) {
    mpl_node = ProcessUnaOperatorMpl(skind, op, bn, block);
  }

  return mpl_node;
}

maple::BaseNode *A2M::ProcessBinOperator(StmtExprKind skind, TreeNode *tnode, BlockNode *block) {
  BinOperatorNode *bon = static_cast<BinOperatorNode *>(tnode);
  OprId ast_op = bon->mOprId;
  TreeNode *ast_lhs = bon->mOpndA;
  TreeNode *ast_rhs = bon->mOpndB;
  maple::BaseNode *lhs = ProcessNode(SK_Expr, ast_lhs, block);
  maple::BaseNode *rhs = ProcessNode(SK_Expr, ast_rhs, block);
  maple::BaseNode *mpl_node = nullptr;

  if (!lhs || !rhs) {
    NOTYETIMPL("ProcessBinOperator() null lhs and/or rhs");
    return mpl_node;
  }

  maple::Opcode op = maple::OP_undef;

  op = MapBinOpcode(ast_op);
  if (op != maple::OP_undef) {
    mpl_node = new maple::BinaryNode(op, lhs->GetPrimType(), lhs, rhs);
    return mpl_node;
  }

  op = MapBinCmpOpcode(ast_op);
  if (op != maple::OP_undef) {
    mpl_node = new maple::CompareNode(op, maple::PTY_u1, lhs->GetPrimType(), lhs, rhs);
    return mpl_node;
  }

  if (ast_op == OPR_Assign) {
    mpl_node = ProcessBinOperatorMplAssign(SK_Stmt, lhs, rhs, block);
    return mpl_node;
  }

  op = MapBinComboOpcode(ast_op);
  if (op != maple::OP_undef) {
    mpl_node = ProcessBinOperatorMplComboAssign(SK_Stmt, op, lhs, rhs, block);
    return mpl_node;
  }

  if (ast_op == OPR_Arrow) {
    mpl_node = ProcessBinOperatorMplArror(SK_Expr, lhs, rhs, block);
    return mpl_node;
  }

  return mpl_node;
}

maple::BaseNode *A2M::ProcessTerOperator(StmtExprKind skind, TreeNode *tnode, BlockNode *block) {
  NOTYETIMPL("ProcessTerOperator()");
  TerOperatorNode *node = static_cast<TerOperatorNode *>(tnode);
  return nullptr;
}

maple::BaseNode *A2M::ProcessLambda(StmtExprKind skind, TreeNode *tnode, BlockNode *block) {
  NOTYETIMPL("ProcessLambda()");
  LambdaNode *node = static_cast<LambdaNode *>(tnode);
  return nullptr;
}

maple::BaseNode *A2M::ProcessBlockDecl(StmtExprKind skind, TreeNode *tnode, BlockNode *block) {
  BlockNode *ast_block = static_cast<BlockNode *>(tnode);
  maple::BlockNode *blk = mBlockNodeMap[block];
  for (int i = 0; i < ast_block->GetChildrenNum(); i++) {
    TreeNode *child = ast_block->GetChildAtIndex(i);
    maple::BaseNode *stmt = ProcessNodeDecl(skind, child, block);
  }
  return nullptr;
}

maple::BaseNode *A2M::ProcessBlock(StmtExprKind skind, TreeNode *tnode, BlockNode *block) {
  BlockNode *ast_block = static_cast<BlockNode *>(tnode);
  maple::BlockNode *blk = mBlockNodeMap[block];
  for (int i = 0; i < ast_block->GetChildrenNum(); i++) {
    TreeNode *child = ast_block->GetChildAtIndex(i);
    maple::BaseNode *stmt = ProcessNode(skind, child, block);
    if (stmt) {
      blk->AddStatement((maple::StmtNode*)stmt);
      if (mTraceA2m) stmt->Dump(0);
    }
  }
  return nullptr;
}

maple::BaseNode *A2M::ProcessFunction(StmtExprKind skind, TreeNode *tnode, BlockNode *block) {
  MASSERT(tnode->IsFunction() && "it is not an FunctionNode");
  NOTYETIMPL("ProcessFunction()");
  return nullptr;
}

maple::BaseNode *A2M::ProcessFuncDecl(StmtExprKind skind, TreeNode *tnode, BlockNode *block) {
  FunctionNode *ast_func = static_cast<FunctionNode *>(tnode);
  const char                     *name = ast_func->GetName();
  // SmallVector<AttrId>          mAttrs;
  // SmallVector<AnnotationNode*> mAnnotations; //annotation or pragma
  // SmallVector<ExceptionNode*>  mThrows;      // exceptions it can throw
  TreeNode                    *ast_rettype = ast_func->GetType();        // return type
  // SmallVector<TreeNode*>       mParams;      //
  BlockNode                   *ast_body = ast_func->GetBody();
  // DimensionNode               *mDims;
  // bool                         mIsConstructor;
  AST2MPLMSG("\n================== function ==================", name);

  TreeNode *parent = tnode->GetParent();
  maple::MIRStructType *stype = nullptr;
  if (parent->IsClass() || parent->IsInterface()) {
    maple::MIRType *ptype = mNodeTypeMap[parent->GetName()];
    if (ptype->IsMIRPtrType()) {
      maple::MIRPtrType * ptrtype = static_cast<maple::MIRPtrType *>(ptype);
      ptype = ptrtype->GetPointedType();
    }
    stype = static_cast<maple::MIRStructType *>(ptype);
    MASSERT(stype && "struct type not valid");
  }

  maple::MIRType *rettype = MapType(ast_rettype);
  maple::MIRFunction *func = mMirBuilder->GetOrCreateFunction(name, rettype->GetTypeIndex());

  // init function fields
  func->SetBody(func->GetCodeMemPool()->New<maple::BlockNode>());
  func->AllocSymTab();
  if (ast_func->IsConstructor()) {
    func->SetAttr(maple::FUNCATTR_constructor);
  }

  mMirModule->AddFunction(func);
  mMirModule->SetCurFunction(func);

  // set class/interface type
  if (stype) {
    func->SetClassTyIdx(stype->GetTypeIndex());
  }

  // process function arguments for function type and formal parameters
  std::vector<maple::TyIdx> funcvectype;
  std::vector<maple::TypeAttrs> funcvecattr;

  // insert this as first parameter
  if (stype) {
    maple::GStrIdx stridx = maple::GlobalTables::GetStrTable().GetOrCreateStrIdxFromName("this");
    maple::TypeAttrs attr = maple::TypeAttrs();
    maple::MIRType *sptype = maple::GlobalTables::GetTypeTable().GetOrCreatePointerType(*stype, maple::PTY_ref);
    maple::MIRSymbol *sym = mMirBuilder->GetOrCreateLocalDecl("this", *sptype);
    sym->SetStorageClass(maple::kScFormal);
    func->AddArgument(sym);
    funcvectype.push_back(sptype->GetTypeIndex());
    funcvecattr.push_back(attr);
  }

  // process remaining parameters
  for (int i = 0; i < ast_func->GetParamsNum(); i++) {
    TreeNode *param = ast_func->GetParam(i);
    maple::MIRType *type = mDefaultType;
    if (param->IsIdentifier()) {
      IdentifierNode *inode = static_cast<IdentifierNode *>(param);
      type = MapType(inode->GetType());
    } else {
      NOTYETIMPL("ProcessFuncDecl arg type()");
      continue;
    }

    maple::GStrIdx stridx = maple::GlobalTables::GetStrTable().GetOrCreateStrIdxFromName(param->GetName());
    maple::TypeAttrs attr = maple::TypeAttrs();
    maple::MIRSymbol *sym = mMirBuilder->GetOrCreateLocalDecl(param->GetName(), *type);
    sym->SetStorageClass(maple::kScFormal);
    func->AddArgument(sym);
    funcvectype.push_back(type->GetTypeIndex());
    funcvecattr.push_back(attr);
  }

  // use className|funcName|_argTypes_retType as function name
  UpdateFuncName(func);
  mFuncMap[ast_func] = func;
  mNameFuncMap[name].push_back(func);

  // create function type
  maple::MIRFuncType *functype = (maple::MIRFuncType*)maple::GlobalTables::GetTypeTable().GetOrCreateFunctionType(*mMirModule, rettype->GetTypeIndex(), funcvectype, funcvecattr, /*isvarg*/ false, false);
  func->SetMIRFuncType(functype);

  // update function symbol's type
  maple::MIRSymbol *funcst = maple::GlobalTables::GetGsymTable().GetSymbolFromStidx(func->GetStIdx().Idx());
  funcst->SetTyIdx(functype->GetTypeIndex());

  // add method with updated funcname to parent stype
  if (stype) {
    maple::StIdx stIdx = func->GetStIdx();
    maple::FuncAttrs funcattrs(func->GetFuncAttrs());
    maple::TyidxFuncAttrPair P0(func->GetMIRFuncType()->GetTypeIndex(), funcattrs);
    maple::MethodPair P1(stIdx, P0);
    stype->GetMethods().push_back(P1);
  }

  if (ast_func->GetBody()) {
    BlockNode *ast_block = static_cast<BlockNode *>(ast_func->GetBody());
    for (int i = 0; i < ast_block->GetChildrenNum(); i++) {
      TreeNode *child = ast_block->GetChildAtIndex(i);
      maple::BaseNode *stmt = ProcessNodeDecl(skind, child, block);
    }
  }

  AST2MPLMSG2("\n================ end function ================", name, func->GetName());
  return nullptr;
}

maple::BaseNode *A2M::ProcessFuncSetup(StmtExprKind skind, TreeNode *tnode, BlockNode *block) {
  FunctionNode *ast_func = static_cast<FunctionNode *>(tnode);

  maple::MIRFunction *func = mFuncMap[ast_func];
  MASSERT(func && "func should have been declared");

  const std::string name = func->GetName();
  AST2MPLMSG("\n================== function ==================", name);

  mMirModule->SetCurFunction(func);

  // init function fields
  func->AllocPregTab();
  func->AllocTypeNameTab();
  func->AllocLabelTab();

  // set up function body
  BlockNode *ast_body = ast_func->GetBody();
  if (ast_body) {
    // update mBlockNodeMap
    mBlockNodeMap[ast_body] = func->GetBody();
    mBlockFuncMap[ast_body] = func;
    ProcessNode(skind, ast_body, ast_body);
  }

  AST2MPLMSG2("\n================ end function ================", name, func->GetName());
  return nullptr;
}

maple::BaseNode *A2M::ProcessClassDecl(StmtExprKind skind, TreeNode *tnode, BlockNode *block) {
  ClassNode *classnode = static_cast<ClassNode *>(tnode);
  const char *name = classnode->GetName();
  maple::MIRType *type = maple::GlobalTables::GetTypeTable().GetOrCreateClassType(name, *mMirModule);
  type->SetMIRTypeKind(maple::kTypeClass);
  // always use pointer type for classes, with PTY_ref
  type = maple::GlobalTables::GetTypeTable().GetOrCreatePointerType(*type, maple::PTY_ref);
  mNodeTypeMap[name] = type;
  AST2MPLMSG("\n================== class =====================", name);

  for (int i=0; i < classnode->GetLocalClassesNum(); i++) {
    ProcessClassDecl(skind, classnode->GetLocalClass(i), block);
  }

  for (int i=0; i < classnode->GetLocalInterfacesNum(); i++) {
    ProcessInterfaceDecl(skind, classnode->GetLocalInterface(i), block);
  }

  for (int i=0; i < classnode->GetFieldsNum(); i++) {
    ProcessFieldDecl(skind, classnode->GetField(i), block);
  }

  for (int i=0; i < classnode->GetConstructorNum(); i++) {
    ProcessFuncDecl(skind, classnode->GetConstructor(i), block);
  }

  for (int i=0; i < classnode->GetMethodsNum(); i++) {
    ProcessFuncDecl(skind, classnode->GetMethod(i), block);
  }

  return nullptr;
}

maple::BaseNode *A2M::ProcessClass(StmtExprKind skind, TreeNode *tnode, BlockNode *block) {
  ClassNode *classnode = static_cast<ClassNode *>(tnode);
  const char *name = classnode->GetName();
  maple::MIRType *type = mNodeTypeMap[name];
  AST2MPLMSG("\n================== class =====================", name);

  for (int i=0; i < classnode->GetLocalClassesNum(); i++) {
    ProcessClass(skind, classnode->GetLocalClass(i), block);
  }

  for (int i=0; i < classnode->GetLocalInterfacesNum(); i++) {
    ProcessInterface(skind, classnode->GetLocalInterface(i), block);
  }

  for (int i=0; i < classnode->GetConstructorNum(); i++) {
    ProcessFuncSetup(skind, classnode->GetConstructor(i), block);
  }

  for (int i=0; i < classnode->GetMethodsNum(); i++) {
    ProcessFuncSetup(skind, classnode->GetMethod(i), block);
  }

  return nullptr;
}

maple::BaseNode *A2M::ProcessInterfaceDecl(StmtExprKind skind, TreeNode *tnode, BlockNode *block) {
  NOTYETIMPL("ProcessInterfaceDecl()");
  InterfaceNode *node = static_cast<InterfaceNode *>(tnode);
  return nullptr;
}

maple::BaseNode *A2M::ProcessInterface(StmtExprKind skind, TreeNode *tnode, BlockNode *block) {
  NOTYETIMPL("ProcessInterface()");
  InterfaceNode *node = static_cast<InterfaceNode *>(tnode);
  return nullptr;
}

maple::BaseNode *A2M::ProcessAnnotationType(StmtExprKind skind, TreeNode *tnode, BlockNode *block) {
  NOTYETIMPL("ProcessAnnotationType()");
  AnnotationTypeNode *node = static_cast<AnnotationTypeNode *>(tnode);
  return nullptr;
}

maple::BaseNode *A2M::ProcessAnnotation(StmtExprKind skind, TreeNode *tnode, BlockNode *block) {
  NOTYETIMPL("ProcessAnnotation()");
  AnnotationNode *node = static_cast<AnnotationNode *>(tnode);
  return nullptr;
}

maple::BaseNode *A2M::ProcessException(StmtExprKind skind, TreeNode *tnode, BlockNode *block) {
  NOTYETIMPL("ProcessException()");
  ExceptionNode *node = static_cast<ExceptionNode *>(tnode);
  return nullptr;
}

maple::BaseNode *A2M::ProcessReturn(StmtExprKind skind, TreeNode *tnode, BlockNode *block) {
  ReturnNode *node = static_cast<ReturnNode *>(tnode);
  maple::BaseNode *val = ProcessNode(SK_Expr, node->GetResult(), block);
  maple::NaryStmtNode *stmt = mMirBuilder->CreateStmtReturn(val);
  return stmt;
}

maple::BaseNode *A2M::ProcessCondBranch(StmtExprKind skind, TreeNode *tnode, BlockNode *block) {
  CondBranchNode *node = static_cast<CondBranchNode *>(tnode);
  maple::BaseNode *cond = ProcessNode(SK_Expr, node->GetCond(), block);
  if (!cond) {
    NOTYETIMPL("ProcessCondBranch() condition");
    maple::MIRType *typeU1 = maple::GlobalTables::GetTypeTable().GetUInt1();
    cond =  new maple::ConstvalNode(maple::PTY_u1, new maple::MIRIntConst(1, *typeU1));
  }
  maple::BlockNode *thenBlock = nullptr;
  maple::BlockNode *elseBlock = nullptr;
  BlockNode *blk = (BlockNode*)node->GetTrueBranch();
  if (blk) {
    MASSERT(blk->IsBlock() && "then body is not a block");
    thenBlock = new maple::BlockNode();
    mBlockNodeMap[blk] = thenBlock;
    ProcessBlock(skind, blk, blk);
  }
  blk = (BlockNode*)node->GetFalseBranch();
  if (blk) {
    MASSERT(blk->IsBlock() && "else body is not a block");
    elseBlock = new maple::BlockNode();
    mBlockNodeMap[blk] = elseBlock;
    ProcessBlock(skind, blk, blk);
  }
  maple::IfStmtNode *ifnode = new maple::IfStmtNode();
  ifnode->SetOpnd(cond, 0);
  ifnode->SetThenPart(thenBlock);
  ifnode->SetElsePart(elseBlock);
  return ifnode;
}

maple::BaseNode *A2M::ProcessBreak(StmtExprKind skind, TreeNode *tnode, BlockNode *block) {
  NOTYETIMPL("ProcessBreak()");
  BreakNode *node = static_cast<BreakNode *>(tnode);
  return nullptr;
}

maple::BaseNode *A2M::ProcessLoopCondBody(StmtExprKind skind, TreeNode *cond, TreeNode *body, BlockNode *block) {
  maple::BaseNode *mircond = ProcessNode(SK_Expr, cond, block);
  if (!mircond) {
    NOTYETIMPL("ProcessLoopCondBody() condition");
    maple::MIRType *typeU1 = maple::GlobalTables::GetTypeTable().GetUInt1();
    mircond =  new maple::ConstvalNode(maple::PTY_u1, new maple::MIRIntConst(1, *typeU1));
  }

  MASSERT(body && body->IsBlock() && "body is nullptr or not a block");
  maple::BlockNode *mbody = new maple::BlockNode();
  BlockNode *blk = static_cast<BlockNode *>(body);
  mBlockNodeMap[blk] = mbody;
  ProcessBlock(skind, blk, blk);

  maple::WhileStmtNode *wsn = new maple::WhileStmtNode(maple::OP_while);
  wsn->SetOpnd(mircond, 0);
  wsn->SetBody(mbody);
  mBlockNodeMap[block]->AddStatement(wsn);
  return nullptr;
}

maple::BaseNode *A2M::ProcessForLoop(StmtExprKind skind, TreeNode *tnode, BlockNode *block) {
  ForLoopNode *node = static_cast<ForLoopNode *>(tnode);
  maple::BlockNode *mblock = mBlockNodeMap[block];
  maple::BaseNode *bn = nullptr;

  // init
  for (int i = 0; i < node->GetInitNum(); i++) {
    bn = ProcessNode(SK_Stmt, node->InitAtIndex(i), block);
    if (bn) {
      mblock->AddStatement((maple::StmtNode*)bn);
      if (mTraceA2m) bn->Dump(0);
    }
  }

  // cond body
  TreeNode *astbody = node->GetBody();
  (void) ProcessLoopCondBody(skind, node->GetCond(), astbody, block);

  maple::BlockNode *mbody = mBlockNodeMap[static_cast<BlockNode*>(astbody)];

  // update stmts are added into loop mbody
  for (int i = 0; i < node->GetUpdateNum(); i++) {
    bn = ProcessNode(SK_Stmt, node->UpdateAtIndex(i), (maplefe::BlockNode*)astbody);
    if (bn) {
      mbody->AddStatement((maple::StmtNode*)bn);
      if (mTraceA2m) bn->Dump(0);
    }
  }

  return nullptr;
}

maple::BaseNode *A2M::ProcessWhileLoop(StmtExprKind skind, TreeNode *tnode, BlockNode *block) {
  WhileLoopNode *node = static_cast<WhileLoopNode *>(tnode);
  return ProcessLoopCondBody(skind, node->GetCond(), node->GetBody(), block);
}

maple::BaseNode *A2M::ProcessDoLoop(StmtExprKind skind, TreeNode *tnode, BlockNode *block) {
  DoLoopNode *node = static_cast<DoLoopNode *>(tnode);
  return ProcessLoopCondBody(skind, node->GetCond(), node->GetBody(), block);
}

maple::BaseNode *A2M::ProcessNew(StmtExprKind skind, TreeNode *tnode, BlockNode *block) {
  NOTYETIMPL("ProcessNew()");
  NewNode *node = static_cast<NewNode *>(tnode);
  return nullptr;
}

maple::BaseNode *A2M::ProcessDelete(StmtExprKind skind, TreeNode *tnode, BlockNode *block) {
  NOTYETIMPL("ProcessDelete()");
  DeleteNode *node = static_cast<DeleteNode *>(tnode);
  return nullptr;
}

maple::BaseNode *A2M::ProcessCall(StmtExprKind skind, TreeNode *tnode, BlockNode *block) {
  NOTYETIMPL("ProcessCall()");
  CallNode *node = static_cast<CallNode *>(tnode);
  maple::MapleVector<maple::BaseNode *> args(mMirModule->CurFuncCodeMemPoolAllocator()->Adapter());
  maple::MIRFunction *func = GetFunc(block);

  // pass this
  maple::MIRSymbol *sym = func->GetFormal(0);
  args.push_back(mMirBuilder->CreateExprDread(sym));
  // pass arg
  for (int i = 0; i < node->GetArgsNum(); i++) {
    maple::BaseNode *arg = ProcessNode(SK_Expr, node->GetArg(i), block);
    if (arg) {
      args.push_back(arg);
    } else {
      NOTYETIMPL("ProcessCall() null arg");
    }
  }

  TreeNode *method = node->GetMethod();
  if (!method->IsIdentifier()) {
    NOTYETIMPL("ProcessCall() method not an identifier");
  }
  IdentifierNode *imethod = static_cast<IdentifierNode *>(method);
  func = SearchFunc(imethod->GetName(), args);
  if (!func) {
    NOTYETIMPL("ProcessCall() method not found");
    return nullptr;
  }
  maple::PUIdx puIdx = func->GetPuidx();

  maple::MIRType *returnType = func->GetReturnType();
  maple::MIRSymbol *rv = nullptr;
  maple::Opcode callop = maple::OP_call;
  if (returnType->GetPrimType() != maple::PTY_void) {
    NOTYETIMPL("ProcessCall() OP_callassigned");
    rv = CreateTempVar("retvar", returnType);
    callop = maple::OP_callassigned;
  }

  maple::StmtNode *stmt = mMirBuilder->CreateStmtCallAssigned(puIdx, args, rv, callop);

  maple::BlockNode *blk = mBlockNodeMap[block];
  blk->AddStatement(stmt);
  if (rv) {
    return mMirBuilder->CreateExprDread(rv);
  } else {
    return stmt;
  }
}

maple::BaseNode *A2M::ProcessSwitchLabel(StmtExprKind skind, TreeNode *tnode, BlockNode *block) {
  NOTYETIMPL("ProcessSwitchLabel()");
  SwitchLabelNode *node = static_cast<SwitchLabelNode *>(tnode);
  return nullptr;
}

maple::BaseNode *A2M::ProcessSwitchCase(StmtExprKind skind, TreeNode *tnode, BlockNode *block) {
  NOTYETIMPL("ProcessSwitchCase()");
  SwitchCaseNode *node = static_cast<SwitchCaseNode *>(tnode);
  return nullptr;
}

maple::BaseNode *A2M::ProcessSwitch(StmtExprKind skind, TreeNode *tnode, BlockNode *block) {
  NOTYETIMPL("ProcessSwitch()");
  SwitchNode *node = static_cast<SwitchNode *>(tnode);
  return nullptr;
}

maple::BaseNode *A2M::ProcessPass(StmtExprKind skind, TreeNode *tnode, BlockNode *block) {
  PassNode *node = static_cast<PassNode *>(tnode);
  maple::BlockNode *blk = mBlockNodeMap[block];
  maple::BaseNode *stmt = nullptr;
  for (int i = 0; i < node->GetChildrenNum(); i++) {
    TreeNode *child = node->GetChild(i);
    stmt = ProcessNode(skind, child, block);
    if (stmt && IsStmt(child)) {
      blk->AddStatement((maple::StmtNode*)stmt);
      if (mTraceA2m) stmt->Dump(0);
    }
  }
  return nullptr;
}

maple::BaseNode *A2M::ProcessUnaOperatorMpl(StmtExprKind skind,
                                       maple::Opcode op,
                                       maple::BaseNode *bn,
                                       BlockNode *block) {
  maple::BaseNode *node = nullptr;
  if (op == maple::OP_incref || op == maple::OP_decref) {
    maple::MIRType *typeI32 = maple::GlobalTables::GetTypeTable().GetInt32();
    maple::MIRIntConst *cst = new maple::MIRIntConst(1, *typeI32);
    maple::BaseNode *one =  new maple::ConstvalNode(maple::PTY_i32, cst);
    if (op == maple::OP_incref) {
      node = new maple::BinaryNode(maple::OP_add, bn->GetPrimType(), bn, one);
    } else if (op == maple::OP_decref) {
      node = new maple::BinaryNode(maple::OP_sub, bn->GetPrimType(), bn, one);
    }
  } else {
    node = new maple::UnaryNode(op, bn->GetPrimType(), bn);
  }

  if (skind == SK_Expr) {
     return node;
  }

  if (skind == SK_Stmt) {
    if (op == maple::OP_incref || op == maple::OP_decref) {
      switch (bn->op) {
        case maple::OP_iread: {
          maple::IreadNode *ir = static_cast<maple::IreadNode *>(bn);
          node = new maple::IassignNode(ir->GetTyIdx(), ir->GetFieldID(), ir->Opnd(0), node);
          break;
        }
        case maple::OP_dread: {
          maple::DreadNode *dr = static_cast<maple::DreadNode *>(bn);
          node = new maple::DassignNode(dr->GetPrimType(), node, dr->GetStIdx(), dr->GetFieldID());
          break;
        }
        default:
          NOTYETIMPL("ProcessUnaOperatorMplAssign() need to support opcode");
          break;
      }
    }
  }

  return node;
}

maple::BaseNode *A2M::ProcessBinOperatorMplAssign(StmtExprKind skind,
                                             maple::BaseNode *lhs,
                                             maple::BaseNode *rhs,
                                             BlockNode *block) {
  if (!lhs || !rhs) {
    NOTYETIMPL("ProcessBinOperatorMplAssign() null lhs or rhs");
    return nullptr;
  }

  maple::BaseNode *node = nullptr;
  switch (lhs->op) {
    case maple::OP_iread: {
      maple::IreadNode *ir = static_cast<maple::IreadNode *>(lhs);
      node = new maple::IassignNode(ir->GetTyIdx(), ir->GetFieldID(), ir->Opnd(0), rhs);
      break;
    }
    case maple::OP_dread: {
      maple::DreadNode *dr = static_cast<maple::DreadNode *>(lhs);
      node = new maple::DassignNode(dr->GetPrimType(), rhs, dr->GetStIdx(), dr->GetFieldID());
      break;
    }
    default:
      NOTYETIMPL("ProcessBinOperatorMplAssign() need to support opcode");
      break;
  }

  return node;
}

maple::BaseNode *A2M::ProcessBinOperatorMplComboAssign(StmtExprKind skind,
                                                  maple::Opcode op,
                                                  maple::BaseNode *lhs,
                                                  maple::BaseNode *rhs,
                                                  BlockNode *block) {
  maple::BaseNode *result = new maple::BinaryNode(op, lhs->GetPrimType(), lhs, rhs);
  maple::BaseNode *assign = ProcessBinOperatorMplAssign(SK_Stmt, lhs, result, block);
  return assign;
}

maple::BaseNode *A2M::ProcessBinOperatorMplArror(StmtExprKind skind,
                                            maple::BaseNode *lhs,
                                            maple::BaseNode *rhs,
                                            BlockNode *block) {
  NOTYETIMPL("ProcessBinOperatorMplArror()");
  return nullptr;
}

}

