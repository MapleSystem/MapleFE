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

namespace maplefe {

class ClassDecls : public AstVisitor {
  private:
    CppDecl     *mCppDecl;
    std::string  mDecls;

  public:
    ClassDecls(CppDecl *c) : mCppDecl(c) {}

    ClassNode *VisitClassNode(ClassNode *node) {
      mDecls += mCppDecl->EmitTreeNode(node);
      return node;
    }
    std::string GetDecls() { return mDecls; }
};

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
using namespace t2crt;

)""";

  // declarations of user defined classes
  ClassDecls clsDecls(this);
  clsDecls.VisitTreeNode(node);
  str += clsDecls.GetDecls();

  str += R"""(
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
    if (node->GetParent() && node->GetParent()->GetKind() != NK_Class) {
      str += EmitTreeNode(node);
    }
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

inline NodeKind IdentifierType(TreeNode* node) {
  return static_cast<IdentifierNode*>(node)->GetType()->GetKind();
}

void GetUserTypeArrayInfo(IdentifierNode* node, int& dims, std::string& typeName) {
  if (auto n = node->GetType()) {
    UserTypeNode* u = static_cast<UserTypeNode*>(n);
    dims = u->GetDimsNum();
    if (auto id = u->GetId()) {
      typeName = static_cast<IdentifierNode*>(id)->GetName();
    }
    if (typeName.compare("Object") == 0) {
      // Use alias type with all text name to work with predefined array types
      // see ObjectP alias declaration in builtins.h
      typeName = "ObjectP";
    }
  }
}

std::string CppDecl::EmitDeclNode(DeclNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str, type;
  if (auto n = node->GetVar()) {
    if (IsVarInitStructLiteral(node)) {   // obj instance decl
      str = "  Object* "s + n->GetName();
    } else if (IsVarInitClass(node)) {    // class constructor instance decl for TS: <var> = <class>
      str = "  Ctor_"s + node->GetInit()->GetName() + " *"s +  n->GetName();
    } else {
      str += " "s + EmitTreeNode(n);
    }

    // Generate initializer for Array decl in header file.
    int dims = 0;
    if (auto init = node->GetInit()) {
      if (init->GetKind() == NK_ArrayLiteral &&
          n->GetKind() == NK_Identifier && static_cast<IdentifierNode*>(n)->GetType()) {
        str += " = "s;
        switch (IdentifierType(n)) {
          case NK_UserType:
            GetUserTypeArrayInfo(static_cast<IdentifierNode*>(n), dims, type);
            str += EmitArrayLiteral(static_cast<ArrayLiteralNode*>(init), dims, type);
            break;
          case NK_PrimArrayType: {
            PrimArrayTypeNode* mtype = static_cast<PrimArrayTypeNode *>(static_cast<IdentifierNode*>(n)->GetType());
            str +=  EmitArrayLiteral(static_cast<ArrayLiteralNode*>(init),
                    mtype->GetDims()->GetDimensionsNum(),
                    EmitPrimTypeNode(mtype->GetPrim()));
            break;
          }
        }
      }
    }
    str += ";\n"s;
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

std::string CppDecl::EmitArrayLiteral(ArrayLiteralNode *node, int dim, std::string type) {
  if (node == nullptr)
    return std::string();

  // Generate ctor call to instantiate Array, e.g. Array1D_long._new(). See builtins.h
  std::string str("Array"s + std::to_string(dim) + "D_"s + type + "._new({"s );
  for (unsigned i = 0; i < node->GetLiteralsNum(); ++i) {
    if (i)
      str += ", "s;
    if (auto n = node->GetLiteral(i)) {
      if (n->GetKind() == NK_ArrayLiteral)
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
  str = "Array<"s + typeStr + ">*"s;;
  for (unsigned i = 1; i < dim; ++i) {
    str = "Array<"s + str + ">*"s;;
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
std::vector<std::string>builtins = {"Object", "Function", "Array"};

bool IsBuiltinObj(std::string name) {
 return std::find(builtins.begin(), builtins.end(), name) != builtins.end();
}

std::string GetUserTypeString(UserTypeNode* n) {
  std::string str="";
  if (n->GetId()->GetTypeId() == TY_Class)
    str = n->GetName() + "*"s;
  else if (IsBuiltinObj(n->GetId()->GetName()))
    str = "t2crt::"s + n->GetId()->GetName() + "*"s;
  return str;
}

//
// For UserTypes, arguments passed in can be from either of the 2
// following node sequences.
//
// Parameters to pass in:
// For case 1, GetTypeString(Identifier, Identifider->GetType())
// For case 2, GetTypeString(UserType, UserType->GetId())
//
// case 1) Non-array var decl
//    [Decl]--Var-->[Identifier]--Type-->[UserType]
// case 2) Array var decl
//    [Decl]--Var-->[Identifier]--Type-->[UserType]--Id-->[Identifier]
//
std::string CppDecl::GetTypeString(TreeNode *node, TreeNode *child) {
  std::string str;
  if (node) {
    TypeId k = node->GetTypeId();
    if (k == TY_None) {
      switch(node->GetKind()) {
        case NK_PrimType:
          k = static_cast<PrimTypeNode*>(node)->GetPrimType();
          break;
        case NK_Identifier:
        case NK_Function:
          if (child && child->GetKind() == NK_UserType) {
            // UserType - handle user defined classes and builtin obj types - case 1
            UserTypeNode* n = node->GetKind() == NK_Identifier?
              static_cast<UserTypeNode*>((static_cast<IdentifierNode *>(node))->GetType()):
              static_cast<UserTypeNode*>((static_cast<FunctionNode *>(node))->GetType());
            if (n && n->GetId() && n->GetId()->GetKind() == NK_Identifier) {
              return GetUserTypeString(n);
            }
          }
          break;
        case NK_UserType: {
          // Handle var decl for array of UserType (class or  builtin objects) - case 2
          MASSERT(node->GetParent()->GetTypeId() == TY_Array && "Unexpected node sequence");
          UserTypeNode* u = static_cast<UserTypeNode*>(node);
          if (u->GetId() && u->GetId()->GetKind() == NK_Identifier) {
            str = GetArrayTypeString(u->GetDims()->GetDimensionsNum(), GetUserTypeString(u));
            return str;
          }
          break;
        }
      }
    }
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
      case TY_Class:
        return "Object* "s;
    }
    {
      str = child ? EmitTreeNode(child) : Emitter::GetEnumTypeId(k);
      if (str != "none"s)
        return str + " "s;
    }
  }
  return "t2crt::JS_Val "s;
}

std::string CppDecl::EmitUserTypeNode(UserTypeNode *node) {
  if (node == nullptr)
    return std::string();

  if (node->GetParent()->GetKind() == NK_Identifier &&
      node->GetParent()->GetTypeId() == TY_Array &&
      node->GetId()) {
    // type to generate is array of usertype
    return GetTypeString(node, node->GetId());
  }

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

std::string CppDecl::EmitClassNode(ClassNode *node) {
  std::string str;
  std::string base;

  if (node == nullptr)
    return std::string();

  // 1. c++ class for JS object
  base = (node->GetSuperClassesNum() != 0)? node->GetSuperClass(0)->GetName() : "Object";
  str += "class "s + node->GetName() + " : public "s + base + " {\n"s;

  str += "public:\n";

  // constructor decl
  str += "  "s + node->GetName() + "(Function* ctor, Object* proto): "s + base + "(ctor, proto)"  + " {}\n"s;
  str += "  ~"s + node->GetName() + "(){}\n";

  // class field decl and init. TODO: handle static, private, protected attrs.
  for (unsigned i = 0; i < node->GetFieldsNum(); ++i) {
    auto n = node->GetField(i);
    str += "  "s + EmitTreeNode(n);
    if (n->GetKind() == NK_Identifier && static_cast<IdentifierNode*>(n)->GetInit()) {
      auto init = static_cast<IdentifierNode*>(n)->GetInit();
      if (init->GetKind() == NK_ArrayLiteral) {
        // Generate initializer for Array member field decl in header file
        PrimArrayTypeNode* mtype = static_cast<PrimArrayTypeNode *>(static_cast<IdentifierNode*>(n)->GetType());
        MASSERT(mtype->GetKind() == NK_PrimArrayType && "Unexpected TreeNode type");
        str += " = "s + EmitArrayLiteral(static_cast<ArrayLiteralNode*>(init),
          mtype->GetDims()->GetDimensionsNum(), EmitPrimTypeNode(mtype->GetPrim()));
      } else
        str += " = "s + EmitTreeNode(static_cast<IdentifierNode *>(n)->GetInit());
    }
    str += ";\n";
  }
  for (unsigned i = 0; i < node->GetMethodsNum(); ++i) {
    str += EmitFunctionNode(node->GetMethod(i));
  }
  str += "};\n\n";

  // 2. c++ class for JS object's corresponding JS constructor
  base = (node->GetSuperClassesNum() != 0)? ("Ctor_"s+node->GetSuperClass(0)->GetName()) : "Function";
  str += "class "s + "Ctor_" + node->GetName() + " : public "s + base + " {\n"s;
  str += "public:\n";
  str += "  Ctor_"s+node->GetName()+"(Function* ctor, Object* proto, Object* prototype_proto) : "+base+"(ctor, proto, prototype_proto) {}\n";

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
      ctor += ");";
      str += ctor;
    }
  }

  // Generate decl for default constructor function if none declared for class
  if (node->GetConstructorsNum() == 0)
    str += "  "s + node->GetName() + "* operator()("s + node->GetName() + "* obj);"s;

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
  if (node->GetId() && node->GetId()->GetTypeId() == TY_Class) {
    // Generate code to create new obj and call constructor
    str = node->GetId()->GetName() + "_ctor("s + node->GetId()->GetName() + "_ctor._new("s;
  } else if (IsBuiltinObj(node->GetId()->GetName())) {
    // Check for builtin obejcts: Object, Function, etc.
    str = node->GetId()->GetName() + "_ctor._new("s;
  } else  {
    str = "new "s + EmitTreeNode(node->GetId());
    str += "("s;
  }

  auto num = node->GetArgsNum();
  for (unsigned i = 0; i < num; ++i) {
    if (i || node->GetId()->GetTypeId()==TY_Class)
      str += ", "s;
    if (auto n = node->GetArg(i)) {
      str += EmitTreeNode(n);
    }
  }
  str += ")"s;
  mPrecedence = '\024';
  if (node->IsStmt()) {
    str += ";\n"s;
  }
  return str;
}

} // namespace maplefe
