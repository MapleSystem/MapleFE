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

class )""" + name + R"""( : public t2crt::Object {
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
  // exports
  Object exports;
};

extern )""" + name + " _"s + name + ";\n#endif\n"s;
  return str;
}

std::string CppDecl::EmitFunctionNode(FunctionNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str(GetTypeString(node->GetType(), node->GetType()));
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
  std::string str(GetTypeString(node, node->GetType()));
  str += " "s + node->GetName();
  return str;
}

std::string CppDecl::EmitPrimTypeNode(PrimTypeNode *node) {
  if (node == nullptr)
    return std::string();
  return GetTypeString(node);
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
    str = "std::vector<"s + EmitPrimTypeNode(n) + "> "s;;
    auto d = node->GetDims();
    if(d && d->GetKind() == NK_Dimension) {
      auto num = (static_cast<DimensionNode *>(d))->GetDimensionsNum();
      for (unsigned i = 1; i < num; ++i) {
        str = "std::vector<"s + str + "> "s;;
      }
    }
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

std::string CppDecl::GetTypeString(TreeNode *node, TreeNode *child) {
  if (node) {
    TypeId k = node->GetTypeId();
    if(k == TY_None && node->GetKind() == NK_PrimType)
      k = static_cast<PrimTypeNode*>(node)->GetPrimType();
    switch(k) {
      case TY_Object:
        return "t2crt::Object* "s;
      case TY_Function:
        return "t2crt::Object* "s;
      case TY_Boolean:
        return "bool "s;
      case TY_Int:
        return "long "s;
      case TY_String:
        return "std::string "s;
      case TY_Number:
      case TY_Double:
        return "double "s;
    }
    {
      std::string str = child ? EmitTreeNode(child) : Emitter::GetEnumTypeId(k);
      if (str != "none"s)
        return str + " "s;
    }
  }
  return "t2crt::JS_Val "s;
}

std::string CppDecl::EmitUserTypeNode(UserTypeNode *node) {
  if (node == nullptr)
    return std::string();
  {
    auto k = node->GetType();
    if(k == UT_Union || k == UT_Inter)
      return "t2crt::JS_Val"s;
  }
  std::string str;
  if (auto n = node->GetId()) {
    str = EmitTreeNode(n);
    auto num = node->GetTypeGenericsNum();
    if(num) {
      str += "<"s;
      for (unsigned i = 0; i < num; ++i) {
        if (i)
          str += ", "s;
        if (auto n = node->GetTypeGeneric(i)) {
          str += EmitTreeNode(n);
        }
      }
      str += ">"s;
    }
  }

  auto k = node->GetType();
  if(k != UT_Regular) {
    if(!str.empty())
      str = "using "s + str + " = "s;
    std::string op = k == UT_Union ? " | "s : " & "s;
    for (unsigned i = 0; i < node->GetUnionInterTypesNum(); ++i) {
      if(i)
        str += op;
      str += EmitTreeNode(node->GetUnionInterType(i));
    }
  }

  if (auto n = node->GetDims()) {
    str += EmitDimensionNode(n);
  }
  if (node->IsStmt())
    str += ";\n"s;
  return HandleTreeNode(str, node);
}

} // namespace maplefe
