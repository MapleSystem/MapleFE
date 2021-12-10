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
#include <algorithm>
#include <cstring>
#include "helper.h"

namespace maplefe {

class ImportExportModules : public AstVisitor {
  private:
    CppDecl     *mCppDecl;
    Emitter     *mEmitter;
    std::string  mIncludes;
    std::string  mImports;
    std::string  mExports;
    bool         mExFlag;

  public:
    ImportExportModules(CppDecl *c) : mCppDecl(c),
      mIncludes("// include directives\n"),
      mImports("// imports\n"),
      mExports("// exports\n"),
      mExFlag(false) {
      mEmitter = new Emitter(c->GetModuleHandler());
    }
    ~ImportExportModules() { delete mEmitter; }

    std::string GetIncludes() { return mIncludes; }
    std::string GetImports() { return mImports; }
    std::string GetExports() { return mExports; }

    std::string AddIncludes(TreeNode *node) {
      std::string filename;
      if (node) {
        filename = mEmitter->EmitTreeNode(node);
        auto len = filename.size();
        filename = len >= 2 && filename.back() == '"' ? filename.substr(1, len - 2) : std::string();
        // may have some duplicated include directives which do not hurt
        if (!filename.empty()) {
          std::string incl = "#include \""s + filename + ".h\"\n"s;
          std::size_t found = mIncludes.find(incl);
          if (found == std::string::npos) {
            mIncludes += "#include \""s + filename + ".h\"\n"s;
            mCppDecl->AddInit(mCppDecl->GetModuleName(filename.c_str()) + "::__init_func__();\n"s);
          }
        }
      }
      return filename;
    }

    std::string Comment(TreeNode *node) {
      std::string s = mEmitter->EmitTreeNode(node);
      return s.empty() ? s : "//--- "s + s.substr(0, s.find('\n')) + '\n';
    }

    ImportNode *VisitImportNode(ImportNode *node) {
      std::string filename = AddIncludes(node->GetTarget());
      std::string module = mCppDecl->GetModuleName(filename.c_str());
      for (unsigned i = 0; i < node->GetPairsNum(); ++i) {
        if (auto x = node->GetPair(i)) {
          mImports += Comment(node);
          std::string str;
          if (x->IsDefault()) {
            if (auto b = x->GetBefore()) {
              std::string v = module + "::__export::__default"s;
              std::string s = mEmitter->EmitTreeNode(b);
              if (b->GetTypeId() == TY_Class)
                mImports += "using "s + s + " = "s + v + ";\n"s;
              else
                mImports += "inline const decltype("s + v + ") &"s + s + " = "s + v + ";\n"s;
            }
          } else if (x->IsSingle()) {
            if (auto b = x->GetBefore(); b->IsLiteral()) {
                if (auto a = x->GetAfter()) {
                  // import X = require("./module");
                  std::string after = mEmitter->EmitTreeNode(a);
                  filename = AddIncludes(b);
                  module = mCppDecl->GetModuleName(filename.c_str());
                  std::string v = module + "::__export::__default"s;
                  if(a->GetTypeId() == TY_Module || a->GetTypeId() == TY_Namespace)
                    mImports += "namespace "s + after + " = "s + module + v + ";\n"s;
                  else if (a->GetTypeId() == TY_Class)
                    mImports += "using "s + after + " = "s + v + ";\n"s;
                  else
                    mImports += "inline const decltype("s + v + ") &"s + after + " = "s + v + ";\n"s;
                }
            }
          } else if (x->IsEverything()) {
            if (auto n = x->GetBefore()) {
              std::string s = mEmitter->EmitTreeNode(n);
              mImports += "namespace "s + s + " = " + module + "::__export;\n"s;
              mCppDecl->AddImportedModule(s);
            }
          } else {
            if (auto n = x->GetBefore()) {
              std::string v = mEmitter->EmitTreeNode(n);
              if (auto a = x->GetAfter()) {
                std::string after = mEmitter->EmitTreeNode(a);
                if (node->GetTarget()) {
                  v = "::__export::"s + (v == "default" ? "__"s + v : v);
                  if (node->IsImportType())
                    mImports += "using "s + after + " = "s + module + v + ";\n"s;
                  else if (a->GetTypeId() == TY_Module || a->GetTypeId() == TY_Namespace)
                    mImports += "namespace "s + after + " = "s + module + v + ";\n"s;
                  else
                    mImports += "inline const decltype("s + module + v + ") &"s + after
                      + " = "s + module + v + ";\n"s;
                } else {
                  mEmitter->Replace(v, ".", "::");
                  mImports += "inline const decltype("s + v + ") &"s + after + " = "s + v + ";\n"s;
                }
                if (mExFlag)
                  mExports += "namespace __export { using "s + mCppDecl->GetModuleName() + "::"s + after + "; }\n"s;
              } else {
                auto u = module + "::__export::"s + v;
                if (node->IsImportType())
                  mImports += "using "s + v + " = "s + u + ";\n"s;
                else
                  mImports += "inline const decltype("s + u + ") &"s + v + " = "s + u + ";\n"s;
              }
            }
          }
        }
      }
      return node;
    }

