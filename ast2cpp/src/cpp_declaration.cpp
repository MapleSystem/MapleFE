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

#include "cpp_declaration.h"

namespace maplefe {

std::string CppDecl::EmitModuleNode(ModuleNode *node) {
  if (node == nullptr)
    return std::string();
  std::string name = GetModuleName();
  std::string str("// Filename: "s + node->GetFileName() + "\n"s);
  str += R"""(
//#include "ts2cpp.h"
#include <iostream>

class )""" + name + R"""( {
public: // all top level variables in the module
)""";
  for (unsigned i = 0; i < node->GetTreesNum(); ++i) {
    if (auto n = node->GetTree(i)) {
      str += EmitTreeNode(n);
    }
  }

  str += R"""(

public: // all top level functions in the module
void __init_func__();
)""";

  str += R"""(
  // export table here
};

extern )""" + name + " _"s + name + ";\n";
  return str;
}


std::string CppDecl::EmitFunctionNode(FunctionNode *node) {
  if (node == nullptr)
    return std::string();
  return Emitter::EmitFunctionNode(node);
}

std::string CppDecl::EmitBinOperatorNode(BinOperatorNode *node) {
  return std::string();
}

std::string CppDecl::EmitIdentifierNode(IdentifierNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str;
  if (auto n = node->GetType()) {
    str += EmitTreeNode(n);
  }
  str += " "s + node->GetName();
  return str;
}


std::string CppDecl::EmitPrimTypeNode(PrimTypeNode *node) {
  if (node == nullptr)
    return std::string();
  switch(node->GetPrimType()) {
    case TY_Number:
      return "double "s;
    case TY_Boolean:
      return "bool "s;
  }
  return Emitter::GetEnumTypeId(node->GetPrimType());
}

std::string CppDecl::EmitDeclNode(DeclNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str;
  if (auto n = node->GetVar()) {
    str += " "s + EmitTreeNode(n) + ";"s;
  }
  return str;
}

std::string CppDecl::EmitCallNode(CallNode *node) {
  return std::string();
}

} // namespace maplefe
