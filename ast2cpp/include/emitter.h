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

#ifndef __EMITTER_HEADER__
#define __EMITTER_HEADER__

#include "ast.h"
#include "ast_attr.h"
#include "ast_module.h"
#include "ast_type.h"

#include "gen_astdump.h"
using namespace std::string_literals;

namespace maplefe {

class Emitter {

protected:
  using Precedence = char;
  Precedence mPrecedence;

  ModuleNode *mASTModule;

public:
  Emitter(ModuleNode *m) : mASTModule(m) {}

  std::string Emit(const char *title);
  std::string GetEnding(TreeNode *n);
  std::string Clean(std::string &s);
  std::string GetBaseFilename();
  std::string GetModuleName(const char *p = nullptr);

  ModuleNode *GetASTModule() { return mASTModule; }
  void SetASTModule(ModuleNode *m) { mASTModule = m; }

  virtual std::string EmitAnnotationNode(AnnotationNode *node);
  virtual std::string EmitAsTypeNode(AsTypeNode *node);
  virtual std::string EmitIdentifierNode(IdentifierNode *node);
  virtual std::string EmitFunctionNode(FunctionNode *node);
  virtual std::string EmitUserTypeNode(UserTypeNode *node);
  virtual std::string EmitComputedNameNode(ComputedNameNode *node);
  virtual std::string EmitPackageNode(PackageNode *node);
  virtual std::string EmitXXportAsPairNode(XXportAsPairNode *node);
  virtual std::string EmitDeclareNode(DeclareNode *node);
  virtual std::string EmitExportNode(ExportNode *node);
  virtual std::string EmitImportNode(ImportNode *node);
  virtual std::string EmitUnaOperatorNode(UnaOperatorNode *node);
  virtual std::string EmitBinOperatorNode(BinOperatorNode *node);
  virtual std::string EmitTerOperatorNode(TerOperatorNode *node);
  virtual std::string EmitTypeAliasNode(TypeAliasNode *node);
  virtual std::string EmitConditionalTypeNode(ConditionalTypeNode *node);
  virtual std::string EmitTypeParameterNode(TypeParameterNode *node);
  virtual std::string EmitBlockNode(BlockNode *node);
  virtual std::string EmitNewNode(NewNode *node);
  virtual std::string EmitDeleteNode(DeleteNode *node);
  virtual std::string EmitAnnotationTypeNode(AnnotationTypeNode *node);
  virtual std::string EmitDimensionNode(DimensionNode *node);
  virtual std::string EmitDeclNode(DeclNode *node);
  virtual std::string EmitCastNode(CastNode *node);
  virtual std::string EmitParenthesisNode(ParenthesisNode *node);
  virtual std::string EmitFieldNode(FieldNode *node);
  virtual std::string EmitArrayElementNode(ArrayElementNode *node);
  virtual std::string EmitArrayLiteralNode(ArrayLiteralNode *node);
  virtual std::string EmitBindingElementNode(BindingElementNode *node);
  virtual std::string EmitBindingPatternNode(BindingPatternNode *node);
  virtual std::string EmitNumIndexSigNode(NumIndexSigNode *node);
  virtual std::string EmitStrIndexSigNode(StrIndexSigNode *node);
  virtual std::string EmitStructNode(StructNode *node);
  virtual std::string EmitFieldLiteralNode(FieldLiteralNode *node);
  virtual std::string EmitStructLiteralNode(StructLiteralNode *node);
  virtual std::string EmitNamespaceNode(NamespaceNode *node);
  virtual std::string EmitVarListNode(VarListNode *node);
  virtual std::string EmitExprListNode(ExprListNode *node);
  virtual std::string EmitTemplateLiteralNode(TemplateLiteralNode *node);
  virtual std::string EmitLiteralNode(LiteralNode *node);
  virtual std::string EmitRegExprNode(RegExprNode *node);
  virtual std::string EmitThrowNode(ThrowNode *node);
  virtual std::string EmitCatchNode(CatchNode *node);
  virtual std::string EmitFinallyNode(FinallyNode *node);
  virtual std::string EmitTryNode(TryNode *node);
  virtual std::string EmitExceptionNode(ExceptionNode *node);
  virtual std::string EmitReturnNode(ReturnNode *node);
  virtual std::string EmitCondBranchNode(CondBranchNode *node);
  virtual std::string EmitBreakNode(BreakNode *node);
  virtual std::string EmitContinueNode(ContinueNode *node);
  virtual std::string EmitForLoopNode(ForLoopNode *node);
  virtual std::string EmitWhileLoopNode(WhileLoopNode *node);
  virtual std::string EmitDoLoopNode(DoLoopNode *node);
  virtual std::string EmitSwitchLabelNode(SwitchLabelNode *node);
  virtual std::string EmitSwitchCaseNode(SwitchCaseNode *node);
  virtual std::string EmitSwitchNode(SwitchNode *node);
  virtual std::string EmitAssertNode(AssertNode *node);
  virtual std::string EmitCallNode(CallNode *node);
  virtual std::string EmitInterfaceNode(InterfaceNode *node);
  virtual std::string EmitClassNode(ClassNode *node);
  virtual std::string EmitPassNode(PassNode *node);
  virtual std::string EmitLambdaNode(LambdaNode *node);
  virtual std::string EmitInstanceOfNode(InstanceOfNode *node);
  virtual std::string EmitTypeOfNode(TypeOfNode *node);
  virtual std::string EmitKeyOfNode(KeyOfNode *node);
  virtual std::string EmitInferNode(InferNode *node);
  virtual std::string EmitInNode(InNode *node);
  virtual std::string EmitIsNode(IsNode *node);
  virtual std::string EmitNameTypePairNode(NameTypePairNode *node);
  virtual std::string EmitTupleTypeNode(TupleTypeNode *node);
  virtual std::string EmitModuleNode(ModuleNode *node);
  virtual std::string EmitAttrNode(AttrNode *node);
  virtual std::string EmitPrimTypeNode(PrimTypeNode *node);
  virtual std::string EmitPrimArrayTypeNode(PrimArrayTypeNode *node);

  virtual std::string EmitTreeNode(TreeNode *node);
  virtual std::string& HandleTreeNode(std::string &str, TreeNode *node);

  static void Replace(std::string &str, const char *o, const char *n, int cnt = 0);
  static std::string GetEnumTypeId(TypeId k);
  //static const char *GetEnumSepId(SepId k);
  static const char *GetEnumOprId(OprId k);
  //static const char *GetEnumLitId(LitId k);
  static std::string GetEnumAttrId(AttrId k);
  //static const char *GetEnumImportProperty(ImportProperty k);
  //static const char *GetEnumOperatorProperty(OperatorProperty k);
  static std::string GetEnumDeclProp(DeclProp k);
  //static const char *GetEnumStructProp(StructProp k);
  //static const char *GetEnumForLoopProp(ForLoopProp k);
  //static const char *GetEnumLambdaProperty(LambdaProperty k);

};

} // namespace maplefe
#endif
