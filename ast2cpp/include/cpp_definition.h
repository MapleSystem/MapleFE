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

namespace maplefe {

class CppDef : public Emitter {
public:
  Module_Handler mHandler;

  CppDef(Module_Handler *h) : mHandler(h), Emitter(h->GetASTModule()) {}

  std::string Emit() {
    return EmitTreeNode(GetASTModule());
  }

  virtual std::string EmitBinOperatorNode(BinOperatorNode *node);
  virtual std::string EmitIdentifierNode(IdentifierNode *node);
  virtual std::string EmitDeclNode(DeclNode *node);
  virtual std::string EmitFieldNode(FieldNode *node);
  virtual std::string EmitArrayLiteralNode(ArrayLiteralNode *node);
  virtual std::string EmitBreakNode(BreakNode *node);
  virtual std::string EmitContinueNode(ContinueNode *node);
  virtual std::string EmitForLoopNode(ForLoopNode *node);
  virtual std::string EmitCallNode(CallNode *node);
  virtual std::string EmitFunctionNode(FunctionNode *node);
  virtual std::string EmitPrimTypeNode(PrimTypeNode *node);
  virtual std::string EmitPrimArrayTypeNode(PrimArrayTypeNode *node);
  virtual std::string EmitModuleNode(ModuleNode *node);
};

} // namespace maplefe
#endif
