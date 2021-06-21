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

#define NOTYETIMPL(K)      { if (mTrace) { MNYI(K);      }}

namespace maplefe {

void TypeInfer::TypeInference() {
  if (mTrace) std::cout << "============== TypeInfer ==============" << std::endl;
  TypeInferVisitor visitor(mHandler, mTrace, true);
  visitor.SetUpdated(true);
  int count = 0;
  while (visitor.GetUpdated()) {
    count++;
    visitor.SetUpdated(false);
    visitor.Visit(mHandler->GetASTModule());
    if (count > 10) break;
  }
  if (mTrace) std::cout << "\n>>>>>> TypeInference() iterated " << count << " times\n" << std::endl;
}

TypeId TypeInferVisitor::MergeTypeId(TypeId tia,  TypeId tib) {
  TypeId result = TY_None;
  if (tia == tib) {
    result = tia;
  } else if (tib == TY_None) {
    result = tia;
  } else if (tia == TY_None) {
    result = tib;
  } else if (tib == TY_Object) {
    result = tib;
  } else if (tia == TY_Object) {
    result = tia;
  } else if ((tia == TY_Int && tib == TY_Long) ||
             (tib == TY_Int && tia == TY_Long)) {
    result = TY_Long;
  } else if ((tia == TY_Int && tib == TY_Float) ||
             (tib == TY_Int && tia == TY_Float)) {
    result = TY_Float;
  } else if ((tia == TY_Long && tib == TY_Float) ||
             (tib == TY_Long && tia == TY_Float)) {
    result = TY_Double;
  } else {
    NOTYETIMPL("MergeTypeId()");
  }
  return result;
}

void TypeInferVisitor::UpdateTypeId(TreeNode *node, TypeId id) {
  if (!node || id == TY_None) {
    return;
  }
  id = MergeTypeId(node->GetTypeId(), id);
  if (node->GetTypeId() != id) {
    node->SetTypeId(id);
    mUpdated = true;
  }
}

void TypeInferVisitor::UpdateArrayElemTypeIdMap(TreeNode *node, TypeId id) {
  if (!node || id == TY_None || !IsArray(node)) {
    return;
  }
  unsigned nid = node->GetNodeId();
  auto it = mHandler->mArrayDeclId2EleTypeIdMap.find(nid);
  if (it != mHandler->mArrayDeclId2EleTypeIdMap.end()) {
    id = MergeTypeId(id, mHandler->mArrayDeclId2EleTypeIdMap[nid]);
  }
  if (mHandler->mArrayDeclId2EleTypeIdMap[nid] != id) {
    mHandler->mArrayDeclId2EleTypeIdMap[node->GetNodeId()] = id;
    mUpdated = true;
  }
}

bool TypeInferVisitor::IsArray(DeclNode *node) {
  if (!node) {
    return false;
  }
  TreeNode *var = node->GetVar();
  return IsArray(var);
}

bool TypeInferVisitor::IsArray(TreeNode *node) {
  if (!node) {
    return false;
  }
  if (node->IsIdentifier()) {
    IdentifierNode *id = static_cast<IdentifierNode *>(node);
    if (id && id->GetType() && id->GetType()->GetKind() == NK_PrimArrayType) {
      return true;
    }
  }
  return false;
}

TreeNode *TypeInferVisitor::VisitClassField(TreeNode *node) {
  if (node->IsIdentifier()) {
    IdentifierNode *id = static_cast<IdentifierNode *>(node);
    TreeNode *init = id->GetInit();
    if (init) {
      VisitTreeNode(init);
      UpdateTypeId(node, init->GetTypeId());
    }
  }
  return node;
}

AnnotationNode *TypeInferVisitor::VisitAnnotationNode(AnnotationNode *node) {
  (void) AstVisitor::VisitAnnotationNode(node);
  return node;
}

AnnotationTypeNode *TypeInferVisitor::VisitAnnotationTypeNode(AnnotationTypeNode *node) {
  (void) AstVisitor::VisitAnnotationTypeNode(node);
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
        decl->SetTypeId(TY_Array);
        UpdateArrayElemTypeIdMap(decl, node->GetTypeId());
        UpdateTypeId(node, mHandler->mArrayDeclId2EleTypeIdMap[decl->GetNodeId()]);
      }
    }
  }
  return node;
}

