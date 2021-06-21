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

#ifndef __AST_TYPE_INFERENCE_HEADER__
#define __AST_TYPE_INFERENCEHEADER__

#include <stack>
#include <utility>
#include "ast_module.h"
#include "ast.h"
#include "ast_type.h"
#include "gen_astvisitor.h"

namespace maplefe {

class Module_Handler;

class TypeInfer {
 private:
  Module_Handler *mHandler;
  bool            mTrace;

 public:
  explicit TypeInfer(Module_Handler *h, bool t) : mHandler(h), mTrace(t) {}
  ~TypeInfer() {}

  void TypeInference();
};

class TypeInferVisitor : public AstVisitor {
 private:
  Module_Handler *mHandler;
  bool            mTrace;
  bool            mUpdated;

  CfgFunc *mCurrentFunction;
  CfgBB   *mCurrentBB;

 public:
  explicit TypeInferVisitor(Module_Handler *h, bool t, bool base = false)
    : mHandler(h), mTrace(t), AstVisitor(t && base) {}
  ~TypeInferVisitor() = default;

  bool GetUpdated() {return mUpdated;}
  void SetUpdated(bool b) {mUpdated = b;}

  void UpdateTypeId(TreeNode *node, TypeId id);
  void UpdateArrayElemTypeIdMap(TreeNode *node, TypeId id);

  TypeId MergeTypeId(TypeId tia, TypeId tib);

  bool IsArray(DeclNode *node);
  bool IsArray(TreeNode *node);

  TreeNode *VisitClassField(TreeNode *node);

  AnnotationNode *VisitAnnotationNode(AnnotationNode *node);
  AnnotationTypeNode *VisitAnnotationTypeNode(AnnotationTypeNode *node);
  ArrayElementNode *VisitArrayElementNode(ArrayElementNode *node);
  ArrayLiteralNode *VisitArrayLiteralNode(ArrayLiteralNode *node);
  AssertNode *VisitAssertNode(AssertNode *node);
  AsTypeNode *VisitAsTypeNode(AsTypeNode *node);
  AttrNode *VisitAttrNode(AttrNode *node);
  BindingElementNode *VisitBindingElementNode(BindingElementNode *node);
  BindingPatternNode *VisitBindingPatternNode(BindingPatternNode *node);
  BinOperatorNode *VisitBinOperatorNode(BinOperatorNode *node);
  BlockNode *VisitBlockNode(BlockNode *node);
  BreakNode *VisitBreakNode(BreakNode *node);
  CallNode *VisitCallNode(CallNode *node);
  CastNode *VisitCastNode(CastNode *node);
  CatchNode *VisitCatchNode(CatchNode *node);
  ClassNode *VisitClassNode(ClassNode *node);
  CondBranchNode *VisitCondBranchNode(CondBranchNode *node);
  ContinueNode *VisitContinueNode(ContinueNode *node);
  DeclareNode *VisitDeclareNode(DeclareNode *node);
  DeclNode *VisitDeclNode(DeclNode *node);
  DeleteNode *VisitDeleteNode(DeleteNode *node);
  DimensionNode *VisitDimensionNode(DimensionNode *node);
  DoLoopNode *VisitDoLoopNode(DoLoopNode *node);
  ExceptionNode *VisitExceptionNode(ExceptionNode *node);
  ExportNode *VisitExportNode(ExportNode *node);
  ExprListNode *VisitExprListNode(ExprListNode *node);
  FieldLiteralNode *VisitFieldLiteralNode(FieldLiteralNode *node);
  FieldNode *VisitFieldNode(FieldNode *node);
  FinallyNode *VisitFinallyNode(FinallyNode *node);
  ForLoopNode *VisitForLoopNode(ForLoopNode *node);
  FunctionNode *VisitFunctionNode(FunctionNode *node);
  IdentifierNode *VisitIdentifierNode(IdentifierNode *node);
  ImportNode *VisitImportNode(ImportNode *node);
  InNode *VisitInNode(InNode *node);
  InstanceOfNode *VisitInstanceOfNode(InstanceOfNode *node);
  InterfaceNode *VisitInterfaceNode(InterfaceNode *node);
  KeyOfNode *VisitKeyOfNode(KeyOfNode *node);
  LambdaNode *VisitLambdaNode(LambdaNode *node);
  LiteralNode *VisitLiteralNode(LiteralNode *node);
  ModuleNode *VisitModuleNode(ModuleNode *node);
  NamespaceNode *VisitNamespaceNode(NamespaceNode *node);
  NewNode *VisitNewNode(NewNode *node);
  NumIndexSigNode *VisitNumIndexSigNode(NumIndexSigNode *node);
  PackageNode *VisitPackageNode(PackageNode *node);
  ParenthesisNode *VisitParenthesisNode(ParenthesisNode *node);
  PassNode *VisitPassNode(PassNode *node);
  PrimArrayTypeNode *VisitPrimArrayTypeNode(PrimArrayTypeNode *node);
  PrimTypeNode *VisitPrimTypeNode(PrimTypeNode *node);
  ReturnNode *VisitReturnNode(ReturnNode *node);
  StrIndexSigNode *VisitStrIndexSigNode(StrIndexSigNode *node);
  StructLiteralNode *VisitStructLiteralNode(StructLiteralNode *node);
  StructNode *VisitStructNode(StructNode *node);
  SwitchCaseNode *VisitSwitchCaseNode(SwitchCaseNode *node);
  SwitchLabelNode *VisitSwitchLabelNode(SwitchLabelNode *node);
  SwitchNode *VisitSwitchNode(SwitchNode *node);
  TemplateLiteralNode *VisitTemplateLiteralNode(TemplateLiteralNode *node);
  TerOperatorNode *VisitTerOperatorNode(TerOperatorNode *node);
  ThrowNode *VisitThrowNode(ThrowNode *node);
  TryNode *VisitTryNode(TryNode *node);
  TypeOfNode *VisitTypeOfNode(TypeOfNode *node);
  TypeParameterNode *VisitTypeParameterNode(TypeParameterNode *node);
  UnaOperatorNode *VisitUnaOperatorNode(UnaOperatorNode *node);
  UserTypeNode *VisitUserTypeNode(UserTypeNode *node);
  VarListNode *VisitVarListNode(VarListNode *node);
  WhileLoopNode *VisitWhileLoopNode(WhileLoopNode *node);
  XXportAsPairNode *VisitXXportAsPairNode(XXportAsPairNode *node);
};

}
#endif
