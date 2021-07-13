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

#define NOTYETIMPL(K) {if(mTrace){MNYI(K);}}

namespace maplefe {

void TypeInfer::TypeInference() {
  if (mTrace) std::cout << "============== TypeInfer ==============" << std::endl;

  ModuleNode *module = mHandler->GetASTModule();

  // build mNodeId2Decl
  BuildIdNodeToDeclVisitor visitor_pass0(mHandler, mTrace, true);
  visitor_pass0.Visit(module);

  // type inference
  TypeInferVisitor visitor_pass1(mHandler, mTrace, true);
  visitor_pass1.SetUpdated(true);
  int count = 0;
  while (visitor_pass1.GetUpdated()) {
    count++;
    visitor_pass1.SetUpdated(false);
    visitor_pass1.Visit(module);
    if (count > 10) break;
  }

  // share UserType
  ShareUTVisitor visitor_pass2(mHandler, mTrace, true);
  visitor_pass2.Push(module->GetRootScope());
  visitor_pass2.Visit(module);

  if (mTrace) std::cout << "\n>>>>>> TypeInference() iterated " << count << " times\n" << std::endl;
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
    case TY_User:        result = tia;

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
        case TY_Merge:
        case TY_Undefined:
        case TY_String:
        case TY_Function:
        case TY_Class:
        case TY_Array:   result = TY_Merge;  break;
        default: break;
      }
      break;
    }
    case TY_Int: {
      switch (tib) {
        case TY_Boolean: result = TY_Int;    break;
        case TY_Long:
        case TY_Float:
        case TY_Double:  result = tib;       break;
        case TY_Merge:
        case TY_Undefined:
        case TY_String:
        case TY_Function:
        case TY_Class:
        case TY_Array:   result = TY_Merge;  break;
        default: break;
      }
      break;
    }
    case TY_Long: {
      switch (tib) {
        case TY_Boolean:
        case TY_Int:     result = TY_Long;   break;
        case TY_Float:
        case TY_Double:  result = TY_Double; break;
        case TY_Merge:
        case TY_Undefined:
        case TY_String:
        case TY_Function:
        case TY_Class:
        case TY_Array:   result = TY_Merge;  break;
        default: break;
      }
      break;
    }
    case TY_Float: {
      switch (tib) {
        case TY_Boolean:
        case TY_Int:     result = TY_Float;  break;
        case TY_Long:
        case TY_Double:  result = TY_Double; break;
        case TY_Merge:
        case TY_Undefined:
        case TY_String:
        case TY_Function:
        case TY_Class:
        case TY_Array:   result = TY_Merge;  break;
        default: break;
      }
      break;
    }
    case TY_Double: {
      switch (tib) {
        case TY_Boolean:
        case TY_Int:
        case TY_Long:
        case TY_Double:  result = TY_Double; break;
        case TY_Merge:
        case TY_Undefined:
        case TY_String:
        case TY_Function:
        case TY_Class:
        case TY_Array:   result = TY_Merge;  break;
        default: break;
      }
      break;
    }
    default:
      break;
  }
  if (result == TY_None) {
    NOTYETIMPL("MergeTypeId()");
  }
  return result;
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
    new_pt->SetTypeId(tid);
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

