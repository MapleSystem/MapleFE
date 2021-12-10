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

#ifndef __CPPDEFINITIONEMITTER_HEADER__
#define __CPPDEFINITIONEMITTER_HEADER__

#include "ast_handler.h"
#include "cpp_emitter.h"
#include "cpp_declaration.h"

namespace maplefe {

class CppDef : public CppEmitter {
public:
  CppDecl        &mCppDecl;
  bool            mIsInit;

  CppDef(Module_Handler *h, CppDecl &d) : CppEmitter(h), mCppDecl(d), mIsInit(false) {}

  std::string Emit() {
    return EmitTreeNode(GetASTModule());
  }

  std::string EmitIdentifierNode(IdentifierNode *node) override;
  std::string EmitImportNode(ImportNode *node) override;
  std::string EmitXXportAsPairNode(XXportAsPairNode *node) override;
  std::string EmitExportNode(ExportNode *node) override;
  std::string EmitUnaOperatorNode(UnaOperatorNode *node) override;
  std::string EmitBinOperatorNode(BinOperatorNode *node) override;
  std::string EmitBlockNode(BlockNode *node) override;
  std::string EmitDeclNode(DeclNode *node) override;
  std::string EmitFieldNode(FieldNode *node) override;
  std::string EmitArrayLiteralNode(ArrayLiteralNode *node) override;
  std::string EmitTemplateLiteralNode(TemplateLiteralNode *node) override;
  std::string EmitLiteralNode(LiteralNode *node) override;
  std::string EmitCondBranchNode(CondBranchNode *node) override;
  std::string EmitBreakNode(BreakNode *node) override;
  std::string EmitContinueNode(ContinueNode *node) override;
  std::string EmitForLoopNode(ForLoopNode *node) override;
  std::string EmitSwitchNode(SwitchNode *node) override;
  std::string EmitCallNode(CallNode *node) override;
  std::string EmitFunctionNode(FunctionNode *node) override;
  std::string EmitTypeOfNode(TypeOfNode *node) override;
  std::string EmitModuleNode(ModuleNode *node) override;
  std::string EmitPrimTypeNode(PrimTypeNode *node) override;
  std::string EmitPrimArrayTypeNode(PrimArrayTypeNode *node) override;
  std::string EmitNewNode(NewNode *node) override;
  std::string EmitArrayElementNode(ArrayElementNode *node) override;
  std::string EmitTypeAliasNode(TypeAliasNode* node) override;
  std::string EmitInstanceOfNode(InstanceOfNode *node) override;
  std::string EmitDeclareNode(DeclareNode *node) override;
  std::string EmitAsTypeNode(AsTypeNode *node) override;
  std::string EmitNamespaceNode(NamespaceNode *node) override;
  std::string& HandleTreeNode(std::string &str, TreeNode *node) override;

  std::string EmitClassProps(TreeNode *node);
  std::string EmitFuncScopeVarDecls(FunctionNode *node);
  std::string EmitStructNode(StructNode *node);
  std::string EmitStructLiteralNode(StructLiteralNode* node);
  std::string EmitObjPropInit(std::string varName, TreeNode* idType, StructLiteralNode* n);
  std::string EmitDirectFieldInit(std::string varName, StructLiteralNode* node);
  std::string EmitCppCtor(ClassNode* node);
  std::string EmitCtorInstance(ClassNode *c);
  std::string EmitDefaultCtor(ClassNode *c);
  std::string EmitBracketNotationProp(ArrayElementNode* ae, OprId binOpId, bool isLhs, bool& isDynProp);
  std::string EmitArrayLiteral(TreeNode* arrType, TreeNode* arrLiteral);
  std::string EmitArrayLiterals(TreeNode* arrLiteral, int dim, std::string type);
  TypeId GetTypeIdFromDecl(TreeNode* id);
  bool   IsClassField(ArrayElementNode* node, std::string propKey);
  bool   IsClassId(TreeNode* node);
  std::string GetTypeForTemplateArg(TreeNode* node);
  TreeNode*   FindDeclType(TreeNode* node);
  std::string GetThisParamObjType(TreeNode *node);
};

} // namespace maplefe
#endif
