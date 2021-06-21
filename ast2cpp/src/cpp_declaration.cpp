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
#include "gen_astvisitor.h"

namespace maplefe {

class CollectDecls : public AstVisitor {
  private:
    CppDecl     *mCppDecl;
    std::string  mDecls;

  public:
    CollectDecls(CppDecl *c) : mCppDecl(c) {}

    FunctionNode *VisitFunctionNode(FunctionNode *node) {
      return node;
    }

    LambdaNode *VisitLambdaNode(LambdaNode *node) {
      return node;
    }

    DeclNode *VisitDeclNode(DeclNode *node) {
      mDecls += mCppDecl->EmitTreeNode(node);
      return node;
    }

    std::string GetDecls() { return mDecls; }
};


std::string CppDecl::EmitModuleNode(ModuleNode *node) {
  if (node == nullptr)
    return std::string();
  std::string name = GetModuleName();
  std::string header("__");
  for(auto &c : name)
    header += std::toupper(c);
  header += "__HEADER__\n";
  std::string str("// TypeScript filename: "s + node->GetFileName() + "\n"s);
  str += "#ifndef "s + header + "#define "s + header;
  str += R"""(
#include "ts2cpp.h"

class )""" + name + R"""( : public t2crt::BaseObj {
public: // all top level variables in the module
)""";

  CollectDecls decls(this);
  decls.VisitTreeNode(node);
  str += decls.GetDecls();

  str += R"""(

public: // all top level functions in the module
void __init_func__();
)""";

  // declarations of all top-level functions
  CfgFunc *module = mHandler->GetCfgFunc();
  auto num = module->GetNestedFuncsNum();
  for(unsigned i = 0; i < num; ++i) {
    CfgFunc *func = module->GetNestedFuncAtIndex(i);
    TreeNode *node = func->GetFuncNode();
    str += EmitTreeNode(node);
  }

  str += R"""(
  // export table here
};

extern )""" + name + " _"s + name + ";\n#endif\n"s;
  return str;
}

std::string CppDecl::EmitFunctionNode(FunctionNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str;
  if (auto n = node->GetType()) {
    TypeId k = n->GetTypeId();
    if(k != TY_None)
      str += CppDecl::GetEnumTypeId(k);
    else
      str += EmitTreeNode(n);
  }
  else
    str += "auto"s;
  if(node->GetStrIdx())
    str += " "s + node->GetName();
  str += "("s;
  for (unsigned i = 0; i < node->GetParamsNum(); ++i) {
    if (i)
      str += ", "s;
    if (auto n = node->GetParam(i)) {
      str += EmitTreeNode(n);
    }
  }
  str += ");\n"s;
  return str;
}

std::string CppDecl::EmitBinOperatorNode(BinOperatorNode *node) {
  return std::string();
}

std::string CppDecl::EmitIdentifierNode(IdentifierNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str;
  TypeId k = node->GetTypeId();
  if(k != TY_None)
    str += CppDecl::GetEnumTypeId(node->GetTypeId());
  else if (auto n = node->GetType())
    str += EmitTreeNode(n);
  else
    str += "t2crt::__JSVAL"s; // Use __JSVAL for unknown type
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
    case TY_String:
      return "std::string "s;
    case TY_None:
      return "auto "s;
  }
  return Emitter::GetEnumTypeId(node->GetPrimType());
}

std::string CppDecl::EmitDeclNode(DeclNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str;
  if (auto n = node->GetVar()) {
    str += " "s + EmitTreeNode(n) + ";\n"s;
  }
  return str;
}

std::string CppDecl::EmitCallNode(CallNode *node) {
  return std::string();
}

std::string CppDecl::EmitCondBranchNode(CondBranchNode *node) {
  return std::string();
}

std::string CppDecl::EmitForLoopNode(ForLoopNode *node) {

  return std::string();
}

std::string CppDecl::EmitWhileLoopNode(WhileLoopNode *node) {
  return std::string();
}

std::string CppDecl::EmitDoLoopNode(DoLoopNode *node) {
  return std::string();
}

std::string CppDecl::EmitAssertNode(AssertNode *node) {
  return std::string();
}

std::string CppDecl::EmitArrayLiteralNode(ArrayLiteralNode *node) {
  return std::string();
}

std::string CppDecl::EmitPrimArrayTypeNode(PrimArrayTypeNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str;
  if (auto n = node->GetPrim()) {
    str += "std::vector<"s + EmitPrimTypeNode(n) + "> "s;;
  }
  /*
  if (auto n = node->GetDims()) {
    str += EmitDimensionNode(n);
  }
  */
  return str;
}

std::string CppDecl::EmitFieldNode(FieldNode *node) {
  return std::string();
}

std::string CppDecl::GetEnumTypeId(TypeId k) {
  switch (k) {
  case TY_Object:
    return "t2crt::BaseObj*";
  case TY_Function:
    return "t2crt::BaseObj*";
  case TY_Boolean:
    return "bool";
  case TY_Int:
    return "long";
  case TY_String:
    return "std::string";
  case TY_Number:
    return "double";
  }
  return Emitter::GetEnumTypeId(k);
}

} // namespace maplefe
