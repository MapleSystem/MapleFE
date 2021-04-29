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

#ifndef __A2C_ASTDUMP_HEADER__
#define __A2C_ASTDUMP_HEADER__

#include "ast.h"
#include "ast_attr.h"
#include "ast_module.h"
#include "ast_type.h"

namespace maplefe {

class A2C_AstDump {
public:
private:
  int indent;

public:
  A2C_AstDump() : indent(0) {}

  void dump(TreeNode *node) { AstDumpTreeNode(node); }

private:
  void dump(const std::string &msg) { std::cout << std::string(indent, ' ') << msg << std::endl; }

  void base(TreeNode *node) {
    dump(std::string(". mKind: NodeKind, ") + getNodeKind(node->GetKind()));
    const char *name = node->GetName();
    if (name)
      dump(std::string(". mName: const char*, \"") + node->GetName() + "\"");
    TreeNode *label = node->GetLabel();
    if (label) {
      dump(". mLabel: TreeNode*: ");
      AstDumpTreeNode(label);
    }
  }

  std::string getLitData(LitData lit) {
    std::string str = getLitId(lit.mType);
    switch (lit.mType) {
    case LT_IntegerLiteral:
      return std::to_string(lit.mData.mInt);
    case LT_FPLiteral:
      return std::to_string(lit.mData.mFloat);
    case LT_DoubleLiteral:
      return std::to_string(lit.mData.mDouble);
    case LT_BooleanLiteral:
      return std::to_string(lit.mData.mBool);
    case LT_CharacterLiteral:
      return std::string(1, lit.mData.mChar.mData.mChar); // TODO: Unicode support
    case LT_StringLiteral:
      return std::string(lit.mData.mStr);
    case LT_NullLiteral:
      return std::string("null");
    case LT_ThisLiteral:
      return std::string("this");
    case LT_SuperLiteral:
      return std::string("super");
    case LT_NA:
      return std::string("NA");
    default:;
    }
  }

  AnnotationNode *AstDumpAnnotationNode(AnnotationNode *node);
  PackageNode *AstDumpPackageNode(PackageNode *node);
  ImportNode *AstDumpImportNode(ImportNode *node);
  UnaOperatorNode *AstDumpUnaOperatorNode(UnaOperatorNode *node);
  BinOperatorNode *AstDumpBinOperatorNode(BinOperatorNode *node);
  TerOperatorNode *AstDumpTerOperatorNode(TerOperatorNode *node);
  BlockNode *AstDumpBlockNode(BlockNode *node);
  NewNode *AstDumpNewNode(NewNode *node);
  DeleteNode *AstDumpDeleteNode(DeleteNode *node);
  DimensionNode *AstDumpDimensionNode(DimensionNode *node);
  IdentifierNode *AstDumpIdentifierNode(IdentifierNode *node);
  DeclNode *AstDumpDeclNode(DeclNode *node);
  AnnotationTypeNode *AstDumpAnnotationTypeNode(AnnotationTypeNode *node);
  CastNode *AstDumpCastNode(CastNode *node);
  ParenthesisNode *AstDumpParenthesisNode(ParenthesisNode *node);
  FieldNode *AstDumpFieldNode(FieldNode *node);
  StructNode *AstDumpStructNode(StructNode *node);
  FieldLiteralNode *AstDumpFieldLiteralNode(FieldLiteralNode *node);
  StructLiteralNode *AstDumpStructLiteralNode(StructLiteralNode *node);
  VarListNode *AstDumpVarListNode(VarListNode *node);
  ExprListNode *AstDumpExprListNode(ExprListNode *node);
  LiteralNode *AstDumpLiteralNode(LiteralNode *node);
  ExceptionNode *AstDumpExceptionNode(ExceptionNode *node);
  ReturnNode *AstDumpReturnNode(ReturnNode *node);
  CondBranchNode *AstDumpCondBranchNode(CondBranchNode *node);
  BreakNode *AstDumpBreakNode(BreakNode *node);
  ForLoopNode *AstDumpForLoopNode(ForLoopNode *node);
  WhileLoopNode *AstDumpWhileLoopNode(WhileLoopNode *node);
  DoLoopNode *AstDumpDoLoopNode(DoLoopNode *node);
  SwitchLabelNode *AstDumpSwitchLabelNode(SwitchLabelNode *node);
  SwitchCaseNode *AstDumpSwitchCaseNode(SwitchCaseNode *node);
  SwitchNode *AstDumpSwitchNode(SwitchNode *node);
  AssertNode *AstDumpAssertNode(AssertNode *node);
  CallNode *AstDumpCallNode(CallNode *node);
  FunctionNode *AstDumpFunctionNode(FunctionNode *node);
  InterfaceNode *AstDumpInterfaceNode(InterfaceNode *node);
  ClassNode *AstDumpClassNode(ClassNode *node);
  PassNode *AstDumpPassNode(PassNode *node);
  LambdaNode *AstDumpLambdaNode(LambdaNode *node);
  InstanceOfNode *AstDumpInstanceOfNode(InstanceOfNode *node);
  AttrNode *AstDumpAttrNode(AttrNode *node);
  UserTypeNode *AstDumpUserTypeNode(UserTypeNode *node);
  PrimTypeNode *AstDumpPrimTypeNode(PrimTypeNode *node);
  PrimArrayTypeNode *AstDumpPrimArrayTypeNode(PrimArrayTypeNode *node);

  TreeNode *AstDumpTreeNode(TreeNode *node);

  const char *getTypeId(TypeId k);
  const char *getSepId(SepId k);
  const char *getOprId(OprId k);
  const char *getLitId(LitId k);
  const char *getAttrId(AttrId k);
  const char *getNodeKind(NodeKind k);
  const char *getImportProperty(ImportProperty k);
  const char *getOperatorProperty(OperatorProperty k);
  const char *getDeclProp(DeclProp k);
  const char *getStructProp(StructProp k);
};

} // namespace maplefe
#endif