ArrayLiteralNode *TypeInferVisitor::VisitArrayLiteralNode(ArrayLiteralNode *node) {
  UpdateTypeId(node, TY_Array);
  (void) AstVisitor::VisitArrayLiteralNode(node);
  return node;
}

AssertNode *TypeInferVisitor::VisitAssertNode(AssertNode *node) {
  (void) AstVisitor::VisitAssertNode(node);
  return node;
}

AsTypeNode *TypeInferVisitor::VisitAsTypeNode(AsTypeNode *node) {
  (void) AstVisitor::VisitAsTypeNode(node);
  return node;
}

AttrNode *TypeInferVisitor::VisitAttrNode(AttrNode *node) {
  (void) AstVisitor::VisitAttrNode(node);
  return node;
}

BindingElementNode *TypeInferVisitor::VisitBindingElementNode(BindingElementNode *node) {
  (void) AstVisitor::VisitBindingElementNode(node);
  return node;
}

BindingPatternNode *TypeInferVisitor::VisitBindingPatternNode(BindingPatternNode *node) {
  (void) AstVisitor::VisitBindingPatternNode(node);
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
      UpdateTypeId(node, TY_Boolean);
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
      if (ta->GetTypeId() == TY_None) {
        UpdateTypeId(ta, tib);
        mod = ta;
      } else if (tb->GetTypeId() == TY_None) {
        UpdateTypeId(tb, tia);
        mod = tb;
      }
      UpdateTypeId(node, tib);
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
    }
  }
  return node;
}

BlockNode *TypeInferVisitor::VisitBlockNode(BlockNode *node) {
  (void) AstVisitor::VisitBlockNode(node);
  return node;
}

BreakNode *TypeInferVisitor::VisitBreakNode(BreakNode *node) {
  (void) AstVisitor::VisitBreakNode(node);
  return node;
}

CallNode *TypeInferVisitor::VisitCallNode(CallNode *node) {
  TreeNode *method = node->GetMethod();
  UpdateTypeId(method, TY_Function);
  if (method && method->IsIdentifier()) {
    TreeNode *decl = mHandler->FindDecl(static_cast<IdentifierNode *>(method));
    if (decl && decl->IsFunction()) {
      FunctionNode *func = static_cast<FunctionNode *>(decl);
      if (func->GetType()) {
        UpdateTypeId(node, func->GetType()->GetTypeId());
      }
    }
  }
  (void) AstVisitor::VisitCallNode(node);
  return node;
}

CastNode *TypeInferVisitor::VisitCastNode(CastNode *node) {
  (void) AstVisitor::VisitCastNode(node);
  return node;
}