    ExportNode *VisitExportNode(ExportNode *node) {
      if (mCppDecl->IsInNamespace(node))
        return node;
      // 'export *' does not re-export a default, it re-exports only named exports
      // Multiple 'export *'s fails with tsc if they export multiple exports with same name
      std::string filename = AddIncludes(node->GetTarget());
      std::string module = mCppDecl->GetModuleName(filename.c_str());
      for (unsigned i = 0; i < node->GetPairsNum(); ++i) {
        if (auto x = node->GetPair(i)) {
          mExports += Comment(node);
          if (x->IsDefault()) {
            if (x->IsRef()) {
              auto b = x->GetBefore();
              std::string target = mCppDecl->GetIdentifierName(b);
              bool emit = true;
              if (target == "default") {
                target = module + "::__export::__default";
                mExports += "namespace __export { inline const decltype("s + target + ") &"s
                  + "__"s + "default" + " = "s + target + "; }\n"s;
                emit = false;
              }
              if (emit) {
                mExports += "namespace __export { inline const decltype("s + target + ") &__default = "s + target + "; }\n"s;
                emit = false;
              }
              if (emit)
                mExports += "namespace __export { using "s + module + "::"s + "default; }\n"s;
            } else {
              if (auto n = x->GetBefore()) {
                std::string v = mEmitter->EmitTreeNode(n);
                mExports += "namespace __export { inline decltype("s + v + ") __default; }\n"s;
              }
            }
          } else if (x->IsSingle()) {
            std::string str;
            if (auto a = x->GetAfter())
              mExports += "TODO: " + mEmitter->EmitTreeNode(a) + '\n';
            else if (auto b = x->GetBefore()) {
              std::string s = mEmitter->EmitTreeNode(b);
              if (b->GetTypeId() == TY_Class)
                mExports += "namespace __export { using __default = "s + module + "::"s + s + "; }\n"s;
              else if (b->GetTypeId() == TY_Namespace)
                mExports += "namespace __export { namespace __default = "s + module + "::"s + s + "; }\n"s;
              else
                mExports += "namespace __export { inline const decltype("s + module + "::"s + s + ") &__default = "s
                  + module + "::"s + s + "; }\n"s;
            }
          } else if (x->IsEverything()) {
            if (auto b = x->GetBefore())
              mExports += "namespace __export { namespace "s + mCppDecl->GetIdentifierName(b)
                + " = " + module + "::__export; }\n"s;
            else
              mExports += "namespace __export { using namespace "s + module + "::__export; }\n"s;
          } else {
            if (auto b = x->GetBefore()) {
              if (b->IsImport()) {
                mExFlag = true;
                VisitImportNode(static_cast<ImportNode *>(b));
                mExFlag = false;
                continue;
              }
              std::string target = mCppDecl->GetIdentifierName(b);
              bool emit = true;
              if (auto a = x->GetAfter()) {
                std::string after = mCppDecl->GetIdentifierName(a);
                if (target == "default") {
                  target = module + "::__export::__default";
                  mExports += "namespace __export { inline const decltype("s + target + ") &"s
                    + (after == "default" ? "__"s + after : after) + " = "s + target + "; }\n"s;
                  emit = false;
                }
                else if (target != after) {
                  if (a->GetTypeId() == TY_Namespace)
                    mExports += "namespace __export { namespace "s + after + " = "s + module + "::"s + target + "; }\n"s;
                  else
                    mExports += "namespace __export { using "s + after + " = "s + module + "::"s + target + "; }\n"s;
                  emit = false;
                }
              }
              if (emit)
                mExports += "namespace __export { using "s + module + "::"s + target + "; }\n"s;
            }
          }
        }
      }
      return node;
    }
};

class ClassDecls : public AstVisitor {
  private:
    CppDecl     *mCppDecl;
    std::string  mDecls;

  public:
    ClassDecls(CppDecl *c) : mCppDecl(c), mDecls("// class decls\n") {}

