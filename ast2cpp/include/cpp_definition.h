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
#include "emitter.h"
#include "cpp_declaration.h"

namespace maplefe {

class CppDef : public Emitter {
public:
  bool            isInit;
  CppDecl        &mCppDecl;
  CppDef(Module_Handler *h, CppDecl &d) : mCppDecl(d), Emitter(h) {}

  std::string Emit() {
    return EmitTreeNode(GetASTModule());
  }

  virtual std::string EmitIdentifierNode(IdentifierNode *node);
  virtual std::string EmitImportNode(ImportNode *node);
  virtual std::string EmitXXportAsPairNode(XXportAsPairNode *node);
  virtual std::string EmitExportNode(ExportNode *node);
  virtual std::string EmitUnaOperatorNode(UnaOperatorNode *node);
  virtual std::string EmitBinOperatorNode(BinOperatorNode *node);
  virtual std::string EmitBlockNode(BlockNode *node);
  virtual std::string EmitDeclNode(DeclNode *node);
  virtual std::string EmitFieldNode(FieldNode *node);
  virtual std::string EmitArrayLiteralNode(ArrayLiteralNode *node);
  virtual std::string EmitTemplateLiteralNode(TemplateLiteralNode *node);
  virtual std::string EmitLiteralNode(LiteralNode *node);
  virtual std::string EmitCondBranchNode(CondBranchNode *node);
  virtual std::string EmitBreakNode(BreakNode *node);
  virtual std::string EmitContinueNode(ContinueNode *node);
  virtual std::string EmitForLoopNode(ForLoopNode *node);
  virtual std::string EmitSwitchNode(SwitchNode *node);
  virtual std::string EmitCallNode(CallNode *node);
  virtual std::string EmitFunctionNode(FunctionNode *node);
  virtual std::string EmitTypeOfNode(TypeOfNode *node);
  virtual std::string EmitModuleNode(ModuleNode *node);
  virtual std::string EmitPrimTypeNode(PrimTypeNode *node);
  virtual std::string EmitPrimArrayTypeNode(PrimArrayTypeNode *node);
  virtual std::string EmitNewNode(NewNode *node);
  virtual std::string EmitClassProps(TreeNode *node);
  virtual std::string EmitArrayElementNode(ArrayElementNode *node);
  virtual std::string EmitTypeAliasNode(TypeAliasNode* node);
  virtual std::string EmitInstanceOfNode(InstanceOfNode *node);
  std::string EmitFuncScopeVarDecls(FunctionNode *node);
  std::string EmitStructNode(StructNode *node);
  std::string EmitStructLiteralNode(StructLiteralNode* node);
  std::string EmitObjPropInit(std::string varName, TreeNode* idType, StructLiteralNode* n);
  std::string EmitDirectFieldInit(std::string varName, StructLiteralNode* node);
  std::string EmitCppCtor(ClassNode* node);
  std::string EmitBracketNotationProp(ArrayElementNode* ae, OprId binOpId, bool isLhs, bool& isDynProp);
  TypeId GetTypeIdFromDecl(TreeNode* id);
  bool   IsClassField(ArrayElementNode* node, std::string propKey);
  bool   IsClassId(TreeNode* node);
  std::string GetTypeForTemplateArg(TreeNode* node);
  TreeNode*   FindDeclType(TreeNode* node);
};

} // namespace maplefe
#endif
