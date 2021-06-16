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

#include "emitter.h"

namespace maplefe {

class CppDecl : public Emitter {
public:
  CppDecl(ModuleNode *m) : Emitter(m) {}

  std::string Emit() {
    return EmitTreeNode(GetASTModule());
  }

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

  static std::string GetEnumTypeId(TypeId k);
};

} // namespace maplefe
#endif
