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
#include "ast_handler.h"
#include "ast_ti.h"
#include "typetable.h"
#include "gen_astdump.h"

#define ITERATEMAX 10

namespace maplefe {

void TypeInfer::TypeInference() {
  ModuleNode *module = mHandler->GetASTModule();

  if (mFlags & FLG_trace_3)
    mHandler->GetTypeTable()->Dump();

  // build mNodeId2Decl
  MSGNOLOC0("============== Build NodeId2Decl ==============");
  InitDummyNodes();
  BuildIdNodeToDeclVisitor visitor_build(mHandler, mFlags, true);
  visitor_build.Visit(module);

  // type inference
  MSGNOLOC0("============== TypeInfer ==============");
  TypeInferVisitor visitor_ti(mHandler, mFlags, true);
  visitor_ti.SetUpdated(true);
  int count = 0;
  while (visitor_ti.GetUpdated() && count++ <= ITERATEMAX) {
    MSGNOLOC("\n TypeInference iterate ", count);
    visitor_ti.SetUpdated(false);
    visitor_ti.Visit(module);
  }

  if (mFlags & FLG_trace_3) std::cout << "\n>>>>>> TypeInference() iterated " << count << " times\n" << std::endl;

  // share UserType
  MSGNOLOC0("============== Share UserType ==============");
  ShareUTVisitor visitor_ut(mHandler, mFlags, true);
  visitor_ut.Push(module->GetRootScope());
  visitor_ut.Visit(module);

  // Check Type
  MSGNOLOC0("============== Check Type ==============");
  CheckTypeVisitor visitor_check(mHandler, mFlags, true);
  visitor_check.Visit(module);
}

void TypeInfer::InitDummyNodes() {
  // add dummpy console.log()

  // class console
  ClassNode *console = (ClassNode*)gTreePool.NewTreeNode(sizeof(ClassNode));
  new (console) ClassNode();
  unsigned idx1 = gStringPool.GetStrIdx("console");
  console->SetStrIdx(idx1);

  // method log
  FunctionNode *log = (FunctionNode*)gTreePool.NewTreeNode(sizeof(FunctionNode));
  new (log) FunctionNode();
  unsigned idx2 = gStringPool.GetStrIdx("log");
  log->SetStrIdx(idx2);

  IdentifierNode *name = (IdentifierNode*)gTreePool.NewTreeNode(sizeof(IdentifierNode));
  new (name) IdentifierNode(idx2);
  log->SetFuncName(name);

  // add log to console
  console->AddMethod(log);

  // add console and log to root scope
  ModuleNode *module = mHandler->GetASTModule();
  module->GetRootScope()->AddType(console);
  module->GetRootScope()->AddDecl(console);
  module->GetRootScope()->AddDecl(log);
}

// build up mNodeId2Decl by visiting each Identifier
IdentifierNode *BuildIdNodeToDeclVisitor::VisitIdentifierNode(IdentifierNode *node) {
  (void) AstVisitor::VisitIdentifierNode(node);
  (void) mHandler->FindDecl(node);
  return node;
}

TypeId TypeInferVisitor::MergeTypeId(TypeId tia, TypeId tib) {
  if (tia == tib || tib == TY_None) {
    return tia;
  }

  if (tib == TY_Object || tib == TY_User) {
    return tib;
  }

  // tia != tib && tib != TY_None
  TypeId result = TY_None;
  switch (tia) {
    case TY_None:        result = tib;       break;

    case TY_Object:
    case TY_User:        result = tia;       break;

    case TY_Merge:
    case TY_Undefined:
    case TY_String:
    case TY_Function:
    case TY_Class:
    case TY_Array:       result = TY_Merge;  break;

    case TY_Boolean: {
      switch (tib) {
        case TY_Int:
        case TY_Long:
        case TY_Float:
        case TY_Double:  result = tib;       break;
        default:         result = TY_Merge;  break;
      }
      break;
    }
    case TY_Int: {
      switch (tib) {
        case TY_Boolean: result = TY_Int;    break;
        case TY_Long:
        case TY_Float:
        case TY_Double:  result = tib;       break;
        default:         result = TY_Merge;  break;
      }
      break;
    }
    case TY_Long: {
      switch (tib) {
        case TY_Boolean:
        case TY_Int:     result = TY_Long;   break;
        case TY_Float:
        case TY_Double:  result = TY_Double; break;
        default:         result = TY_Merge;  break;
      }
      break;
    }
    case TY_Float: {
      switch (tib) {
        case TY_Boolean:
        case TY_Int:     result = TY_Float;  break;
        case TY_Long:
        case TY_Double:  result = TY_Double; break;
        default:         result = TY_Merge;  break;
      }
      break;
    }
    case TY_Double: {
      switch (tib) {
        case TY_Boolean:
        case TY_Int:
        case TY_Long:
        case TY_Double:  result = TY_Double; break;
        default:         result = TY_Merge;  break;
      }
      break;
    }
    default:
      break;
  }
  if (result == TY_None) {
    NOTYETIMPL("MergeTypeId()");
  }
  if (mFlags & FLG_trace_3) {
    std::cout << " Type Merge: "
              << AstDump::GetEnumTypeId(tia) << " "
              << AstDump::GetEnumTypeId(tib) << " --> "
              << AstDump::GetEnumTypeId(result) << std::endl;
  }
  return result;
}

void TypeInferVisitor::SetTypeId(TreeNode *node, TypeId tid) {
  if (node) {
    if (mFlags & FLG_trace_3) {
      std::cout << " NodeId : " << node->GetNodeId() << " Update TypeId : "
                << AstDump::GetEnumTypeId(node->GetTypeId()) << " --> "
                << AstDump::GetEnumTypeId(tid) << std::endl;
    }
    node->SetTypeId(tid);
  }
}

void TypeInferVisitor::UpdateTypeId(TreeNode *node, TypeId tid) {
  if (tid == TY_None || !node || node->IsLiteral()) {
    return;
  }
  tid = MergeTypeId(node->GetTypeId(), tid);
  if (node->GetTypeId() != tid) {
    if (mFlags & FLG_trace_3) {
      std::cout << " NodeId : " << node->GetNodeId() << " Set TypeId : "
                << AstDump::GetEnumTypeId(node->GetTypeId()) << " --> "
                << AstDump::GetEnumTypeId(tid) << std::endl;
    }
    SetTypeId(node, tid);
    SetUpdated();
  }
}

PrimTypeNode *TypeInferVisitor::GetOrClonePrimTypeNode(PrimTypeNode *pt, TypeId tid) {
  PrimTypeNode *new_pt = pt;
  TypeId oldtid = pt->GetTypeId();
  // merge tids
  tid = MergeTypeId(oldtid, tid);
  // check if we need update
  if (tid != oldtid) {
    // check if we need clone PrimTypeNode to avoid using the shared one
    if (oldtid == TY_None) {
      new_pt = (PrimTypeNode*)gTreePool.NewTreeNode(sizeof(PrimTypeNode));
      new (new_pt) PrimTypeNode();
      new_pt->SetPrimType(pt->GetPrimType());
    }
    SetTypeId(new_pt, tid);
    SetUpdated();
  }
  return new_pt;
}

bool static IsScalar(TypeId tid) {
  switch (tid) {
    case TY_None:
    case TY_Int:
    case TY_Long:
    case TY_Float:
    case TY_Double:
      return true;
    default:
      return false;
  }
  return false;
}

// when function arry type parameter is updated, need update all
// caller arguments to be consistent with the array type parameter
void TypeInferVisitor::UpdateArgArrayDecls(unsigned nid, TypeId tid) {
  for (auto id: mParam2ArgArrayDeclMap[nid]) {
    mHandler->mArrayDeclId2EleTypeIdMap[nid] = tid;
    if (id && id->IsDecl()) {
      id = static_cast<DeclNode *>(id)->GetVar();
    }
    if (id && id->IsIdentifier()) {
      IdentifierNode *inode = static_cast<IdentifierNode *>(id);
      TreeNode *type = inode->GetType();
      if (type && type->IsPrimArrayType()) {
        PrimArrayTypeNode *pat = static_cast<PrimArrayTypeNode *>(type);
        SetTypeId(pat->GetPrim(), tid);
        SetUpdated();
      }
    }
  }
}

// use input node's type info to update target node's type info
void TypeInferVisitor::UpdateTypeUseNode(TreeNode *target, TreeNode *input) {
  TypeId tid = target->GetTypeId();
  TypeId iid = input->GetTypeId();
  if ((tid == iid && IsScalar(tid)) || (iid == TY_Array && tid != iid)) {
    return;
  }
  switch (iid) {
    case TY_None:
      break;
    case TY_Array: {
      // function's formals with corresponding calls' parameters passed in
      if (input->IsIdentifier()) {
        TreeNode *decl = mHandler->FindDecl(static_cast<IdentifierNode *>(input));
        TypeId old_elemTypeId = GetArrayElemTypeId(target);
        TypeId inid = GetArrayElemTypeId(decl);
        if (old_elemTypeId != inid) {
          UpdateArrayElemTypeIdMap(target, inid);
        }
        TypeId new_elemTypeId = GetArrayElemTypeId(target);
        TreeNode *type = static_cast<IdentifierNode *>(target)->GetType();
        MASSERT(target->IsIdentifier() && "target node not identifier");
        if (type->IsPrimArrayType()) {
          unsigned nid = target->GetNodeId();
          mParam2ArgArrayDeclMap[nid].insert(decl);
          if (old_elemTypeId != new_elemTypeId) {
            PrimArrayTypeNode *pat = static_cast<PrimArrayTypeNode *>(type);
            PrimTypeNode *pt = pat->GetPrim();
            PrimTypeNode *new_pt = GetOrClonePrimTypeNode(pt, new_elemTypeId);
            pat->SetPrim(new_pt);
            SetUpdated();

            UpdateArgArrayDecls(nid, new_elemTypeId);
          }
        }
      }
      // function's return type with return statement
      else if (input->IsArrayLiteral()) {
        TypeId old_elemTypeId = GetArrayElemTypeId(target);
        TypeId inid = GetArrayElemTypeId(input);
        if (old_elemTypeId != inid) {
          UpdateArrayElemTypeIdMap(target, inid);
        }
        TypeId new_elemTypeId = GetArrayElemTypeId(target);
        if (target->IsPrimArrayType()) {
          unsigned nid = target->GetNodeId();
          if (old_elemTypeId != new_elemTypeId) {
            PrimArrayTypeNode *pat = static_cast<PrimArrayTypeNode *>(target);
            PrimTypeNode *pt = pat->GetPrim();
            PrimTypeNode *new_pt = GetOrClonePrimTypeNode(pt, new_elemTypeId);
            pat->SetPrim(new_pt);
            SetUpdated();

            UpdateArgArrayDecls(nid, new_elemTypeId);
          }
        }
      } else {
        NOTYETIMPL("parameter not identifier");
      }
      break;
    }
    case TY_Int:
    case TY_Long:
    case TY_Float:
    case TY_Double:
    case TY_String:
    case TY_Class:
    case TY_Object:
    case TY_Function: {
      TypeId merged = MergeTypeId(tid, iid);
      if (merged != tid) {
        SetTypeId(target, merged);
        SetUpdated();
      }
      break;
    }
    default:
      NOTYETIMPL("TypeId not handled");
      break;
  }
  return;
}

void TypeInferVisitor::UpdateFuncRetTypeId(FunctionNode *node, TypeId tid) {
  if (!node || tid == TY_None || node->GetTypeId() == tid) {
    return;
  }
  TreeNode *type = node->GetType();
  // create new return type node if it was shared

  if (type) {
    if (type->IsPrimType() && type->GetTypeId() == TY_None) {
      type = GetOrClonePrimTypeNode((PrimTypeNode *)type, tid);
      node->SetType(type);
    }
    tid = MergeTypeId(type->GetTypeId(), tid);
    SetTypeId(type, tid);
  }
}

TypeId TypeInferVisitor::GetArrayElemTypeId(TreeNode *node) {
  TypeId tid = TY_None;
  unsigned nodeid = node->GetNodeId();
  auto it = mHandler->mArrayDeclId2EleTypeIdMap.find(nodeid);
  if (it != mHandler->mArrayDeclId2EleTypeIdMap.end()) {
    tid = mHandler->mArrayDeclId2EleTypeIdMap[nodeid];
  }
  return tid;
}

void TypeInferVisitor::UpdateArrayElemTypeIdMap(TreeNode *node, TypeId tid) {
  if (!node || tid == TY_None || !IsArray(node)) {
    return;
  }
  unsigned nodeid = node->GetNodeId();
  auto it = mHandler->mArrayDeclId2EleTypeIdMap.find(nodeid);
  if (it != mHandler->mArrayDeclId2EleTypeIdMap.end()) {
    tid = MergeTypeId(tid, mHandler->mArrayDeclId2EleTypeIdMap[nodeid]);
  }
  if (mHandler->mArrayDeclId2EleTypeIdMap[nodeid] != tid) {
    mHandler->mArrayDeclId2EleTypeIdMap[node->GetNodeId()] = tid;
    SetUpdated();

    // update array's PrimType node with a new node
    if (node->IsDecl()) {
      DeclNode *decl = static_cast<DeclNode *>(node);
      node = decl->GetVar();
    }
    if (node->IsIdentifier()) {
      IdentifierNode *in = static_cast<IdentifierNode *>(node);
      node = in->GetType();
    }
    if (node->IsPrimArrayType()) {
      PrimArrayTypeNode *pat = static_cast<PrimArrayTypeNode *>(node);
      PrimTypeNode *pt = pat->GetPrim();
      PrimTypeNode *new_pt = GetOrClonePrimTypeNode(pt, tid);
      pat->SetPrim(new_pt);
    }
  }
}

bool TypeInferVisitor::IsArray(TreeNode *node) {
  if (!node) {
    return false;
  }
  if (node->IsArrayLiteral() || node->IsPrimArrayType()) {
    return true;
  }
  TreeNode *tn = node;
  if (node->IsDecl()) {
    DeclNode *decl = static_cast<DeclNode *>(node);
    tn = decl->GetVar();
  }
  if (tn->IsIdentifier()) {
    IdentifierNode *idnode = static_cast<IdentifierNode *>(tn);
    if (idnode && idnode->GetType() && idnode->GetType()->GetKind() == NK_PrimArrayType) {
      return true;
    }
  } else if (tn->IsBindingPattern()) {
    // could be either object or array destructuring
    return false;
  } else {
    NOTYETIMPL("array not identifier or bind pattern");
  }
  return false;
}

TreeNode *TypeInferVisitor::VisitClassField(TreeNode *node) {
  (void) AstVisitor::VisitTreeNode(node);
  if (node->IsIdentifier()) {
    IdentifierNode *idnode = static_cast<IdentifierNode *>(node);
    TreeNode *type = idnode->GetType();
    if (type) {
      TypeId tid = type->GetTypeId();
      if (type->IsPrimType()) {
        PrimTypeNode *ptn = static_cast<PrimTypeNode *>(type);
        tid = ptn->GetPrimType();
      }
      // use non TY_Number
      if (tid != TY_Number) {
        SetTypeId(node, tid);
      }
    }
    TreeNode *init = idnode->GetInit();
    if (init) {
      VisitTreeNode(init);
      UpdateTypeId(node, init->GetTypeId());
    }
  } else if (node->IsLiteral()) {
    MSGNOLOC0("field is Literal");
  } else if (node->IsComputedName()) {
    MSGNOLOC0("field is ComputedName");
  } else if (node->IsStrIndexSig()) {
    MSGNOLOC0("field is StrIndexSig");
  } else {
    NOTYETIMPL("field new kind");
  }
  return node;
}

// ArrayElementNode are for
// 1. array access
// 2. indexed access of class/structure for fields, field types
ArrayElementNode *TypeInferVisitor::VisitArrayElementNode(ArrayElementNode *node) {
  (void) AstVisitor::VisitArrayElementNode(node);
  TreeNode *array = node->GetArray();
  if (array) {
    if (array->IsIdentifier()) {
      TreeNode *decl = mHandler->FindDecl(static_cast<IdentifierNode *>(array));
      if (decl) {
        // indexed access of class fields or types
        if (decl->GetTypeId() == TY_Class) {
          SetTypeId(array, TY_Class);
          TreeNode *exp = node->GetExprAtIndex(0);
          if (exp->IsLiteral()) {
            if (exp->GetTypeId() == TY_String) {
              // indexed access of types
              unsigned stridx = (static_cast<LiteralNode *>(exp))->GetData().mData.mStrIdx;
              if (decl->IsStruct()) {
                StructNode *structure = static_cast<StructNode *>(decl);
                for (int i = 0; i < structure->GetFieldsNum(); i++) {
                  TreeNode *f = structure->GetField(i);
                  if (f->GetStrIdx() == stridx) {
                    TypeId tid = f->GetTypeId();
                    UpdateTypeId(node, tid);
                    break;
                  }
                }
              }
            } else if (exp->GetTypeId() == TY_Int) {
              // indexed access of fields
              unsigned i = (static_cast<LiteralNode *>(exp))->GetData().mData.mInt64;
              NOTYETIMPL("indexed access with literal field id");
            } else {
              AstVisitor::VisitTreeNode(exp);
              NOTYETIMPL("indexed access not literal");
            }
          } else {
            AstVisitor::VisitTreeNode(exp);
          }
        } else {
          // default
          UpdateTypeId(array, TY_Array);
          UpdateTypeId(decl, TY_Array);
          UpdateArrayElemTypeIdMap(decl, node->GetTypeId());
          UpdateTypeId(node, mHandler->mArrayDeclId2EleTypeIdMap[decl->GetNodeId()]);
        }
      } else {
        NOTYETIMPL("array not declared");
      }
    } else if (array->IsArrayElement()) {
      NOTYETIMPL("array in ArrayElementNode IsArrayElement");
    } else if (array->IsField()) {
      NOTYETIMPL("array in ArrayElementNode IsField");
    } else if (array->IsUserType()) {
      NOTYETIMPL("array in ArrayElementNode IsUserType");
    } else if (array->IsBinOperator()) {
      NOTYETIMPL("array in ArrayElementNode IsBinOperator");
    } else if (array->IsLiteral() && ((LiteralNode*)array)->IsThis()) {
      NOTYETIMPL("array in ArrayElementNode IsLiteral");
    } else if (array->IsPrimType()) {
      NOTYETIMPL("array in ArrayElementNode IsPrimType");
    } else {
      NOTYETIMPL("array in ArrayElementNode unknown");
    }
  }
  return node;
}

FieldLiteralNode *TypeInferVisitor::VisitFieldLiteralNode(FieldLiteralNode *node) {
  (void) AstVisitor::VisitFieldLiteralNode(node);
  TreeNode *name = node->GetFieldName();
  TreeNode *lit = node->GetLiteral();
  UpdateTypeId(name, lit->GetTypeId());
  UpdateTypeId(node, lit->GetTypeId());
  return node;
}

ArrayLiteralNode *TypeInferVisitor::VisitArrayLiteralNode(ArrayLiteralNode *node) {
  UpdateTypeId(node, TY_Array);
  (void) AstVisitor::VisitArrayLiteralNode(node);
  ArrayLiteralNode *al = node;
  TreeNode *n = node;
  while (n->IsArrayLiteral()) {
    al = static_cast<ArrayLiteralNode *>(n);
    if (al->GetLiteralsNum()) {
      n = al->GetLiteral(0);
    } else {
      break;
    }
  }

  if (al->GetLiteralsNum()) {
    TypeId tid = al->GetLiteral(0)->GetTypeId();
    UpdateArrayElemTypeIdMap(node, tid);
  }
  return node;
}

BinOperatorNode *TypeInferVisitor::VisitBinOperatorNode(BinOperatorNode *node) {
  if (mFlags & FLG_trace_1) std::cout << "Visiting BinOperatorNode, id=" << node->GetNodeId() << "..." << std::endl;
  // (void) AstVisitor::VisitBinOperatorNode(node);
  OprId op = node->GetOprId();
  TreeNode *ta = node->GetOpndA();
  TreeNode *tb = node->GetOpndB();
  (void) VisitTreeNode(tb);
  (void) VisitTreeNode(ta);
  // modified operand
  TreeNode *mod = NULL;
  TypeId tia = ta->GetTypeId();
  TypeId tib = tb->GetTypeId();
  switch (op) {
    case OPR_StEq:
    case OPR_EQ:
    case OPR_NE:
    case OPR_GT:
    case OPR_LT:
    case OPR_GE:
    case OPR_LE: {
      if (tia != TY_None && tib == TY_None) {
        UpdateTypeId(tb, tia);
        mod = tb;
      } else if (tia == TY_None && tib != TY_None) {
        UpdateTypeId(ta, tib);
        mod = ta;
      }
      SetTypeId(node, TY_Boolean);
      break;
    }
    case OPR_Assign:
    case OPR_AddAssign:
    case OPR_SubAssign:
    case OPR_MulAssign:
    case OPR_DivAssign:
    case OPR_ModAssign:
    case OPR_ShlAssign:
    case OPR_ShrAssign:
    case OPR_BandAssign:
    case OPR_BorAssign:
    case OPR_BxorAssign:
    case OPR_ZextAssign: {
      TypeId ti = MergeTypeId(tia, tib);
      if (tia == TY_None || (ta->IsIdentifier() && tia != ti)) {
        UpdateTypeId(ta, ti);
        mod = ta;
      } else if (tib == TY_None) {
        UpdateTypeId(tb, ti);
        mod = tb;
      }
      UpdateTypeId(node, ti);
      break;
    }
    case OPR_Add:
    case OPR_Sub:
    case OPR_Mul:
    case OPR_Div: {
      if (tia != TY_None && tib == TY_None) {
        UpdateTypeId(tb, tia);
        mod = tb;
      } else if (tia == TY_None && tib != TY_None) {
        UpdateTypeId(ta, tib);
        mod = ta;
      }
      TypeId ti = MergeTypeId(tia, tib);
      UpdateTypeId(node, ti);
      break;
    }
    case OPR_Mod:
    case OPR_Band:
    case OPR_Bor:
    case OPR_Bxor:
    case OPR_Shl:
    case OPR_Shr:
    case OPR_Zext: {
      SetTypeId(ta, TY_Int);
      SetTypeId(tb, TY_Int);
      SetTypeId(node, TY_Int);
      break;
    }
    case OPR_Land:
    case OPR_Lor: {
      SetTypeId(ta, TY_Boolean);
      SetTypeId(tb, TY_Boolean);
      SetTypeId(node, TY_Boolean);
      break;
    }
    case OPR_Exp: {
      if (tia == TY_Int && tib == TY_Int) {
        SetTypeId(node, TY_Int);
      }
      break;
    }
    case OPR_NullCoalesce:
      break;
    default: {
      NOTYETIMPL("VisitBinOperatorNode()");
      break;
    }
  }
  // if mod is an identifier, update its decl
  if (mod && mod->IsIdentifier()) {
    TreeNode *decl = mHandler->FindDecl(static_cast<IdentifierNode *>(mod));
    if (decl) {
      UpdateTypeId(decl, mod->GetTypeId());
    } else {
      NOTYETIMPL("mod not declared");
    }
  }
  return node;
}

CallNode *TypeInferVisitor::VisitCallNode(CallNode *node) {
  if (mFlags & FLG_trace_1) std::cout << "Visiting CallNode, id=" << node->GetNodeId() << "..." << std::endl;
  TreeNode *method = node->GetMethod();
  UpdateTypeId(method, TY_Function);
  if (method && method->IsIdentifier()) {
    IdentifierNode *mid = static_cast<IdentifierNode *>(method);
    TreeNode *decl = mHandler->FindDecl(mid);
    if (decl) {
      if (decl->IsDecl()) {
        DeclNode *d = static_cast<DeclNode *>(decl);
        if (d->GetInit()) {
          decl = d->GetInit();
        }
      }
      if (decl && decl->IsIdentifier()) {
        IdentifierNode *id = static_cast<IdentifierNode *>(decl);
        if (id->GetType()) {
          decl = id->GetType();
        } else if (id->GetTypeId() == TY_Function) {
          NOTYETIMPL("VisitCallNode nTY_Function");
        }
      }
      if (decl) {
        if (decl->IsFunction()) {
          FunctionNode *func = static_cast<FunctionNode *>(decl);
          // update call's return type
          if (func->GetType()) {
            UpdateTypeId(node, func->GetType()->GetTypeId());
          }
          // skip imported and exported functions as they are generic
          // so should not restrict their types
          if (ImportedDeclIds.find(decl->GetNodeId()) == ImportedDeclIds.end() &&
              ExportedDeclIds.find(decl->GetNodeId()) == ExportedDeclIds.end()) {

            unsigned min = func->GetParamsNum();
            if (func->GetParamsNum() != node->GetArgsNum()) {
              // count minimun number of args need to be passed
              min = 0;
              // check arg about whether it is optional or has default value
              for (unsigned i = 0; i < func->GetParamsNum(); i++) {
                TreeNode *arg = func->GetParam(i);
                if (arg->IsOptional()) {
                  continue;
                } else if(arg->IsIdentifier()) {
                  IdentifierNode *id = static_cast<IdentifierNode *>(arg);
                  if (!id->GetInit()) {
                    min++;
                  }
                } else {
                  min++;
                }
              }
              if (min > node->GetArgsNum()) {
                NOTYETIMPL("call and func number of arguments not compatible");
                return node;
              }
            }
            // update function's argument types
            for (unsigned i = 0; i < min; i++) {
              UpdateTypeUseNode(func->GetParam(i), node->GetArg(i));
            }
          }
        } else if (decl->IsCall()) {
          (void) VisitCallNode(static_cast<CallNode *>(decl));
        } else if (decl->IsDecl()) {
          DeclNode *d = static_cast<DeclNode *>(decl);
          if (d->GetInit()) {
            NOTYETIMPL("VisitCallNode decl init");
          }
        } else if (decl->IsLiteral()) {
          LiteralNode *l = static_cast<LiteralNode *>(decl);
          NOTYETIMPL("VisitCallNode literal node");
        } else {
          NOTYETIMPL("VisitCallNode not function node");
        }
      } else {
        NOTYETIMPL("VisitCallNode null decl");
      }
    } else {
      // calling constructor like Array(...) could also end up here
      TreeNode *type = mHandler->FindType(mid);
      if (type) {
        NOTYETIMPL("VisitCallNode type");
      } else {
        NOTYETIMPL("VisitCallNode null decl and null type");
      }
    }
  }
  (void) AstVisitor::VisitCallNode(node);
  return node;
}

ClassNode *TypeInferVisitor::VisitClassNode(ClassNode *node) {
  if (mFlags & FLG_trace_1) std::cout << "Visiting ClassNode, id=" << node->GetNodeId() << "..." << std::endl;
  UpdateTypeId(node, TY_Class);
  for (unsigned i = 0; i < node->GetFieldsNum(); ++i) {
    TreeNode *t = node->GetField(i);
    (void) VisitClassField(t);
  }
  (void) AstVisitor::VisitClassNode(node);
  return node;
}

DeclNode *TypeInferVisitor::VisitDeclNode(DeclNode *node) {
  if (mFlags & FLG_trace_1) std::cout << "Visiting DeclNode, id=" << node->GetNodeId() << "..." << std::endl;
  (void) AstVisitor::VisitDeclNode(node);
  TreeNode *init = node->GetInit();
  TreeNode *var = node->GetVar();
  TypeId merged = node->GetTypeId();
  TypeId elemTypeId = TY_None;
  bool isArray = false;
  if (init) {
    merged = MergeTypeId(merged, init->GetTypeId());
    // collect array element typeid if any
    elemTypeId = GetArrayElemTypeId(init);
    isArray = (elemTypeId != TY_None);
  }
  if (var) {
    // normal cases
    if(var->IsIdentifier()) {
      IdentifierNode *id = static_cast<IdentifierNode *>(var);
      TreeNode *type = id->GetType();

      merged = MergeTypeId(merged, var->GetTypeId());
    } else {
      // BindingPatternNode
    }
  } else {
    MASSERT("var null");
  }
  // override TypeId for array
  if (isArray) {
    merged = TY_Array;
    SetTypeId(node, merged);
    SetTypeId(var, merged);
    SetTypeId(init, merged);
  } else {
    UpdateTypeId(node, merged);
    UpdateTypeId(init, merged);
    UpdateTypeId(var, merged);
  }
  if (isArray || IsArray(node)) {
    UpdateArrayElemTypeIdMap(node, elemTypeId);
  }
  return node;
}

ImportNode *TypeInferVisitor::VisitImportNode(ImportNode *node) {
  (void) AstVisitor::VisitImportNode(node);
  return node;
  for (unsigned i = 0; i < node->GetPairsNum(); i++) {
    XXportAsPairNode *p = node->GetPair(i);
    TreeNode *bfnode = p->GetBefore();
    if (bfnode) {
      ImportedDeclIds.insert(bfnode->GetNodeId());
    }
  }
  return node;
}

ExportNode *TypeInferVisitor::VisitExportNode(ExportNode *node) {
  (void) AstVisitor::VisitExportNode(node);
  for (unsigned i = 0; i < node->GetPairsNum(); i++) {
    XXportAsPairNode *p = node->GetPair(i);
    TreeNode *bfnode = p->GetBefore();
    if (bfnode) {
      switch (bfnode->GetKind()) {
        case NK_Struct:
        case NK_Function:
        case NK_Decl: {
          ExportedDeclIds.insert(bfnode->GetNodeId());
          break;
        }
        case NK_Declare: {
          DeclareNode *declare = static_cast<DeclareNode *>(bfnode);
          TreeNode *decl = declare->GetDecl();
          if (decl) {
            ExportedDeclIds.insert(decl->GetNodeId());
          }
          break;
        }
        case NK_Identifier: {
          IdentifierNode *idnode = static_cast<IdentifierNode *>(bfnode);
          TreeNode *decl = mHandler->FindDecl(idnode);
          if (decl) {
            ExportedDeclIds.insert(decl->GetNodeId());
          }
          break;
        }
        case NK_TypeAlias:
          break;
        default: {
          NOTYETIMPL("export node kind");
          break;
        }
      }
    }
  }
  return node;
}

FieldNode *TypeInferVisitor::VisitFieldNode(FieldNode *node) {
  (void) AstVisitor::VisitFieldNode(node);
  TreeNode *upper = node->GetUpper();
  IdentifierNode *field = static_cast<IdentifierNode *>(node->GetField());
  TreeNode *decl = NULL;
  if (!upper) {
    decl = mHandler->FindDecl(field);
  } else {
    switch (upper->GetKind()) {
      case NK_Literal: {
        LiteralNode *ln = static_cast<LiteralNode *>(upper);
        // this.f
        if (ln->GetData().mType == LT_ThisLiteral) {
          decl = mHandler->FindDecl(field);
        }
        break;
      }
      default:
        break;
    }
  }
  if (decl) {
    UpdateTypeId(node, decl->GetTypeId());
  }
  UpdateTypeId(field, node->GetTypeId());
  SetTypeId(node, field->GetTypeId());
  return node;
}

ForLoopNode *TypeInferVisitor::VisitForLoopNode(ForLoopNode *node) {
  if (mFlags & FLG_trace_1) std::cout << "Visiting ForLoopNode, id=" << node->GetNodeId() << "..." << std::endl;
  if (node->GetProp() == FLP_JSIn) {
    TreeNode *var = node->GetVariable();
    if (var) {
      SetTypeId(var, TY_Int);
    }
  }
  (void) AstVisitor::VisitForLoopNode(node);
  return node;
}

FunctionNode *TypeInferVisitor::VisitFunctionNode(FunctionNode *node) {
  if (mFlags & FLG_trace_1) std::cout << "Visiting FunctionNode, id=" << node->GetNodeId() << "..." << std::endl;
  UpdateTypeId(node, node->IsArray() ? TY_Object : TY_Function);
  (void) AstVisitor::VisitFunctionNode(node);
  return node;
}

IdentifierNode *TypeInferVisitor::VisitIdentifierNode(IdentifierNode *node) {
  if (mFlags & FLG_trace_1) std::cout << "Visiting IdentifierNode, id=" << node->GetNodeId() << "..." << std::endl;
  TreeNode *type = node->GetType();
  if (type && type->IsPrimArrayType()) {
    SetTypeId(node, TY_Array);
  }
  (void) AstVisitor::VisitIdentifierNode(node);
  if (node->GetInit()) {
    UpdateTypeId(node, node->GetInit()->GetTypeId());
    return node;
  }
  TreeNode *parent = node->GetParent();
  TreeNode *decl = NULL;
  if (parent && parent->IsField()) {
    FieldNode *field = static_cast<FieldNode *>(parent);
    TreeNode *upper = field->GetUpper();
    TreeNode *fld = field->GetField();
    if (node == upper) {
      decl = mHandler->FindDecl(node);
    } else if (node == fld) {
      if (upper->IsIdentifier()) {
        decl = mHandler->FindDecl(static_cast<IdentifierNode *>(upper));
        if (decl) {
          decl = decl->GetScope()->FindDeclOf(node->GetStrIdx());
        }
      }
    } else {
      NOTYETIMPL("node not in field");
    }
  } else {
    decl = mHandler->FindDecl(node);
  }

  if (decl) {
    UpdateTypeId(node, decl->GetTypeId());
  } else {
    NOTYETIMPL("node not declared");
    MSGNOLOC0(node->GetName());
  }

  return node;
}

InterfaceNode *TypeInferVisitor::VisitInterfaceNode(InterfaceNode *node) {
  UpdateTypeId(node, TY_Class);
  for (unsigned i = 0; i < node->GetFieldsNum(); ++i) {
    TreeNode *t = node->GetFieldAtIndex(i);
    (void) VisitClassField(t);
  }
  (void) AstVisitor::VisitInterfaceNode(node);
  return node;
}

NewNode *TypeInferVisitor::VisitNewNode(NewNode *node) {
  (void) AstVisitor::VisitNewNode(node);
  if (node->GetId()) {
    UpdateTypeId(node, node->GetId()->GetTypeId());
  }
  return node;
}

StructNode *TypeInferVisitor::VisitStructNode(StructNode *node) {
  SetTypeId(node, TY_Class);
  for (unsigned i = 0; i < node->GetFieldsNum(); ++i) {
    TreeNode *t = node->GetField(i);
    (void) VisitClassField(t);
  }
  if (node->GetProp() == SProp_TSEnum) {
    TypeId tid = TY_None;
    for (unsigned i = 0; i < node->GetFieldsNum(); ++i) {
      TreeNode *t = node->GetField(i);
      tid = MergeTypeId(tid, t->GetTypeId());
    }
    for (unsigned i = 0; i < node->GetFieldsNum(); ++i) {
      TreeNode *t = node->GetField(i);
      t->SetTypeId(tid);
    }
  }
  (void) AstVisitor::VisitStructNode(node);
  return node;
}

LambdaNode *TypeInferVisitor::VisitLambdaNode(LambdaNode *node) {
  if (mFlags & FLG_trace_1) std::cout << "Visiting LambdaNode, id=" << node->GetNodeId() << "..." << std::endl;
  UpdateTypeId(node, TY_Function);
  (void) AstVisitor::VisitLambdaNode(node);
  return node;
}

LiteralNode *TypeInferVisitor::VisitLiteralNode(LiteralNode *node) {
  (void) AstVisitor::VisitLiteralNode(node);
  LitId id = node->GetData().mType;
  switch (id) {
    case LT_IntegerLiteral:
      SetTypeId(node, TY_Int);
      break;
    case LT_FPLiteral:
      SetTypeId(node, TY_Float);
      break;
    case LT_DoubleLiteral:
      SetTypeId(node, TY_Double);
      break;
    case LT_StringLiteral:
      SetTypeId(node, TY_String);
      break;
    case LT_VoidLiteral:
      SetTypeId(node, TY_Undefined);
      break;
    default:
      break;
  }
  return node;
}

ReturnNode *TypeInferVisitor::VisitReturnNode(ReturnNode *node) {
  (void) AstVisitor::VisitReturnNode(node);
  TreeNode *res = node->GetResult();
  if (res) {
    UpdateTypeId(node, res->GetTypeId());
  }
  TreeNode *tn = mHandler->FindFunc(node);
  if (tn) {
    FunctionNode *func = static_cast<FunctionNode *>(tn);
    // use dummy PrimTypeNode as return type of function if not set to carry return TypeId
    if (!func->GetType()) {
      PrimTypeNode *type = (PrimTypeNode*)gTreePool.NewTreeNode(sizeof(PrimTypeNode));
      new (type) PrimTypeNode();
      type->SetPrimType(TY_None);
      func->SetType(type);
    }
    UpdateFuncRetTypeId(func, node->GetTypeId());
    if (res) {
      // use res to update function's return type
      UpdateTypeUseNode(func->GetType(), res);
    }
  }
  return node;
}

StructLiteralNode *TypeInferVisitor::VisitStructLiteralNode(StructLiteralNode *node) {
  SetTypeId(node, TY_Class);
  for (unsigned i = 0; i < node->GetFieldsNum(); ++i) {
    FieldLiteralNode *t = node->GetField(i);
    (void) VisitFieldLiteralNode(t);
  }
  (void) AstVisitor::VisitStructLiteralNode(node);
  return node;
}

TemplateLiteralNode *TypeInferVisitor::VisitTemplateLiteralNode(TemplateLiteralNode *node) {
  UpdateTypeId(node, TY_String);
  (void) AstVisitor::VisitTemplateLiteralNode(node);
  return node;
}

TerOperatorNode *TypeInferVisitor::VisitTerOperatorNode(TerOperatorNode *node) {
  TreeNode *ta = node->GetOpndA();
  TreeNode *tb = node->GetOpndB();
  TreeNode *tc = node->GetOpndC();
  (void) VisitTreeNode(ta);
  (void) VisitTreeNode(tb);
  (void) VisitTreeNode(tc);
  UpdateTypeId(node, tb->GetTypeId());
  UpdateTypeId(node, tc->GetTypeId());
  return node;
}

TypeOfNode *TypeInferVisitor::VisitTypeOfNode(TypeOfNode *node) {
  UpdateTypeId(node, TY_String);
  (void) AstVisitor::VisitTypeOfNode(node);
  return node;
}

UnaOperatorNode *TypeInferVisitor::VisitUnaOperatorNode(UnaOperatorNode *node) {
  (void) AstVisitor::VisitUnaOperatorNode(node);
  OprId op = node->GetOprId();
  TreeNode *ta = node->GetOpnd();
  switch (op) {
    case OPR_Plus:
    case OPR_Minus:
      UpdateTypeId(node, ta->GetTypeId());
      break;
    case OPR_PreInc:
    case OPR_Inc:
    case OPR_PreDec:
    case OPR_Dec:
      UpdateTypeId(ta, TY_Int);
      UpdateTypeId(node, TY_Int);
      break;
    case OPR_Bcomp:
      UpdateTypeId(ta, TY_Int);
      UpdateTypeId(node, TY_Int);
      break;
    case OPR_Not:
      UpdateTypeId(ta, TY_Boolean);
      UpdateTypeId(node, TY_Boolean);
      break;
    default: {
      NOTYETIMPL("VisitUnaOperatorNode()");
      break;
    }
  }
  return node;
}

UserTypeNode *TypeInferVisitor::VisitUserTypeNode(UserTypeNode *node) {
  (void) AstVisitor::VisitUserTypeNode(node);
  if (node->GetId()) {
    UpdateTypeId(node, node->GetId()->GetTypeId());
  }
  TreeNode *parent = node->GetParent();
  if (parent && parent->IsIdentifier()) {
    // typeid: merge -> user
    if (parent->GetTypeId() == TY_Merge) {
      SetTypeId(parent, TY_User);
      SetUpdated();
    } else if (parent->GetTypeId() == TY_Array) {
      TreeNode *idnode = node->GetId();
      if (idnode && idnode->IsIdentifier()) {
        // number, string could have been used
        // as usertype identifier instand of primtype by parser
        IdentifierNode *id = static_cast<IdentifierNode *>(idnode);
        TypeId tid = TY_None;
        unsigned stridx = id->GetStrIdx();
        if (stridx == gStringPool.GetStrIdx("number")) {
          tid = TY_Number;
        } else if (stridx == gStringPool.GetStrIdx("string")) {
          tid = TY_String;
        }
        if (tid != TY_None) {
          PrimArrayTypeNode *type = (PrimArrayTypeNode*)gTreePool.NewTreeNode(sizeof(PrimArrayTypeNode));
          new (type) PrimArrayTypeNode();
          PrimTypeNode *pt = (PrimTypeNode*)gTreePool.NewTreeNode(sizeof(PrimTypeNode));
          new (pt) PrimTypeNode();
          pt->SetPrimType(tid);
          type->SetPrim(pt);
          type->SetDims(node->GetDims());
          IdentifierNode *parentid = static_cast<IdentifierNode *>(parent);
          parentid->SetType(type);

          // clean up parent info
          id->SetParent(NULL);
          node->SetParent(NULL);

          // set updated
          SetUpdated();
        }
      }
    }
  }
  return node;
}

UserTypeNode *ShareUTVisitor::VisitUserTypeNode(UserTypeNode *node) {
  (void) AstVisitor::VisitUserTypeNode(node);

  TreeNode *idnode = node->GetId();
  if (idnode && idnode->IsIdentifier()) {
    IdentifierNode *id = static_cast<IdentifierNode *>(idnode);
    ASTScope *scope = id->GetScope();
    TreeNode *type = scope->FindTypeOf(id->GetStrIdx());
    if (type && type != node && type->IsUserType()) {
      UserTypeNode *ut = static_cast<UserTypeNode *>(type);
      if (node->GetType() == ut->GetType()) {
        // do not share if there are generics
        if (ut->GetTypeGenericsNum() == 0) {
          return ut;
        }
      }
    }
  }
  return node;
}

bool CheckTypeVisitor::IsCompatible(TypeId tid, TypeId target) {
  if (tid == target) {
    return true;
  }

  bool result = false;
  switch (target) {
    case TY_Number: {
      switch (tid) {
        case TY_None:
        case TY_Int:
        case TY_Long:
        case TY_Float:
        case TY_Double:
          result = true;
          break;
        default:
          result = false;
          break;
      }
      break;
    }
    case TY_Any:
      // TY_Any or unspecified matches everything
      result = true;
      break;
    default:
      // ok if same typeid
      result = (target == tid);
      break;
  }

  // tid being TY_None means untouched
  result = result || (tid == TY_None);

  return result;
}

IdentifierNode *CheckTypeVisitor::VisitIdentifierNode(IdentifierNode *node) {
  (void) AstVisitor::VisitIdentifierNode(node);

  TreeNode *d = mHandler->FindDecl(node);
  if (d && d->IsDecl()) {
    DeclNode *decl = static_cast<DeclNode *>(d);
    TreeNode *var = decl->GetVar();
    if (var && var != node && var->IsIdentifier()) {
      IdentifierNode *id = static_cast<IdentifierNode *>(var);
      bool result = false;
      TreeNode *type = id->GetType();
      if (type) {
        TypeId id = node->GetTypeId();
        TypeId target = TY_None;
        switch (type->GetKind()) {
          case NK_PrimType: {
            PrimTypeNode *ptn = static_cast<PrimTypeNode *>(type);
            target = ptn->GetPrimType();
            break;
          }
          case NK_UserType: {
            target = var->GetTypeId();
            break;
          }
          default: {
            target = var->GetTypeId();
            break;
          }
        }
        result = IsCompatible(id, target);
        if (!result || (mFlags & FLG_trace_3)) {
          std::cout << " Type Compatiblity : " << result << " : "
                    << AstDump::GetEnumTypeId(target) << " "
                    << AstDump::GetEnumTypeId(id) << std::endl;
        }
      }
    }
  }
  return node;
}

}
