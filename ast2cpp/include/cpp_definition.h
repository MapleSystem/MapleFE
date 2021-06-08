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

#include "ast.h"
#include "ast_attr.h"
#include "ast_module.h"
#include "ast_handler.h"
#include "ast_type.h"

#include "emitter.h"

namespace maplefe {

class CppDef : public Emitter {
public:
  CppDef(ModuleNode *m) : Emitter(m) {}

  std::string Emit() {
    std::string code;
    code = "// [Beginning of CppDef:\n"s;
    code += EmitTreeNode(GetASTModule());
    code += "// End of CppDef]\n"s;
    return code;
  }

  std::string EmitModuleNode(ModuleNode *node);
  std::string EmitFunctionNode(FunctionNode *node);
  std::string EmitBinOperatorNode(BinOperatorNode *node);
  std::string EmitDeclNode(DeclNode *node);
  std::string EmitCallNode(CallNode *node);
  std::string EmitIdentifierNode(IdentifierNode *node);
  std::string EmitPrimTypeNode(PrimTypeNode *node);
};

} // namespace maplefe
#endif