// use input node's type info to update target node's type info
// used to refine function's formals with corresponding calls' parameters passed in
void TypeInferVisitor::UpdateTypeUseNode(TreeNode *target, TreeNode *input) {
  TypeId nid = target->GetTypeId();
  TypeId iid = input->GetTypeId();
  if ((nid == iid && IsScalar(nid)) || (iid == TY_Array && nid != iid)) {
    return;
  }
  switch (iid) {
    case TY_Array: {
      if (input->IsIdentifier()) {
        TreeNode *decl = mHandler->FindDecl(static_cast<IdentifierNode *>(input));
        TypeId old_elemTypeId = GetArrayElemTypeId(target);
        TypeId inid = GetArrayElemTypeId(decl);
        if (old_elemTypeId != inid) {
          UpdateArrayElemTypeIdMap(target, inid);
        }
        TypeId new_elemTypeId = GetArrayElemTypeId(target);
        if (old_elemTypeId != new_elemTypeId) {
          TreeNode *type = static_cast<IdentifierNode *>(target)->GetType();
          MASSERT(target->IsIdentifier() && "target node not identifier");
          if (type->IsPrimArrayType()) {
            PrimArrayTypeNode *pat = static_cast<PrimArrayTypeNode *>(type);
            PrimTypeNode *pt = pat->GetPrim();
            PrimTypeNode *new_pt = GetOrClonePrimTypeNode(pt, new_elemTypeId);
            pat->SetPrim(new_pt);
            SetUpdated();
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
      TypeId merged = MergeTypeId(nid, iid);
      if (merged != nid) {
        target->SetTypeId(merged);
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

void TypeInferVisitor::UpdateTypeId(TreeNode *node, TypeId tid) {
  if (tid == TY_None || !node || node->IsLiteral()) {
    return;
  }
  tid = MergeTypeId(node->GetTypeId(), tid);
  if (node->GetTypeId() != tid) {
    node->SetTypeId(tid);
    SetUpdated();
  }
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
    type->SetTypeId(tid);
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
      TreeNode *type = in->GetType();
      if (type->IsPrimArrayType()) {
        PrimArrayTypeNode *pat = static_cast<PrimArrayTypeNode *>(type);
        PrimTypeNode *pt = pat->GetPrim();
        PrimTypeNode *new_pt = GetOrClonePrimTypeNode(pt, tid);
        pat->SetPrim(new_pt);
      }
    }
  }
}

bool TypeInferVisitor::IsArray(TreeNode *node) {
  if (!node) {
    return false;
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
        // use mPrimType for non TY_Number
        if (tid != TY_Number) {
          node->SetTypeId(tid);
        }
      }
      node->SetTypeId(tid);
    }
    TreeNode *init = idnode->GetInit();
    if (init) {
      VisitTreeNode(init);
      UpdateTypeId(node, init->GetTypeId());
    }
  } else {
    NOTYETIMPL("field not idenfier");
  }
  return node;
}

ArrayElementNode *TypeInferVisitor::VisitArrayElementNode(ArrayElementNode *node) {
  (void) AstVisitor::VisitArrayElementNode(node);
  TreeNode *array = node->GetArray();
  if (array) {
    array->SetTypeId(TY_Array);
    if (array->IsIdentifier()) {
      TreeNode *decl = mHandler->FindDecl(static_cast<IdentifierNode *>(array));
      if (decl) {
        // indexed access type
        if (decl->IsStruct()) {
          array->SetTypeId(TY_Class);
          TreeNode *exp = node->GetExprAtIndex(0);
          if (exp->IsLiteral() && exp->GetTypeId() == TY_String) {
            unsigned stridx = (static_cast<LiteralNode *>(exp))->GetData().mData.mStrIdx;
            StructNode *structure = static_cast<StructNode *>(decl);
            for (int i = 0; i < structure->GetFieldsNum(); i++) {
              IdentifierNode *f = structure->GetField(i);
              if (f->GetStrIdx() == stridx) {
                TypeId tid = f->GetTypeId();
                UpdateTypeId(node, tid);
              }
            }
          } else {
            NOTYETIMPL("indexed access type index not literal");
          }
        } else {
          // default
          decl->SetTypeId(TY_Array);
          UpdateArrayElemTypeIdMap(decl, node->GetTypeId());
          UpdateTypeId(node, mHandler->mArrayDeclId2EleTypeIdMap[decl->GetNodeId()]);
        }
      } else {
        NOTYETIMPL("array not declared");
      }
    } else {
      NOTYETIMPL("array not idenfier");
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
  return node;
}

BinOperatorNode *TypeInferVisitor::VisitBinOperatorNode(BinOperatorNode *node) {
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
      node->SetTypeId(TY_Boolean);
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
    case OPR_Div:
    case OPR_Mod:
    case OPR_Band:
    case OPR_Bor:
    case OPR_Bxor:
    case OPR_Shl:
    case OPR_Shr:
    case OPR_Zext:
    case OPR_Land:
    case OPR_Lor: {
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
  TreeNode *method = node->GetMethod();
  UpdateTypeId(method, TY_Function);
  if (method && method->IsIdentifier()) {
    TreeNode *decl = mHandler->FindDecl(static_cast<IdentifierNode *>(method));
    if (decl && decl->IsFunction()) {
      FunctionNode *func = static_cast<FunctionNode *>(decl);
      // update call's return type
      if (func->GetType()) {
        UpdateTypeId(node, func->GetType()->GetTypeId());
      }
      // update function's argument types
      if (func->GetParamsNum() != node->GetArgsNum()) {
        NOTYETIMPL("call and func with different number of arguments");
        return node;
      }
      if (ExportedDeclIds.find(decl->GetNodeId()) == ExportedDeclIds.end()) {
        for (unsigned i = 0; i < node->GetArgsNum(); i++) {
          UpdateTypeUseNode(func->GetParam(i), node->GetArg(i));
        }
      }
    } else {
      NOTYETIMPL("VisitCallNode null method or not function node");
    }
  }
  (void) AstVisitor::VisitCallNode(node);
  return node;
}

ClassNode *TypeInferVisitor::VisitClassNode(ClassNode *node) {
  UpdateTypeId(node, TY_Class);
  for (unsigned i = 0; i < node->GetFieldsNum(); ++i) {
    TreeNode *t = node->GetField(i);
    (void) VisitClassField(t);
  }
  (void) AstVisitor::VisitClassNode(node);
  return node;
}

DeclNode *TypeInferVisitor::VisitDeclNode(DeclNode *node) {
  (void) AstVisitor::VisitDeclNode(node);
  TreeNode *init = node->GetInit();
  TreeNode *var = node->GetVar();
  TypeId merged = TY_None;
  TypeId elemTypeId = TY_None;
  bool isArray = false;
  if (init) {
    merged = MergeTypeId(node->GetTypeId(), init->GetTypeId());
    // collect array element typeid
    TreeNode *n = init;
    while (n->IsArrayLiteral()) {
      isArray = true;
      ArrayLiteralNode *al = static_cast<ArrayLiteralNode *>(n);
      if (al->GetLiteralsNum()) {
        n = al->GetLiteral(0);
      } else {
        break;
      }
    }
    elemTypeId = n->GetTypeId();
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
    node->SetTypeId(merged);
    var->SetTypeId(merged);
    init->SetTypeId(merged);
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

ExportNode *TypeInferVisitor::VisitExportNode(ExportNode *node) {
  (void) AstVisitor::VisitExportNode(node);
  XXportAsPairNode *p = node->GetPair(0);
  TreeNode *bfnode = p->GetBefore();
  if (bfnode && bfnode->IsIdentifier()) {
    IdentifierNode *idnode = static_cast<IdentifierNode *>(bfnode);
    TreeNode *decl = mHandler->FindDecl(idnode);
    if (decl) {
      ExportedDeclIds.insert(decl->GetNodeId());
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
  return node;
}

FunctionNode *TypeInferVisitor::VisitFunctionNode(FunctionNode *node) {
  UpdateTypeId(node, node->IsArray() ? TY_Object : TY_Function);
  (void) AstVisitor::VisitFunctionNode(node);
  return node;
}

IdentifierNode *TypeInferVisitor::VisitIdentifierNode(IdentifierNode *node) {
  TreeNode *type = node->GetType();
  if (type && type->IsPrimArrayType()) {
    node->SetTypeId(TY_Array);
  }
  (void) AstVisitor::VisitIdentifierNode(node);
  TreeNode *decl = mHandler->FindDecl(node);
  if (decl) {
    UpdateTypeId(node, decl->GetTypeId());
  } else {
    NOTYETIMPL("node not declared");
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

StructNode *TypeInferVisitor::VisitStructNode(StructNode *node) {
  UpdateTypeId(node, TY_Class);
  for (unsigned i = 0; i < node->GetFieldsNum(); ++i) {
    TreeNode *t = node->GetField(i);
    (void) VisitClassField(t);
  }
  (void) AstVisitor::VisitStructNode(node);
  return node;
}

LambdaNode *TypeInferVisitor::VisitLambdaNode(LambdaNode *node) {
  UpdateTypeId(node, TY_Function);
  (void) AstVisitor::VisitLambdaNode(node);
  return node;
}

LiteralNode *TypeInferVisitor::VisitLiteralNode(LiteralNode *node) {
  (void) AstVisitor::VisitLiteralNode(node);
  LitId id = node->GetData().mType;
  switch (id) {
    case LT_IntegerLiteral:
      node->SetTypeId(TY_Int);
      break;
    case LT_FPLiteral:
      node->SetTypeId(TY_Float);
      break;
    case LT_DoubleLiteral:
      node->SetTypeId(TY_Double);
      break;
    case LT_StringLiteral:
      node->SetTypeId(TY_String);
      break;
    case LT_VoidLiteral:
      node->SetTypeId(TY_Undefined);
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
    UpdateFuncRetTypeId(func, node->GetTypeId());
  }
  return node;
}

StructLiteralNode *TypeInferVisitor::VisitStructLiteralNode(StructLiteralNode *node) {
  UpdateTypeId(node, TY_Object);
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
    case OPR_Add:
    case OPR_Sub:
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
  TreeNode *parent = node->GetParent();
  if (parent && parent->IsIdentifier()) {
    // typeid: merge -> user
    if (parent->GetTypeId() == TY_Merge) {
      parent->SetTypeId(TY_User);
      SetUpdated();
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

}
