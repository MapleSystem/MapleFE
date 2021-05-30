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

#ifndef __TSEMITTER_HEADER__
#define __TSEMITTER_HEADER__

#include "ast.h"
#include "ast_attr.h"
#include "ast_module.h"
#include "ast_type.h"

#include "gen_astdump.h"

namespace maplefe {

class TsEmitter {
  using Precedence = char;

private:
  ModuleNode *mASTModule;
  std::ostream *mOs;
  Precedence mPrecedence;

public:
  TsEmitter(ModuleNode *m) : mASTModule(m), mOs(nullptr) {}

  void TsEmit(const char *title, std::ostream *os) {
    mOs = os;
    *mOs << "// [Beginning of TsEmitter: " << title << "\n";
    *mOs << TsEmitTreeNode(mASTModule);
    *mOs << "// End of TsEmitter]\n";
  }

  std::string Clean(std::string &s) {
    auto len = s.length();
    if (len >= 2 && s.substr(len - 2) == ";\n")
      return s.erase(len - 2);
    return s;
  }

  std::string TsEmitAnnotationNode(AnnotationNode *node);
  std::string TsEmitPackageNode(PackageNode *node);
  std::string TsEmitXXportAsPairNode(XXportAsPairNode *node);
  std::string TsEmitExportNode(ExportNode *node);
  std::string TsEmitImportNode(ImportNode *node);
  std::string TsEmitUnaOperatorNode(UnaOperatorNode *node);
  std::string TsEmitBinOperatorNode(BinOperatorNode *node);
  std::string TsEmitTerOperatorNode(TerOperatorNode *node);
  std::string TsEmitBlockNode(BlockNode *node);
  std::string TsEmitNewNode(NewNode *node);
  std::string TsEmitDeleteNode(DeleteNode *node);
  std::string TsEmitDimensionNode(DimensionNode *node);
  std::string TsEmitIdentifierNode(IdentifierNode *node);
  std::string TsEmitDeclNode(DeclNode *node);
  std::string TsEmitAnnotationTypeNode(AnnotationTypeNode *node);
  std::string TsEmitCastNode(CastNode *node);
  std::string TsEmitParenthesisNode(ParenthesisNode *node);
  std::string TsEmitFieldNode(FieldNode *node);
  std::string TsEmitArrayElementNode(ArrayElementNode *node);
  std::string TsEmitArrayLiteralNode(ArrayLiteralNode *node);
  std::string TsEmitBindingElementNode(BindingElementNode *node);
  std::string TsEmitBindingPatternNode(BindingPatternNode *node);
  std::string TsEmitNumIndexSigNode(NumIndexSigNode *node);
  std::string TsEmitStrIndexSigNode(StrIndexSigNode *node);
  std::string TsEmitStructNode(StructNode *node);
  std::string TsEmitFieldLiteralNode(FieldLiteralNode *node);
  std::string TsEmitStructLiteralNode(StructLiteralNode *node);
  std::string TsEmitVarListNode(VarListNode *node);
  std::string TsEmitExprListNode(ExprListNode *node);
  std::string TsEmitTemplateLiteralNode(TemplateLiteralNode *node);
  std::string TsEmitLiteralNode(LiteralNode *node);
  std::string TsEmitThrowNode(ThrowNode *node);
  std::string TsEmitCatchNode(CatchNode *node);
  std::string TsEmitFinallyNode(FinallyNode *node);
  std::string TsEmitTryNode(TryNode *node);
  std::string TsEmitExceptionNode(ExceptionNode *node);
  std::string TsEmitReturnNode(ReturnNode *node);
  std::string TsEmitCondBranchNode(CondBranchNode *node);
  std::string TsEmitBreakNode(BreakNode *node);
  std::string TsEmitContinueNode(ContinueNode *node);
  std::string TsEmitForLoopNode(ForLoopNode *node);
  std::string TsEmitWhileLoopNode(WhileLoopNode *node);
  std::string TsEmitDoLoopNode(DoLoopNode *node);
  std::string TsEmitSwitchLabelNode(SwitchLabelNode *node);
  std::string TsEmitSwitchCaseNode(SwitchCaseNode *node);
  std::string TsEmitSwitchNode(SwitchNode *node);
  std::string TsEmitAssertNode(AssertNode *node);
  std::string TsEmitCallNode(CallNode *node);
  std::string TsEmitFunctionNode(FunctionNode *node);
  std::string TsEmitInterfaceNode(InterfaceNode *node);
  std::string TsEmitClassNode(ClassNode *node);
  std::string TsEmitPassNode(PassNode *node);
  std::string TsEmitLambdaNode(LambdaNode *node);
  std::string TsEmitInstanceOfNode(InstanceOfNode *node);
  std::string TsEmitTypeOfNode(TypeOfNode *node);
  std::string TsEmitInNode(InNode *node);
  std::string TsEmitAttrNode(AttrNode *node);
  std::string TsEmitUserTypeNode(UserTypeNode *node);
  std::string TsEmitPrimTypeNode(PrimTypeNode *node);
  std::string TsEmitPrimArrayTypeNode(PrimArrayTypeNode *node);
  std::string TsEmitModuleNode(ModuleNode *node);

  std::string TsEmitTreeNode(TreeNode *node);

  static const char *GetEnumTypeId(TypeId k);
  //static const char *GetEnumSepId(SepId k);
  static const char *GetEnumOprId(OprId k);
  //static const char *GetEnumLitId(LitId k);
  static const char *GetEnumAttrId(AttrId k);
  //static const char *GetEnumImportProperty(ImportProperty k);
  //static const char *GetEnumOperatorProperty(OperatorProperty k);
  static const char *GetEnumDeclProp(DeclProp k);
  //static const char *GetEnumStructProp(StructProp k);
  //static const char *GetEnumForLoopProp(ForLoopProp k);
  //static const char *GetEnumLambdaProperty(LambdaProperty k);

};

} // namespace maplefe
#endif
