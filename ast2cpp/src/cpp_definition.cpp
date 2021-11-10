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
#include "helper.h"

namespace maplefe {

std::string EmitCtorInstance(ClassNode *c) {
  std::string str, thisClass, ctor, proto, prototypeProto;
  ctor = "&t2crt::Function_ctor";
  thisClass = c->GetName();
  if (c->GetSuperClassesNum() == 0) {
    proto = "t2crt::Function_ctor.prototype";
    prototypeProto = "t2crt::Object_ctor.prototype";
  } else {
    proto = c->GetSuperClass(0)->GetName() + "_ctor"s;
    prototypeProto = proto + ".prototype"s;
    proto.insert(0, "&"s, 0, std::string::npos);
  }
  str = "\n// Instantiate constructor\n"s;
  str += "Ctor_"s+thisClass +" "+ thisClass+"_ctor("s +ctor+","s+proto+","+prototypeProto+");\n"s;
  return str;
}

// Emit default constructor func def and instance
std::string EmitDefaultCtor(ClassNode *c) {
  if (c == nullptr)
    return std::string();

  std::string str, className;
  className = c->GetName();
  str = "\n"s;
  str += className + "*"s + " Ctor_"s + className + "::operator()("s + className + "* obj)"s;
  str += "{ return obj; }\n"s;
  str += EmitCtorInstance(c);

  return str;
}

std::string CppDef::EmitCppCtor(ClassNode* node) {
  std::string str, base, props;
  props = EmitClassProps(node);
  base = (node->GetSuperClassesNum() != 0)? node->GetSuperClass(0)->GetName() : "t2crt::Object";
  str += node->GetName() + "::"s + node->GetName() + "(t2crt::Function* ctor, t2crt::Object* proto): "s
    + base + "(ctor, proto)"  + " {\n"s + props +"}\n"s;
  return str;
}

std::string CppDef::EmitModuleNode(ModuleNode *node) {
  if (node == nullptr)
    return std::string();
  std::string module = GetModuleName();

  // include directives
  std::string str("// TypeScript filename: "s + node->GetFilename() + "\n"s);
  str += "#include <iostream>\n#include \""s + GetBaseFilename() + ".h\"\n\n"s;

  // start a namespace for this module
  str += "namespace "s + module + " {\n"s;

  // definitions of default class constructors
  for (unsigned i = 0; i < node->GetTreesNum(); ++i) {
    if (auto n = node->GetTree(i))
      if (n->IsClass()) {
        str += EmitCppCtor(static_cast<ClassNode*>(n));
        if (static_cast<ClassNode*>(n)->GetConstructorsNum() == 0)
          str += EmitDefaultCtor(static_cast<ClassNode*>(n));
      }
  }

  // definitions
  str += mCppDecl.GetDefinitions();

  // definitions of all functions in current module
  CfgFunc *mod = mHandler->GetCfgFunc();
  auto num = mod->GetNestedFuncsNum();
  for(unsigned i = 0; i < num; ++i) {
    CfgFunc *func = mod->GetNestedFuncAtIndex(i);
    TreeNode *node = func->GetFuncNode();
    std::string s = EmitTreeNode(node) + GetEnding(node);
    std::string id = EmitTreeNode(static_cast<FunctionNode *>(node)->GetFuncName());
    str += s;
  }

  // definition of init function of current module
  str += R"""(void __init_func__() {
  // bind "this" to current module
  static bool __init_once = false;
  if (__init_once) return;
  __init_once = true;
)""" + mCppDecl.GetInits();
  mIsInit = true;
  for (unsigned i = 0; i < node->GetTreesNum(); ++i) {
    if (auto n = node->GetTree(i)) {
#if 0
      if (!n->IsClass())
        str += "  "s + EmitTreeNode(n) + ";\n"s;
#else
      if (n->GetKind() != NK_Class) {
        std::string s = EmitTreeNode(n);
        if (s.back() == '\n') {
          str += ' ' + s;
          continue;
        } else if (s.back() == ';')
          str += ' ' + s + "\n"s;
        else if (!s.empty())
          str += ' ' + s + ";\n"s;
      }
#endif
    }
  }
  str += R"""(}

  t2crt::Object __module;
} // namespace of current module
)""";

  AST_Handler *handler = mHandler->GetASTHandler();
  HandlerIndex idx = handler->GetHandlerIndex(node->GetFilename());
  // If the program starts from this module, generate the main function
  if (idx == 0) {
    str += R"""(
int main(int argc, char **argv) {
  std::cout << std::boolalpha;
)""" + "  "s + module + R"""(::__init_func__(); // call its __init_func__()
  return 0;
}
)""";
  }
  return str;
}

std::string CppDef::EmitExportNode(ExportNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str;
  auto num = node->GetPairsNum();
  for (unsigned i = 0; i < node->GetPairsNum(); ++i) {
    if (auto x = node->GetPair(i)) {
      if (x->IsDefault()) {
        if (auto n = x->GetBefore()) {
          std::string v = EmitTreeNode(n);
          str += "__default_"s + v + " = "s + v + ";\n"s;
        }
      }
      if(node->GetTarget() == nullptr &&
          !x->IsDefault() &&
          !x->IsEverything() &&
          !x->GetAsNamespace() &&
          !x->IsSingle()) {
        //str += EmitXXportAsPairNode(x);
        if (auto n = x->GetBefore())
          if ((!n->IsIdentifier() || static_cast<IdentifierNode *>(n)->GetInit() != nullptr) &&
              !n->IsStruct() &&
              !n->IsClass() &&
              !n->IsUserType())
            str += EmitTreeNode(n);
      }
    }
  }
  return HandleTreeNode(str, node);
}

std::string CppDef::EmitImportNode(ImportNode *node) {
  return std::string();
}

std::string CppDef::EmitXXportAsPairNode(XXportAsPairNode *node) {
  return std::string();
}

inline std::string GetClassName(FunctionNode* f) {
  TreeNode* n = f->GetParent();
  if (n && n->IsClass())
    return n->GetName();
  return ""s;
}

inline bool IsClassMethod(FunctionNode* f) {
  return (f && f->GetParent() && f->GetParent()->IsClass());
}

std::string CppDef::EmitClassProps(TreeNode* node) {
  std::string clsFd, addProp;
  MASSERT(node->IsClass() && "Not NK_Class node");
  ClassNode* c = static_cast<ClassNode*>(node);
  for (unsigned i = 0; i < c->GetFieldsNum(); ++i) {
    auto node = c->GetField(i);
    if (!node->IsIdentifier())
      // StrIndexSig, NumIndexSig, ComputedName have to be handled at run time
      continue;
    std::string fdName = node->GetName();
    std::string fdType = mCppDecl.GetTypeString(node, static_cast<IdentifierNode*>(node)->GetType());
    TypeId typeId = node->GetTypeId();
    if (typeId == TY_None) {
      if (auto n = static_cast<IdentifierNode*>(node)->GetType()) {
        if (n->IsPrimType())
          typeId = static_cast<PrimTypeNode*>(n)->GetPrimType();
        else if (n->IsUserType())
          typeId = static_cast<UserTypeNode*>(n)->GetTypeId();
      }
    }
    addProp += "  "s + hlpClassFldAddProp("this", c->GetName(), fdName, fdType, TypeIdToJSTypeCXX[typeId]) + ";\n"s;
  }
  return "  // Add class fields to obj prop list\n"s + clsFd + addProp;
}

// "var" declarations in TS/JS functions are function scope.
// Duplicate decls may appear in different blocks within
// function but should all refer to the same function scope var.
//
// So we scan for JS_Var decls with dup names in a function and emit
// a decl list with unique names for insert into function definition.
//
// The var may be initialized to different values in different
// blocks which will done in the individual DeclNodes (re: var-dup.ts)
std::string CppDef::EmitFuncScopeVarDecls(FunctionNode *node) {
  std::unordered_map<std::string, DeclNode*>varDeclsInScope;
  ASTScope* s = node->GetScope();
  for (int i = 0; i < s->GetDeclNum(); i++) {
    // build list of var decls (no dups) in function scope
    TreeNode* n = s->GetDecl(i);
    if (!n->IsDecl())
      continue;
    DeclNode* d = static_cast<DeclNode*>(n);
    if (d->GetProp() == JS_Var) {
      // skip var decl name duplicates - same name but diff
      // type is illegal in typescript so not checked here
      std::unordered_map<std::string, DeclNode*>::iterator it;
      it = varDeclsInScope.find(d->GetVar()->GetName());
      if (it == varDeclsInScope.end()) {
        varDeclsInScope[d->GetVar()->GetName()] = d;
      }
    }
  }
  std::string str;
  for (auto const&[key, val] : varDeclsInScope) {
    // Emit decl for the var  (just type, name - init part emitted
    // when corresponding DeclNode processed
    str += mCppDecl.EmitTreeNode(val->GetVar()) + ";"s;
  }
  return str;
}

std::string CppDef::EmitFunctionNode(FunctionNode *node) {
  if (mIsInit || node == nullptr)
    return std::string();

  std::string str, className;
  if (node->IsConstructor()) {
    className = GetClassName(node);
    str = "\n"s;
    str += className + "*"s + " Ctor_"s + className + "::operator()("s + className + "* obj"s;
  } else {
    str = mCppDecl.GetTypeString(node->GetType(), node->GetType());
    if(node->GetStrIdx())
      str += " "s + (IsClassMethod(node) ? GetClassName(node) + "::"s : ""s) + node->GetName();
    str += "("s;
  }

  for (unsigned i = 0; i < node->GetParamsNum(); ++i) {
    if (i || node->IsConstructor())
      str += ", "s;
    if (auto n = node->GetParam(i)) {
      str += mCppDecl.EmitTreeNode(n);
    }
  }
  str += ")"s;
  int bodyPos = str.size();
  if (auto n = node->GetBody()) {
    auto varDecls = EmitFuncScopeVarDecls(node);
    auto s = EmitBlockNode(n);
    if(s.empty() || s.front() != '{')
      str += "{"s + s + "}\n"s;
    else
      str += s;
    str.insert(bodyPos+1, varDecls);
  } else
    str += "{}\n"s;

  if (node->IsConstructor()) {
    Emitter::Replace(str, "this->", "obj->", 0);
    std::string ctorBody;
    ctorBody += "  return obj;\n"s;
    str.insert(str.size()-2, ctorBody, 0, std::string::npos);
    str += EmitCtorInstance(static_cast<ClassNode*>(node->GetParent()));
  }

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
  return str;
}

// Generate vector of t2crt::ObjectProp to be passed to t2crt::Object constructor.
std::string CppDef::EmitStructLiteralNode(StructLiteralNode* node) {
  std::string str;
  int stops = 2;
  str += "\n"s + indent(stops) + "std::vector<t2crt::ObjectProp>({\n"s;
  for (unsigned i = 0; i < node->GetFieldsNum(); ++i) {
    if (i)
      str += ",\n"s;
    if (auto field = node->GetField(i)) {
      auto lit = field->GetLiteral();
      std::string fieldName = EmitTreeNode(field->GetFieldName());
      TypeId typId = lit->GetTypeId();
      std::string fieldVal = EmitTreeNode(lit);
      str += indent(stops+1);
      switch(typId) {
        case TY_Object:
          break;
        case TY_Function:
          break;
        case TY_Array:
          //str += "std::make_pair(\""s + fieldName + "\", t2crt::JS_Val(t2crt::Object*("s + fieldVal + ")))"s;
          break;
        case TY_Boolean:
          str += "std::make_pair(\""s + fieldName + "\", t2crt::JS_Val(bool("s + fieldVal + ")))"s;
          break;
        case TY_None:
          if (fieldVal.compare("true") == 0 || fieldVal.compare("false") == 0)
            str += "std::make_pair(\""s + fieldName + "\", t2crt::JS_Val(bool("s + fieldVal + ")))"s;
          break;
        case TY_Int:
          str += "std::make_pair(\""s + fieldName + "\", t2crt::JS_Val(int64_t("s + fieldVal + ")))"s;
          break;
        case TY_String:
          str += "std::make_pair(\""s + fieldName + "\", t2crt::JS_Val("s + fieldVal + "))"s;
          break;
        case TY_Number:
        case TY_Double:
          str += "std::make_pair(\""s + fieldName + "\", t2crt::JS_Val(double("s + fieldVal + ")))"s;
          break;
        case TY_Class:
          // Handle embedded t2crt::ObjectLiterals recursively
          if (lit->IsStructLiteral()) {
            std::string props = EmitStructLiteralNode(static_cast<StructLiteralNode*>(lit));
            str += "std::make_pair(\""s + fieldName + "\", t2crt::JS_Val(t2crt::Object_ctor._new("s + props + ")))"s;
          }
          break;
      }
    }
  }
  str += " })"s;
  return str;
}

std::string CppDef::EmitDirectFieldInit(std::string varName, StructLiteralNode* node) {
  std::string str;
  //str += ";\n"s;
  for (unsigned i = 0; i < node->GetFieldsNum(); ++i) {
    if (auto field = node->GetField(i)) {
      auto lit = field->GetLiteral();
      std::string fieldName = EmitTreeNode(field->GetFieldName());
      std::string fieldVal = EmitTreeNode(lit);
      if (false) // TODO: Check if it accesses a Cxx class field
        str += indent(2) + varName + "->"s + fieldName + " = "s + fieldVal + ";\n"s;
      else
        str += indent(2) + "(*"s + varName + ")[\""s + fieldName + "\"] = "s + fieldVal + ";\n"s;
    }
  }
  return str;
}

std::string CppDef::EmitObjPropInit(std::string varName, TreeNode* varIdType, StructLiteralNode* node) {
  if (varName.empty())
    return std::string();

  std::string str;
  // UserType can be TS interface, class, type alias, builtin (t2crt::Object, t2crt::Record..)
  UserTypeNode* userType = (varIdType && varIdType->IsUserType())? (UserTypeNode*)varIdType: nullptr;

  if (userType == nullptr) {
    // no type info - create instance of builtin t2crt::Object with proplist
    str = varName+ " = t2crt::Object_ctor._new("s + EmitTreeNode(node) + ")"s;
  } else if (IsClassId(userType->GetId())) {
    // user def class type
    // - create obj instance of user defined class and do direct field access init
    // - todo: handle class with generics
    // - todo: generate addprop instead of direct field access if prop is not a field in class decl
    str = varName+ " = "s +userType->GetId()->GetName()+ "_ctor._new();\n"s;
    str += EmitDirectFieldInit(varName, node);
  } else {
    // type is builtin (e.g. t2crt::Record) and StructNode types (e.g. TSInterface)
    // create instance of type but set constructor to the builtin t2crt::Object.
    str = varName+ " = new "s +EmitUserTypeNode(userType)+ "(&t2crt::Object_ctor, t2crt::Object_ctor.prototype);\n"s;
    auto n = mHandler->FindDecl(static_cast<IdentifierNode*>(userType->GetId()));
    if (n && n->IsStruct() && static_cast<StructNode*>(n)->GetProp() == SProp_TSInterface) {
      str += EmitDirectFieldInit(varName, node); // do direct field init
    }
  }
  return str;
}

std::string CppDef::EmitArrayLiterals(TreeNode *node, int dim, std::string type) {
  if (node == nullptr || !node->IsArrayLiteral())
    return std::string();

  if (type.back() == ' ')
    type.pop_back();

  // Generate ctor call to instantiate t2crt::Array, e.g. t2crt::Array1D_long._new(). See builtins.h
  std::string str("t2crt::Array"s + std::to_string(dim) + "D_"s + type + "._new({"s );
  for (unsigned i = 0; i < static_cast<ArrayLiteralNode*>(node)->GetLiteralsNum(); ++i) {
    if (i)
      str += ", "s;
    if (auto n = static_cast<ArrayLiteralNode*>(node)->GetLiteral(i)) {
      if (n->IsArrayLiteral())
        str += EmitArrayLiterals(static_cast<ArrayLiteralNode*>(n), dim-1, type);
      else
        str += EmitTreeNode(n);
    }
  }
  str += "})"s;
  return str;
}

std::string CppDef::EmitArrayLiteral(TreeNode* arrType, TreeNode* arrLiteral) {
  std::string str, type;
  int dims = 1;  // default to 1 dim array if no Dims info
  if (arrType == nullptr || arrLiteral == nullptr)
    return "nullptr"s;

  if (arrType->IsUserType()) {             // array of usertyp
    if (static_cast<UserTypeNode*>(arrType)->GetDims())
      dims = static_cast<UserTypeNode*>(arrType)->GetDimsNum();
    if (auto id = static_cast<UserTypeNode*>(arrType)->GetId()) {
      // Get class name or TS builtin obj name for array of usertyp objects.
      // ("Object" need to be translated into its type alias "ObjectP"
      // see builtins.h).
      type = id->GetName();
      if (type.compare("t2crt::Object") == 0 ||
          type.compare("Object") == 0)
        type = "ObjectP";
    }
    str = EmitArrayLiterals(arrLiteral, dims, type);
  } else if (arrType->IsPrimArrayType()) { // array of prim type
    if (static_cast<PrimArrayTypeNode *>(arrType)->GetDims())
      dims = static_cast<PrimArrayTypeNode *>(arrType)->GetDims()->GetDimensionsNum();
    type= EmitPrimTypeNode(static_cast<PrimArrayTypeNode*>(arrType)->GetPrim());
    str = EmitArrayLiterals(arrLiteral, dims, type);
  }
  return str;
}

// decl of global var is handled by EmitDeclNode in cpp_declaration
// decl of function vars of type JS_Var is handled in EmitFuncSCopeVarDecls
// This function handles init of global/func var, and decl/init of func let/const.
std::string CppDef::EmitDeclNode(DeclNode *node) {
  if (node == nullptr)
    return std::string();

  std::string str, varStr;
  TreeNode* idType = nullptr;
  //std::string str(Emitter::GetEnumDeclProp(node->GetProp()));

  // For func var of JS_Var and global vars, emit var name
  // For func var of JS_Let/JS_Const, emit both var type & name
  if (auto n = node->GetVar()) {
    if (mIsInit || node->GetProp() == JS_Var) {
      // handle declnode inside for-of/for-in (uses GetSet() and has null GetInit())
      if (!node->GetInit() && node->GetParent() && !node->GetParent()->IsForLoop())
        return std::string();
      varStr = EmitTreeNode(n);            // emit var name only
    } else {
      varStr = mCppDecl.EmitTreeNode(n);   // emit both var type and name
    }
    if (n->IsIdentifier()) {
      idType = static_cast<IdentifierNode*>(n)->GetType();
    }
  }
  if (auto n = node->GetInit()) {
    if (n->IsArrayLiteral())
      str += varStr + " = " + EmitArrayLiteral(idType, n);
    else if (n->IsStructLiteral())
      str += EmitObjPropInit(varStr, idType, static_cast<StructLiteralNode*>(n));
    else if (node->GetVar()->IsIdentifier() && n->IsIdentifier() && n->IsTypeIdClass())
      str += varStr + "= &"s + n->GetName() + "_ctor"s;           // init with ctor address
    else
      str += varStr + " = "s + EmitTreeNode(n);
  } else {
    str = varStr;
  }
  return str;
}

static bool QuoteStringLiteral(std::string &s) {
  if(s.front() != '"' || s.back() != '"')
    return false;
  s = s.substr(1, s.length() - 2);
  Emitter::Replace(s, "\"", "\\\"", 0);
  s = "\"" + s + "\"";
  return true;
}

std::string EmitSuperCtorCall(TreeNode* node) {
  while (node->GetKind() && !node->IsClass())
    node = node->GetParent();
  if (node && node->IsClass()) {
    std::string base, str;
    base = (static_cast<ClassNode*>(node)->GetSuperClassesNum() != 0)? static_cast<ClassNode*>(node)->GetSuperClass(0)->GetName() : "t2crt::Object";
    str = "  "s + base + "_ctor"s;
    return str;
  }
  return ""s;
}

std::string CppDef::EmitCallNode(CallNode *node) {
  if (node == nullptr)
    return std::string();
  bool log = false;
  bool isSuper = false;
  std::string str;
  if (auto n = node->GetMethod()) {
    if(n->IsFunction()) {
      str += static_cast<FunctionNode *>(n)->GetName();
    } else {
      auto s = EmitTreeNode(n);
      if(s.compare(0, 12, "console->log") == 0) {
        str += "std::cout"s;
        log = true;
      } else if (s.compare("super") == 0) {
        isSuper = true;
        str += EmitSuperCtorCall(node);
      }
      else {
        Emitter::Replace(s, ".", "->", 0);
        str += s;
      }
    }
  }
  if(!log)
    str += isSuper? "(obj"s : "("s;
  unsigned num = node->GetArgsNum();
  for (unsigned i = 0; i < num; ++i) {
    if(log) {
      std::string s = EmitTreeNode(node->GetArg(i));
      if(QuoteStringLiteral(s)) {
        //if(num > 1)
        //  s = "\"'\""s + s + "\"'\""s;
      } else if(mPrecedence <= 13) // '\015'
        s = "("s + s + ")"s;
      if (i)
        str += " << ' ' "s;
      str += " << "s + s;
    } else {
      if (i || isSuper)
        str += ", "s;
      if (auto n = node->GetArg(i))
        str += EmitTreeNode(n);
    }
  }
  if(!log)
    str += ")"s;
  else
    str += " << std::endl;";
  mPrecedence = '\024';
  return str;
}

std::string CppDef::EmitPrimTypeNode(PrimTypeNode *node) {
  return mCppDecl.EmitPrimTypeNode(node);
}

std::string CppDef::EmitPrimArrayTypeNode(PrimArrayTypeNode *node) {
  return std::string();
}

inline bool IsBracketNotationProp(TreeNode *node) {
  return node->IsArrayElement() &&
         static_cast<ArrayElementNode*>(node)->GetArray()->IsTypeIdClass();
}

std::string CppDef::EmitArrayElementNode(ArrayElementNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str;
  if (IsBracketNotationProp(node)) {
    bool unused;
    str = EmitBracketNotationProp(node, OPR_Arrow, false, unused);
    return str;
  }
  if (auto n = node->GetArray()) {
    std::string s = EmitTreeNode(n);
    if (mIsInit && s == "this")
      str = "__module"s;
    else
      str = "(*"s + s + ")"s;
    if(mPrecedence < '\024')
      str = "("s + str + ")"s;
  }


  for (unsigned i = 0; i < node->GetExprsNum(); ++i) {
    if (auto n = node->GetExprAtIndex(i)) {
      std::string expr;
      expr = "["s + EmitTreeNode(n) + "]"s;
      if (i < node->GetExprsNum()-1)
        str  = "(*"s + str + expr  + ")"s;
      else
        str += expr;
    }
  }

  mPrecedence = '\030';
  return HandleTreeNode(str, node);
}

std::string CppDef::EmitArrayLiteralNode(ArrayLiteralNode *node) {
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

std::string CppDef::EmitFieldNode(FieldNode *node) {
  if (node == nullptr)
    return std::string();
  std::string upper, field;
  auto upnode = node->GetUpper();
  if (upnode)
    upper = EmitTreeNode(upnode);
  if (auto n = node->GetField())
    field = EmitTreeNode(n);
  if (upper.empty() || field.empty()) // Error if either is empty
    return "%%%Empty%%%";
  if (field == "length") // for length property
    return upper + "->size()"s;
  if (mCppDecl.IsImportedModule(upper) || upnode->GetTypeId() == TY_Module) // for imported module
    return upper + "::"s + field;
  if (true) // TODO: needs to check if it accesses a Cxx class field
    return upper + "->"s + field;
  return "(*"s + upper + ")[\""s + field + "\"]"s;
}

std::string CppDef::EmitCondBranchNode(CondBranchNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str("if("s);
  if (auto n = node->GetCond()) {
    auto cond = EmitTreeNode(n);
    str += Clean(cond);
  }
  str += ")"s;
  if (auto n = node->GetTrueBranch()) {
    str += EmitTreeNode(n) + GetEnding(n);
  }
  if (auto n = node->GetFalseBranch()) {
    str += "else"s + EmitTreeNode(n) + GetEnding(n);
  }
  if(auto n = node->GetLabel()) {
    str += "__label_break_"s + EmitTreeNode(n) + ":;\n"s;
  }
  return str;
}

std::string CppDef::EmitBlockNode(BlockNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str("{\n");
  for (unsigned i = 0; i < node->GetChildrenNum(); ++i) {
    if (auto n = node->GetChildAtIndex(i)) {
      str += "  "s + EmitTreeNode(n) + GetEnding(n);
    }
  }
  str += "}\n"s;
  if(auto n = node->GetLabel()) {
    str += "__label_break_"s + EmitTreeNode(n) + ":;\n"s;
  }
  mPrecedence = '\030';
  return str;
}

std::string CppDef::EmitForLoopNode(ForLoopNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str;
  str += "for("s;
  switch(node->GetProp()) {
    case FLP_Regular:
      {
        for (unsigned i = 0; i < node->GetInitsNum(); ++i)
          if (auto n = node->GetInitAtIndex(i)) {
            if (i)
              str += ", "s;
            str += EmitTreeNode(n);
          }
        str += "; "s;
        if (auto n = node->GetCond()) {
          str += EmitTreeNode(n);
        }
        str += "; "s;
        for (unsigned i = 0; i < node->GetUpdatesNum(); ++i)
          if (auto n = node->GetUpdateAtIndex(i)) {
            if (i)
              str += ", "s;
            str += EmitTreeNode(n);
          }
        break;
      }
    case FLP_JSIn:
      {
        if (auto n = node->GetVariable()) {
          str += EmitTreeNode(n);
        }
        str += " in "s;
        if (auto n = node->GetSet()) {
          str += EmitTreeNode(n);
        }
        break;
      }
    case FLP_JSOf:
      {
        if (auto n = node->GetVariable()) {
          std::string s = EmitTreeNode(n);
          str += "auto "s + Clean(s);
        }
        str += " : "s;
        if (auto n = node->GetSet()) {
          str += EmitTreeNode(n);
          if (n->IsIdentifier() && static_cast<IdentifierNode*>(n)->IsTypeIdArray()) {
            str += "->elements"s;
          }
        }
        break;
      }
    case FLP_NA:
      return "FLP_NA"s;
    default:
      MASSERT(0 && "Unexpected enumerator");
  }
  str += ")"s;

  auto label = node->GetLabel();
  std::string lstr;
  if(label) {
    lstr = EmitTreeNode(label);
    str += "{\n"s;
  }
  if (auto n = node->GetBody()) {
    str += EmitTreeNode(n) + GetEnding(n);
  }
  if(label)
    str += "__label_cont_"s + lstr + ":;\n}\n"s + "__label_break_"s + lstr + ":;\n"s;
  return str;
}

std::string CppDef::EmitBreakNode(BreakNode *node) {
  if (node == nullptr)
    return std::string();
  auto target = node->GetTarget();
  std::string str = target ? "goto __label_break_"s + EmitTreeNode(target) : "break"s;
  return str;
}

std::string CppDef::EmitContinueNode(ContinueNode *node) {
  if (node == nullptr)
    return std::string();
  auto target = node->GetTarget();
  std::string str = target ? "goto __label_cont_"s + EmitTreeNode(target) : "continue"s;
  return str;
}

TypeId CppDef::GetTypeIdFromDecl(TreeNode* node) {
  TypeId typeId = TY_None;

  if (auto typ = FindDeclType(node)) {
    if (typ->IsPrimType())
      typeId = static_cast<PrimTypeNode*>(typ)->GetPrimType();
  }
  return typeId;
}

// Check if a bracket notation property is a class member field
bool CppDef::IsClassField(ArrayElementNode* node, std::string propKey) {
  if (!node->GetArray()->IsIdentifier()) {
    return false;
  }
  // find declared type of bracket notation obj; if class type
  // if class type, lookup class decl and check if prop is a class member fd
  if (auto typ = FindDeclType(node->GetArray()))
    if (typ->IsUserType() && static_cast<UserTypeNode*>(typ)->IsTypeIdClass())
      if (auto classId = static_cast<UserTypeNode*>(typ)->GetId())
        if (auto n = mHandler->FindDecl(static_cast<IdentifierNode*>(classId)))
          if (n->IsClass())
            for (unsigned i = 0; i < static_cast<ClassNode*>(n)->GetFieldsNum(); ++i) {
              auto fd = static_cast<ClassNode*>(n)->GetField(i);
              if (fd->IsIdentifier())
                // skip leading and trailing quote in propKey when comparing
                if (propKey.compare(1, propKey.length()-2, static_cast<IdentifierNode*>(fd)->GetName()) == 0)
                  return true;
            }
  return false;
}

//
// For property access using bracket notation (e.g. bar["prop"]):
//   1) If the property is a member field in the object's class, emit: bar->prop
//   2) Otherwise it is a property created dynamically at runtime:
//      - If it is a lvalue, emit: (*bar)["prop"] - [] operator overloaded in ts2cpp.h
//      - If it is a rvalue, emit: bar->GetPropXX("prop") - XX is one of union types in t2crt::JS_Val
//   3) For OP_Assign, if lvalue on lhs is dynamic prop, wrap rhs with t2crt::JS_Val() macro.
//      e.g.   (*bar)["p1"] = t2crt::JS_Val(0xa);
//             (*bar)["p2"] = t2crt::JS_Val(bar->f2);
//             (*bar)["p2"] = t2crt::JS_Val(bar->GetPropLong("p1"));
//             (*bar)["p2"] = t2crt::JS_Val((uint32_t)(bar->GetPropLong("p2") >> bar->GetPropLong("p1")));
//
// *note: to do 1), the property key must be a string literal. if the property key
//        is an identfier, then we have to do 2) because the identfier can be
//        a var or a TS symobol resolvable only at runtime.
//        Also, the object may be an expression, in which case, it can only be evaluated at runtime
//
std::string CppDef::EmitBracketNotationProp(ArrayElementNode* ae, OprId binOpId, bool isLhs, bool& isDynProp) {
  if (ae == nullptr)
    return std::string();

  isDynProp = true;
  std::string str, propKey;
  std::string objName;

  if (ae->GetArray()->IsIdentifier()) {
    objName = ae->GetArray()->GetName();
  } else {
    // case where the object is an expression - re: call-func.ts
    objName = EmitTreeNode(ae->GetArray());
  }

  TypeId propKeyType  = ae->GetExprAtIndex(0)->GetTypeId();
  if (propKeyType == TY_String && ae->GetExprAtIndex(0)->IsLiteral()) {
    propKey = EmitTreeNode(ae->GetExprAtIndex(0));
    if (IsClassField(ae, propKey)) {
      // property is class member field
      str = objName + "->"s + propKey.substr(1, propKey.length()-2);
      isDynProp = false;
      return str;
    }
  }
  if (propKeyType == TY_None) {
    propKeyType = GetTypeIdFromDecl(ae->GetExprAtIndex(0));
  }
  // resolve propKey at runtime
  switch (propKeyType) {
    case TY_Int:
      propKey = "t2crt::to_string("s + EmitTreeNode(ae->GetExprAtIndex(0)) + ")"s;
      break;
    case TY_String:
      propKey = EmitTreeNode(ae->GetExprAtIndex(0));
      break;
    case TY_Symbol:
      propKey = "t2crt::to_string("s + EmitTreeNode(ae->GetExprAtIndex(0)) + ")"s;
      break;
    default:
      MASSERT(0 && "Encounter unsupported prop key type in bracket notation");
      break;
  }

  if (binOpId == OPR_Assign && isLhs) {
    // prop is lvalue
    str = "(*"s + objName + ")["s + propKey + "]"s;
  } else {
    switch(ae->GetTypeId()) {
      case TY_Long:
      case TY_Int:
        str = objName + "->GetPropLong("s + propKey + ")"s;
        break;
      case TY_Double:
        str = objName + "->GetPropDouble("s + propKey + ")"s;
        break;
      case TY_String:
        str = objName + "->GetPropString("s + propKey + ")"s;
        break;
      case TY_Boolean:
        str = objName + "->GetPropBool("s + propKey + ")"s;
        break;
      case TY_Function:
      case TY_Object:
        str = objName + "->GetPropObj("s + propKey + ")"s;
        break;
      default:
        str = "(*"s + objName + ")["s + propKey + ']';
    }

    // prop is rvalue
    // emit: bar->GetPropXX("prop")
    // Need type info for each property
  }
  return str;
}

// return true if identifier is a class
bool CppDef::IsClassId(TreeNode* node) {
  if (node != nullptr &&
      node->IsIdentifier() &&
      node->IsTypeIdClass() &&
      mHandler->FindDecl(static_cast<IdentifierNode*>(node)) &&
      mHandler->FindDecl(static_cast<IdentifierNode*>(node))->IsClass())
    return true;
  else
    return false;
}

std::string CppDef::EmitBinOperatorNode(BinOperatorNode *node) {
  if (node == nullptr)
    return std::string();
  const char *op = Emitter::GetEnumOprId(node->GetOprId());
  const Precedence precd = *op & 0x3f;
  const bool rl_assoc = *op >> 6; // false: left-to-right, true: right-to-left
  std::string lhs, rhs;
  bool lhsIsDynProp = false;
  bool rhsIsDynProp = false;

  if (auto n = node->GetOpndA()) {
    if (IsBracketNotationProp(n)) {
      lhs = EmitBracketNotationProp(static_cast<ArrayElementNode*>(n), node->GetOprId(), true, lhsIsDynProp);
    } else {
      lhs = EmitTreeNode(n);
      if (n->IsIdentifier() && n->IsTypeIdArray())
        lhs = "*"s + lhs;
    }
    if(precd > mPrecedence || (precd == mPrecedence && rl_assoc))
      lhs = "("s + lhs + ")"s;
  }
  else
    lhs = "(NIL) "s;

  if (auto n = node->GetOpndB()) {
    if (IsBracketNotationProp(n)) {
      rhs = EmitBracketNotationProp(static_cast<ArrayElementNode*>(n), node->GetOprId(), false, rhsIsDynProp);
    } else if (IsClassId(n)) {
      rhs = "&"s + n->GetName() + "_ctor"s;
    } else
      rhs = EmitTreeNode(n);
    if(precd > mPrecedence || (precd == mPrecedence && !rl_assoc))
      rhs = "("s + rhs + ")"s;
  }
  else
    rhs = " (NIL)"s;

  OprId k = node->GetOprId();
  std::string str;
  switch(k) {
    case OPR_Exp:
      str = "std::pow("s + lhs + ", "s + rhs + ")";
      break;
    case OPR_StEq:
      str = "t2crt::StrictEqu("s + lhs + ',' + rhs + ')';
      break;
    case OPR_StNe:
      str = "t2crt::StrictNotEqu("s + lhs + ',' + rhs + ')';
      break;
    default:
      switch(k) {
        case OPR_Band:
        case OPR_Bor:
        case OPR_Bxor:
        case OPR_Shl:
        case OPR_Shr:
          lhs = "static_cast<int64_t>(static_cast<int32_t>("s + lhs + "))"s;
          break;
        case OPR_Zext:
          lhs = "static_cast<int64_t>(static_cast<uint32_t>("s + lhs + "))"s;
          op = "\015>>";
          break;
      }
      if (k == OPR_Assign && lhsIsDynProp)
        rhs = "t2crt::JS_Val("s + rhs + ")"s;
      str = lhs + " "s + std::string(op + 1) + " "s + rhs;
  }
  mPrecedence = precd;
  return str;
}

std::string CppDef::EmitUnaOperatorNode(UnaOperatorNode *node) {
  if (node == nullptr)
    return std::string();
  bool isPost = node->IsPost();
  const char *op = Emitter::GetEnumOprId(node->GetOprId());
  const Precedence precd = *op & 0x3f;
  const bool rl_assoc = *op >> 6; // false: left-to-right, true: right-to-left
  std::string opr;
  if (auto n = node->GetOpnd()) {
    opr = EmitTreeNode(n);
    if(precd > mPrecedence || (precd == mPrecedence && (rl_assoc && isPost || !rl_assoc && !isPost)))
      opr = "("s + opr + ")"s;
  }
  else
      opr = "(NIL)"s;
  if(node->GetOprId() == OPR_Bcomp)
    opr = "static_cast<int32_t>("s + opr + ")"s;
  std::string str;
  if(node->IsPost())
    str = opr + std::string(op + 1) + " "s;
  else
    str = " "s + std::string(op + 1) + opr;
  mPrecedence = precd;
  return str;
}

std::string CppDef::EmitTemplateLiteralNode(TemplateLiteralNode *node) {
  if (node == nullptr)
    return std::string();
  auto num = node->GetTreesNum();
  std::string str;
  for (unsigned i = 0; i < num; ++i) {
    if (auto n = node->GetTreeAtIndex(i)) {
      if (!std::empty(str))
        str += " + "s;
      std::string s(EmitTreeNode(n));
      if(i & 0x1)
        str += "t2crt::to_string("s + s+ ")"s;
      else {
        QuoteStringLiteral(s);
        str += s;
      }
    }
  }
  mPrecedence = '\016';
  return str;
}

std::string CppDef::EmitLiteralNode(LiteralNode *node) {
  if (node == nullptr)
    return std::string();
  LitData lit = node->GetData();
  if(lit.mType == LT_VoidLiteral)
    return "undefined";
  std::string str = Emitter::EmitLiteralNode(node);
  if (node->GetTypeId() == TY_Int)
    str = "(int)"s + str;
  return str;
}

std::string CppDef::EmitSwitchNode(SwitchNode *node) {
  if (node == nullptr)
    return std::string();
  bool doable = true;
  for (unsigned i = 0; i < node->GetCasesNum(); ++i)
    if (SwitchCaseNode* c = node->GetCaseAtIndex(i))
      for (unsigned j = 0; j < c->GetLabelsNum(); ++j) {
        auto l = c->GetLabelAtIndex(j);
        if (l && l->IsSwitchLabel()) {
          auto ln = static_cast<SwitchLabelNode*>(l);
          if (auto v = ln->GetValue())
            if(!v->IsLiteral() || !v->IsTypeIdInt()) {
              doable = false;
              goto out_of_loops;
            }
        }
      }
out_of_loops:
  std::string label;
  TreeNode* lab = node->GetLabel();
  if(lab)
    label = "__label_break_"s + EmitTreeNode(lab);
  else
    label = "__label_switch_" + std::to_string(node->GetNodeId());
  std::string str;
  if(doable) {
    str = "switch("s;
    if (TreeNode* n = node->GetExpr()) {
      std::string expr = EmitTreeNode(n);
      str += Clean(expr);
    }
    str += "){\n"s;
    for (unsigned i = 0; i < node->GetCasesNum(); ++i) {
      if(SwitchCaseNode* n = node->GetCaseAtIndex(i))
        str += EmitTreeNode(n);
    }
    str += "}\n"s;
  } else {
    std::string tmp = "__tmp_"s + std::to_string(node->GetNodeId());
    str = "do { // switch\nauto "s + tmp + " = "s;
    if (TreeNode* n = node->GetExpr()) {
      std::string expr = EmitTreeNode(n);
      str += Clean(expr);
    }
    str += ";\n"s;
    std::string body;
    std::string other = "goto "s + label + ";\n"s;;
    for (unsigned i = 0; i < node->GetCasesNum(); ++i)
      if (SwitchCaseNode* cn = node->GetCaseAtIndex(i)) {
        for (unsigned j = 0; j < cn->GetLabelsNum(); ++j)
          if (SwitchLabelNode* ln = cn->GetLabelAtIndex(j)) {
            if(ln->IsDefault())
              other = "goto __case_"s + std::to_string(cn->GetNodeId()) + ";\n"s;
            else {
              std::string le = EmitTreeNode(ln->GetValue());
              str += "if("s + tmp + " == ("s + Clean(le)
                + "))\ngoto __case_"s + std::to_string(cn->GetNodeId()) + ";\n"s;
            }
          }
        body += "__case_"s + std::to_string(cn->GetNodeId()) + ":\n"s;
        for (unsigned s = 0; s < cn->GetStmtsNum(); ++s)
          if (TreeNode* t = cn->GetStmtAtIndex(s))
            body += EmitTreeNode(t) + ";\n"s;
      }
    str += other + body;
    str += "} while(0);\n"s;
  }
  if(!doable || lab)
    str += label + ":;\n"s;
  return str;
}

std::string CppDef::EmitTypeOfNode(TypeOfNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str("t2crt::__js_typeof("s), rhs;
  if (auto n = node->GetExpr())
    rhs = EmitTreeNode(n);
  str += rhs + ")"s;
  return HandleTreeNode(str, node);
}

std::string CppDef::EmitNewNode(NewNode *node) {
  if (node == nullptr)
    return std::string();

  std::string str;
  MASSERT(node->GetId() && "No mId on NewNode");
  if (node->GetId() && node->GetId()->IsTypeIdClass()) {
    // Generate code to create new obj and call constructor
    std::string clsName = EmitTreeNode(node->GetId());
    if (IsBuiltinObj(clsName))
      clsName = "t2crt::"s + clsName;
    str = clsName + "_ctor("s + clsName + "_ctor._new()"s;
  } else {
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
  if (auto n = node->GetBody()) {
    str += " "s + EmitBlockNode(n);
  }
  mPrecedence = '\024';
  return HandleTreeNode(str, node);
}

static std::string MethodString(std::string &func) {
  size_t s = func.substr(0, 9) == "function " ? 9 : 0;
  return func.back() == '}' ? func.substr(s) + "\n"s : func.substr(s) + ";\n"s;
}

std::string CppDef::EmitStructNode(StructNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str;
  const char *suffix = ";\n";
  switch(node->GetProp()) {
    case SProp_CStruct:
      str = "struct "s;
      break;
    case SProp_TSInterface:
      return std::string(); // decl already generation by CppDecl
    case SProp_TSEnum: {
      // Create the enum type object
      std::string enumClsName;
      if (auto n = node->GetStructId()) {
        enumClsName = "Enum_"s + n->GetName();
        str += enumClsName + "* "s + n->GetName() + " = new "s + enumClsName + "();\n"s;
        return str;
      }
      break;
    }
    case SProp_NA:
      str = ""s;
      return str; // todo: handle anonymous struct created for untyped object literals.
      break;
    default:
      MASSERT(0 && "Unexpected enumerator");
  }

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

  if (auto n = node->GetNumIndexSig()) {
    str += EmitNumIndexSigNode(n) + "\n"s;;
  }
  if (auto n = node->GetStrIndexSig()) {
    str += EmitStrIndexSigNode(n) + "\n";
  }

  for (unsigned i = 0; i < node->GetMethodsNum(); ++i) {
    if (auto n = node->GetMethod(i)) {
      std::string func = EmitFunctionNode(n);
      func = Clean(func);
      str += MethodString(func);
    }
  }

  str += "}\n"s;
  return HandleTreeNode(str, node);
}

std::string CppDef::EmitTypeAliasNode(TypeAliasNode *node) {
  return std::string();
}

// Return the declared type for an identifier
TreeNode* CppDef::FindDeclType(TreeNode* node) {
  if (node == nullptr || !node->IsIdentifier())
    return nullptr;

  if (auto n = mHandler->FindDecl(static_cast<IdentifierNode*>(node)))
    if (n->IsDecl())
      if (auto var = static_cast<DeclNode*>(n)->GetVar())
        if (var->IsIdentifier())
          if (auto type = static_cast<IdentifierNode*>(var)->GetType())
            return type;

  return nullptr;
}

// Get template argument for calling InstanceOf template func.
std::string CppDef::GetTypeForTemplateArg(TreeNode* node) {
  if (node == nullptr)
    return std::string();

  std::string str;
  if (auto n = FindDeclType(node)) {
    // lhs type of instanceof operator is either object or ANY
    switch(n->GetKind()) {
      case NK_UserType:
        str = EmitTreeNode(n);
        break;
      case NK_PrimType:
        str = EmitTreeNode(n);
        if (str.find("t2crt::JS_Val") != std::string::npos)
          str = "t2crt::JS_Val";
        break;
      default:
        MASSERT(0 && "Unexpected node type");
    }
  } else if (node->IsField()) {
    // Lookup declared type of the obj in mUpper, then find
    // mField from the obj field list to get the field type.
    //
    // The result is used as template argument for the InstanceOf
    // template func. However, without this information
    // the compiler does template argument deduction
    // to call the template func, and has work ok for testcases.
    // So implementation of this part is deferred until needed.
  } else {
    // No info - return ANY for now...
    str = "t2crt::JS_Val"s;
    //MASSERT(0 && "Unexpected node type");
  }
  return str;
}

std::string CppDef::EmitInstanceOfNode(InstanceOfNode *node) {
  if (node == nullptr)
    return std::string();
  const Precedence precd = '\014';
  const bool rl_assoc = false;         // false: left-to-right
  std::string lhs, rhs, typ;
  if (auto n = node->GetLeft()) {
    lhs = EmitTreeNode(n);
    typ = GetTypeForTemplateArg(n);
    if (typ.compare("t2crt::JS_Val") == 0) {
      lhs = "t2crt::JS_Val("s + lhs + ")"s;
      typ = "";
    }
    else if (!typ.empty())
      typ = "<"s + typ + ">"s;        // InstanceOf<typ>

    if(precd > mPrecedence)
      lhs = '(' + lhs + ')';
  }
  else
    lhs = "(NIL) "s;

  if (auto n = node->GetRight()) {
    if (IsClassId(n) || IsBuiltinObj(n->GetName()))
      rhs = "&"s + n->GetName() + "_ctor"s;
    else
      rhs = EmitTreeNode(n);
    if(precd > mPrecedence || (precd == mPrecedence && !rl_assoc))
      rhs = '(' + rhs + ')';
  }
  else
    rhs = " (NIL)"s;

  std::string str("t2crt::InstanceOf"s + typ + "("s + lhs + ", "s + rhs + ")"s);
  mPrecedence = precd;
  return HandleTreeNode(str, node);
}

std::string CppDef::EmitDeclareNode(DeclareNode *node) {
  return std::string();
}

std::string CppDef::EmitAsTypeNode(AsTypeNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str;
  if (auto n = node->GetType())
    str = EmitTreeNode(n);
  if (!str.empty())
    str = '(' + str + ')';
  return str;
}

std::string &CppDef::HandleTreeNode(std::string &str, TreeNode *node) {
  auto num = node->GetAsTypesNum();
  if(num > 0) {
    std::string as;
    for (unsigned i = 0; i < num; ++i)
      if (auto t = node->GetAsTypeAtIndex(i))
        as = EmitAsTypeNode(t) + as;
    str = as + '(' + str + ')';
  }
  /*
  if(node->IsOptional())
    str = AddParentheses(str, node) + '?';
  if(node->IsNonNull())
    str = AddParentheses(str, node) + '!';
  */
  if(node->IsRest())
    str = "..."s; // + AddParentheses(str, node);
  /*
  if(node->IsConst())
    if(node->IsField())
      str += " as const"s;
    else
      str = AddParentheses(str, node) + " as const"s;
  */
  return str;
}

} // namespace maplefe
