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
#include <string.h>

namespace maplefe {

class ImportExportModules : public AstVisitor {
  private:
    CppDecl     *mCppDecl;
    Emitter     *mEmitter;
    std::string  mIncludes;
    std::string  mImports;
    std::string  mExports;
    std::string  mExportDefault;

  public:
    ImportExportModules(CppDecl *c) : mCppDecl(c),
      mIncludes("// include directives\n"),
      mImports("// imports\n"),
      mExports("// exports\n"),
      mExportDefault("// export default\n") {
      mEmitter = new Emitter(c->GetModuleHandler());
    }
    ~ImportExportModules() { delete mEmitter; }

    std::string GetIncludes() { return mIncludes; }
    std::string GetImports() { return mImports; }
    std::string GetExports() { return mExports; }
    std::string GetExportDefault() { return mExportDefault; }

    std::string AddIncludes(TreeNode *node) {
      std::string filename;
      if (node) {
        filename = mEmitter->EmitTreeNode(node);
        auto len = filename.size();
        filename = len >= 2 && filename.back() == '"' ? filename.substr(1, len - 2) : std::string();
        // may have some duplicated include directives which do not hurt
        if (!filename.empty())
          mIncludes += "#include \""s + filename + ".h\"\n"s;
      }
      return filename;
    }

    ImportNode *VisitImportNode(ImportNode *node) {
      std::string filename = AddIncludes(node->GetTarget());
      std::string module = mCppDecl->GetModuleName(filename.c_str());
      for (unsigned i = 0; i < node->GetPairsNum(); ++i) {
        if (auto x = node->GetPair(i)) {
          std::string str;
          if (x->IsDefault()) {
            if (auto n = x->GetBefore()) {
              std::string v = module + "__default"s;
              std::string s = mEmitter->EmitTreeNode(n);
              mImports += "inline const decltype("s + v + ") &"s + s + " = "s + v + ";\n"s;
            }
          } else if (x->IsSingle()) {
            if (auto a = x->GetAfter())
              str += mEmitter->EmitTreeNode(a);
            if (auto n = x->GetBefore()) {
              str += " = "s;
              std::string s = mEmitter->EmitTreeNode(n);
              str += n->IsLiteral() ? "require("s + s + ')' : s;
            }
          } else if (x->IsEverything()) {
            if (auto n = x->GetBefore()) {
              std::string s = mEmitter->EmitTreeNode(n);
              mImports += "namespace "s + s + " = " + module + ";\n"s;
              mCppDecl->AddImportedModule(s);
            }
          } else {
            if (auto n = x->GetBefore()) {
              std::string v = mEmitter->EmitTreeNode(n);
              if (auto a = x->GetAfter()) {
                std::string after = mEmitter->EmitTreeNode(a);
                if (after == "default") {
                }
              } else {
                mImports += "inline const decltype("s + module + "::"s + v + ") &"s + v + " = "s + module + "::"s + v + ";\n"s;
              }
            }
          }
        }
      }
      return node;
    }

