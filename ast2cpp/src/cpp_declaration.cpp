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

class ClassDecls : public AstVisitor {
  private:
    CppDecl     *mCppDecl;
    std::string  mDecls;

  public:
    ClassDecls(CppDecl *c) : mCppDecl(c) {}

    ClassNode *VisitClassNode(ClassNode *node) {
      mDecls += mCppDecl->EmitTreeNode(node) + ";\n"s;
      return node;
    }

    StructNode *VisitStructNode(StructNode *node) {
      mDecls += mCppDecl->EmitStructNode(node);
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
      mDecls += mCppDecl->EmitTreeNode(node) + ";\n"s;
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
      str += EmitTreeNode(node) + GetEnding(node);
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

    // Generate initializer for Array decl in header file.
    int dims = 0;
    if (auto init = node->GetInit()) {
      if (init->IsArrayLiteral() &&
          n->IsIdentifier() && static_cast<IdentifierNode*>(n)->GetType()) {
        IdentifierNode* id = static_cast<IdentifierNode*>(n);
        str += " = "s;
        switch (id->GetType()->GetKind()) {
          case NK_UserType: {
            UserTypeNode* u = static_cast<UserTypeNode*>(id->GetType());
            if (auto id = u->GetId()) {
              // Get name of user defined class or TS builtin objects to work with
              // names of multi dim array generated with templates in builtins.h
              // ("Object" need to be translated into ObjectP which is alias to type
              // Object* (see builtins.h)). Works ok with user defined classes and
              // builtin such as in array-new-elems.ts but still need more work
              // to handle more TS builtin object types.
              type = id->GetName();
              if (type.compare("Object") == 0)
                type = "ObjectP";
            }
            dims = u->GetDims()? u->GetDimsNum(): 0;
            str += EmitArrayLiteral(static_cast<ArrayLiteralNode*>(init), dims, type);
            break;
          }
          case NK_PrimArrayType: {
            PrimArrayTypeNode* mtype = static_cast<PrimArrayTypeNode *>(id->GetType());
            str +=  EmitArrayLiteral(static_cast<ArrayLiteralNode*>(init),
                    mtype->GetDims()->GetDimensionsNum(),
                    EmitPrimTypeNode(mtype->GetPrim()));
            break;
          }
        }
      }
    }
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
std::vector<std::string>builtins = {"Object", "Function", "Array", "Record"};

bool IsBuiltinObj(std::string name) {
 return std::find(builtins.begin(), builtins.end(), name) != builtins.end();
}

std::string GetUserTypeString(UserTypeNode* n) {
  std::string str="";
  if (n->GetId()->GetTypeId() == TY_Class)
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
        return "Object* "s;
      case TY_Any:
        return "t2crt::JS_Val "s;
    }
    {
      if (child && child->IsStruct() && static_cast<StructNode*>(child)->GetProp() == SProp_NA) {
        // This will change pending solution for issue #69.
        return "Object * "s; // object literal type - dynamic-import.ts
      }
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
  {
    auto k = node->GetType();
    if(k == UT_Union || k == UT_Inter)
      // Generate both vars and arrays of union/intersect type as JS_Val of type TY_Object.
      return "t2crt::JS_Val"s;
  }
  std::string str, usrType;

  if (auto n = node->GetId()) {
    if (n->GetTypeId() == TY_Class)
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
  base = (node->GetSuperClassesNum() != 0)? node->GetSuperClass(0)->GetName() : "Object";
  str += "class "s + node->GetName() + " : public "s + base + " {\n"s;

  str += "public:\n";

  // constructor decl
  str += "  "s + node->GetName() + "(Function* ctor, Object* proto): "s + base + "(ctor, proto)"  + " {}\n"s;
  str += "  "s + node->GetName() + "(Function* ctor, Object* proto, std::vector<ObjectProp> props): "s + base + "(ctor, proto, props) {}\n"s;
  str += "  ~"s + node->GetName() + "(){}\n";

  // class field decl and init. TODO: handle static, private, protected attrs.
  for (unsigned i = 0; i < node->GetFieldsNum(); ++i) {
    auto n = node->GetField(i);
    str += "  "s + EmitTreeNode(n);
    if (n->GetKind() == NK_Identifier && static_cast<IdentifierNode*>(n)->GetInit()) {
      if (auto init = static_cast<IdentifierNode*>(n)->GetInit()) {
        if (init->GetKind() == NK_ArrayLiteral &&
            static_cast<IdentifierNode*>(n)->GetType() &&
            static_cast<IdentifierNode*>(n)->GetType()->GetKind() == NK_PrimArrayType ) {
              // Generate initializer for Array member field decl in header file
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
      ctor += ");\n";
      str += ctor;
    }
  }

  // Generate decl for default constructor function if none declared for class
  if (node->GetConstructorsNum() == 0)
    str += "  "s + node->GetName() + "* operator()("s + node->GetName() + "* obj);\n"s;

  // Generate new() function
  str += "  "s+node->GetName()+"* _new() {return new "s+node->GetName()+"(this, this->prototype);}\n"s;
  str += "  "s+node->GetName()+"* _new(std::vector<ObjectProp> props) {return new "s+node->GetName()+"(this, this->prototype, props);}\n"s;
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
  return str;
}

std::string CppDecl::EmitInterface(StructNode *node) {
  std::string str, ifName;

  if (node == nullptr)
    return std::string();

  std::string superClass = "Object";
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
  str += "    "s  + ifName + "(Function* ctor, Object* proto, std::vector<ObjectProp> props): "s + superClass + "(ctor, proto, props) {}\n"s;
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
    if (node->GetField(i)->GetTypeId() == TY_None)
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
    str += enumClsName + " : public Object {\n"s;
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

  auto num = node->GetTypeParametersNum();
  if(num) {
    str += "<"s;
    for (unsigned i = 0; i < num; ++i) {
      if (i)
        str += ", "s;
      if (auto n = node->GetTypeParameterAtIndex(i))
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

std::string ident(int n) {
  std::string str;
  for (unsigned i = 0; i < n; ++i)
    str += "  "s;
  return str;
}

} // namespace maplefe