    ClassNode *VisitClassNode(ClassNode *node) {
      std::string ns = mCppDecl->GetNamespace(node);
      if (ns.empty())
        mDecls += mCppDecl->EmitTreeNode(node) + ";\n"s;
      else
        mDecls += "namespace "s + ns + " {\n"s + mCppDecl->EmitTreeNode(node) + ";\n}\n"s;
      return node;
    }

    StructNode *VisitStructNode(StructNode *node) {
      std::string ns = mCppDecl->GetNamespace(node);
      if (ns.empty())
        mDecls += mCppDecl->EmitStructNode(node);
      else
        mDecls += "namespace "s + ns + " {\n"s + mCppDecl->EmitTreeNode(node) + "}\n"s;
      return node;
    }

    TypeAliasNode *VisitTypeAliasNode(TypeAliasNode* node) {
      std::string ns = mCppDecl->GetNamespace(node);
      if (ns.empty())
        mDecls += mCppDecl->EmitTypeAliasNode(node);
      else
        mDecls += "namespace "s + ns + " {\n"s + mCppDecl->EmitTypeAliasNode(node) + "}\n"s;
      return node;
    }

    std::string GetDecls() { return mDecls; }
};

class CollectDecls : public AstVisitor {
  private:
    CppDecl     *mCppDecl;
    std::string  mDecls;

  public:
    CollectDecls(CppDecl *c) : mCppDecl(c), mDecls("// var decls\n") {}

    FunctionNode *VisitFunctionNode(FunctionNode *node) {
      return node;
    }

    LambdaNode *VisitLambdaNode(LambdaNode *node) {
      return node;
    }

    DeclNode *VisitDeclNode(DeclNode *node) {
      std::string def = mCppDecl->EmitTreeNode(node);
      std::string var = mCppDecl->EmitTreeNode(node->GetVar());
      std::string ns = mCppDecl->GetNamespace(node);
      std::string ext = "extern "s + def.substr(0, def.find('=')) + ";\n"s;
      if (ns.empty())
        mDecls += ext;
      else {
        mDecls += "namespace "s + ns + " {\n"s + ext + "}\n"s;
        def = "namespace "s + ns + " {\n"s + def + ";\n}\n"s;
      }
      mCppDecl->AddDefinition(def + ";\n"s);
      return node;
    }