    ExportNode *VisitExportNode(ExportNode *node) {
      std::string filename = AddIncludes(node->GetTarget());
      std::string module = mCppDecl->GetModuleName(filename.c_str());
      for (unsigned i = 0; i < node->GetPairsNum(); ++i) {
        if (auto x = node->GetPair(i)) {
          if (x->IsDefault()) {
            if (auto n = x->GetBefore()) {
              std::string v = mEmitter->EmitTreeNode(n);
              std::string def = "decltype("s + v + ") __default_"s + v + ";\n"s;
              mCppDecl->AddDefinition(def);
              mImports += "extern "s + def;
              mExportDefault = "#define "s + module + "__default "s + module + "::__default_"s + v + '\n';
            }
          } else if (x->IsSingle()) {
            std::string str;
            if (auto a = x->GetAfter())
              str += mEmitter->EmitTreeNode(a);
            if (auto n = x->GetBefore()) {
              str += " = "s;
              std::string s = mEmitter->EmitTreeNode(n);
              str += n->IsLiteral() ? "require("s + s + ')' : s;
            }
          } else if (x->IsEverything()) {
            mImports += "using namespace "s + module + "::__export;\n"s;
          } else {
            if (auto b = x->GetBefore()) {
              std::string before = mEmitter->EmitTreeNode(b);
              mCppDecl->AddExportedId(before);
              if (auto a = x->GetAfter()) {
                std::string after = mEmitter->EmitTreeNode(a);
                if (before == "default") {
                  mImports += "inline const decltype("s + module + "__default) &"s + after + " = "s + module + "__default;\n"s;
                  before = module + "__default";
                }
                if (after == "default")
                  mExportDefault = "#define "s + module + "__default "s + module + "::"s + before + '\n';
              }
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
      mDecls += mCppDecl->EmitTreeNode(node) + ";\n"s;
      return node;
    }

    StructNode *VisitStructNode(StructNode *node) {
      mDecls += mCppDecl->EmitStructNode(node);
      return node;
    }

    TypeAliasNode *VisitTypeAliasNode(TypeAliasNode* node) {
      mDecls += mCppDecl->EmitTypeAliasNode(node);
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
      if (mCppDecl->IsExportedId(var)) {
        mDecls += "namespace __export { extern "s + def.substr(0, def.find('=')) + ";}\n"s;
        mCppDecl->AddDefinition("namespace __export { "s + def + ";}\n"s);
      } else {
        mDecls += "extern "s + def.substr(0, def.find('=')) + ";\n"s;
        mCppDecl->AddDefinition(def + ";\n"s);
      }
      return node;
    }

    std::string GetDecls() { return mDecls; }
};

void CppDecl::AddExportedId(const std::string& id) {
  mExportedIds.insert(id);
}

bool CppDecl::IsExportedId(const std::string& id) {
  std::size_t loc = id.rfind(' ');
  std::string key = loc == std::string::npos ? id : id.substr(loc + 1);
  auto res = mExportedIds.find(key);
  return res != mExportedIds.end();
}

void CppDecl::AddImportedModule(const std::string& module) {
  mImportedModules.insert(module);
}

bool CppDecl::IsImportedModule(const std::string& module) {
  auto res = mImportedModules.find(module);
  return res != mImportedModules.end();
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

  ClassDecls clsDecls(this);
  clsDecls.VisitTreeNode(node);
  // declarations of user defined classes
  str += clsDecls.GetDecls();

  CollectDecls decls(this);
  decls.VisitTreeNode(node);
  // declarations of all variables
  str += decls.GetDecls();

  // declarations of all functions
  CfgFunc *mod = mHandler->GetCfgFunc();
  auto num = mod->GetNestedFuncsNum();
  for(unsigned i = 0; i < num; ++i) {
    CfgFunc *func = mod->GetNestedFuncAtIndex(i);
    TreeNode *node = func->GetFuncNode();
    if (node->GetParent() && !node->GetParent()->IsClass()) {
      std::string func = EmitTreeNode(node);
      std::string id = EmitTreeNode(static_cast<FunctionNode *>(node)->GetFuncName());
      if (IsExportedId(id)) {
        AddDefinition("namespace __export { "s + func + "}\n"s);
        str += "namespace __export { extern "s + func + "}\n"s;
      } else {
        AddDefinition(func);
        str += "extern "s + func;
      }
    }
  }

  str += R"""(
  namespace __export {}
  using namespace __export;
)""";

  // Generate code for all imports
  str += xxportModules.GetImports();

  // export default
  str += xxportModules.GetExportDefault();

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
  return str;
}

std::string IdentifierName(TreeNode* node) {
  if (node == nullptr)
    return std::string();
  MASSERT(node->IsIdentifier() && "Unexpected node type");
  return node->GetName();
}

std::string CppDecl::EmitPrimTypeNode(PrimTypeNode *node) {
  if (node == nullptr)
    return std::string();
  return GetTypeString(node);
}

std::string CppDecl::EmitDeclNode(DeclNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str, type;
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

// TODO: Add other builtin obj types
std::vector<std::string>builtins = {"t2crt::Object", "t2crt::Function", "t2crt::Number", "t2crt::Array", "t2crt::Record"};

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
        return "t2crt::Function* "s;
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

  if (node == nullptr)
    return std::string();

  // 1. c++ class for JS object
  base = (node->GetSuperClassesNum() != 0)? node->GetSuperClass(0)->GetName() : "t2crt::Object";
  str += "class "s + node->GetName() + " : public "s + base + " {\n"s;

  str += "public:\n";

  // constructor decl
  str += "  "s + node->GetName() + "(t2crt::Function* ctor, t2crt::Object* proto);\n"s;
  str += "  ~"s + node->GetName() + "(){}\n";

  // class field decl and init. TODO: handle static, private, protected attrs.
  for (unsigned i = 0; i < node->GetFieldsNum(); ++i) {
    auto n = node->GetField(i);
    str += "  "s + EmitTreeNode(n);
    if (n->IsIdentifier() && static_cast<IdentifierNode*>(n)->GetInit()) {
      if (auto init = static_cast<IdentifierNode*>(n)->GetInit()) {
        if (init->IsArrayLiteral() &&
            static_cast<IdentifierNode*>(n)->GetType() &&
            static_cast<IdentifierNode*>(n)->GetType()->IsPrimArrayType() ) {
              // Generate initializer for t2crt::Array member field decl in header file
              PrimArrayTypeNode* mtype = static_cast<PrimArrayTypeNode *>(static_cast<IdentifierNode*>(n)->GetType());
              str += " = "s + EmitArrayLiteral(static_cast<ArrayLiteralNode*>(init),
                                mtype->GetDims()->GetDimensionsNum(),
                                EmitPrimTypeNode(mtype->GetPrim()));
        } else {
          str += " = "s + EmitTreeNode(static_cast<IdentifierNode *>(n)->GetInit());
        }
      }
    }
    str += ";\n";
  }
  for (unsigned i = 0; i < node->GetMethodsNum(); ++i) {
    str += EmitFunctionNode(node->GetMethod(i));
  }
  str += "};\n\n";

  // 2. c++ class for JS object's corresponding JS constructor
  base = (node->GetSuperClassesNum() != 0)? ("Ctor_"s+node->GetSuperClass(0)->GetName()) : "t2crt::Function";
  str += "class "s + "Ctor_" + node->GetName() + " : public "s + base + " {\n"s;
  str += "public:\n";
  str += "  Ctor_"s+node->GetName()+"(t2crt::Function* ctor, t2crt::Object* proto, t2crt::Object* prototype_proto) : "+base+"(ctor, proto, prototype_proto) {}\n";

  // constructor function
  for (unsigned i = 0; i < node->GetConstructorsNum(); ++i) {
    std::string ctor;
    if (auto c = node->GetConstructor(i)) {
      ctor = "  "s + node->GetName() + "* operator()("s + node->GetName() + "* obj"s;
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
    str += "  "s + node->GetName() + "* operator()("s + node->GetName() + "* obj);\n"s;

  // Generate new() function
  str += "  "s+node->GetName()+"* _new() {return new "s+node->GetName()+"(this, this->prototype);}\n"s;
  str += "};\n\n";
  return str;
}

std::string CppDecl::EmitNewNode(NewNode *node) {
  if (node == nullptr)
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
    ifName = IdentifierName(n);
    str = "class "s + ifName + " : public "s + superClass + " {\n"s;
  }
  str += "  public:\n"s;
  for (unsigned i = 0; i < node->GetFieldsNum(); ++i) {
    if (auto n = node->GetField(i)) {
      str += "    "s + EmitTreeNode(n) + ";\n"s;
    }
  }
  str += "    "s  + ifName + "() {};\n"s;
  str += "    ~"s + ifName + "() {};\n"s;
  str += "    "s  + ifName + "(t2crt::Function* ctor, t2crt::Object* proto, std::vector<t2crt::ObjectProp> props): "s + superClass + "(ctor, proto, props) {}\n"s;
  str += "};\n"s;
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
    enumClsName = "Enum_"s + IdentifierName(n);
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
            init = IdentifierName(node->GetField(i-1)) + "+1"s;
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
    } else if (m->IsStruct()) {
      // todo
    }
  }
  return str;
}

std::string ident(int n) {
  std::string str;
  for (unsigned i = 0; i < n; ++i)
    str += "  "s;
  return str;
}

} // namespace maplefe
