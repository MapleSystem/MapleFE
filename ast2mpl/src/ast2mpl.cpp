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
#include "gen_astdump.h"
#include "mir_function.h"
#include "maplefe_mir_builder.h"
#include "cvt_block.h"

namespace maplefe {

A2M::A2M(ModuleNode *m) : mASTModule(m) {
  mFilename = mASTModule->GetFilename();
  mMirModule = new maple::MIRModule(mFilename);
  maple::theMIRModule = mMirModule;
  mMirBuilder = new FEMIRBuilder(mMirModule);
  mFieldData = new FieldData();
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
  mDefaultType = mMirBuilder->GetOrCreatePointerType(type);

  // setup flavor and srclang
  mMirModule->SetFlavor(maple::kFeProduced);
  mMirModule->SetSrcLang(maple::kSrcLangJava);

  // setup INFO_filename
  maple::GStrIdx idx = maple::GlobalTables::GetStrTable().GetOrCreateStrIdxFromName(mFilename);
  SET_INFO_PAIR(mMirModule, "INFO_filename", idx.GetIdx(), true);

  // add to src file list
  std::string str(mFilename);
  size_t pos = str.rfind('/');
  if (pos != std::string::npos) {
    idx = maple::GlobalTables::GetStrTable().GetOrCreateStrIdxFromName(str.substr(pos+1));
  }
  mMirModule->PushbackFileInfo(maple::MIRInfoPair(idx, 2));

  // initialize unique serial number for temporary variables and inner classes
  mUniqNum = 1;
}

// starting point of AST to MPL process
void A2M::ProcessAST(bool trace_a2m) {
  mTraceA2m = trace_a2m;
  AstDump astdump(mASTModule);

  if (mTraceA2m) {
    std::cout << "============= in ProcessAST ===========" << std::endl;
    std::cout << "srcLang : " << mASTModule->GetSrcLangString() << std::endl;
  }
  // pass 0: convert to use BlockNode for if-then-else and loop bodies
  CvtToBlockVisitor visitor(mASTModule);
  visitor.CvtToBlock();

  // pass 1: collect class/interface/function decl
  for (unsigned i = 0; i < mASTModule->GetTreesNum(); i++) {
    TreeNode *tnode = mASTModule->GetTree(i);
    ProcessNodeDecl(SK_Stmt, tnode, nullptr);
  }
  // pass 2: handle function def
  for (unsigned i = 0; i < mASTModule->GetTreesNum(); i++) {
    TreeNode *tnode = mASTModule->GetTree(i);
    ProcessNode(SK_Stmt, tnode, nullptr);
  }
  if (mTraceA2m) { astdump.Dump("ProcessAST", &std::cout); }
}

maple::MIRType *A2M::MapType(TreeNode *type) {
  if (!type) {
    return maple::GlobalTables::GetTypeTable().GetVoid();
  }

  maple::MIRType *mir_type = mDefaultType;

  unsigned idx = type->GetStrIdx();
  if (mNodeTypeMap.find(idx) != mNodeTypeMap.end()) {
    return mNodeTypeMap[idx];
  }

  if (type->IsPrimType()) {
    PrimTypeNode *ptnode = static_cast<PrimTypeNode *>(type);
    mir_type = MapPrimType(ptnode);

    // update mNodeTypeMap
    mNodeTypeMap[idx] = mir_type;
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
    mNodeTypeMap[idx] = mir_type;
  } else if (idx) {
    AST2MPLMSG("MapType add a class type by idx", idx);
    mir_type = maple::GlobalTables::GetTypeTable().GetOrCreateClassType(type->GetName(), *mMirModule);
    mir_type->SetMIRTypeKind(maple::kTypeClass);
    mir_type = mMirBuilder->GetOrCreatePointerType(mir_type);
    mNodeTypeMap[idx] = mir_type;
  } else {
    NOTYETIMPL("MapType unknown type");
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
      break;
    }
    case NK_BinOperator: {
      BinOperatorNode *bon = static_cast<BinOperatorNode *>(tnode);
      maple::Opcode op = MapBinComboOpcode(bon->GetOprId());
      if (bon->GetOprId() != OPR_Assign && op == maple::OP_undef) {
        status = false;
      }
      break;
    }
    case NK_Block:
    case NK_Break:
    case NK_Call:
    case NK_CondBranch:
    case NK_Delete:
    case NK_DoLoop:
    case NK_ExprList:
    case NK_ForLoop:
    case NK_Function:
    case NK_Interface:
    case NK_Lambda:
    case NK_New:
    case NK_Return:
    case NK_Switch:
    case NK_SwitchCase:
    case NK_VarList:
    case NK_WhileLoop:
      status = true;
      break;
    case NK_Annotation:
    case NK_AnnotationType:
    case NK_Assert:
    case NK_Attr:
    case NK_Cast:
    case NK_Class:
    case NK_Dimension:
    case NK_Exception:
    case NK_Field:
    case NK_Identifier:
    case NK_Import:
    case NK_InstanceOf:
    case NK_Literal:
    case NK_Package:
    case NK_Pass:
    case NK_PrimArrayType:
    case NK_PrimType:
    case NK_SwitchLabel:
    case NK_TerOperator:
    case NK_UserType:
    default:
      status = false;
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

// update to use uniq name: str --> str|mUniqNum
void A2M::UpdateUniqName(std::string &str) {
  str.append(SEP);
  str.append(std::to_string(mUniqNum++));
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

ClassNode *A2M::GetSuperClass(ClassNode *klass) {
  TreeNode *tnode = klass->GetParent();
  while (tnode && !tnode->IsClass()) {
    tnode = tnode->GetParent();
  }
  return static_cast<ClassNode *>(tnode);
}

BlockNode *A2M::GetSuperBlock(BlockNode *block) {
  TreeNode *tnode = block->GetParent();
  while (tnode && !tnode->IsBlock()) {
    tnode = tnode->GetParent();
  }
  return static_cast<BlockNode *>(tnode);
}

maple::MIRSymbol *A2M::GetSymbol(TreeNode *tnode, BlockNode *block) {
  unsigned idx = tnode->GetStrIdx();
  maple::MIRSymbol *symbol = nullptr;

  // global symbol
  if (!block) {
    std::pair<unsigned, BlockNode*> P(idx, block);
    symbol = mNameBlockVarMap[P];
    return symbol;
  }

  // trace block hirachy for defined symbol
  BlockNode *blk = block;
  do {
    std::pair<unsigned, BlockNode*> P(idx, blk);
    symbol = mNameBlockVarMap[P];
    if (symbol) {
      return symbol;
    }
    blk = GetSuperBlock(blk);
  } while (blk);

  // check parameters
  maple::MIRFunction *func = GetCurrFunc(blk);
  if (!func) {
    NOTYETIMPL("Block parent hirachy");
    return symbol;
  }
  maple::GStrIdx stridx = maple::GlobalTables::GetStrTable().GetOrCreateStrIdxFromName(tnode->GetName());
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
  str.append(std::to_string(mUniqNum++));
  maple::MIRFunction *func = mMirModule->CurFunction();
  maple::MIRSymbol *var = mMirBuilder->CreateLocalDecl(str, *type);
  return var;
}

maple::MIRSymbol *A2M::CreateSymbol(TreeNode *tnode, BlockNode *block) {
  std::string name = tnode->GetName();
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
    mir_type = mMirBuilder->GetOrCreatePointerType(mir_type);
  }

  maple::MIRSymbol *symbol = nullptr;
  if (block) {
    maple::MIRFunction *func = GetCurrFunc(block);
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

  std::pair<unsigned, BlockNode*> P(tnode->GetStrIdx(), block);
  mNameBlockVarMap[P] = symbol;

  return symbol;
}

maple::MIRFunction *A2M::GetCurrFunc(BlockNode *block) {
  maple::MIRFunction *func = nullptr;
  // func = mBlockFuncMap[block];
  func = mMirModule->CurFunction();
  return func;
}

maple::MIRClassType *A2M::GetClass(BlockNode *block) {
  maple::TyIdx tyidx = GetCurrFunc(block)->GetClassTyIdx();
  return (maple::MIRClassType*)maple::GlobalTables::GetTypeTable().GetTypeFromTyIdx(tyidx);
}

bool3 A2M::IsCompatibleTo(maple::PrimType expected, maple::PrimType prim) {
  if (expected == prim)
    return true3;

  maple::PrimitiveType type(prim);
  bool3 comp = false3;
  switch (expected) {
    case maple::PTY_i8:
    case maple::PTY_i16:
    case maple::PTY_i32:
    case maple::PTY_i64:
    case maple::PTY_u8:
    case maple::PTY_u16:
    case maple::PTY_u32:
    case maple::PTY_u64:
    case maple::PTY_u1:
      if (type.IsInteger()) {
        comp = true3;
      }
      break;
    case maple::PTY_ptr:
    case maple::PTY_ref:
      if (type.IsPointer()) {
        comp = true3;
      }
      if (type.IsInteger()) {
        comp = maybe3;
      }
      break;
    case maple::PTY_a32:
    case maple::PTY_a64:
      if (type.IsAddress()) {
        comp = true3;
      }
      break;
    case maple::PTY_f32:
    case maple::PTY_f64:
    case maple::PTY_f128:
      if (type.IsFloat()) {
        comp = true3;
      }
      break;
    case maple::PTY_c64:
    case maple::PTY_c128:
      if (type.IsInteger()) {
        comp = true3;
      }
      break;
    case maple::PTY_constStr:
    case maple::PTY_gen:
    case maple::PTY_agg:
    case maple::PTY_unknown:
    case maple::PTY_v2i64:
    case maple::PTY_v4i32:
    case maple::PTY_v8i16:
    case maple::PTY_v16i8:
    case maple::PTY_v2f64:
    case maple::PTY_v4f32:
    case maple::PTY_void:
    default:
      break;
  }
  return comp;
}

maple::MIRFunction *A2M::SearchFunc(unsigned idx, maple::MapleVector<maple::BaseNode *> &args) {
  if (mNameFuncMap.find(idx) == mNameFuncMap.end()) {
    return nullptr;
  }
  std::vector<maple::MIRFunction *> candidates;
  for (auto it: mNameFuncMap[idx]) {
    if (it->GetFormalCount() != args.size()) {
      continue;
    }
    bool matched = true;
    bool3 mightmatched = true3;
    for (int i = 0; i < it->GetFormalCount(); i++) {
      maple::MIRType *type = maple::GlobalTables::GetTypeTable().GetTypeFromTyIdx(it->GetFormalDefAt(i).formalTyIdx);
      bool3 comp = IsCompatibleTo(type->GetPrimType(), args[i]->GetPrimType());
      if (comp == false3) {
        matched = false;
        mightmatched = false3;
        break;
      } else if (comp == maybe3) {
        matched = false;
      }
    }
    if (matched) {
      return it;
    }
    if (mightmatched != false3) {
      candidates.push_back(it);
    }
  }
  if (candidates.size()) {
    return candidates[0];
  }
  return nullptr;
}

maple::MIRFunction *A2M::SearchFunc(TreeNode *method, maple::MapleVector<maple::BaseNode *> &args, BlockNode *block) {
  maple::MIRFunction *func = nullptr;
  switch (method->GetKind()) {
    case NK_Function: {
      func = SearchFunc(method->GetStrIdx(), args);
      break;
    }
    case NK_Identifier: {
      IdentifierNode *imethod = static_cast<IdentifierNode *>(method);
      func = SearchFunc(imethod->GetStrIdx(), args);
      break;
    }
    case NK_Field: {
      FieldNode *node = static_cast<FieldNode *>(method);
      TreeNode *upper = node->GetUpper();
      TreeNode *field = node->GetField();
      if (field->IsIdentifier()) {
        // pass upper as this
        maple::BaseNode *bn = ProcessNode(SK_Expr, upper, block);
        if (bn) {
          args[0] = bn;
        }
      }
      func = SearchFunc(field, args, block);
      break;
    }
    default:
      NOTYETIMPL("GetFuncName() method to be handled");
  }

  return func;
}

void A2M::MapAttr(maple::GenericAttrs &attr, AttrId id) {
  switch (id) {
#undef ATTRIBUTE
#define ATTRIBUTE(X) case ATTR_##X: attr.SetAttr(maple::GENATTR_##X); break;
// #include "supported_attributes.def"
ATTRIBUTE(abstract)
ATTRIBUTE(const)
ATTRIBUTE(volatile)
ATTRIBUTE(final)
ATTRIBUTE(native)
ATTRIBUTE(private)
ATTRIBUTE(protected)
ATTRIBUTE(public)
ATTRIBUTE(static)
ATTRIBUTE(default)
ATTRIBUTE(synchronized)

// ATTRIBUTE(strictfp)
    case ATTR_strictfp: attr.SetAttr(maple::GENATTR_strict); break;

    default:
      break;
  }
}

void A2M::MapAttr(maple::GenericAttrs &attr, IdentifierNode *inode) {
  // SmallVector<AttrId> mAttrs
  unsigned anum = inode->GetAttrsNum();
  for (int i = 0; i < anum; i++) {
    const AttrId ast_attr = inode->GetAttrAtIndex(i);
    MapAttr(attr, ast_attr);
  }
}

void A2M::MapAttr(maple::GenericAttrs &attr, FunctionNode *fnode) {
  unsigned anum = fnode->GetAttrsNum();
  for (int i = 0; i < anum; i++) {
    const AttrId ast_attr = fnode->GetAttrAtIndex(i);
    MapAttr(attr, ast_attr);
  }
}
}

