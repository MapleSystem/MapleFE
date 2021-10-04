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
#include "emitter.h"

namespace maplefe {

class CppDecl : public Emitter {
private:
  std::set<std::string> mImportedModules;
  std::string           mDecls;

public:
  CppDecl(Module_Handler *h) : Emitter(h) {}
  CppDecl() : CppDecl(nullptr) {}

  std::string Emit() {
    return EmitTreeNode(GetASTModule());
  }

  void AddImportedModule(const std::string& module);
  bool IsImportedModule(const std::string& module);

  void AddDecl(const std::string& decl) { mDecls += decl; }
  std::string GetDecls() { return mDecls; }

  virtual std::string EmitUserTypeNode(UserTypeNode *node);
  virtual std::string EmitBinOperatorNode(BinOperatorNode *node);
  virtual std::string EmitIdentifierNode(IdentifierNode *node);
  virtual std::string EmitDeclNode(DeclNode *node);
  virtual std::string EmitFieldNode(FieldNode *node);
  virtual std::string EmitArrayLiteralNode(ArrayLiteralNode *node);
  virtual std::string EmitCondBranchNode(CondBranchNode *node);
  virtual std::string EmitForLoopNode(ForLoopNode *node);
  virtual std::string EmitWhileLoopNode(WhileLoopNode *node);
  virtual std::string EmitDoLoopNode(DoLoopNode *node);
  virtual std::string EmitAssertNode(AssertNode *node);
  virtual std::string EmitCallNode(CallNode *node);
  virtual std::string EmitFunctionNode(FunctionNode *node);
  virtual std::string EmitPrimTypeNode(PrimTypeNode *node);
  virtual std::string EmitPrimArrayTypeNode(PrimArrayTypeNode *node);
  virtual std::string EmitModuleNode(ModuleNode *node);
  virtual std::string EmitClassNode(ClassNode *node);

  virtual std::string EmitNumIndexSigNode(NumIndexSigNode *node);
  virtual std::string EmitStrIndexSigNode(StrIndexSigNode *node);

  virtual std::string EmitNewNode(NewNode *node);
  virtual std::string EmitStructNode(StructNode *node);
  virtual std::string EmitTypeAliasNode(TypeAliasNode* node);
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

std::string ident(int n);

bool IsBuiltinObj(std::string name);

} // namespace maplefe
#endif