    std::string GetDecls() { return mDecls; }
};

void CppDecl::AddImportedModule(const std::string& module) {
  mImportedModules.insert(module);
}

bool CppDecl::IsImportedModule(const std::string& module) {
  auto res = mImportedModules.find(module);
  return res != mImportedModules.end();
}

// Generate class to encap TS/JS required func interfaces
// note: for top level and nested functions only. Not for class methods.
std::string CppDecl::GenFunctionClass(FunctionNode* node) {
  std::string params, args, retType;
  for (unsigned i = 0; i < node->GetParamsNum(); ++i) {
    if (i) {
      params += ", "s;
      args += ", "s;
    }
    if (auto n = node->GetParam(i)) {
      params += EmitTreeNode(n);
      args += GetIdentifierName(n);
      if (i==0)
        HandleThisParam(node->GetParamsNum(), n, params, args);
    }
  }
  if (node->GetParamsNum() == 0)
    HandleThisParam(0, nullptr, params, args);

  return GenFuncClass(GetTypeString(node->GetType(), nullptr), GetIdentifierName(node), params, args);
}

std::string CppDecl::EmitModuleNode(ModuleNode *node) {
  if (node == nullptr)
    return std::string();
  std::string module = GetModuleName();
  std::string header("__");
  for(auto &c : module)
    header += std::toupper(c);
  header += "__HEADER__\n";
  std::string str("// TypeScript filename: "s + node->GetFilename() + "\n"s);
  str += "#ifndef "s + header + "#define "s + header;
  str += R"""(
#include "ts2cpp.h"
)""";

  ImportExportModules xxportModules(this);
  xxportModules.Visit(node);
  // All include directived from import/export statements
  str += xxportModules.GetIncludes();

  // Generate the namespace of current module
  str += R"""(
namespace )""" + module + R"""( {
)""";

  // Generate code for all imports
  str += xxportModules.GetImports();

  ClassDecls clsDecls(this);
  clsDecls.VisitTreeNode(node);
  // declarations of user defined classes
  str += clsDecls.GetDecls();

  // declarations of all top level functions
  CfgFunc *mod = mHandler->GetCfgFunc();
  auto num = mod->GetNestedFuncsNum();
  for(unsigned i = 0; i < num; ++i) {
    CfgFunc *func = mod->GetNestedFuncAtIndex(i);
    TreeNode *node = func->GetFuncNode();
    if (!IsClassMethod(node)) {
      std::string ns = GetNamespace(node);
      if (!ns.empty())
        str += "namespace "s + ns + " {\n"s;
      str += GenFunctionClass(static_cast<FunctionNode*>(node));  // gen func cls for each top level func
      if (!mHandler->IsFromLambda(node)) {
        // top level funcs instantiated here as function objects from their func class
        // top level lamda funcs instantiated later in assignment stmts
        std::string funcinit = ClsName(node->GetName()) + "* "s + node->GetName() + " = new "s + ClsName(node->GetName()) + "();\n"s;
        if (ns.empty())
          AddDefinition(funcinit);
        else
          AddDefinition("namespace "s + ns + " {\n"s + funcinit + "\n}\n"s);
        str += "extern "s + ClsName(node->GetName()) + "* "s + node->GetName() + ";\n"s;
      }
      if (!ns.empty())
        str += "\n} // namespace " + ns + '\n';
    }
  }

  CollectDecls decls(this);
  decls.VisitTreeNode(node);
  // declarations of all variables
  str += decls.GetDecls();

  // Generate code for all exports
  str += xxportModules.GetExports() + "\nnamespace __export {}\n"s;

  // init function and an object for dynamic properties
  str += R"""(
  // init function for current module
  void __init_func__();

  // all dynamic properties of current module
  extern t2crt::Object __module;

} // namespace of current module

#endif
)""";
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

  if (HasAttrStatic<FunctionNode>(node))
    str = "static "s + str;
  return str;
}

std::string CppDecl::EmitBinOperatorNode(BinOperatorNode *node) {
  const char *op = Emitter::GetEnumOprId(node->GetOprId());
  const Precedence precd = *op & 0x3f;
  const bool rl_assoc = *op >> 6; // false: left-to-right, true: right-to-left
  std::string lhs, rhs;
  if (auto n = node->GetOpndA()) {
    lhs = EmitTreeNode(n);
    if(precd > mPrecedence || (precd == mPrecedence && rl_assoc))
      lhs = "("s + lhs + ")"s;
  }
  else
    lhs = "(NIL) "s;
  if (auto n = node->GetOpndB()) {
    rhs = EmitTreeNode(n);
    if(precd > mPrecedence || (precd == mPrecedence && !rl_assoc))
      rhs = "("s + rhs + ")"s;
  }
  else
    rhs = " (NIL)"s;
  OprId k = node->GetOprId();
  std::string str;
  if(k == OPR_Exp) {
    str = "std::pow("s + lhs + ", "s + rhs + ")";
  } else {
    switch(k) {
      case OPR_Band:
      case OPR_Bor:
      case OPR_Bxor:
      case OPR_Shl:
      case OPR_Shr:
        lhs = "static_cast<int32_t>("s + lhs + ")"s;
        break;
      case OPR_Zext:
        lhs = "static_cast<uint32_t>("s + lhs + ")"s;
        op = "\015>>";
        break;
    }
    str = lhs + " "s + std::string(op + 1) + " "s + rhs;
  }
  mPrecedence = precd;
  if (node->IsStmt())
    str += ";\n"s;
  return str;

}

std::string CppDecl::EmitIdentifierNode(IdentifierNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str(GetTypeString(node, node->GetType()));
  str += " "s + node->GetName();

  if (HasAttrStatic<IdentifierNode>(node))
    str = "static "s + str;
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
    str += "  "s + EmitTreeNode(n);
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
  if (node == nullptr)
    return std::string();
  std::string str("{"s);
  for (unsigned i = 0; i < node->GetLiteralsNum(); ++i) {
    if (i)
      str += ", "s;
    if (auto n = node->GetLiteral(i)) {
      str += EmitTreeNode(n);
    }
  }
  str += "}"s;
  return str;
}

std::string CppDecl::EmitArrayLiteral(ArrayLiteralNode *node, int dim, std::string type) {
  if (node == nullptr)
    return std::string();

  // Generate ctor call to instantiate array, e.g. t2crt::Array1D_long._new(). See builtins.h
  std::string str("t2crt::Array"s + std::to_string(dim) + "D_"s + type + "._new({"s );
  for (unsigned i = 0; i < node->GetLiteralsNum(); ++i) {
    if (i)
      str += ", "s;
    if (auto n = node->GetLiteral(i)) {
      if (n->IsArrayLiteral())
        str += EmitArrayLiteral(static_cast<ArrayLiteralNode*>(n), dim-1, type);
      else
        str += EmitTreeNode(n);
    }
  }
  str += "})"s;
  return str;
}

std::string GetArrayTypeString(int dim, std::string typeStr) {
  std::string str;
  str = "t2crt::Array<"s + typeStr + ">*"s;;
  for (unsigned i = 1; i < dim; ++i) {
    str = "t2crt::Array<"s + str + ">*"s;;
  }
  return str;
}

std::string CppDecl::EmitPrimArrayTypeNode(PrimArrayTypeNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str;
  if (node->GetPrim() && node->GetDims()) {
    str = GetArrayTypeString(
      static_cast<DimensionNode*>(node->GetDims())->GetDimensionsNum(),
      EmitPrimTypeNode(node->GetPrim()));
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

// note: entries below are to match values from ast nodes. Do not prepend with "t2crt::"
std::vector<std::string>builtins = {"Object", "Function", "Number", "Array", "Record"};

bool IsBuiltinObj(std::string name) {
 return std::find(builtins.begin(), builtins.end(), name) != builtins.end();
}

std::string GetUserTypeString(UserTypeNode* n) {
  std::string str="";
  if (n->GetId()->IsTypeIdClass())
    str = n->GetId()->GetName() + "*"s;
  else if (IsBuiltinObj(n->GetId()->GetName()))
    str = "t2crt::"s + n->GetId()->GetName() + "*"s;
  else // TypeAlias Id
    str = n->GetId()->GetName();
  return str;
}

std::string CppDecl::GetTypeString(TreeNode *node, TreeNode *child) {
  std::string str;
  if (node) {
    TypeId k = node->GetTypeId();
    if (k == TY_None || k == TY_Class) {
      switch(node->GetKind()) {
        case NK_PrimType:
          k = static_cast<PrimTypeNode*>(node)->GetPrimType();
          break;
        case NK_Identifier:
        case NK_Function:
          if (child && child->IsUserType())
            return EmitTreeNode(child);
          break;
        case NK_UserType:
          return EmitTreeNode(node);
      }
    }
    switch(k) {
      case TY_Object:
        return "t2crt::Object* "s;
      case TY_Function: // Need to handle class constructor type: Ctor_<class>*
      {
        std::string funcName = GetClassOfAssignedFunc(node);
        if (!funcName.empty())
          return funcName + "* ";
        else
          return "t2crt::Function* "s;
      }
      case TY_Boolean:
        return "bool "s;
      case TY_Int:
        return "long "s;
      case TY_String:
        return "std::string "s;
      case TY_Number:
      case TY_Double:
        return "double "s;
      case TY_Class:
        return "t2crt::Object* "s;
      case TY_Any:
        return "t2crt::JS_Val "s;
    }
    {
      if (child && child->IsStruct() && static_cast<StructNode*>(child)->GetProp() == SProp_NA) {
        // This will change pending solution for issue #69.
        return "t2crt::Object * "s; // object literal type - dynamic-import.ts
      }
      str = child ? EmitTreeNode(child) : (k == TY_Array ? "t2crt::Array<t2crt::JS_Val>* "s : Emitter::GetEnumTypeId(k));
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
      // Generate both vars and arrays of union/intersect type as t2crt::JS_Val of type TY_Object.
      return "t2crt::JS_Val"s;
  }
  std::string str, usrType;

  if (auto n = node->GetId()) {
    if (n->IsTypeIdClass())
      usrType = n->GetName() + "*"s;
    else if (IsBuiltinObj(n->GetName()))
      usrType = "t2crt::"s + n->GetName() + "*"s;
    else // TypeAlias Id gets returned here
      usrType = n->GetName();

    if (node->GetDims()) {
      return GetArrayTypeString(node->GetDims()->GetDimensionsNum(), usrType);
    } else {
      str = usrType;
    }
    auto num = node->GetTypeGenericsNum();
    if(num) {
      std::string lastChar = "";
      if (str.back() == '*') {
        str.pop_back();
        lastChar = "*";
      }
      str += "<"s;
      for (unsigned i = 0; i < num; ++i) {
        if (i)
          str += ", "s;
        if (auto n = node->GetTypeGeneric(i)) {
          str += EmitTreeNode(n);
        }
      }
      str += ">"s;
      str += lastChar;
    }
  }
#if 0
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
#endif
  return str;
}

std::string CppDecl::EmitClassNode(ClassNode *node) {
  std::string str;
  std::string base;
  std::string staticProps;

  if (node == nullptr)
    return std::string();

  // 1. c++ class for JS object
  base = (node->GetSuperClassesNum() != 0)? node->GetSuperClass(0)->GetName() : "t2crt::Object";
  str += "class "s + node->GetName() + " : public "s + base + " {\n"s;

  str += "public:\n";

  // constructor decl
  str += "  "s + node->GetName() + "(t2crt::Function* ctor, t2crt::Object* proto);\n"s;
  str += "  ~"s + node->GetName() + "(){}\n";

  // class field decl and init. TODO: handle private, protected attrs.
  for (unsigned i = 0; i < node->GetFieldsNum(); ++i) {
    auto n = node->GetField(i);
    if (n->IsIdentifier() && GetIdentifierName(n).compare("private")==0)
      continue;
    str += "  "s + EmitTreeNode(n);

    if (n->IsIdentifier()) {
      IdentifierNode* id = static_cast<IdentifierNode*>(n);
      if (HasAttrStatic<IdentifierNode>(id)) {
        // static field - add field to ctor prop and init later at field def in cpp
        staticProps += tab(3) + "this->AddProp(\""s + n->GetName() + "\", t2crt::JS_Val("s +
          TypeIdToJSTypeCXX[n->GetTypeId()] + ", &"s + node->GetName() + "::"s + id->GetName() + "));\n"s;
      } else if (auto init = id->GetInit()) {
        if (init->IsArrayLiteral() && id->GetType() && id->GetType()->IsPrimArrayType()) {
          // Generate initializer for t2crt::Array member field decl in header file
          PrimArrayTypeNode* mtype = static_cast<PrimArrayTypeNode *>(id->GetType());
          str += " = "s + EmitArrayLiteral(static_cast<ArrayLiteralNode*>(init),
                            mtype->GetDims()->GetDimensionsNum(),
                            EmitPrimTypeNode(mtype->GetPrim()));
        } else {
          str += " = "s + EmitTreeNode(init);
        }
      }
    }
    str += ";\n";
  }
  for (unsigned i = 0; i < node->GetMethodsNum(); ++i) {
    str += tab(1) + EmitFunctionNode(node->GetMethod(i));
  }

  // 2. c++ class for the JS object's JS constructor
  std::string indent = tab(1);
  if (!staticProps.empty()) staticProps = "\n"s + staticProps;
  base = (node->GetSuperClassesNum() != 0)? (node->GetSuperClass(0)->GetName()+"::Ctor"s) : "t2crt::Function";
  str += indent + "class Ctor : public "s + base + " {\n"s;
  str += indent + "public:\n";
  str += indent + "  Ctor(t2crt::Function* ctor, t2crt::Object* proto, t2crt::Object* prototype_proto) : "s +
    base + "(ctor, proto, prototype_proto) {"s + staticProps + tab(2) + "}\n"s;

  // constructor function
  for (unsigned i = 0; i < node->GetConstructorsNum(); ++i) {
    std::string ctor;
    if (auto c = node->GetConstructor(i)) {
      ctor = indent + "  "s + node->GetName() + "* operator()("s + node->GetName() + "* obj"s;
      for (unsigned k = 0; k < c->GetParamsNum(); ++k) {
        ctor += ", "s;
        if (auto n = c->GetParam(k)) {
          ctor += EmitTreeNode(n);
        }
      }
      ctor += ");\n";
      str += ctor;
    }
  }

  // Generate decl for default constructor function if none declared for class
  if (node->GetConstructorsNum() == 0)
    str += indent + "  "s + node->GetName() + "* operator()("s + node->GetName() + "* obj);\n"s;

  // Generate new() function
  str += indent + "  "s+node->GetName()+"* _new() {return new "s+node->GetName()+"(this, this->prototype);}\n"s;
  str += indent + "  virtual const char* __GetClassName() const {return \""s + node->GetName() + " \";}\n"s;
  str += indent + "};\n";
  str += "};\n\n";
  return str;
}

std::string CppDecl::EmitNewNode(NewNode *node) {
  if (node == nullptr || node->GetAttrsNum() > 0)
    return std::string();

  std::string str;
  MASSERT(node->GetId() && "No mId on NewNode");
  if (node->GetId() && node->GetId()->IsTypeIdClass()) {
    // Generate code to create new obj and call constructor
    str = node->GetId()->GetName() + "_ctor("s + node->GetId()->GetName() + "_ctor._new("s;
  } else if (IsBuiltinObj(node->GetId()->GetName())) {
    // Check for builtin obejcts: t2crt::Object, t2crt::Function, etc.
    str = node->GetId()->GetName() + "_ctor._new("s;
  } else  {
    str = "new "s + EmitTreeNode(node->GetId());
    str += "("s;
  }

  auto num = node->GetArgsNum();
  for (unsigned i = 0; i < num; ++i) {
    if (i || node->GetId()->IsTypeIdClass())
      str += ", "s;
    if (auto n = node->GetArg(i)) {
      str += EmitTreeNode(n);
    }
  }
  str += ")"s;
  mPrecedence = '\024';
  return str;
}

std::string CppDecl::EmitInterface(StructNode *node) {
  std::string str, ifName;
  std::string def;

  if (node == nullptr)
    return std::string();

  std::string superClass = "t2crt::Object";
  if (node->GetSupersNum() > 0) {
    auto n = node->GetSuper(0);
    superClass = EmitTreeNode(n);
    if (superClass.back() == '*')
      superClass.pop_back();
  }

  if (auto n = node->GetStructId()) {
    ifName = GetIdentifierName(n);
    str = "class "s + ifName + " : public "s + superClass + " {\n"s;
  }
  str += "  public:\n"s;

  // Generate code to add prop in class constructor
  for (unsigned i = 0; i < node->GetFieldsNum(); ++i) {
    if (auto n = node->GetField(i)) {
      str += "    "s + EmitTreeNode(n) + ";\n"s;
      if (n->IsIdentifier()) {
        def += tab(1) + GenClassFldAddProp("this", ifName, n->GetName(),
          GetTypeString(n, static_cast<IdentifierNode*>(n)->GetType()),
          TypeIdToJSTypeCXX[hlpGetTypeId(n)]) + ";\n"s;
      }
    }
  }
  if (!def.empty())
    def = "\n"+def;

  str += "    "s  + ifName + "() {};\n"s;
  str += "    ~"s + ifName + "() {};\n"s;
  str += "    "s + ifName + "(t2crt::Function* ctor, t2crt::Object* proto);\n"s;
  str += "    "s + ifName + "(t2crt::Function* ctor, t2crt::Object* proto, std::vector<t2crt::ObjectProp> props): "s + superClass + "(ctor, proto, props) {}\n"s;
  str += "};\n"s;

  def = ifName + "::"s + ifName + "(t2crt::Function* ctor, t2crt::Object* proto): "s + superClass + "(ctor, proto) {" + def + "}\n";
  AddDefinition(def);
  return str;
}

// Generate C++ class def for the TS num type here and instance in CppDef EmitStructNode.
// TS enum member field can be either Identifier or Literal string (of any character).
// TS enum member value can be either long, double, or string.
// Numeric enum member with no assigned value default to val of preceding member plus 1.
std::string CppDecl::EmitTSEnum(StructNode *node) {
  std::string str, init;
  TypeId memberValType = TY_None;

  if (node == nullptr)
    return std::string();

  for (unsigned i = 0; i < node->GetFieldsNum(); ++i) {
    // Enum member field default to TY_Int if not specified - should this be set by FE?
    if (node->GetField(i)->IsTypeIdNone())
      node->GetField(i)->SetTypeId(TY_Int);
  }

  if (node->GetFieldsNum() > 0) {
    memberValType = node->GetField(0)->GetTypeId();
    MASSERT(memberValType == TY_String ||
            memberValType == TY_Double ||
            memberValType == TY_Int && "Unsupported Enum type");
  }

  str = "class "s;
  std::string enumClsName;
  if (auto n = node->GetStructId()) {
    enumClsName = "Enum_"s + GetIdentifierName(n);
    str += enumClsName + " : public t2crt::Object {\n"s;
  }
  str += "  public:\n"s;
  for (unsigned i = 0; i < node->GetFieldsNum(); ++i) {
    if (auto n = node->GetField(i)) {
      if (n->IsLiteral()) {
        MWARNING("Literal enum field not supported yet");
        continue;
      }
      IdentifierNode* id = static_cast<IdentifierNode*>(n);
      if (id->GetInit()) {
        init = EmitTreeNode(id->GetInit());
      } else {
        if (memberValType == TY_Int || memberValType == TY_Double) {
           // if numeric member and no initialzer, set to 0 or last field + 1
          if (i == 0)
            init = "0";
          else
            init = GetIdentifierName(node->GetField(i-1)) + "+1"s;
        }
      }
      str += "    const "s + EmitTreeNode(n) + " = "s + init + ";\n"s;
    }
  }
  str += "    "s  + enumClsName + "() {};\n"s;
  str += "    ~"s + enumClsName + "() {};\n"s;

  str += "};\n"s;
  return str;
}

std::string CppDecl::EmitStructNode(StructNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str;
  const char *suffix = ";\n";
  switch(node->GetProp()) {
#if 0
    case SProp_CStruct:
      str = "struct "s;
      break;
    case SProp_NA:
      str = ""s;
      break;
#endif
    case SProp_TSEnum:
      str = EmitTSEnum(node);
      return str;
    case SProp_TSInterface:
    case SProp_NA: // for classes generated by TI
      str = EmitInterface(node);
      return str;
    default:
      return std::string();
      MASSERT(0 && "Unexpected enumerator");
  }
#if 0
  if (auto n = node->GetStructId()) {
    str += EmitIdentifierNode(n);
  }

  auto num = node->GetTypeParamsNum();
  if(num) {
    str += "<"s;
    for (unsigned i = 0; i < num; ++i) {
      if (i)
        str += ", "s;
      if (auto n = node->GetTypeParamAtIndex(i))
        str += EmitTreeNode(n);
    }
    str += ">"s;
  }

  for (unsigned i = 0; i < node->GetSupersNum(); ++i) {
    str += i ? ", "s : " extends "s;
    if (auto n = node->GetSuper(i))
      str += EmitTreeNode(n);
  }
  str += " {\n"s;
  for (unsigned i = 0; i < node->GetFieldsNum(); ++i) {
    if (auto n = node->GetField(i)) {
      str += EmitTreeNode(n) + suffix;
    }
  }

  if (auto n = node->GetNumIndexSig())
    str += EmitNumIndexSigNode(n) + "\n"s;;
  }
  if (auto n = node->GetStrIndexSig()) {
    str += EmitStrIndexSigNode(n) + "\n";
  }

  for (unsigned i = 0; i < node->GetMethodsNum(); ++i) {
    if (auto n = node->GetMethod(i)) {
      std::string func = EmitFunctionNode(n);
      if (func.substr(0, 9) == "function ")
        func = func.substr(9);
      size_t index = func.rfind(')');
      if (index != std::string::npos) {
        std::string t = func.substr(index);
        Replace(t, "=>", ":");
        func = func.substr(0, index) + t;
      }
      str += func.length() > 2 && func.substr(func.length() - 2) == ";\n" ? func : func + ";\n"s;
    }
  }
  str += "}\n"s;
#endif
  return HandleTreeNode(str, node);
}

std::string CppDecl::EmitNumIndexSigNode(NumIndexSigNode *node) {
  return std::string();
}

std::string CppDecl::EmitStrIndexSigNode(StrIndexSigNode *node) {
  return std::string();
}

std::string CppDecl::EmitTypeAliasNode(TypeAliasNode* node) {
  if (node == nullptr)
    return std::string();
  std::string str, alias;

  if (auto n = node->GetId()) {
    if (n->IsUserType()) {
      str = EmitTreeNode(n);
      if (str.back() == '*')
        str.pop_back();
    }
  }
  if (auto m = node->GetAlias()) {
    if (m->IsUserType()) {
      alias = EmitTreeNode(m);
      if (alias.back() == '*')
        alias.pop_back();
      str = "using "s + str + " = "s + alias + ";\n";
    } else { // if (m->IsStruct()) {
      // todo
      str = "// type alias for "s + str + '\n';;
    }
  }
  return str;
}

std::string CppDecl::EmitLiteralNode(LiteralNode *node) {
  if (node == nullptr)
    return std::string();
  LitData lit = node->GetData();
  std::string str(AstDump::GetEnumLitData(lit));
  if(lit.mType == LT_StringLiteral || lit.mType == LT_CharacterLiteral)
    str = '"' + str + '"';
  mPrecedence = '\030';
  str = HandleTreeNode(str, node);
  if (auto n = node->GetType()) {
    if (str.compare("this") == 0) {
      // handle special literal "this"
      std::string type = EmitTreeNode(n);
      if (type.compare("t2crt::JS_Val ") == 0)
        // map type ANY for "this" to generic object type
        str = "t2crt::Object* "s + "_this"s;
      else
        str = type + " _this";
    }
    else
      str += ": "s + EmitTreeNode(n);
  }
  if (auto n = node->GetInit()) {
    str += " = "s + EmitTreeNode(n);
  }
  return str;
}

} // namespace maplefe