CatchNode *TypeInferVisitor::VisitCatchNode(CatchNode *node) {
  (void) AstVisitor::VisitCatchNode(node);
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

CondBranchNode *TypeInferVisitor::VisitCondBranchNode(CondBranchNode *node) {
  (void) AstVisitor::VisitCondBranchNode(node);
  return node;
}

ContinueNode *TypeInferVisitor::VisitContinueNode(ContinueNode *node) {
  (void) AstVisitor::VisitContinueNode(node);
  return node;
}

DeclareNode *TypeInferVisitor::VisitDeclareNode(DeclareNode *node) {
  (void) AstVisitor::VisitDeclareNode(node);
  return node;
}

DeclNode *TypeInferVisitor::VisitDeclNode(DeclNode *node) {
  (void) AstVisitor::VisitDeclNode(node);
  TreeNode *init = node->GetInit();
  TreeNode *var = node->GetVar();
  TypeId merged = TY_None;
  TypeId elemTypeId = TY_None;
  if (init) {
    merged = MergeTypeId(node->GetTypeId(), init->GetTypeId());
    // collect array element typeid
    if (init->IsArrayLiteral()) {
      ArrayLiteralNode *al = static_cast<ArrayLiteralNode *>(init);
      if (al->GetLiteralsNum()) {
        elemTypeId = al->GetLiteral(0)->GetTypeId();
      }
    }
  }
  if (var) {
    merged = MergeTypeId(merged, var->GetTypeId());
  }
  UpdateTypeId(node, merged);
  UpdateTypeId(init, merged);
  UpdateTypeId(var, merged);
  if (IsArray(node)) {
    UpdateArrayElemTypeIdMap(node, elemTypeId);
  }
  return node;
}

DeleteNode *TypeInferVisitor::VisitDeleteNode(DeleteNode *node) {
  (void) AstVisitor::VisitDeleteNode(node);
  return node;
}

DimensionNode *TypeInferVisitor::VisitDimensionNode(DimensionNode *node) {
  (void) AstVisitor::VisitDimensionNode(node);
  return node;
}

DoLoopNode *TypeInferVisitor::VisitDoLoopNode(DoLoopNode *node) {
  (void) AstVisitor::VisitDoLoopNode(node);
  return node;
}

ExceptionNode *TypeInferVisitor::VisitExceptionNode(ExceptionNode *node) {
  (void) AstVisitor::VisitExceptionNode(node);
  return node;
}

ExportNode *TypeInferVisitor::VisitExportNode(ExportNode *node) {
  (void) AstVisitor::VisitExportNode(node);
  return node;
}

ExprListNode *TypeInferVisitor::VisitExprListNode(ExprListNode *node) {
  (void) AstVisitor::VisitExprListNode(node);
  return node;
}

FieldLiteralNode *TypeInferVisitor::VisitFieldLiteralNode(FieldLiteralNode *node) {
  (void) AstVisitor::VisitFieldLiteralNode(node);
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

FinallyNode *TypeInferVisitor::VisitFinallyNode(FinallyNode *node) {
  (void) AstVisitor::VisitFinallyNode(node);
  return node;
}

ForLoopNode *TypeInferVisitor::VisitForLoopNode(ForLoopNode *node) {
  (void) AstVisitor::VisitForLoopNode(node);
  return node;
}

FunctionNode *TypeInferVisitor::VisitFunctionNode(FunctionNode *node) {
  UpdateTypeId(node, node->IsArray() ? TY_Object : TY_Function);
  (void) AstVisitor::VisitFunctionNode(node);
  return node;
}

IdentifierNode *TypeInferVisitor::VisitIdentifierNode(IdentifierNode *node) {
  (void) AstVisitor::VisitIdentifierNode(node);
  TreeNode *decl = mHandler->FindDecl(node);
  if (decl) {
    UpdateTypeId(node, decl->GetTypeId());
  }
  return node;
}

ImportNode *TypeInferVisitor::VisitImportNode(ImportNode *node) {
  (void) AstVisitor::VisitImportNode(node);
  return node;
}

InNode *TypeInferVisitor::VisitInNode(InNode *node) {
  (void) AstVisitor::VisitInNode(node);
  return node;
}

InstanceOfNode *TypeInferVisitor::VisitInstanceOfNode(InstanceOfNode *node) {
  (void) AstVisitor::VisitInstanceOfNode(node);
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

KeyOfNode *TypeInferVisitor::VisitKeyOfNode(KeyOfNode *node) {
  (void) AstVisitor::VisitKeyOfNode(node);
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
      UpdateTypeId(node, TY_Int);
      break;
    case LT_FPLiteral:
      UpdateTypeId(node, TY_Float);
      break;
    case LT_DoubleLiteral:
      UpdateTypeId(node, TY_Double);
      break;
    case LT_StringLiteral:
      UpdateTypeId(node, TY_String);
      break;
    default:
      break;
  }
  return node;
}

ModuleNode *TypeInferVisitor::VisitModuleNode(ModuleNode *node) {
  (void) AstVisitor::VisitModuleNode(node);
  return node;
}

NamespaceNode *TypeInferVisitor::VisitNamespaceNode(NamespaceNode *node) {
  (void) AstVisitor::VisitNamespaceNode(node);
  return node;
}

NewNode *TypeInferVisitor::VisitNewNode(NewNode *node) {
  (void) AstVisitor::VisitNewNode(node);
  return node;
}

NumIndexSigNode *TypeInferVisitor::VisitNumIndexSigNode(NumIndexSigNode *node) {
  (void) AstVisitor::VisitNumIndexSigNode(node);
  return node;
}

PackageNode *TypeInferVisitor::VisitPackageNode(PackageNode *node) {
  (void) AstVisitor::VisitPackageNode(node);
  return node;
}

ParenthesisNode *TypeInferVisitor::VisitParenthesisNode(ParenthesisNode *node) {
  (void) AstVisitor::VisitParenthesisNode(node);
  return node;
}

PassNode *TypeInferVisitor::VisitPassNode(PassNode *node) {
  (void) AstVisitor::VisitPassNode(node);
  return node;
}

PrimArrayTypeNode *TypeInferVisitor::VisitPrimArrayTypeNode(PrimArrayTypeNode *node) {
  (void) AstVisitor::VisitPrimArrayTypeNode(node);
  return node;
}

PrimTypeNode *TypeInferVisitor::VisitPrimTypeNode(PrimTypeNode *node) {
  (void) AstVisitor::VisitPrimTypeNode(node);
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
    UpdateTypeId(func->GetType(), node->GetTypeId());
  }
  return node;
}

StrIndexSigNode *TypeInferVisitor::VisitStrIndexSigNode(StrIndexSigNode *node) {
  (void) AstVisitor::VisitStrIndexSigNode(node);
  return node;
}

StructLiteralNode *TypeInferVisitor::VisitStructLiteralNode(StructLiteralNode *node) {
  UpdateTypeId(node, TY_Object);
  (void) AstVisitor::VisitStructLiteralNode(node);
  return node;
}

StructNode *TypeInferVisitor::VisitStructNode(StructNode *node) {
  (void) AstVisitor::VisitStructNode(node);
  return node;
}

SwitchCaseNode *TypeInferVisitor::VisitSwitchCaseNode(SwitchCaseNode *node) {
  (void) AstVisitor::VisitSwitchCaseNode(node);
  return node;
}

SwitchLabelNode *TypeInferVisitor::VisitSwitchLabelNode(SwitchLabelNode *node) {
  (void) AstVisitor::VisitSwitchLabelNode(node);
  return node;
}

SwitchNode *TypeInferVisitor::VisitSwitchNode(SwitchNode *node) {
  (void) AstVisitor::VisitSwitchNode(node);
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

ThrowNode *TypeInferVisitor::VisitThrowNode(ThrowNode *node) {
  (void) AstVisitor::VisitThrowNode(node);
  return node;
}

TryNode *TypeInferVisitor::VisitTryNode(TryNode *node) {
  (void) AstVisitor::VisitTryNode(node);
  return node;
}

TypeOfNode *TypeInferVisitor::VisitTypeOfNode(TypeOfNode *node) {
  UpdateTypeId(node, TY_String);
  (void) AstVisitor::VisitTypeOfNode(node);
  return node;
}

TypeParameterNode *TypeInferVisitor::VisitTypeParameterNode(TypeParameterNode *node) {
  (void) AstVisitor::VisitTypeParameterNode(node);
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
  TreeNode *parent = node->GetParent();
  if (parent) {
    UpdateTypeId(parent, TY_Object);
  }
  (void) AstVisitor::VisitUserTypeNode(node);
  return node;
}

VarListNode *TypeInferVisitor::VisitVarListNode(VarListNode *node) {
  (void) AstVisitor::VisitVarListNode(node);
  return node;
}

WhileLoopNode *TypeInferVisitor::VisitWhileLoopNode(WhileLoopNode *node) {
  (void) AstVisitor::VisitWhileLoopNode(node);
  return node;
}

XXportAsPairNode *TypeInferVisitor::VisitXXportAsPairNode(XXportAsPairNode *node) {
  (void) AstVisitor::VisitXXportAsPairNode(node);
  return node;
}

}
