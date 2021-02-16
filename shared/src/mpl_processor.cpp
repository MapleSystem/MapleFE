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
    MIRSymbol *symbol = CreateSymbol(node, block);
    return nullptr;
  }

  // true false
  if (!strcmp(name, "true")) {
    MIRType *typeU1 = GlobalTables::GetTypeTable().GetUInt1();
    MIRIntConst *cst = new MIRIntConst(1, typeU1);
    BaseNode *one =  new maple::ConstvalNode(PTY_u1, cst);
    return one;
  } else if (!strcmp(name, "false")) {
    MIRType *typeU1 = GlobalTables::GetTypeTable().GetUInt1();
    MIRIntConst *cst = new MIRIntConst(0, typeU1);
    BaseNode *zero =  new maple::ConstvalNode(PTY_u1, cst);
    return zero;
  }

  // check local var
  MIRSymbol *symbol = GetSymbol(node, block);
  if (symbol) {
    AST2MPLMSG("ProcessIdentifier() found local symbol", name);
    return mMirBuilder->CreateExprDread(symbol);
  }

  GStrIdx stridx = GlobalTables::GetStrTable().GetOrCreateStrIdxFromName(name);
  MIRFunction *func = GetFunc(block);

  // check parameters
  for (auto it: func->formalDefVec) {
    if (it.formalStrIdx == stridx) {
      AST2MPLMSG("ProcessIdentifier() found parameter", name);
      return mMirBuilder->CreateExprDread(it.formalSym);
    }
  }

  // check class fields
  TyIdx tyidx = func->formalDefVec[0].formalTyIdx;
  MIRType *ctype = GlobalTables::GetTypeTable().GetTypeFromTyIdx(tyidx);
  if (ctype->GetPrimType() == PTY_ptr || ctype->GetPrimType() == PTY_ref) {
    MIRPtrType *ptype = static_cast<MIRPtrType *>(ctype);
    ctype = ptype->GetPointedType();
  }
  mFieldData->ResetStrIdx(stridx);
  uint32 fid = 0;
  bool status = mMirBuilder->TraverseToNamedField(ctype, fid, mFieldData);
  if (status) {
    MIRSymbol *sym = func->formalDefVec[0].formalSym; // this
    maple::BaseNode *bn = mMirBuilder->CreateExprDread(sym);
    maple::MIRType *ftype = GlobalTables::GetTypeTable().GetTypeFromTyIdx(mFieldData->GetTyIdx());
    AST2MPLMSG("ProcessIdentifier() found match field", name);
    return mMirBuilder->CreateExprIread(ftype, sym->GetType(), FieldID(fid), bn);
  }

  // check global var
  symbol = GetSymbol(node, nullptr);
  if (symbol) {
    AST2MPLMSG("ProcessIdentifier() found global symbol", name);
    return mMirBuilder->CreateExprDread(symbol);
  }

  AST2MPLMSG("ProcessIdentifier() unknown identifier", name);
  symbol = mMirBuilder->GetOrCreateLocalDecl(name, mDefaultType, func);
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

  MIRType *ctype = nullptr;
  TyIdx cptyidx(0);
  maple::BaseNode *dr = nullptr;
  if (upper->IsLiteral()) {
    LiteralNode *lt = static_cast<LiteralNode *>(upper);
    if (lt->GetData().mType == LT_ThisLiteral) {
      MIRFunction *func = GetFunc(block);
      MIRSymbol *sym = func->formalDefVec[0].formalSym; // this
      cptyidx = sym->GetTyIdx();
      ctype = GetClass(block);
      dr = new DreadNode(OP_dread, PTY_ptr, sym->GetStIdx(), 0);
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
  GStrIdx stridx = GlobalTables::GetStrTable().GetOrCreateStrIdxFromName(fname);
  mFieldData->ResetStrIdx(stridx);
  uint32 fid = 0;
  bool status = mMirBuilder->TraverseToNamedField(ctype, fid, mFieldData);
  if (status) {
    maple::MIRType *ftype = GlobalTables::GetTypeTable().GetTypeFromTyIdx(mFieldData->GetTyIdx());
    bn = new IreadNode(OP_iread, ftype->GetPrimType(), cptyidx, fid, dr);
  }
  return bn;
}

maple::BaseNode *A2M::ProcessFieldSetup(StmtExprKind skind, TreeNode *tnode, BlockNode *block) {
  FieldNode *node = static_cast<FieldNode *>(tnode);
  maple::BaseNode *bn = nullptr;

  if (!tnode->IsIdentifier()) {
    NOTYETIMPL("ProcessFieldSetup() not Identifier");
    return bn;
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
  MIRType *mir_type = MapType(type);
  // always use pointer type for classes, with PTY_ref
  if (mir_type->typeKind == kTypeClass || mir_type->typeKind == kTypeClassIncomplete ||
      mir_type->typeKind == kTypeInterface || mir_type->typeKind == kTypeInterfaceIncomplete) {
    mir_type = GlobalTables::GetTypeTable().GetOrCreatePointerType(mir_type, PTY_ref);
  }
  if (mir_type) {
    TyidxFieldAttrPair P0(mir_type->GetTypeIndex(), genAttrs.ConvertToFieldAttrs());
    FieldPair P1(stridx, P0);
    stype->fields.push_back(P1);
  }

  return bn;
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
    MIRSymbol *symbol = CreateSymbol(inode, block);
    TreeNode *init = inode->GetInit(); // Init value
    if (init) {
      maple::BaseNode *bn = ProcessNode(SK_Expr, init, block);
      MIRType *mir_type = MapType(inode->GetType());
      bn = new DassignNode(mir_type->GetPrimType(), bn, symbol->stIdx, 0);
      if (bn) {
        maple::BlockNode *blk = mBlockNodeMap[block];
        blk->AddStatement(bn);
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
      MIRIntConst *cst = new MIRIntConst(data.mData.mInt, GlobalTables::GetTypeTable().GetInt32());
      bn =  new maple::ConstvalNode(PTY_i32, cst);
      break;
    }
    case LT_FPLiteral:
    case LT_DoubleLiteral:
    case LT_BooleanLiteral:
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

  maple::Opcode op = maple::kOpUndef;

  op = MapUnaOpcode(ast_op);
  if (op != kOpUndef) {
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

  maple::Opcode op = maple::kOpUndef;

  op = MapBinOpcode(ast_op);
  if (op != kOpUndef) {
    mpl_node = new BinaryNode(op, lhs->GetPrimType(), lhs, rhs);
    return mpl_node;
  }

  op = MapBinCmpOpcode(ast_op);
  if (op != kOpUndef) {
    mpl_node = new CompareNode(op, PTY_u1, lhs->GetPrimType(), lhs, rhs);
    return mpl_node;
  }

  if (ast_op == OPR_Assign) {
    mpl_node = ProcessBinOperatorMplAssign(SK_Stmt, lhs, rhs, block);
    return mpl_node;
  }

  op = MapBinComboOpcode(ast_op);
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

maple::BaseNode *A2M::ProcessBlock(StmtExprKind skind, TreeNode *tnode, BlockNode *block) {
  BlockNode *ast_block = static_cast<BlockNode *>(tnode);
  maple::BlockNode *blk = mBlockNodeMap[block];
  for (int i = 0; i < ast_block->GetChildrenNum(); i++) {
    TreeNode *child = ast_block->GetChildAtIndex(i);
    BaseNode *stmt = ProcessNode(skind, child, block);
    if (stmt) {
      blk->AddStatement(stmt);
      if (mTraceA2m) stmt->Dump(mMirModule, 0);
    }
  }
  return nullptr;
}

maple::BaseNode *A2M::ProcessFunction(StmtExprKind skind, TreeNode *tnode, BlockNode *block) {
  MASSERT(tnode->IsFunction() && "it is not an FunctionNode");
  NOTYETIMPL("ProcessFunction()");
  return nullptr;
}

maple::BaseNode *A2M::ProcessFuncSetup(StmtExprKind skind, TreeNode *tnode, BlockNode *block) {
  // NOTYETIMPL("ProcessFuncSetup()");
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
  MIRStructType *stype = nullptr;
  if (parent->IsClass() || parent->IsInterface()) {
    MIRType *ptype = mNodeTypeMap[parent->GetName()];
    stype = static_cast<MIRStructType *>(ptype);
    MASSERT(stype && "struct type not valid");
  }

  MIRType *rettype = MapType(ast_rettype);
  TyIdx tyidx = rettype ? rettype->GetTypeIndex() : TyIdx(0);
  MIRFunction *func = mMirBuilder->GetOrCreateFunction(name, tyidx);

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
    MIRType *sptype = GlobalTables::GetTypeTable().GetOrCreatePointerType(stype, PTY_ref);
    MIRSymbol *sym = mMirBuilder->GetOrCreateLocalDecl("this", sptype, func);
    sym->SetStorageClass(kScFormal);
    FormalDef formalDef(stridx, sym, sptype->GetTypeIndex(), attr);
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
      NOTYETIMPL("ProcessFuncSetup arg type()");
    }

    GStrIdx stridx = GlobalTables::GetStrTable().GetOrCreateStrIdxFromName(param->GetName());
    TypeAttrs attr = TypeAttrs();
    MIRSymbol *sym = mMirBuilder->GetOrCreateLocalDecl(param->GetName(), type, func);
    sym->SetStorageClass(kScFormal);
    FormalDef formalDef(stridx, sym, type->GetTypeIndex(), attr);
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
    mBlockFuncMap[ast_body] = func;
    ProcessNode(skind, ast_body, ast_body);
  }

  // add method with updated funcname to parent stype
  if (stype) {
    StIdx stIdx = func->stIdx;
    FuncAttrs funcattrs(func->GetAttrs());
    TyidxFuncAttrPair P0(funcst->tyIdx, funcattrs);
    MethodPair P1(stIdx, P0);
    stype->methods.push_back(P1);
  }

  AST2MPLMSG2("\n================ end function ================", name, func->GetName());
  return nullptr;
}

maple::BaseNode *A2M::ProcessClass(StmtExprKind skind, TreeNode *tnode, BlockNode *block) {
  ClassNode *classnode = static_cast<ClassNode *>(tnode);
  const char *name = classnode->GetName();
  MIRType *type = GlobalTables::GetTypeTable().GetOrCreateClassType(name, mMirModule);
  mNodeTypeMap[name] = type;
  AST2MPLMSG("\n================== class =====================", name);

  for (int i=0; i < classnode->GetFieldsNum(); i++) {
    ProcessFieldSetup(skind, classnode->GetField(i), block);
  }

  for (int i=0; i < classnode->GetConstructorNum(); i++) {
    ProcessFuncSetup(skind, classnode->GetConstructor(i), block);
  }

  for (int i=0; i < classnode->GetMethodsNum(); i++) {
    ProcessFuncSetup(skind, classnode->GetMethod(i), block);
  }

  // set kind to kTypeClass from kTypeClassIncomplete
  type->typeKind = maple::MIRTypeKind::kTypeClass;
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
  BaseNode *val = ProcessNode(SK_Expr, node->GetResult(), block);
  NaryStmtNode *stmt = mMirBuilder->CreateStmtReturn(val);
  return stmt;
}

maple::BaseNode *A2M::ProcessCondBranch(StmtExprKind skind, TreeNode *tnode, BlockNode *block) {
  CondBranchNode *node = static_cast<CondBranchNode *>(tnode);
  maple::BaseNode *cond = ProcessNode(SK_Expr, node->GetCond(), block);
  if (!cond) {
    NOTYETIMPL("ProcessCondBranch() condition");
    cond =  new maple::ConstvalNode(PTY_u1, new MIRIntConst(1, GlobalTables::GetTypeTable().GetUInt8()));
  }
  maple::BlockNode *thenBlock = nullptr;
  maple::BlockNode *elseBlock = nullptr;
  BlockNode *blk = node->GetTrueBranch();
  if (blk) {
    MASSERT(blk->IsBlock() && "then body is not a block");
    thenBlock = new maple::BlockNode();
    mBlockNodeMap[blk] = thenBlock;
    ProcessBlock(skind, blk, blk);
  }
  blk = node->GetFalseBranch();
  if (blk) {
    MASSERT(blk->IsBlock() && "else body is not a block");
    elseBlock = new maple::BlockNode();
    mBlockNodeMap[blk] = elseBlock;
    ProcessBlock(skind, blk, blk);
  }
  maple::IfStmtNode *ifnode = new maple::IfStmtNode();
  ifnode->uOpnd = cond;
  ifnode->thenPart = thenBlock;
  ifnode->elsePart = elseBlock;
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
    mircond =  new maple::ConstvalNode(PTY_u1, new MIRIntConst(1, GlobalTables::GetTypeTable().GetUInt8()));
  }

  MASSERT(body && body->IsBlock() && "body is nullptr or not a block");
  maple::BlockNode *mbody = new maple::BlockNode();
  BlockNode *blk = static_cast<BlockNode *>(body);
  mBlockNodeMap[blk] = mbody;
  ProcessBlock(skind, blk, blk);

  WhileStmtNode *wsn = new WhileStmtNode(OP_while);
  wsn->uOpnd = mircond;
  wsn->body = mbody;
  mBlockNodeMap[block] ->AddStatement(wsn);
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
      mblock->AddStatement(bn);
      if (mTraceA2m) bn->Dump(mMirModule, 0);
    }
  }

  // cond body
  TreeNode *astbody = node->GetBody();
  (void) ProcessLoopCondBody(skind, node->GetCond(), astbody, block);

  maple::BlockNode *mbody = mBlockNodeMap[static_cast<BlockNode*>(astbody)];

  // update stmts are added into loop mbody
  for (int i = 0; i < node->GetUpdateNum(); i++) {
    bn = ProcessNode(SK_Stmt, node->UpdateAtIndex(i), astbody);
    if (bn) {
      mbody->AddStatement(bn);
      if (mTraceA2m) bn->Dump(mMirModule, 0);
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
  return nullptr;
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
  BaseNode *stmt = nullptr;
  for (int i = 0; i < node->GetChildrenNum(); i++) {
    TreeNode *child = node->GetChild(i);
    stmt = ProcessNode(skind, child, block);
    if (stmt && IsStmt(child)) {
      blk->AddStatement(stmt);
      if (mTraceA2m) stmt->Dump(mMirModule, 0);
    }
  }
  return nullptr;
}

maple::BaseNode *A2M::ProcessUnaOperatorMpl(StmtExprKind skind,
                                       maple::Opcode op,
                                       maple::BaseNode *bn,
                                       BlockNode *block) {
  maple::BaseNode *node = nullptr;
  if (op == OP_incref || op == OP_decref) {
    MIRType *typeI32 = GlobalTables::GetTypeTable().GetInt32();
    MIRIntConst *cst = new MIRIntConst(1, typeI32);
    BaseNode *one =  new maple::ConstvalNode(PTY_i32, cst);
    if (op == OP_incref) {
      node = new BinaryNode(OP_add, bn->GetPrimType(), bn, one);
    } else if (op == OP_decref) {
      node = new BinaryNode(OP_sub, bn->GetPrimType(), bn, one);
    }
  } else {
    node = new UnaryNode(op, bn->GetPrimType(), bn);
  }

  if (skind == SK_Expr) {
     return node;
  }

  if (skind == SK_Stmt) {
    if (op == OP_incref || op == OP_decref) {
      switch (bn->op) {
        case OP_iread: {
          IreadNode *ir = static_cast<maple::IreadNode *>(bn);
          node = new IassignNode(ir->tyIdx, ir->fieldID, ir->uOpnd, node);
          break;
        }
        case OP_dread: {
          DreadNode *dr = static_cast<maple::DreadNode *>(bn);
          node = new DassignNode(dr->GetPrimType(), node, dr->stIdx, dr->fieldID);
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

  BaseNode *node = nullptr;
  switch (lhs->op) {
    case OP_iread: {
      IreadNode *ir = static_cast<maple::IreadNode *>(lhs);
      node = new IassignNode(ir->tyIdx, ir->fieldID, ir->uOpnd, rhs);
      break;
    }
    case OP_dread: {
      DreadNode *dr = static_cast<maple::DreadNode *>(lhs);
      node = new DassignNode(dr->GetPrimType(), rhs, dr->stIdx, dr->fieldID);
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
  maple::BaseNode *result = new BinaryNode(op, lhs->GetPrimType(), lhs, rhs);
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

