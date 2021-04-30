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

#include "a2c_astdump.h"

namespace maplefe {

void A2C_AstDump::AstDumpAnnotationNode(AnnotationNode *node) {
  if (node == nullptr) {
    dump("  AnnotationNode: null");
    return;
  }
  dump("  AnnotationNode {");
  indent += 4;
  base(node);
  // Declared at shared/include/ast.h:465
  dump("mId: IdentifierNode*");
  AstDumpIdentifierNode(node->GetId());
  dump("mType: AnnotationTypeNode*");
  AstDumpAnnotationTypeNode(node->GetType());
  dump("mExpr: TreeNode*");
  AstDumpTreeNode(node->GetExpr());
  indent -= 4;
  dump("  }");
  return;
}

void A2C_AstDump::AstDumpPackageNode(PackageNode *node) {
  if (node == nullptr) {
    dump("  PackageNode: null");
    return;
  }
  dump("  PackageNode {");
  indent += 4;
  base(node);
  // Declared at shared/include/ast.h:113
  dump("mPackage: TreeNode*");
  AstDumpTreeNode(node->GetPackage());
  indent -= 4;
  dump("  }");
  return;
}

void A2C_AstDump::AstDumpImportNode(ImportNode *node) {
  if (node == nullptr) {
    dump("  ImportNode: null");
    return;
  }
  dump("  ImportNode {");
  indent += 4;
  base(node);
  // Declared at shared/include/ast.h:157
  dump(std::string("mProperty: ImportProperty: ") + getImportProperty(node->GetProperty()));

  dump("mTarget: TreeNode*");
  AstDumpTreeNode(node->GetTarget());
  indent -= 4;
  dump("  }");
  return;
}

void A2C_AstDump::AstDumpUnaOperatorNode(UnaOperatorNode *node) {
  if (node == nullptr) {
    dump("  UnaOperatorNode: null");
    return;
  }
  dump("  UnaOperatorNode {");
  indent += 4;
  base(node);
  // Declared at shared/include/ast.h:211
  dump(std::string("mIsPost: bool, ") + (node->IsPost() ? "true" : "false"));

  dump(std::string("mOprId: OprId: ") + getOprId(node->GetOprId()));

  dump("mOpnd: TreeNode*");
  AstDumpTreeNode(node->GetOpnd());
  indent -= 4;
  dump("  }");
  return;
}

void A2C_AstDump::AstDumpBinOperatorNode(BinOperatorNode *node) {
  if (node == nullptr) {
    dump("  BinOperatorNode: null");
    return;
  }
  dump("  BinOperatorNode {");
  indent += 4;
  base(node);
  // Declared at shared/include/ast.h:235
  dump(std::string("mOprId: OprId: ") + getOprId(node->GetOprId()));

  dump("mOpndA: TreeNode*");
  AstDumpTreeNode(node->GetOpndA());
  dump("mOpndB: TreeNode*");
  AstDumpTreeNode(node->GetOpndB());
  indent -= 4;
  dump("  }");
  return;
}

void A2C_AstDump::AstDumpTerOperatorNode(TerOperatorNode *node) {
  if (node == nullptr) {
    dump("  TerOperatorNode: null");
    return;
  }
  dump("  TerOperatorNode {");
  indent += 4;
  base(node);
  // Declared at shared/include/ast.h:256
  dump(std::string("mOprId: OprId: ") + getOprId(node->GetOprId()));

  dump("mOpndA: TreeNode*");
  AstDumpTreeNode(node->GetOpndA());
  dump("mOpndB: TreeNode*");
  AstDumpTreeNode(node->GetOpndB());
  dump("mOpndC: TreeNode*");
  AstDumpTreeNode(node->GetOpndC());
  indent -= 4;
  dump("  }");
  return;
}

void A2C_AstDump::AstDumpBlockNode(BlockNode *node) {
  if (node == nullptr) {
    dump("  BlockNode: null");
    return;
  }
  dump("  BlockNode {");
  indent += 4;
  base(node);
  // Declared at shared/include/ast.h:951
  dump("mChildren: SmallList<class maplefe::TreeNode *>, size = " + std::to_string(node->GetChildrenNum()) + " [");
  indent += 2;
  for (unsigned i = 0; i < node->GetChildrenNum(); ++i) {
    dump(std::to_string(i + 1) + ": TreeNode*");
    AstDumpTreeNode(node->GetChildAtIndex(i));
  }
  indent -= 2;
  dump(" ]");
  dump(std::string("mIsInstInit: bool, ") + (node->IsInstInit() ? "true" : "false"));

  dump("mAttrs: SmallVector<enum maplefe::AttrId>, size = " + std::to_string(node->GetAttrsNum()) + " [");
  indent += 2;
  for (unsigned i = 0; i < node->GetAttrsNum(); ++i) {
    dump(std::to_string(i) + ". AttrId: " + getAttrId(node->GetAttrAtIndex(i)));
  }
  indent -= 2;
  dump(" ]");
  dump("mSync: TreeNode*");
  AstDumpTreeNode(const_cast<TreeNode *>(node->GetSync()));
  indent -= 4;
  dump("  }");
  return;
}

void A2C_AstDump::AstDumpNewNode(NewNode *node) {
  if (node == nullptr) {
    dump("  NewNode: null");
    return;
  }
  dump("  NewNode {");
  indent += 4;
  base(node);
  // Declared at shared/include/ast.h:288
  dump("mId: TreeNode*");
  AstDumpTreeNode(node->GetId());
  dump("mArgs: SmallVector<class maplefe::TreeNode *>, size = " + std::to_string(node->GetArgsNum()) + " [");
  indent += 2;
  for (unsigned i = 0; i < node->GetArgsNum(); ++i) {
    dump(std::to_string(i + 1) + ": TreeNode*");
    AstDumpTreeNode(node->GetArg(i));
  }
  indent -= 2;
  dump(" ]");
  dump("mBody: BlockNode*");
  AstDumpBlockNode(node->GetBody());
  indent -= 4;
  dump("  }");
  return;
}

void A2C_AstDump::AstDumpDeleteNode(DeleteNode *node) {
  if (node == nullptr) {
    dump("  DeleteNode: null");
    return;
  }
  dump("  DeleteNode {");
  indent += 4;
  base(node);
  indent -= 4;
  dump("  }");
  return;
}

void A2C_AstDump::AstDumpDimensionNode(DimensionNode *node) {
  if (node == nullptr) {
    dump("  DimensionNode: null");
    return;
  }
  dump("  DimensionNode {");
  indent += 4;
  base(node);
  // Declared at shared/include/ast.h:341
  dump("mDimensions: SmallVector<unsigned int>, size = " + std::to_string(node->GetDimsNum()) + " [");
  indent += 2;
  for (unsigned i = 0; i < node->GetDimsNum(); ++i) {
    dump(std::to_string(i) + ". unsigned int, " + std::to_string(node->GetNthDim(i)));
  }
  indent -= 2;
  dump(" ]");
  indent -= 4;
  dump("  }");
  return;
}

void A2C_AstDump::AstDumpIdentifierNode(IdentifierNode *node) {
  if (node == nullptr) {
    dump("  IdentifierNode: null");
    return;
  }
  dump("  IdentifierNode {");
  indent += 4;
  base(node);
  // Declared at shared/include/ast.h:363
  dump("mAttrs: SmallVector<enum maplefe::AttrId>, size = " + std::to_string(node->GetAttrsNum()) + " [");
  indent += 2;
  for (unsigned i = 0; i < node->GetAttrsNum(); ++i) {
    dump(std::to_string(i) + ". AttrId: " + getAttrId(node->GetAttrAtIndex(i)));
  }
  indent -= 2;
  dump(" ]");
  dump("mType: TreeNode*");
  AstDumpTreeNode(node->GetType());
  dump("mInit: TreeNode*");
  AstDumpTreeNode(node->GetInit());
  dump("mDims: DimensionNode*");
  AstDumpDimensionNode(node->GetDims());
  indent -= 4;
  dump("  }");
  return;
}

void A2C_AstDump::AstDumpDeclNode(DeclNode *node) {
  if (node == nullptr) {
    dump("  DeclNode: null");
    return;
  }
  dump("  DeclNode {");
  indent += 4;
  base(node);
  // Declared at shared/include/ast.h:416
  dump("mVar: TreeNode*");
  AstDumpTreeNode(node->GetVar());
  dump(std::string("mProp: DeclProp: ") + getDeclProp(node->GetProp()));

  indent -= 4;
  dump("  }");
  return;
}

void A2C_AstDump::AstDumpAnnotationTypeNode(AnnotationTypeNode *node) {
  if (node == nullptr) {
    dump("  AnnotationTypeNode: null");
    return;
  }
  dump("  AnnotationTypeNode {");
  indent += 4;
  base(node);
  // Declared at shared/include/ast.h:452
  dump("mId: IdentifierNode*");
  AstDumpIdentifierNode(node->GetId());
  indent -= 4;
  dump("  }");
  return;
}

void A2C_AstDump::AstDumpCastNode(CastNode *node) {
  if (node == nullptr) {
    dump("  CastNode: null");
    return;
  }
  dump("  CastNode {");
  indent += 4;
  base(node);
  // Declared at shared/include/ast.h:488
  dump("mDestType: TreeNode*");
  AstDumpTreeNode(node->GetDestType());
  dump("mExpr: TreeNode*");
  AstDumpTreeNode(node->GetExpr());
  indent -= 4;
  dump("  }");
  return;
}

void A2C_AstDump::AstDumpParenthesisNode(ParenthesisNode *node) {
  if (node == nullptr) {
    dump("  ParenthesisNode: null");
    return;
  }
  dump("  ParenthesisNode {");
  indent += 4;
  base(node);
  // Declared at shared/include/ast.h:518
  dump("mExpr: TreeNode*");
  AstDumpTreeNode(node->GetExpr());
  indent -= 4;
  dump("  }");
  return;
}

void A2C_AstDump::AstDumpFieldNode(FieldNode *node) {
  if (node == nullptr) {
    dump("  FieldNode: null");
    return;
  }
  dump("  FieldNode {");
  indent += 4;
  base(node);
  // Declared at shared/include/ast.h:537
  dump("mUpper: TreeNode*");
  AstDumpTreeNode(node->GetUpper());
  dump("mField: IdentifierNode*");
  AstDumpIdentifierNode(node->GetField());
  indent -= 4;
  dump("  }");
  return;
}

void A2C_AstDump::AstDumpStructNode(StructNode *node) {
  if (node == nullptr) {
    dump("  StructNode: null");
    return;
  }
  dump("  StructNode {");
  indent += 4;
  base(node);
  // Declared at shared/include/ast.h:573
  dump(std::string("mProp: StructProp: ") + getStructProp(node->GetProp()));

  dump("mStructId: IdentifierNode*");
  AstDumpIdentifierNode(node->GetStructId());
  dump("mFields: SmallVector<class maplefe::IdentifierNode *>, size = " + std::to_string(node->GetFieldsNum()) + " [");
  indent += 2;
  for (unsigned i = 0; i < node->GetFieldsNum(); ++i) {
    dump(std::to_string(i + 1) + ": IdentifierNode*");
    AstDumpIdentifierNode(node->GetField(i));
  }
  indent -= 2;
  dump(" ]");
  indent -= 4;
  dump("  }");
  return;
}

void A2C_AstDump::AstDumpFieldLiteralNode(FieldLiteralNode *node) {
  if (node == nullptr) {
    dump("  FieldLiteralNode: null");
    return;
  }
  dump("  FieldLiteralNode {");
  indent += 4;
  base(node);
  // Declared at shared/include/ast.h:598
  dump("mFieldName: IdentifierNode*");
  AstDumpIdentifierNode(node->GetFieldName());
  dump("mLiteral: TreeNode*");
  AstDumpTreeNode(node->GetLiteral());
  indent -= 4;
  dump("  }");
  return;
}

void A2C_AstDump::AstDumpStructLiteralNode(StructLiteralNode *node) {
  if (node == nullptr) {
    dump("  StructLiteralNode: null");
    return;
  }
  dump("  StructLiteralNode {");
  indent += 4;
  base(node);
  // Declared at shared/include/ast.h:613
  dump("mFields: SmallVector<class maplefe::FieldLiteralNode *>, size = " + std::to_string(node->GetFieldsNum()) +
       " [");
  indent += 2;
  for (unsigned i = 0; i < node->GetFieldsNum(); ++i) {
    dump(std::to_string(i + 1) + ": FieldLiteralNode*");
    AstDumpFieldLiteralNode(node->GetField(i));
  }
  indent -= 2;
  dump(" ]");
  indent -= 4;
  dump("  }");
  return;
}

void A2C_AstDump::AstDumpVarListNode(VarListNode *node) {
  if (node == nullptr) {
    dump("  VarListNode: null");
    return;
  }
  dump("  VarListNode {");
  indent += 4;
  base(node);
  // Declared at shared/include/ast.h:633
  dump("mVars: SmallVector<class maplefe::IdentifierNode *>, size = " + std::to_string(node->GetNum()) + " [");
  indent += 2;
  for (unsigned i = 0; i < node->GetNum(); ++i) {
    dump(std::to_string(i + 1) + ": IdentifierNode*");
    AstDumpIdentifierNode(node->VarAtIndex(i));
  }
  indent -= 2;
  dump(" ]");
  indent -= 4;
  dump("  }");
  return;
}

void A2C_AstDump::AstDumpExprListNode(ExprListNode *node) {
  if (node == nullptr) {
    dump("  ExprListNode: null");
    return;
  }
  dump("  ExprListNode {");
  indent += 4;
  base(node);
  // Declared at shared/include/ast.h:655
  dump("mExprs: SmallVector<class maplefe::TreeNode *>, size = " + std::to_string(node->GetNum()) + " [");
  indent += 2;
  for (unsigned i = 0; i < node->GetNum(); ++i) {
    dump(std::to_string(i + 1) + ": TreeNode*");
    AstDumpTreeNode(node->ExprAtIndex(i));
  }
  indent -= 2;
  dump(" ]");
  indent -= 4;
  dump("  }");
  return;
}

void A2C_AstDump::AstDumpLiteralNode(LiteralNode *node) {
  if (node == nullptr) {
    dump("  LiteralNode: null");
    return;
  }
  dump("  LiteralNode {");
  indent += 4;
  base(node);
  // Declared at shared/include/ast.h:675
  dump(std::string("mData: LitData: LitId, ") + getLitId(node->GetData().mType) + ", " + getLitData(node->GetData()));

  indent -= 4;
  dump("  }");
  return;
}

void A2C_AstDump::AstDumpExceptionNode(ExceptionNode *node) {
  if (node == nullptr) {
    dump("  ExceptionNode: null");
    return;
  }
  dump("  ExceptionNode {");
  indent += 4;
  base(node);
  // Declared at shared/include/ast.h:695
  dump("mException: IdentifierNode*");
  AstDumpIdentifierNode(node->GetException());
  indent -= 4;
  dump("  }");
  return;
}

void A2C_AstDump::AstDumpReturnNode(ReturnNode *node) {
  if (node == nullptr) {
    dump("  ReturnNode: null");
    return;
  }
  dump("  ReturnNode {");
  indent += 4;
  base(node);
  // Declared at shared/include/ast.h:714
  dump("mResult: TreeNode*");
  AstDumpTreeNode(node->GetResult());
  indent -= 4;
  dump("  }");
  return;
}

void A2C_AstDump::AstDumpCondBranchNode(CondBranchNode *node) {
  if (node == nullptr) {
    dump("  CondBranchNode: null");
    return;
  }
  dump("  CondBranchNode {");
  indent += 4;
  base(node);
  // Declared at shared/include/ast.h:726
  dump("mCond: TreeNode*");
  AstDumpTreeNode(node->GetCond());
  dump("mTrueBranch: TreeNode*");
  AstDumpTreeNode(node->GetTrueBranch());
  dump("mFalseBranch: TreeNode*");
  AstDumpTreeNode(node->GetFalseBranch());
  indent -= 4;
  dump("  }");
  return;
}

void A2C_AstDump::AstDumpBreakNode(BreakNode *node) {
  if (node == nullptr) {
    dump("  BreakNode: null");
    return;
  }
  dump("  BreakNode {");
  indent += 4;
  base(node);
  // Declared at shared/include/ast.h:747
  dump("mTarget: TreeNode*");
  AstDumpTreeNode(node->GetTarget());
  indent -= 4;
  dump("  }");
  return;
}

void A2C_AstDump::AstDumpForLoopNode(ForLoopNode *node) {
  if (node == nullptr) {
    dump("  ForLoopNode: null");
    return;
  }
  dump("  ForLoopNode {");
  indent += 4;
  base(node);
  // Declared at shared/include/ast.h:759
  dump("mInits: SmallVector<class maplefe::TreeNode *>, size = " + std::to_string(node->GetInitsNum()) + " [");
  indent += 2;
  for (unsigned i = 0; i < node->GetInitsNum(); ++i) {
    dump(std::to_string(i + 1) + ": TreeNode*");
    AstDumpTreeNode(node->GetInitAtIndex(i));
  }
  indent -= 2;
  dump(" ]");
  dump("mCond: TreeNode*");
  AstDumpTreeNode(node->GetCond());
  dump("mUpdates: SmallVector<class maplefe::TreeNode *>, size = " + std::to_string(node->GetUpdatesNum()) + " [");
  indent += 2;
  for (unsigned i = 0; i < node->GetUpdatesNum(); ++i) {
    dump(std::to_string(i + 1) + ": TreeNode*");
    AstDumpTreeNode(node->GetUpdateAtIndex(i));
  }
  indent -= 2;
  dump(" ]");
  dump("mBody: TreeNode*");
  AstDumpTreeNode(node->GetBody());
  indent -= 4;
  dump("  }");
  return;
}

void A2C_AstDump::AstDumpWhileLoopNode(WhileLoopNode *node) {
  if (node == nullptr) {
    dump("  WhileLoopNode: null");
    return;
  }
  dump("  WhileLoopNode {");
  indent += 4;
  base(node);
  // Declared at shared/include/ast.h:785
  dump("mCond: TreeNode*");
  AstDumpTreeNode(node->GetCond());
  dump("mBody: TreeNode*");
  AstDumpTreeNode(node->GetBody());
  indent -= 4;
  dump("  }");
  return;
}

void A2C_AstDump::AstDumpDoLoopNode(DoLoopNode *node) {
  if (node == nullptr) {
    dump("  DoLoopNode: null");
    return;
  }
  dump("  DoLoopNode {");
  indent += 4;
  base(node);
  // Declared at shared/include/ast.h:802
  dump("mCond: TreeNode*");
  AstDumpTreeNode(node->GetCond());
  dump("mBody: TreeNode*");
  AstDumpTreeNode(node->GetBody());
  indent -= 4;
  dump("  }");
  return;
}

void A2C_AstDump::AstDumpSwitchLabelNode(SwitchLabelNode *node) {
  if (node == nullptr) {
    dump("  SwitchLabelNode: null");
    return;
  }
  dump("  SwitchLabelNode {");
  indent += 4;
  base(node);
  // Declared at shared/include/ast.h:827
  dump(std::string("mIsDefault: bool, ") + (node->IsDefault() ? "true" : "false"));

  dump("mValue: TreeNode*");
  AstDumpTreeNode(node->GetValue());
  indent -= 4;
  dump("  }");
  return;
}

void A2C_AstDump::AstDumpSwitchCaseNode(SwitchCaseNode *node) {
  if (node == nullptr) {
    dump("  SwitchCaseNode: null");
    return;
  }
  dump("  SwitchCaseNode {");
  indent += 4;
  base(node);
  // Declared at shared/include/ast.h:846
  dump("mLabels: SmallVector<class maplefe::TreeNode *>, size = " + std::to_string(node->GetLabelsNum()) + " [");
  indent += 2;
  for (unsigned i = 0; i < node->GetLabelsNum(); ++i) {
    dump(std::to_string(i + 1) + ": TreeNode*");
    AstDumpTreeNode(node->GetLabelAtIndex(i));
  }
  indent -= 2;
  dump(" ]");
  dump("mStmts: SmallVector<class maplefe::TreeNode *>, size = " + std::to_string(node->GetStmtsNum()) + " [");
  indent += 2;
  for (unsigned i = 0; i < node->GetStmtsNum(); ++i) {
    dump(std::to_string(i + 1) + ": TreeNode*");
    AstDumpTreeNode(node->GetStmtAtIndex(i));
  }
  indent -= 2;
  dump(" ]");
  indent -= 4;
  dump("  }");
  return;
}

void A2C_AstDump::AstDumpSwitchNode(SwitchNode *node) {
  if (node == nullptr) {
    dump("  SwitchNode: null");
    return;
  }
  dump("  SwitchNode {");
  indent += 4;
  base(node);
  // Declared at shared/include/ast.h:867
  dump("mCond: TreeNode*");
  AstDumpTreeNode(node->GetCond());
  dump("mCases: SmallVector<class maplefe::TreeNode *>, size = " + std::to_string(node->GetCasesNum()) + " [");
  indent += 2;
  for (unsigned i = 0; i < node->GetCasesNum(); ++i) {
    dump(std::to_string(i + 1) + ": TreeNode*");
    AstDumpTreeNode(node->GetCaseAtIndex(i));
  }
  indent -= 2;
  dump(" ]");
  indent -= 4;
  dump("  }");
  return;
}

void A2C_AstDump::AstDumpAssertNode(AssertNode *node) {
  if (node == nullptr) {
    dump("  AssertNode: null");
    return;
  }
  dump("  AssertNode {");
  indent += 4;
  base(node);
  // Declared at shared/include/ast.h:892
  dump("mExpr: TreeNode*");
  AstDumpTreeNode(node->GetExpr());
  dump("mMsg: TreeNode*");
  AstDumpTreeNode(node->GetMsg());
  indent -= 4;
  dump("  }");
  return;
}

void A2C_AstDump::AstDumpCallNode(CallNode *node) {
  if (node == nullptr) {
    dump("  CallNode: null");
    return;
  }
  dump("  CallNode {");
  indent += 4;
  base(node);
  // Declared at shared/include/ast.h:912
  dump("mMethod: TreeNode*");
  AstDumpTreeNode(node->GetMethod());
  dump("mArgs: ExprListNode<class maplefe::TreeNode *>, size = " + std::to_string(node->GetArgsNum()) + " [");
  indent += 2;
  for (unsigned i = 0; i < node->GetArgsNum(); ++i) {
    dump(std::to_string(i + 1) + ": TreeNode*");
    AstDumpTreeNode(node->GetArg(i));
  }
  indent -= 2;
  dump(" ]");
  indent -= 4;
  dump("  }");
  return;
}

void A2C_AstDump::AstDumpFunctionNode(FunctionNode *node) {
  if (node == nullptr) {
    dump("  FunctionNode: null");
    return;
  }
  dump("  FunctionNode {");
  indent += 4;
  base(node);
  // Declared at shared/include/ast.h:993
  dump("mAttrs: SmallVector<enum maplefe::AttrId>, size = " + std::to_string(node->GetAttrsNum()) + " [");
  indent += 2;
  for (unsigned i = 0; i < node->GetAttrsNum(); ++i) {
    dump(std::to_string(i) + ". AttrId: " + getAttrId(node->GetAttrAtIndex(i)));
  }
  indent -= 2;
  dump(" ]");
  dump("mAnnotations: SmallVector<class maplefe::AnnotationNode *>, size = " +
       std::to_string(node->GetAnnotationsNum()) + " [");
  indent += 2;
  for (unsigned i = 0; i < node->GetAnnotationsNum(); ++i) {
    dump(std::to_string(i + 1) + ": AnnotationNode*");
    AstDumpAnnotationNode(node->GetAnnotationAtIndex(i));
  }
  indent -= 2;
  dump(" ]");
  dump("mThrows: SmallVector<class maplefe::ExceptionNode *>, size = " + std::to_string(node->GetThrowsNum()) + " [");
  indent += 2;
  for (unsigned i = 0; i < node->GetThrowsNum(); ++i) {
    dump(std::to_string(i + 1) + ": ExceptionNode*");
    AstDumpExceptionNode(node->GetThrowAtIndex(i));
  }
  indent -= 2;
  dump(" ]");
  dump("mType: TreeNode*");
  AstDumpTreeNode(node->GetType());
  dump("mParams: SmallVector<class maplefe::TreeNode *>, size = " + std::to_string(node->GetParamsNum()) + " [");
  indent += 2;
  for (unsigned i = 0; i < node->GetParamsNum(); ++i) {
    dump(std::to_string(i + 1) + ": TreeNode*");
    AstDumpTreeNode(node->GetParam(i));
  }
  indent -= 2;
  dump(" ]");
  dump("mBody: BlockNode*");
  AstDumpBlockNode(node->GetBody());
  dump("mDims: DimensionNode*");
  AstDumpDimensionNode(node->GetDims());
  dump(std::string("mIsConstructor: bool, ") + (node->IsConstructor() ? "true" : "false"));

  indent -= 4;
  dump("  }");
  return;
}

void A2C_AstDump::AstDumpInterfaceNode(InterfaceNode *node) {
  if (node == nullptr) {
    dump("  InterfaceNode: null");
    return;
  }
  dump("  InterfaceNode {");
  indent += 4;
  base(node);
  // Declared at shared/include/ast.h:1064
  dump(std::string("mIsAnnotation: bool, ") + (node->IsAnnotation() ? "true" : "false"));

  dump("mSuperInterfaces: SmallVector<class maplefe::InterfaceNode *>, size = " +
       std::to_string(node->GetSuperInterfacesNum()) + " [");
  indent += 2;
  for (unsigned i = 0; i < node->GetSuperInterfacesNum(); ++i) {
    dump(std::to_string(i + 1) + ": InterfaceNode*");
    AstDumpInterfaceNode(node->GetSuperInterfaceAtIndex(i));
  }
  indent -= 2;
  dump(" ]");
  dump("mFields: SmallVector<class maplefe::IdentifierNode *>, size = " + std::to_string(node->GetFieldsNum()) + " [");
  indent += 2;
  for (unsigned i = 0; i < node->GetFieldsNum(); ++i) {
    dump(std::to_string(i + 1) + ": IdentifierNode*");
    AstDumpIdentifierNode(node->GetFieldAtIndex(i));
  }
  indent -= 2;
  dump(" ]");
  dump("mMethods: SmallVector<class maplefe::FunctionNode *>, size = " + std::to_string(node->GetMethodsNum()) + " [");
  indent += 2;
  for (unsigned i = 0; i < node->GetMethodsNum(); ++i) {
    dump(std::to_string(i + 1) + ": FunctionNode*");
    AstDumpFunctionNode(node->GetMethodAtIndex(i));
  }
  indent -= 2;
  dump(" ]");
  indent -= 4;
  dump("  }");
  return;
}

void A2C_AstDump::AstDumpClassNode(ClassNode *node) {
  if (node == nullptr) {
    dump("  ClassNode: null");
    return;
  }
  dump("  ClassNode {");
  indent += 4;
  base(node);
  // Declared at shared/include/ast.h:1116
  dump(std::string("mIsJavaEnum: bool, ") + (node->IsJavaEnum() ? "true" : "false"));

  dump("mSuperClasses: SmallVector<class maplefe::ClassNode *>, size = " + std::to_string(node->GetSuperClassesNum()) +
       " [");
  indent += 2;
  for (unsigned i = 0; i < node->GetSuperClassesNum(); ++i) {
    dump(std::to_string(i + 1) + ": ClassNode*");
    AstDumpClassNode(node->GetSuperClass(i));
  }
  indent -= 2;
  dump(" ]");
  dump("mSuperInterfaces: SmallVector<class maplefe::InterfaceNode *>, size = " +
       std::to_string(node->GetSuperInterfacesNum()) + " [");
  indent += 2;
  for (unsigned i = 0; i < node->GetSuperInterfacesNum(); ++i) {
    dump(std::to_string(i + 1) + ": InterfaceNode*");
    AstDumpInterfaceNode(node->GetSuperInterface(i));
  }
  indent -= 2;
  dump(" ]");
  dump("mAttributes: SmallVector<enum maplefe::AttrId>, size = " + std::to_string(node->GetAttributesNum()) + " [");
  indent += 2;
  for (unsigned i = 0; i < node->GetAttributesNum(); ++i) {
    dump(std::to_string(i) + ". AttrId: " + getAttrId(node->GetAttribute(i)));
  }
  indent -= 2;
  dump(" ]");
  dump("mBody: BlockNode*");
  AstDumpBlockNode(node->GetBody());
  dump("mFields: SmallVector<class maplefe::IdentifierNode *>, size = " + std::to_string(node->GetFieldsNum()) + " [");
  indent += 2;
  for (unsigned i = 0; i < node->GetFieldsNum(); ++i) {
    dump(std::to_string(i + 1) + ": IdentifierNode*");
    AstDumpIdentifierNode(node->GetField(i));
  }
  indent -= 2;
  dump(" ]");
  dump("mMethods: SmallVector<class maplefe::FunctionNode *>, size = " + std::to_string(node->GetMethodsNum()) + " [");
  indent += 2;
  for (unsigned i = 0; i < node->GetMethodsNum(); ++i) {
    dump(std::to_string(i + 1) + ": FunctionNode*");
    AstDumpFunctionNode(node->GetMethod(i));
  }
  indent -= 2;
  dump(" ]");
  dump("mConstructors: SmallVector<class maplefe::FunctionNode *>, size = " +
       std::to_string(node->GetConstructorsNum()) + " [");
  indent += 2;
  for (unsigned i = 0; i < node->GetConstructorsNum(); ++i) {
    dump(std::to_string(i + 1) + ": FunctionNode*");
    AstDumpFunctionNode(node->GetConstructor(i));
  }
  indent -= 2;
  dump(" ]");
  dump("mInstInits: SmallVector<class maplefe::BlockNode *>, size = " + std::to_string(node->GetInstInitsNum()) + " [");
  indent += 2;
  for (unsigned i = 0; i < node->GetInstInitsNum(); ++i) {
    dump(std::to_string(i + 1) + ": BlockNode*");
    AstDumpBlockNode(node->GetInstInit(i));
  }
  indent -= 2;
  dump(" ]");
  dump("mLocalClasses: SmallVector<class maplefe::ClassNode *>, size = " + std::to_string(node->GetLocalClassesNum()) +
       " [");
  indent += 2;
  for (unsigned i = 0; i < node->GetLocalClassesNum(); ++i) {
    dump(std::to_string(i + 1) + ": ClassNode*");
    AstDumpClassNode(node->GetLocalClass(i));
  }
  indent -= 2;
  dump(" ]");
  dump("mLocalInterfaces: SmallVector<class maplefe::InterfaceNode *>, size = " +
       std::to_string(node->GetLocalInterfacesNum()) + " [");
  indent += 2;
  for (unsigned i = 0; i < node->GetLocalInterfacesNum(); ++i) {
    dump(std::to_string(i + 1) + ": InterfaceNode*");
    AstDumpInterfaceNode(node->GetLocalInterface(i));
  }
  indent -= 2;
  dump(" ]");
  indent -= 4;
  dump("  }");
  return;
}

void A2C_AstDump::AstDumpPassNode(PassNode *node) {
  if (node == nullptr) {
    dump("  PassNode: null");
    return;
  }
  dump("  PassNode {");
  indent += 4;
  base(node);
  // Declared at shared/include/ast.h:1183
  dump("mChildren: SmallVector<class maplefe::TreeNode *>, size = " + std::to_string(node->GetChildrenNum()) + " [");
  indent += 2;
  for (unsigned i = 0; i < node->GetChildrenNum(); ++i) {
    dump(std::to_string(i + 1) + ": TreeNode*");
    AstDumpTreeNode(node->GetChild(i));
  }
  indent -= 2;
  dump(" ]");
  indent -= 4;
  dump("  }");
  return;
}

void A2C_AstDump::AstDumpLambdaNode(LambdaNode *node) {
  if (node == nullptr) {
    dump("  LambdaNode: null");
    return;
  }
  dump("  LambdaNode {");
  indent += 4;
  base(node);
  // Declared at shared/include/ast.h:1214
  dump(std::string("mProperty: LambdaProperty: ") + getLambdaProperty(node->GetProperty()));

  dump("mType: TreeNode*");
  AstDumpTreeNode(node->GetType());
  dump("mParams: SmallVector<class maplefe::TreeNode *>, size = " + std::to_string(node->GetParamsNum()) + " [");
  indent += 2;
  for (unsigned i = 0; i < node->GetParamsNum(); ++i) {
    dump(std::to_string(i + 1) + ": TreeNode*");
    AstDumpTreeNode(node->GetParam(i));
  }
  indent -= 2;
  dump(" ]");
  dump("mBody: TreeNode*");
  AstDumpTreeNode(node->GetBody());
  indent -= 4;
  dump("  }");
  return;
}

void A2C_AstDump::AstDumpInstanceOfNode(InstanceOfNode *node) {
  if (node == nullptr) {
    dump("  InstanceOfNode: null");
    return;
  }
  dump("  InstanceOfNode {");
  indent += 4;
  base(node);
  // Declared at shared/include/ast.h:1249
  dump("mLeft: TreeNode*");
  AstDumpTreeNode(node->GetLeft());
  dump("mRight: TreeNode*");
  AstDumpTreeNode(node->GetRight());
  indent -= 4;
  dump("  }");
  return;
}

void A2C_AstDump::AstDumpAttrNode(AttrNode *node) {
  if (node == nullptr) {
    dump("  AttrNode: null");
    return;
  }
  dump("  AttrNode {");
  indent += 4;
  base(node);
  // Declared at shared/include/ast_attr.h:39
  dump(std::string("mId: AttrId: ") + getAttrId(node->GetId()));

  indent -= 4;
  dump("  }");
  return;
}

void A2C_AstDump::AstDumpUserTypeNode(UserTypeNode *node) {
  if (node == nullptr) {
    dump("  UserTypeNode: null");
    return;
  }
  dump("  UserTypeNode {");
  indent += 4;
  base(node);
  // Declared at shared/include/ast_type.h:70
  dump("mId: IdentifierNode*");
  AstDumpIdentifierNode(node->GetId());
  dump("mTypeArguments: SmallVector<class maplefe::IdentifierNode *>, size = " +
       std::to_string(node->GetTypeArgumentsNum()) + " [");
  indent += 2;
  for (unsigned i = 0; i < node->GetTypeArgumentsNum(); ++i) {
    dump(std::to_string(i + 1) + ": IdentifierNode*");
    AstDumpIdentifierNode(node->GetTypeArgument(i));
  }
  indent -= 2;
  dump(" ]");
  indent -= 4;
  dump("  }");
  return;
}

void A2C_AstDump::AstDumpPrimTypeNode(PrimTypeNode *node) {
  if (node == nullptr) {
    dump("  PrimTypeNode: null");
    return;
  }
  dump("  PrimTypeNode {");
  indent += 4;
  base(node);
  // Declared at shared/include/ast_type.h:102
  dump(std::string("mPrimType: TypeId: ") + getTypeId(node->GetPrimType()));

  indent -= 4;
  dump("  }");
  return;
}

void A2C_AstDump::AstDumpPrimArrayTypeNode(PrimArrayTypeNode *node) {
  if (node == nullptr) {
    dump("  PrimArrayTypeNode: null");
    return;
  }
  dump("  PrimArrayTypeNode {");
  indent += 4;
  base(node);
  // Declared at shared/include/ast_type.h:116
  dump("mPrim: PrimTypeNode*");
  AstDumpPrimTypeNode(node->GetPrim());
  dump("mDims: DimensionNode*");
  AstDumpDimensionNode(node->GetDims());
  indent -= 4;
  dump("  }");
  return;
}

void A2C_AstDump::AstDumpTreeNode(TreeNode *node) {
  if (node == nullptr) {
    dump("  TreeNode: null");
    return;
  }
  switch (node->GetKind()) {
  case NK_Package:
    AstDumpPackageNode(static_cast<PackageNode *>(node));
    break;
  case NK_Import:
    AstDumpImportNode(static_cast<ImportNode *>(node));
    break;
  case NK_Decl:
    AstDumpDeclNode(static_cast<DeclNode *>(node));
    break;
  case NK_Identifier:
    AstDumpIdentifierNode(static_cast<IdentifierNode *>(node));
    break;
  case NK_Field:
    AstDumpFieldNode(static_cast<FieldNode *>(node));
    break;
  case NK_Dimension:
    AstDumpDimensionNode(static_cast<DimensionNode *>(node));
    break;
  case NK_Attr:
    AstDumpAttrNode(static_cast<AttrNode *>(node));
    break;
  case NK_PrimType:
    AstDumpPrimTypeNode(static_cast<PrimTypeNode *>(node));
    break;
  case NK_PrimArrayType:
    AstDumpPrimArrayTypeNode(static_cast<PrimArrayTypeNode *>(node));
    break;
  case NK_UserType:
    AstDumpUserTypeNode(static_cast<UserTypeNode *>(node));
    break;
  case NK_Cast:
    AstDumpCastNode(static_cast<CastNode *>(node));
    break;
  case NK_Parenthesis:
    AstDumpParenthesisNode(static_cast<ParenthesisNode *>(node));
    break;
  case NK_Struct:
    AstDumpStructNode(static_cast<StructNode *>(node));
    break;
  case NK_StructLiteral:
    AstDumpStructLiteralNode(static_cast<StructLiteralNode *>(node));
    break;
  case NK_FieldLiteral:
    AstDumpFieldLiteralNode(static_cast<FieldLiteralNode *>(node));
    break;
  case NK_VarList:
    AstDumpVarListNode(static_cast<VarListNode *>(node));
    break;
  case NK_ExprList:
    AstDumpExprListNode(static_cast<ExprListNode *>(node));
    break;
  case NK_Literal:
    AstDumpLiteralNode(static_cast<LiteralNode *>(node));
    break;
  case NK_UnaOperator:
    AstDumpUnaOperatorNode(static_cast<UnaOperatorNode *>(node));
    break;
  case NK_BinOperator:
    AstDumpBinOperatorNode(static_cast<BinOperatorNode *>(node));
    break;
  case NK_TerOperator:
    AstDumpTerOperatorNode(static_cast<TerOperatorNode *>(node));
    break;
  case NK_Lambda:
    AstDumpLambdaNode(static_cast<LambdaNode *>(node));
    break;
  case NK_InstanceOf:
    AstDumpInstanceOfNode(static_cast<InstanceOfNode *>(node));
    break;
  case NK_Block:
    AstDumpBlockNode(static_cast<BlockNode *>(node));
    break;
  case NK_Function:
    AstDumpFunctionNode(static_cast<FunctionNode *>(node));
    break;
  case NK_Class:
    AstDumpClassNode(static_cast<ClassNode *>(node));
    break;
  case NK_Interface:
    AstDumpInterfaceNode(static_cast<InterfaceNode *>(node));
    break;
  case NK_AnnotationType:
    AstDumpAnnotationTypeNode(static_cast<AnnotationTypeNode *>(node));
    break;
  case NK_Annotation:
    AstDumpAnnotationNode(static_cast<AnnotationNode *>(node));
    break;
  case NK_Exception:
    AstDumpExceptionNode(static_cast<ExceptionNode *>(node));
    break;
  case NK_Return:
    AstDumpReturnNode(static_cast<ReturnNode *>(node));
    break;
  case NK_CondBranch:
    AstDumpCondBranchNode(static_cast<CondBranchNode *>(node));
    break;
  case NK_Break:
    AstDumpBreakNode(static_cast<BreakNode *>(node));
    break;
  case NK_ForLoop:
    AstDumpForLoopNode(static_cast<ForLoopNode *>(node));
    break;
  case NK_WhileLoop:
    AstDumpWhileLoopNode(static_cast<WhileLoopNode *>(node));
    break;
  case NK_DoLoop:
    AstDumpDoLoopNode(static_cast<DoLoopNode *>(node));
    break;
  case NK_New:
    AstDumpNewNode(static_cast<NewNode *>(node));
    break;
  case NK_Delete:
    AstDumpDeleteNode(static_cast<DeleteNode *>(node));
    break;
  case NK_Call:
    AstDumpCallNode(static_cast<CallNode *>(node));
    break;
  case NK_Assert:
    AstDumpAssertNode(static_cast<AssertNode *>(node));
    break;
  case NK_SwitchLabel:
    AstDumpSwitchLabelNode(static_cast<SwitchLabelNode *>(node));
    break;
  case NK_SwitchCase:
    AstDumpSwitchCaseNode(static_cast<SwitchCaseNode *>(node));
    break;
  case NK_Switch:
    AstDumpSwitchNode(static_cast<SwitchNode *>(node));
    break;
  case NK_Pass:
    AstDumpPassNode(static_cast<PassNode *>(node));
    break;
  case NK_Null:
    // Ignore NullNode
    break;
  default:; // Unexpected kind
  }
  return;
}

const char *A2C_AstDump::getTypeId(TypeId k) {
  switch (k) {
  case TY_Boolean:
    return "TY_Boolean";
  case TY_Byte:
    return "TY_Byte";
  case TY_Short:
    return "TY_Short";
  case TY_Int:
    return "TY_Int";
  case TY_Long:
    return "TY_Long";
  case TY_Char:
    return "TY_Char";
  case TY_Float:
    return "TY_Float";
  case TY_Double:
    return "TY_Double";
  case TY_Void:
    return "TY_Void";
  case TY_Null:
    return "TY_Null";
  case TY_Undefined:
    return "TY_Undefined";
  case TY_String:
    return "TY_String";
  case TY_Number:
    return "TY_Number";
  case TY_Any:
    return "TY_Any";
  case TY_NA:
    return "TY_NA";
  default:; // Unexpected kind
  }
  return "UNEXPECTED TypeId";
}

const char *A2C_AstDump::getSepId(SepId k) {
  switch (k) {
  case SEP_Lparen:
    return "SEP_Lparen";
  case SEP_Rparen:
    return "SEP_Rparen";
  case SEP_Lbrace:
    return "SEP_Lbrace";
  case SEP_Rbrace:
    return "SEP_Rbrace";
  case SEP_Lbrack:
    return "SEP_Lbrack";
  case SEP_Rbrack:
    return "SEP_Rbrack";
  case SEP_Semicolon:
    return "SEP_Semicolon";
  case SEP_Comma:
    return "SEP_Comma";
  case SEP_Dot:
    return "SEP_Dot";
  case SEP_Dotdotdot:
    return "SEP_Dotdotdot";
  case SEP_Colon:
    return "SEP_Colon";
  case SEP_Of:
    return "SEP_Of";
  case SEP_At:
    return "SEP_At";
  case SEP_Pound:
    return "SEP_Pound";
  case SEP_Whitespace:
    return "SEP_Whitespace";
  case SEP_ArrowFunction:
    return "SEP_ArrowFunction";
  case SEP_NA:
    return "SEP_NA";
  default:; // Unexpected kind
  }
  return "UNEXPECTED SepId";
}

const char *A2C_AstDump::getOprId(OprId k) {
  switch (k) {
  case OPR_Add:
    return "OPR_Add";
  case OPR_Sub:
    return "OPR_Sub";
  case OPR_Mul:
    return "OPR_Mul";
  case OPR_Div:
    return "OPR_Div";
  case OPR_Mod:
    return "OPR_Mod";
  case OPR_Inc:
    return "OPR_Inc";
  case OPR_Dec:
    return "OPR_Dec";
  case OPR_EQ:
    return "OPR_EQ";
  case OPR_NE:
    return "OPR_NE";
  case OPR_GT:
    return "OPR_GT";
  case OPR_LT:
    return "OPR_LT";
  case OPR_GE:
    return "OPR_GE";
  case OPR_LE:
    return "OPR_LE";
  case OPR_Band:
    return "OPR_Band";
  case OPR_Bor:
    return "OPR_Bor";
  case OPR_Bxor:
    return "OPR_Bxor";
  case OPR_Bcomp:
    return "OPR_Bcomp";
  case OPR_Shl:
    return "OPR_Shl";
  case OPR_Shr:
    return "OPR_Shr";
  case OPR_Zext:
    return "OPR_Zext";
  case OPR_Land:
    return "OPR_Land";
  case OPR_Lor:
    return "OPR_Lor";
  case OPR_Not:
    return "OPR_Not";
  case OPR_Assign:
    return "OPR_Assign";
  case OPR_AddAssign:
    return "OPR_AddAssign";
  case OPR_SubAssign:
    return "OPR_SubAssign";
  case OPR_MulAssign:
    return "OPR_MulAssign";
  case OPR_DivAssign:
    return "OPR_DivAssign";
  case OPR_ModAssign:
    return "OPR_ModAssign";
  case OPR_ShlAssign:
    return "OPR_ShlAssign";
  case OPR_ShrAssign:
    return "OPR_ShrAssign";
  case OPR_BandAssign:
    return "OPR_BandAssign";
  case OPR_BorAssign:
    return "OPR_BorAssign";
  case OPR_BxorAssign:
    return "OPR_BxorAssign";
  case OPR_ZextAssign:
    return "OPR_ZextAssign";
  case OPR_Arrow:
    return "OPR_Arrow";
  case OPR_Select:
    return "OPR_Select";
  case OPR_Cond:
    return "OPR_Cond";
  case OPR_Diamond:
    return "OPR_Diamond";
  case OPR_StEq:
    return "OPR_StEq";
  case OPR_StNe:
    return "OPR_StNe";
  case OPR_ArrowFunction:
    return "OPR_ArrowFunction";
  case OPR_NA:
    return "OPR_NA";
  default:; // Unexpected kind
  }
  return "UNEXPECTED OprId";
}

const char *A2C_AstDump::getLitId(LitId k) {
  switch (k) {
  case LT_IntegerLiteral:
    return "LT_IntegerLiteral";
  case LT_FPLiteral:
    return "LT_FPLiteral";
  case LT_DoubleLiteral:
    return "LT_DoubleLiteral";
  case LT_BooleanLiteral:
    return "LT_BooleanLiteral";
  case LT_CharacterLiteral:
    return "LT_CharacterLiteral";
  case LT_StringLiteral:
    return "LT_StringLiteral";
  case LT_NullLiteral:
    return "LT_NullLiteral";
  case LT_ThisLiteral:
    return "LT_ThisLiteral";
  case LT_SuperLiteral:
    return "LT_SuperLiteral";
  case LT_NA:
    return "LT_NA";
  default:; // Unexpected kind
  }
  return "UNEXPECTED LitId";
}

const char *A2C_AstDump::getAttrId(AttrId k) {
  switch (k) {
  case ATTR_abstract:
    return "ATTR_abstract";
  case ATTR_const:
    return "ATTR_const";
  case ATTR_volatile:
    return "ATTR_volatile";
  case ATTR_final:
    return "ATTR_final";
  case ATTR_native:
    return "ATTR_native";
  case ATTR_private:
    return "ATTR_private";
  case ATTR_protected:
    return "ATTR_protected";
  case ATTR_public:
    return "ATTR_public";
  case ATTR_static:
    return "ATTR_static";
  case ATTR_strictfp:
    return "ATTR_strictfp";
  case ATTR_default:
    return "ATTR_default";
  case ATTR_synchronized:
    return "ATTR_synchronized";
  case ATTR_NA:
    return "ATTR_NA";
  default:; // Unexpected kind
  }
  return "UNEXPECTED AttrId";
}

const char *A2C_AstDump::getActionId(ActionId k) {
  switch (k) {
  case ACT_BuildPackageName:
    return "ACT_BuildPackageName";
  case ACT_BuildSingleTypeImport:
    return "ACT_BuildSingleTypeImport";
  case ACT_BuildAllTypeImport:
    return "ACT_BuildAllTypeImport";
  case ACT_BuildSingleStaticImport:
    return "ACT_BuildSingleStaticImport";
  case ACT_BuildAllStaticImport:
    return "ACT_BuildAllStaticImport";
  case ACT_BuildAllImport:
    return "ACT_BuildAllImport";
  case ACT_BuildBlock:
    return "ACT_BuildBlock";
  case ACT_AddToBlock:
    return "ACT_AddToBlock";
  case ACT_AddSyncToBlock:
    return "ACT_AddSyncToBlock";
  case ACT_BuildCast:
    return "ACT_BuildCast";
  case ACT_BuildParenthesis:
    return "ACT_BuildParenthesis";
  case ACT_BuildBinaryOperation:
    return "ACT_BuildBinaryOperation";
  case ACT_BuildUnaryOperation:
    return "ACT_BuildUnaryOperation";
  case ACT_BuildPostfixOperation:
    return "ACT_BuildPostfixOperation";
  case ACT_BuildLambda:
    return "ACT_BuildLambda";
  case ACT_BuildInstanceOf:
    return "ACT_BuildInstanceOf";
  case ACT_BuildDecl:
    return "ACT_BuildDecl";
  case ACT_SetJSVar:
    return "ACT_SetJSVar";
  case ACT_SetJSLet:
    return "ACT_SetJSLet";
  case ACT_SetJSConst:
    return "ACT_SetJSConst";
  case ACT_BuildField:
    return "ACT_BuildField";
  case ACT_BuildVarList:
    return "ACT_BuildVarList";
  case ACT_BuildStruct:
    return "ACT_BuildStruct";
  case ACT_SetTSInterface:
    return "ACT_SetTSInterface";
  case ACT_AddStructField:
    return "ACT_AddStructField";
  case ACT_BuildFieldLiteral:
    return "ACT_BuildFieldLiteral";
  case ACT_BuildStructLiteral:
    return "ACT_BuildStructLiteral";
  case ACT_AddParams:
    return "ACT_AddParams";
  case ACT_BuildFunction:
    return "ACT_BuildFunction";
  case ACT_BuildConstructor:
    return "ACT_BuildConstructor";
  case ACT_AddFunctionBody:
    return "ACT_AddFunctionBody";
  case ACT_AddFunctionBodyTo:
    return "ACT_AddFunctionBodyTo";
  case ACT_BuildCall:
    return "ACT_BuildCall";
  case ACT_AddArguments:
    return "ACT_AddArguments";
  case ACT_BuildExprList:
    return "ACT_BuildExprList";
  case ACT_BuildClass:
    return "ACT_BuildClass";
  case ACT_SetClassIsJavaEnum:
    return "ACT_SetClassIsJavaEnum";
  case ACT_AddSuperClass:
    return "ACT_AddSuperClass";
  case ACT_AddSuperInterface:
    return "ACT_AddSuperInterface";
  case ACT_AddClassBody:
    return "ACT_AddClassBody";
  case ACT_BuildInstInit:
    return "ACT_BuildInstInit";
  case ACT_AddModifier:
    return "ACT_AddModifier";
  case ACT_AddModifierTo:
    return "ACT_AddModifierTo";
  case ACT_AddInitTo:
    return "ACT_AddInitTo";
  case ACT_AddType:
    return "ACT_AddType";
  case ACT_BuildAnnotationType:
    return "ACT_BuildAnnotationType";
  case ACT_BuildAnnotation:
    return "ACT_BuildAnnotation";
  case ACT_AddAnnotationTypeBody:
    return "ACT_AddAnnotationTypeBody";
  case ACT_BuildInterface:
    return "ACT_BuildInterface";
  case ACT_AddInterfaceBody:
    return "ACT_AddInterfaceBody";
  case ACT_BuildDim:
    return "ACT_BuildDim";
  case ACT_BuildDims:
    return "ACT_BuildDims";
  case ACT_AddDims:
    return "ACT_AddDims";
  case ACT_AddDimsTo:
    return "ACT_AddDimsTo";
  case ACT_BuildAssignment:
    return "ACT_BuildAssignment";
  case ACT_BuildAssert:
    return "ACT_BuildAssert";
  case ACT_BuildReturn:
    return "ACT_BuildReturn";
  case ACT_BuildCondBranch:
    return "ACT_BuildCondBranch";
  case ACT_AddCondBranchTrueStatement:
    return "ACT_AddCondBranchTrueStatement";
  case ACT_AddCondBranchFalseStatement:
    return "ACT_AddCondBranchFalseStatement";
  case ACT_AddLabel:
    return "ACT_AddLabel";
  case ACT_BuildBreak:
    return "ACT_BuildBreak";
  case ACT_BuildForLoop:
    return "ACT_BuildForLoop";
  case ACT_BuildWhileLoop:
    return "ACT_BuildWhileLoop";
  case ACT_BuildDoLoop:
    return "ACT_BuildDoLoop";
  case ACT_BuildNewOperation:
    return "ACT_BuildNewOperation";
  case ACT_BuildDeleteOperation:
    return "ACT_BuildDeleteOperation";
  case ACT_BuildSwitchLabel:
    return "ACT_BuildSwitchLabel";
  case ACT_BuildDefaultSwitchLabel:
    return "ACT_BuildDefaultSwitchLabel";
  case ACT_BuildOneCase:
    return "ACT_BuildOneCase";
  case ACT_BuildAllCases:
    return "ACT_BuildAllCases";
  case ACT_BuildSwitch:
    return "ACT_BuildSwitch";
  case ACT_BuildThrows:
    return "ACT_BuildThrows";
  case ACT_AddThrowsTo:
    return "ACT_AddThrowsTo";
  case ACT_BuildUserType:
    return "ACT_BuildUserType";
  case ACT_AddTypeArgument:
    return "ACT_AddTypeArgument";
  case ACT_PassChild:
    return "ACT_PassChild";
  case ACT_NA:
    return "ACT_NA";
  default:; // Unexpected kind
  }
  return "UNEXPECTED ActionId";
}

const char *A2C_AstDump::getTK_Type(TK_Type k) {
  switch (k) {
  case TT_ID:
    return "TT_ID";
  case TT_KW:
    return "TT_KW";
  case TT_LT:
    return "TT_LT";
  case TT_SP:
    return "TT_SP";
  case TT_OP:
    return "TT_OP";
  case TT_CM:
    return "TT_CM";
  case TT_NA:
    return "TT_NA";
  default:; // Unexpected kind
  }
  return "UNEXPECTED TK_Type";
}

const char *A2C_AstDump::getEntryType(EntryType k) {
  switch (k) {
  case ET_Oneof:
    return "ET_Oneof";
  case ET_Zeroormore:
    return "ET_Zeroormore";
  case ET_Zeroorone:
    return "ET_Zeroorone";
  case ET_Concatenate:
    return "ET_Concatenate";
  case ET_Data:
    return "ET_Data";
  case ET_Null:
    return "ET_Null";
  default:; // Unexpected kind
  }
  return "UNEXPECTED EntryType";
}

const char *A2C_AstDump::getDataType(DataType k) {
  switch (k) {
  case DT_Char:
    return "DT_Char";
  case DT_String:
    return "DT_String";
  case DT_Type:
    return "DT_Type";
  case DT_Token:
    return "DT_Token";
  case DT_Subtable:
    return "DT_Subtable";
  case DT_Null:
    return "DT_Null";
  default:; // Unexpected kind
  }
  return "UNEXPECTED DataType";
}

const char *A2C_AstDump::getRuleProp(RuleProp k) {
  switch (k) {
  case RP_NA:
    return "RP_NA";
  case RP_Single:
    return "RP_Single";
  case RP_ZomFast:
    return "RP_ZomFast";
  case RP_Top:
    return "RP_Top";
  case RP_SecondTry:
    return "RP_SecondTry";
  default:; // Unexpected kind
  }
  return "UNEXPECTED RuleProp";
}

const char *A2C_AstDump::getLAType(LAType k) {
  switch (k) {
  case LA_Char:
    return "LA_Char";
  case LA_String:
    return "LA_String";
  case LA_Token:
    return "LA_Token";
  case LA_Identifier:
    return "LA_Identifier";
  case LA_Literal:
    return "LA_Literal";
  case LA_NA:
    return "LA_NA";
  default:; // Unexpected kind
  }
  return "UNEXPECTED LAType";
}

const char *A2C_AstDump::getNodeKind(NodeKind k) {
  switch (k) {
  case NK_Package:
    return "NK_Package";
  case NK_Import:
    return "NK_Import";
  case NK_Decl:
    return "NK_Decl";
  case NK_Identifier:
    return "NK_Identifier";
  case NK_Field:
    return "NK_Field";
  case NK_Dimension:
    return "NK_Dimension";
  case NK_Attr:
    return "NK_Attr";
  case NK_PrimType:
    return "NK_PrimType";
  case NK_PrimArrayType:
    return "NK_PrimArrayType";
  case NK_UserType:
    return "NK_UserType";
  case NK_Cast:
    return "NK_Cast";
  case NK_Parenthesis:
    return "NK_Parenthesis";
  case NK_Struct:
    return "NK_Struct";
  case NK_StructLiteral:
    return "NK_StructLiteral";
  case NK_FieldLiteral:
    return "NK_FieldLiteral";
  case NK_VarList:
    return "NK_VarList";
  case NK_ExprList:
    return "NK_ExprList";
  case NK_Literal:
    return "NK_Literal";
  case NK_UnaOperator:
    return "NK_UnaOperator";
  case NK_BinOperator:
    return "NK_BinOperator";
  case NK_TerOperator:
    return "NK_TerOperator";
  case NK_Lambda:
    return "NK_Lambda";
  case NK_InstanceOf:
    return "NK_InstanceOf";
  case NK_Block:
    return "NK_Block";
  case NK_Function:
    return "NK_Function";
  case NK_Class:
    return "NK_Class";
  case NK_Interface:
    return "NK_Interface";
  case NK_AnnotationType:
    return "NK_AnnotationType";
  case NK_Annotation:
    return "NK_Annotation";
  case NK_Exception:
    return "NK_Exception";
  case NK_Return:
    return "NK_Return";
  case NK_CondBranch:
    return "NK_CondBranch";
  case NK_Break:
    return "NK_Break";
  case NK_ForLoop:
    return "NK_ForLoop";
  case NK_WhileLoop:
    return "NK_WhileLoop";
  case NK_DoLoop:
    return "NK_DoLoop";
  case NK_New:
    return "NK_New";
  case NK_Delete:
    return "NK_Delete";
  case NK_Call:
    return "NK_Call";
  case NK_Assert:
    return "NK_Assert";
  case NK_SwitchLabel:
    return "NK_SwitchLabel";
  case NK_SwitchCase:
    return "NK_SwitchCase";
  case NK_Switch:
    return "NK_Switch";
  case NK_Pass:
    return "NK_Pass";
  case NK_Null:
    return "NK_Null";
  default:; // Unexpected kind
  }
  return "UNEXPECTED NodeKind";
}

const char *A2C_AstDump::getImportProperty(ImportProperty k) {
  switch (k) {
  case ImpNone:
    return "ImpNone";
  case ImpType:
    return "ImpType";
  case ImpStatic:
    return "ImpStatic";
  case ImpSingle:
    return "ImpSingle";
  case ImpAll:
    return "ImpAll";
  case ImpLocal:
    return "ImpLocal";
  case ImpSystem:
    return "ImpSystem";
  default:; // Unexpected kind
  }
  return "UNEXPECTED ImportProperty";
}

const char *A2C_AstDump::getOperatorProperty(OperatorProperty k) {
  switch (k) {
  case Unary:
    return "Unary";
  case Binary:
    return "Binary";
  case Ternary:
    return "Ternary";
  case Pre:
    return "Pre";
  case Post:
    return "Post";
  case OperatorProperty_NA:
    return "OperatorProperty_NA";
  default:; // Unexpected kind
  }
  return "UNEXPECTED OperatorProperty";
}

const char *A2C_AstDump::getDeclProp(DeclProp k) {
  switch (k) {
  case JS_Var:
    return "JS_Var";
  case JS_Let:
    return "JS_Let";
  case JS_Const:
    return "JS_Const";
  case DP_NA:
    return "DP_NA";
  default:; // Unexpected kind
  }
  return "UNEXPECTED DeclProp";
}

const char *A2C_AstDump::getStructProp(StructProp k) {
  switch (k) {
  case SProp_CStruct:
    return "SProp_CStruct";
  case SProp_TSInterface:
    return "SProp_TSInterface";
  case SProp_NA:
    return "SProp_NA";
  default:; // Unexpected kind
  }
  return "UNEXPECTED StructProp";
}

const char *A2C_AstDump::getLambdaProperty(LambdaProperty k) {
  switch (k) {
  case LP_JavaLambda:
    return "LP_JavaLambda";
  case LP_JSArrowFunction:
    return "LP_JSArrowFunction";
  case LP_TSFunctionType:
    return "LP_TSFunctionType";
  case LP_TSConstructorType:
    return "LP_TSConstructorType";
  case LP_NA:
    return "LP_NA";
  default:; // Unexpected kind
  }
  return "UNEXPECTED LambdaProperty";
}

} // namespace maplefe
