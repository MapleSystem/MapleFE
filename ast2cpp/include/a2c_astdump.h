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

  void AstDumpAnnotationNode(AnnotationNode *node);
  void AstDumpPackageNode(PackageNode *node);
  void AstDumpImportNode(ImportNode *node);
  void AstDumpUnaOperatorNode(UnaOperatorNode *node);
  void AstDumpBinOperatorNode(BinOperatorNode *node);
  void AstDumpTerOperatorNode(TerOperatorNode *node);
  void AstDumpBlockNode(BlockNode *node);
  void AstDumpNewNode(NewNode *node);
  void AstDumpDeleteNode(DeleteNode *node);
  void AstDumpDimensionNode(DimensionNode *node);
  void AstDumpIdentifierNode(IdentifierNode *node);
  void AstDumpDeclNode(DeclNode *node);
  void AstDumpAnnotationTypeNode(AnnotationTypeNode *node);
  void AstDumpCastNode(CastNode *node);
  void AstDumpParenthesisNode(ParenthesisNode *node);
  void AstDumpFieldNode(FieldNode *node);
  void AstDumpStructNode(StructNode *node);
  void AstDumpFieldLiteralNode(FieldLiteralNode *node);
  void AstDumpStructLiteralNode(StructLiteralNode *node);
  void AstDumpVarListNode(VarListNode *node);
  void AstDumpExprListNode(ExprListNode *node);
  void AstDumpLiteralNode(LiteralNode *node);
  void AstDumpExceptionNode(ExceptionNode *node);
  void AstDumpReturnNode(ReturnNode *node);
  void AstDumpCondBranchNode(CondBranchNode *node);
  void AstDumpBreakNode(BreakNode *node);
  void AstDumpForLoopNode(ForLoopNode *node);
  void AstDumpWhileLoopNode(WhileLoopNode *node);
  void AstDumpDoLoopNode(DoLoopNode *node);
  void AstDumpSwitchLabelNode(SwitchLabelNode *node);
  void AstDumpSwitchCaseNode(SwitchCaseNode *node);
  void AstDumpSwitchNode(SwitchNode *node);
  void AstDumpAssertNode(AssertNode *node);
  void AstDumpCallNode(CallNode *node);
  void AstDumpFunctionNode(FunctionNode *node);
  void AstDumpInterfaceNode(InterfaceNode *node);
  void AstDumpClassNode(ClassNode *node);
  void AstDumpPassNode(PassNode *node);
  void AstDumpLambdaNode(LambdaNode *node);
  void AstDumpInstanceOfNode(InstanceOfNode *node);
  void AstDumpAttrNode(AttrNode *node);
  void AstDumpUserTypeNode(UserTypeNode *node);
  void AstDumpPrimTypeNode(PrimTypeNode *node);
  void AstDumpPrimArrayTypeNode(PrimArrayTypeNode *node);

  void AstDumpTreeNode(TreeNode *node);

  const char *getTypeId(TypeId k);
  const char *getSepId(SepId k);
  const char *getOprId(OprId k);
  const char *getLitId(LitId k);
  const char *getAttrId(AttrId k);
  const char *getActionId(ActionId k);
  const char *getTK_Type(TK_Type k);
  const char *getEntryType(EntryType k);
  const char *getDataType(DataType k);
  const char *getRuleProp(RuleProp k);
  const char *getLAType(LAType k);
  const char *getNodeKind(NodeKind k);
  const char *getImportProperty(ImportProperty k);
  const char *getOperatorProperty(OperatorProperty k);
  const char *getDeclProp(DeclProp k);
  const char *getStructProp(StructProp k);
  const char *getLambdaProperty(LambdaProperty k);
};

} // namespace maplefe
#endif
