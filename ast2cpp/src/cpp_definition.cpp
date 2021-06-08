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

#include "cpp_definition.h"

namespace maplefe {

std::string CppDef::EmitModuleNode(ModuleNode *node) {
  if (node == nullptr)
    return std::string();
  std::string name = GetModuleName();
  std::string str("// TypeScript filename: "s + node->GetFileName() + "\n"s);
  str += "#include <iostream>\n#include \""s + GetBaseFileName() + ".h\""s + R"""(

void )""" + name + R"""(::__init_func__() { // bind "this" to current module
)""";
  for (unsigned i = 0; i < node->GetTreesNum(); ++i) {
    if (auto n = node->GetTree(i)) {
      str += EmitTreeNode(n);
    }
  }
  str += R"""(}

)""" + name + " _"s + name + R"""(;

// If the program starts from this module, generate the main function
int main(int argc, char **argv) {
)""" + "  _"s + name + R"""(.__init_func__(); // only call to its __init_func__()
  return 0;
}
)""";
  return str;
}


std::string CppDef::EmitFunctionNode(FunctionNode *node) {
  return std::string();
}

std::string CppDef::EmitBinOperatorNode(BinOperatorNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str = Emitter::EmitBinOperatorNode(node);
  Emitter::Replace(str, ">>>", ">>");
  return str;
}

std::string CppDef::EmitIdentifierNode(IdentifierNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str;
  str += node->GetName();
  if (auto n = node->GetInit()) {
    str += " = "s + EmitTreeNode(n);
  }
  mPrecedence = '\030';
  if (node->IsStmt())
    str += ";\n"s;
  return str;
}


std::string CppDef::EmitPrimTypeNode(PrimTypeNode *node) {
  return std::string();
}


std::string CppDef::EmitDeclNode(DeclNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str;
  //std::string str(Emitter::GetEnumDeclProp(node->GetProp()));
  if (auto n = node->GetVar()) {
    str += " "s + EmitTreeNode(n);
  }
  if (auto n = node->GetInit()) {
    str += " = "s + EmitTreeNode(n);
  }
  mPrecedence = '\030';
  if (node->IsStmt())
    str += ";\n"s;
  return str;
}

std::string CppDef::EmitCallNode(CallNode *node) {
  if (node == nullptr)
    return std::string();
  bool log = false;
  std::string str;
  if (auto n = node->GetMethod()) {
    auto s = EmitTreeNode(n);
    if(n->GetKind() == NK_Function)
      str += "("s + s + ")"s;
    else {
      if(s.compare(0, 11, "console.log") == 0) {
        str += "std::cout"s;
        log = true;
      } else
        str += s;
    }
  }
  if(!log)
    str += "("s;
  for (unsigned i = 0; i < node->GetArgsNum(); ++i) {
    if(log)
      str += " << "s + EmitTreeNode(node->GetArg(i));
    else {

      if (i)
        str += ", "s;
      if (auto n = node->GetArg(i))
        str += EmitTreeNode(n);
    }
  }
  if(!log)
    str += ")"s;
  else
    str += " << std::endl";
  mPrecedence = '\024';
  if (node->IsStmt())
    str += ";\n"s;
  return str;
}

} // namespace maplefe
