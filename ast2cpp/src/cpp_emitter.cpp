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

#include "cpp_emitter.h"

namespace maplefe {

std::string CppEmitter::EmitModuleNode(ModuleNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str("// Filename: "s);
  str += node->GetFileName() + "\n"s;
  str += R"""(//#include "ts2cpp.h"
#include <iostream>

class Module_1 {
public:
)""";
  mPhase = PK_DECL;
  for (unsigned i = 0; i < node->GetTreesNum(); ++i) {
    if (auto n = node->GetTree(i)) {
      str += EmitTreeNode(n);
    }
  }

  str += R"""(

public:
void __init_func__() {
)""";

  mPhase = PK_CODE;
  for (unsigned i = 0; i < node->GetTreesNum(); ++i) {
    if (auto n = node->GetTree(i)) {
      str += EmitTreeNode(n);
    }
  }

  str += R"""(}
};

Module_1 module_1;

int main(int argc, char **argv) {
  module_1.__init_func__();
  return 0;
}
)""";
  return str;
}


std::string CppEmitter::EmitIdentifierNode(IdentifierNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str;
  if(mPhase == PK_CODE) {
    str += node->GetName();
    if (auto n = node->GetInit()) {
      str += " = "s + EmitTreeNode(n);
    }
    mPrecedence = '\030';
    if (node->IsStmt())
      str += ";\n"s;
  } else if(mPhase == PK_DECL) {
    if (auto n = node->GetType()) {
      str += EmitTreeNode(n);
    }
    str += node->GetName();
  }
  return str;
}


std::string CppEmitter::EmitPrimTypeNode(PrimTypeNode *node) {
  if (node == nullptr && mPhase != PK_DECL)
    return std::string();
  if(node->GetPrimType() == TY_Number)
    return "double "s;
  return Emitter::GetEnumTypeId(node->GetPrimType());
}


std::string CppEmitter::EmitDeclNode(DeclNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str;
  if(mPhase == PK_CODE) {
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
  } else if(mPhase == PK_DECL) {
    if (auto n = node->GetVar()) {
      str += " "s + EmitTreeNode(n) + ";"s;
    }
  }
  return str;
}

std::string CppEmitter::EmitCallNode(CallNode *node) {
  if (node == nullptr || mPhase != PK_CODE)
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
