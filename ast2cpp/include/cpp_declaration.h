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

#ifndef __CPPDECL_HEADER__
#define __CPPDECL_HEADER__

#include <set>
#include "ast_handler.h"
#include "cpp_emitter.h"

namespace maplefe {

class CppDecl : public CppEmitter {
private:
  std::set<std::string> mImportedModules;
  std::string           mDefinitions;
  std::string           mInits;

public:
  CppDecl(Module_Handler *h) : CppEmitter(h) {}
  CppDecl() : CppDecl(nullptr) {}

  std::string Emit() {
    return EmitTreeNode(GetASTModule());
  }

  std::string GenFunctionClass(FunctionNode* node);

  void AddImportedModule(const std::string& module);
  bool IsImportedModule(const std::string& module);

  void AddDefinition(const std::string& def) { mDefinitions += def; }
  std::string GetDefinitions() { return mDefinitions; }
  void AddInit(const std::string& init) { mInits += init; }
  std::string GetInits() { return mInits; }

  std::string EmitUserTypeNode(UserTypeNode *node) override;
  std::string EmitBinOperatorNode(BinOperatorNode *node) override;
  std::string EmitIdentifierNode(IdentifierNode *node) override;
  std::string EmitDeclNode(DeclNode *node) override;
  std::string EmitFieldNode(FieldNode *node) override;
  std::string EmitArrayLiteralNode(ArrayLiteralNode *node) override;
  std::string EmitCondBranchNode(CondBranchNode *node) override;
  std::string EmitForLoopNode(ForLoopNode *node) override;
  std::string EmitWhileLoopNode(WhileLoopNode *node) override;
  std::string EmitDoLoopNode(DoLoopNode *node) override;
  std::string EmitAssertNode(AssertNode *node) override;
  std::string EmitCallNode(CallNode *node) override;
  std::string EmitFunctionNode(FunctionNode *node) override;
  std::string EmitPrimTypeNode(PrimTypeNode *node) override;
  std::string EmitPrimArrayTypeNode(PrimArrayTypeNode *node) override;
  std::string EmitModuleNode(ModuleNode *node) override;
  std::string EmitClassNode(ClassNode *node) override;

  std::string EmitNumIndexSigNode(NumIndexSigNode *node) override;
  std::string EmitStrIndexSigNode(StrIndexSigNode *node) override;

  std::string EmitNewNode(NewNode *node) override;
  std::string EmitStructNode(StructNode *node) override;
  std::string EmitTypeAliasNode(TypeAliasNode* node) override;
  std::string EmitLiteralNode(LiteralNode* node) override;
  std::string GetIdentifierName(TreeNode *node);

  std::string GetTypeString(TreeNode *node, TreeNode *child = nullptr);
  std::string EmitArrayLiteral(ArrayLiteralNode *node, int dim, std::string type);
  std::string EmitTSEnum(StructNode *node);
  std::string EmitInterface(StructNode *node);
};

inline bool IsVarInitStructLiteral(DeclNode* node) {
  return node->GetInit() &&
         node->GetInit()->IsTypeIdClass() &&
         node->GetInit()->IsStructLiteral();
}

inline bool IsVarInitClass(DeclNode* node) {
  return node->GetInit() &&
         node->GetInit()->IsTypeIdClass() &&
         node->GetInit()->IsIdentifier();
}

bool IsBuiltinObj(std::string name);

} // namespace maplefe
#endif
