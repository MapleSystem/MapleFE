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
#include "constant_fold.h"

namespace maplefe {

A2M::A2M(const char *filename) : mFileName(filename) {
  mMirModule = new maple::MIRModule(mFileName);
  mMirBuilder = mMirModule->mirBuilder;
  mDefaultType = GlobalTables::GetTypeTable().GetOrCreateClassType("DEFAULT_TYPE", mMirModule);
  mDefaultType->typeKind = maple::MIRTypeKind::kTypeClass;
}

A2M::~A2M() {
  delete mMirModule;
  mNodeTypeMap.clear();
}

// starting point of AST to MPL process
void A2M::ProcessAST(bool trace_a2m) {
  mTraceA2m = trace_a2m;
  if (mTraceA2m) std::cout << "============= in ProcessAST ===========" << std::endl;
  for(auto it: gModule.mTrees) {
    TreeNode *tnode = it->mRootNode;
    if (mTraceA2m) { tnode->Dump(0); fflush(0); }
    ProcessNode(SK_Stmt, tnode, nullptr);
  }
}

MIRType *A2M::MapType(TreeNode *type) {
  MIRType *mir_type = mDefaultType;
  if (!type) {
    return mir_type;
  }

  char *name = type->GetName();
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

maple::Opcode A2M::MapOpcode(OprId ast_op) {
  maple::Opcode op = maple::kOpUndef;
  switch (ast_op) {
    case OPR_Add: op = OP_add; break;
    case OPR_Sub: op = OP_sub; break;
    case OPR_Mul: op = OP_mul; break;
    case OPR_Div: op = OP_div; break;
    case OPR_Mod: op = OP_rem; break;
    case OPR_EQ: op = OP_eq; break;
    case OPR_NE: op = OP_ne; break;
    case OPR_GT: op = OP_gt; break;
    case OPR_LT: op = OP_lt; break;
    case OPR_GE: op = OP_ge; break;
    case OPR_LE: op = OP_le; break;
    case OPR_Band: op = OP_band; break;
    case OPR_Bor: op = OP_bior; break;
    case OPR_Shl: op = OP_shl; break;
    case OPR_Shr: op = OP_ashr; break;
    case OPR_Zext: op = OP_zext; break;
    case OPR_Land: op = OP_land; break;
    case OPR_Lor: op = OP_lior; break;
    default: break;
  }
  return op;
}

maple::Opcode A2M::MapComboOpcode(OprId ast_op) {
  maple::Opcode op = maple::kOpUndef;
  switch (ast_op) {
    case OPR_AddAssign: op = OP_add; break;
    case OPR_SubAssign: op = OP_sub; break;
    case OPR_MulAssign: op = OP_mul; break;
    case OPR_DivAssign: op = OP_div; break;
    case OPR_ModAssign: op = OP_rem; break;
    case OPR_ShlAssign: op = OP_shl; break;
    case OPR_ShrAssign: op = OP_ashr; break;
    case OPR_BandAssign: op = OP_band; break;
    case OPR_BorAssign: op = OP_bior; break;
    case OPR_BxorAssign: op = OP_bxor; break;
    case OPR_ZextAssign: op = OP_zext; break;
    default:
      break;
  }
  return op;
}

const char *A2M::Type2Label(const MIRType *type) {
  PrimType pty = type->GetPrimType();
  switch (pty) {
    case PTY_u1:   return "Z";
    case PTY_i8:   return "C";
    case PTY_u8:   return "UC";
    case PTY_i16:  return "S";
    case PTY_u16:  return "US";
    case PTY_i32:  return "I";
    case PTY_u32:  return "UI";
    case PTY_i64:  return "J";
    case PTY_u64:  return "UJ";
    case PTY_f32:  return "F";
    case PTY_f64:  return "D";
    case PTY_void: return "V";
    default:       return "L";
  }
}

// used to form mangled function name
#define CLASSEND "_3B" // ';'
#define SEP      "_7C" // '|'
#define LARG     "_28" // '('
#define RARG     "_29" // ')'

const char *A2M::Type2Name(const MIRType *type) {
  PrimType pty = type->GetPrimType();
  std::string str;
  const char *n = Type2Label(type);
  str.append(n);
  if (n[0] == 'L') {
    while (type->GetPrimType() == PTY_ptr) {
      const MIRPtrType *ptype = static_cast<const MIRPtrType *>(type);
      type = ptype->GetPointedType();
    }
    str.append(type->GetName());
    str.append(CLASSEND);
  }
  return str.c_str();
}

// update to use mangled name: className|funcName|(argTypes)retType
void A2M::UpdateFuncName(MIRFunction *func) {
  std::string str;
  TyIdx tyIdx = func->GetClassTyIdx();
  MIRType *type;
  if (tyIdx.GetIdx() != 0) {
    type = GlobalTables::GetTypeTable().GetTypeFromTyIdx(tyIdx);
    str.append(Type2Name(type));
    str.append(SEP);
  }
  str.append(func->GetName());
  str.append(SEP);
  str.append(LARG);
  for (auto def: func->formalDefVec) {
    MIRType *type = GlobalTables::GetTypeTable().GetTypeFromTyIdx(def.formalTyIdx);
    str.append(Type2Name(type));
  }
  str.append(RARG);
  type = func->GetReturnType();
  str.append(Type2Name(type));
  MIRSymbol *funcst = GlobalTables::GetGsymTable().GetSymbolFromStIdx(func->stIdx.Idx());
  GStrIdx stridx = GlobalTables::GetStrTable().GetOrCreateStrIdxFromName(str);
  funcst->SetNameStridx(stridx);
}

MIRSymbol *A2M::MapGlobalSymbol(TreeNode *tnode) {
  const char *name = tnode->GetName();
  MIRType *mir_type;

  if (tnode->IsIdentifier()) {
    IdentifierNode *inode = static_cast<IdentifierNode *>(tnode);
    mir_type = MapType(inode->GetType());
  } else if (tnode->IsLiteral()) {
    NOTYETIMPL("MapLocalSymbol LiteralNode()");
    mir_type = mDefaultType;
  }

  MIRSymbol *symbol = mMirBuilder->GetOrCreateGlobalDecl(name, mir_type);
  return symbol;
}

MIRSymbol *A2M::MapLocalSymbol(TreeNode *tnode, maple::MIRFunction *func) {
  const char *name = tnode->GetName();
  MIRType *mir_type;

  if (tnode->IsIdentifier()) {
    IdentifierNode *inode = static_cast<IdentifierNode *>(tnode);
    mir_type = MapType(inode->GetType());
  } else if (tnode->IsLiteral()) {
    NOTYETIMPL("MapLocalSymbol LiteralNode()");
    mir_type = mDefaultType;
  }

  MIRSymbol *symbol = mMirBuilder->GetOrCreateLocalDecl(name, mir_type, func);
  return symbol;
}

MIRSymbol *A2M::GetSymbol(IdentifierNode *tnode, maple::BlockNode *block) {
  MIRSymbol *symbol;
  if (block) {
    maple::MIRFunction *func = mBlockFuncMap[block];
    symbol = MapLocalSymbol(tnode, func);
  } else {
    symbol = MapGlobalSymbol(tnode);
  }
  return symbol;
}

void A2M::MapAttr(GenericAttrs &attr, const IdentifierNode *inode) {
  // SmallVector<AttrId> mAttrs
  unsigned anum = inode->GetAttrsNum();
  for (int i = 0; i < anum; i++) {
  }
}
}

