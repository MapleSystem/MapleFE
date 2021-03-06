/*
* Copyright (C) [2020] Futurewei Technologies, Inc. All rights reverved.
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
#include "mir_function.h"
#include "maplefe_mir_builder.h"

namespace maplefe {

static unsigned mVarUniqNum;

A2M::A2M(const char *filename) : mFileName(filename) {
  mMirModule = new maple::MIRModule(mFileName);
  maple::theMIRModule = mMirModule;
  mMirBuilder = new FEMIRBuilder(mMirModule);
  mFieldData = new FieldData();
  mVarUniqNum = 1;
  Init();
}

A2M::~A2M() {
  delete mMirModule;
  delete mMirBuilder;
  delete mFieldData;
  mNodeTypeMap.clear();
}

void A2M::Init() {
  // create mDefaultType
  maple::MIRType *type = maple::GlobalTables::GetTypeTable().GetOrCreateClassType("DEFAULT_TYPE", *mMirModule);
  type->SetMIRTypeKind(maple::kTypeClass);
  mDefaultType = maple::GlobalTables::GetTypeTable().GetOrCreatePointerType(*type);

  // setup flavor and srclang
  mMirModule->SetFlavor(maple::kFeProduced);
  mMirModule->SetSrcLang(maple::kSrcLangJava);

  // setup INFO_filename
  maple::GStrIdx idx = maple::GlobalTables::GetStrTable().GetOrCreateStrIdxFromName(mFileName);
  SET_INFO_PAIR(mMirModule, "INFO_filename", idx.GetIdx(), true);

  // add to java src file list
  std::string str(mFileName);
  size_t pos = str.rfind('/');
  if (pos != std::string::npos) {
    idx = maple::GlobalTables::GetStrTable().GetOrCreateStrIdxFromName(str.substr(pos+1));
  }
  mMirModule->PushbackFileInfo(maple::MIRInfoPair(idx, 2));
}

// starting point of AST to MPL process
void A2M::ProcessAST(bool trace_a2m) {
  mTraceA2m = trace_a2m;
  if (mTraceA2m) std::cout << "============= in ProcessAST ===========" << std::endl;
  // pass 1: collect class/interface/function decl
  for(auto it: gModule.mTrees) {
    TreeNode *tnode = it->mRootNode;
    ProcessNodeDecl(SK_Stmt, tnode, nullptr);
  }
  // pass 2: handle function def
  for(auto it: gModule.mTrees) {
    TreeNode *tnode = it->mRootNode;
    if (mTraceA2m) { tnode->Dump(0); fflush(0); }
    ProcessNode(SK_Stmt, tnode, nullptr);
  }
}

maple::MIRType *A2M::MapType(TreeNode *type) {
  if (!type) {
    return maple::GlobalTables::GetTypeTable().GetVoid();
  }

  maple::MIRType *mir_type = mDefaultType;

  const char *name = type->GetName();
  if (mNodeTypeMap.find(name) != mNodeTypeMap.end()) {
    return mNodeTypeMap[name];
  }

  if (type->IsPrimType()) {
    PrimTypeNode *ptnode = static_cast<PrimTypeNode *>(type);
    mir_type = MapPrimType(ptnode);

    // update mNodeTypeMap
    mNodeTypeMap[name] = mir_type;
  } else  if (type->IsUserType()) {
    if (type->IsIdentifier()) {
      IdentifierNode *inode = static_cast<IdentifierNode *>(type);
      mir_type = MapType(inode->GetType());
    } else if (type->IsLiteral()) {
      NOTYETIMPL("MapType IsUserType IsLiteral");
    } else {
      NOTYETIMPL("MapType IsUserType");
    }
    // DimensionNode *mDims
    // unsigned dnum = inode->GetDimsNum();
    mNodeTypeMap[name] = mir_type;
  } else {
    NOTYETIMPL("MapType Unknown");
  }
  return mir_type;
}

maple::Opcode A2M::MapUnaOpcode(OprId ast_op) {
  maple::Opcode op = maple::OP_undef;
  switch (ast_op) {
    case OPR_Add: op = maple::OP_add; break;
    case OPR_Sub: op = maple::OP_neg; break;
    case OPR_Inc: op = maple::OP_incref; break;
    case OPR_Dec: op = maple::OP_decref; break;
    case OPR_Bcomp: op = maple::OP_bnot; break;
    case OPR_Not: op = maple::OP_lnot; break;
    default: break;
  }
  return op;
}

maple::Opcode A2M::MapBinOpcode(OprId ast_op) {
  maple::Opcode op = maple::OP_undef;
  switch (ast_op) {
    case OPR_Add: op = maple::OP_add; break;
    case OPR_Sub: op = maple::OP_sub; break;
    case OPR_Mul: op = maple::OP_mul; break;
    case OPR_Div: op = maple::OP_div; break;
    case OPR_Mod: op = maple::OP_rem; break;
    case OPR_Band: op = maple::OP_band; break;
    case OPR_Bor: op = maple::OP_bior; break;
    case OPR_Bxor: op = maple::OP_bxor; break;
    case OPR_Shl: op = maple::OP_shl; break;
    case OPR_Shr: op = maple::OP_ashr; break;
    case OPR_Zext: op = maple::OP_zext; break;
    case OPR_Land: op = maple::OP_land; break;
    case OPR_Lor: op = maple::OP_lior; break;
    default: break;
  }
  return op;
}

maple::Opcode A2M::MapBinCmpOpcode(OprId ast_op) {
  maple::Opcode op = maple::OP_undef;
  switch (ast_op) {
    case OPR_EQ: op = maple::OP_eq; break;
    case OPR_NE: op = maple::OP_ne; break;
    case OPR_GT: op = maple::OP_gt; break;
    case OPR_LT: op = maple::OP_lt; break;
    case OPR_GE: op = maple::OP_ge; break;
    case OPR_LE: op = maple::OP_le; break;
    default: break;
  }
  return op;
}

maple::Opcode A2M::MapBinComboOpcode(OprId ast_op) {
  maple::Opcode op = maple::OP_undef;
  switch (ast_op) {
    case OPR_AddAssign: op = maple::OP_add; break;
    case OPR_SubAssign: op = maple::OP_sub; break;
    case OPR_MulAssign: op = maple::OP_mul; break;
    case OPR_DivAssign: op = maple::OP_div; break;
    case OPR_ModAssign: op = maple::OP_rem; break;
    case OPR_ShlAssign: op = maple::OP_shl; break;
    case OPR_ShrAssign: op = maple::OP_ashr; break;
    case OPR_BandAssign: op = maple::OP_band; break;
    case OPR_BorAssign: op = maple::OP_bior; break;
    case OPR_BxorAssign: op = maple::OP_bxor; break;
    case OPR_ZextAssign: op = maple::OP_zext; break;
    default:
      break;
  }
  return op;
}

const char *A2M::Type2Label(const maple::MIRType *type) {
  maple::PrimType pty = type->GetPrimType();
  switch (pty) {
    case maple::PTY_u1:   return "Z";
    case maple::PTY_i8:   return "C";
    case maple::PTY_u8:   return "UC";
    case maple::PTY_i16:  return "S";
    case maple::PTY_u16:  return "US";
    case maple::PTY_i32:  return "I";
    case maple::PTY_u32:  return "UI";
    case maple::PTY_i64:  return "J";
    case maple::PTY_u64:  return "UJ";
    case maple::PTY_f32:  return "F";
    case maple::PTY_f64:  return "D";
    case maple::PTY_void: return "V";
    default:       return "L";
  }
}

bool A2M::IsStmt(TreeNode *tnode) {
  bool status = true;
  if (!tnode) return false;

  switch (tnode->GetKind()) {
    case NK_Literal:
    case NK_Identifier:
      status = false;
      break;
    case NK_Parenthesis: {
      ParenthesisNode *node = static_cast<ParenthesisNode *>(tnode);
      status = IsStmt(node->GetExpr());
      break;
    }
    case NK_UnaOperator: {
      UnaOperatorNode *node = static_cast<UnaOperatorNode *>(tnode);
      OprId ast_op = node->GetOprId();
      if (ast_op != OPR_Inc && ast_op != OPR_Dec) {
        status = false;
      }
    }
    case NK_BinOperator: {
      BinOperatorNode *bon = static_cast<BinOperatorNode *>(tnode);
      maple::Opcode op = MapBinComboOpcode(bon->mOprId);
      if (bon->mOprId != OPR_Assign && op == maple::OP_undef) {
        status = false;
      }
      break;
    }
    default:
      break;
  }
  return status;
}

#define USE_SHORT 1

// used to form mangled function name
#if USE_SHORT
#define CLASSEND ";"
#define SEP      "|"
#define LARG     "_"   // "("
#define RARG     "_"   // ")"
#else
#define CLASSEND "_3B" // ";"
#define SEP      "_7C" // "|"
#define LARG     "_28" // "("
#define RARG     "_29" // ")"
#endif

void A2M::Type2Name(std::string &str, const maple::MIRType *type) {
  maple::PrimType pty = type->GetPrimType();
  const char *n = Type2Label(type);
  str.append(n);
  if (n[0] == 'L') {
    while (type->GetPrimType() == maple::PTY_ptr || type->GetPrimType() == maple::PTY_ref) {
      const maple::MIRPtrType *ptype = static_cast<const maple::MIRPtrType *>(type);
      type = ptype->GetPointedType();
    }
    str.append(type->GetName());
    str.append(CLASSEND);
  }
}

// update to use uniq name: str --> str|mVarUniqNum
void A2M::UpdateUniqName(std::string &str) {
  str.append(SEP);
  str.append(std::to_string(mVarUniqNum));
  mVarUniqNum++;
  return;
}

// update to use mangled name: className|funcName|(argTypes)retType
void A2M::UpdateFuncName(maple::MIRFunction *func) {
  std::string str;
  maple::TyIdx tyIdx = func->GetClassTyIdx();
  maple::MIRType *type;
  if (tyIdx.GetIdx() != 0) {
    type = maple::GlobalTables::GetTypeTable().GetTypeFromTyIdx(tyIdx);
    str.append(type->GetName());
    str.append(SEP);
  }
  str.append(func->GetName());
  str.append(SEP);
  str.append(LARG);
  maple::GStrIdx stridx = maple::GlobalTables::GetStrTable().GetOrCreateStrIdxFromName("this");
  for (size_t i = 0; i < func->GetFormalCount(); i++) {
    maple::FormalDef def = func->GetFormalDefAt(i);
    // exclude type of extra "this" in the function name
    if (stridx == def.formalStrIdx) continue;
    maple::MIRType *type = maple::GlobalTables::GetTypeTable().GetTypeFromTyIdx(def.formalTyIdx);
    Type2Name(str, type);
  }
  str.append(RARG);
  type = func->GetReturnType();
  Type2Name(str, type);

  maple::MIRSymbol *funcst = maple::GlobalTables::GetGsymTable().GetSymbolFromStidx(func->GetStIdx().Idx());
  // remove old entry in strIdxToStIdxMap
  maple::GlobalTables::GetGsymTable().RemoveFromStringSymbolMap(*funcst);
  AST2MPLMSG("UpdateFuncName()", str);
  stridx = maple::GlobalTables::GetStrTable().GetOrCreateStrIdxFromName(str);
  funcst->SetNameStrIdx(stridx);
  // add new entry in strIdxToStIdxMap
  maple::GlobalTables::GetGsymTable().AddToStringSymbolMap(*funcst);
}

BlockNode *A2M::GetSuperBlock(BlockNode *block) {
  TreeNode *blk = block->GetParent();
  while (blk && !blk->IsBlock()) {
    blk = blk->GetParent();
  }
  return (BlockNode*)blk;
}

maple::MIRSymbol *A2M::GetSymbol(TreeNode *tnode, BlockNode *block) {
  const char *name = tnode->GetName();
  maple::MIRSymbol *symbol = nullptr;

  // global symbol
  if (!block) {
    std::pair<const char *, BlockNode*> P(tnode->GetName(), block);
    symbol = mNameBlockVarMap[P];
    return symbol;
  }

  // trace block hirachy for defined symbol
  BlockNode *blk = block;
  do {
    std::pair<const char *, BlockNode*> P(tnode->GetName(), blk);
    symbol = mNameBlockVarMap[P];
    if (symbol) {
      return symbol;
    }
    blk = GetSuperBlock(blk);
  } while (blk);

  // check parameters
  maple::MIRFunction *func = GetFunc(blk);
  if (!func) {
    NOTYETIMPL("Block parent hirachy");
    return symbol;
  }
  maple::GStrIdx stridx = maple::GlobalTables::GetStrTable().GetOrCreateStrIdxFromName(name);
  if (func->IsAFormalName(stridx)) {
    maple::FormalDef def = func->GetFormalFromName(stridx);
    return def.formalSym;
  }
  return symbol;
}

maple::MIRSymbol *A2M::CreateTempVar(const char *prefix, maple::MIRType *type) {
  if (!type) {
    return nullptr;
  }
  std::string str(prefix);
  str.append(SEP);
  str.append(std::to_string(mVarUniqNum));
  mVarUniqNum++;
  maple::MIRFunction *func = mMirModule->CurFunction();
  maple::MIRSymbol *var = mMirBuilder->CreateLocalDecl(str, *type);
  return var;
}

maple::MIRSymbol *A2M::CreateSymbol(TreeNode *tnode, BlockNode *block) {
  const char *name = tnode->GetName();
  maple::MIRType *mir_type;

  if (tnode->IsIdentifier()) {
    IdentifierNode *inode = static_cast<IdentifierNode *>(tnode);
    mir_type = MapType(inode->GetType());
  } else if (tnode->IsLiteral()) {
    NOTYETIMPL("CreateSymbol LiteralNode()");
    mir_type = mDefaultType;
  }

  // always use pointer type for classes, with PTY_ref
  maple::MIRTypeKind kind = mir_type->GetKind();
  if (kind == maple::kTypeClass || kind == maple::kTypeClassIncomplete ||
      kind == maple::kTypeInterface || kind == maple::kTypeInterfaceIncomplete) {
    mir_type = maple::GlobalTables::GetTypeTable().GetOrCreatePointerType(*mir_type, maple::PTY_ref);
  }

  maple::MIRSymbol *symbol = nullptr;
  if (block) {
    maple::MIRFunction *func = GetFunc(block);
    symbol = mMirBuilder->GetLocalDecl(name);
    std::string str(name);
    // symbol with same name already exist, use a uniq new name
    if (symbol) {
      UpdateUniqName(str);
    }
    symbol = mMirBuilder->CreateLocalDecl(str, *mir_type);
  } else {
    symbol = mMirBuilder->GetGlobalDecl(name);
    std::string str(name);
    // symbol with same name already exist, use a uniq new name
    if (symbol) {
      UpdateUniqName(str);
    }
    symbol = mMirBuilder->CreateGlobalDecl(str, *mir_type, maple::kScGlobal);
  }

  std::pair<const char *, BlockNode*> P(name, block);
  mNameBlockVarMap[P] = symbol;

  return symbol;
}

maple::MIRFunction *A2M::GetFunc(BlockNode *block) {
  maple::MIRFunction *func = nullptr;
  // func = mBlockFuncMap[block];
  func = mMirModule->CurFunction();
  return func;
}

maple::MIRClassType *A2M::GetClass(BlockNode *block) {
  maple::TyIdx tyidx = GetFunc(block)->GetClassTyIdx();
  return (maple::MIRClassType*)maple::GlobalTables::GetTypeTable().GetTypeFromTyIdx(tyidx);
}

maple::MIRFunction *A2M::SearchFunc(const char *name, const maple::MapleVector<maple::BaseNode *> &args) {
  if (mNameFuncMap.find(name) == mNameFuncMap.end()) {
    return nullptr;
  }
  for (auto it: mNameFuncMap[name]) {
    if (it->GetFormalCount() != args.size()) {
      continue;
    }
    bool matched = true;
    for (int i = 0; i < it->GetFormalCount(); i++) {
      // TODO: allow compatible types
      maple::MIRType *type = maple::GlobalTables::GetTypeTable().GetTypeFromTyIdx(it->GetFormalDefAt(i).formalTyIdx);
      if (type->GetPrimType() != args[i]->GetPrimType()) {
        matched = false;
        break;
      }
    }
    if (!matched) {
      continue;
    }
    return it;
  }
  return nullptr;
}

void A2M::MapAttr(maple::GenericAttrs &attr, const IdentifierNode *inode) {
  // SmallVector<AttrId> mAttrs
  unsigned anum = inode->GetAttrsNum();
  for (int i = 0; i < anum; i++) {
  }
}
}

