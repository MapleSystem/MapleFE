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
#include "helper.h"

namespace maplefe {

std::unordered_map<TypeId, std::string>TypeIdToJSTypeCXX = {
  // AST TypeId to t2crt JS_Type mapping for JS_Val type of obj props that pts to CXX class fields 
  {TY_Object,  "t2crt::TY_CXX_Object"},
  {TY_Function,"t2crt::TY_CXX_Function"},
  {TY_Boolean, "t2crt::TY_CXX_Bool"},
  {TY_Int,     "t2crt::TY_CXX_Long"},
  {TY_String,  "t2crt::TY_CXX_String"},
  {TY_Number,  "t2crt::TY_CXX_Double"},
  {TY_Double,  "t2crt::TY_CXX_Double"},
  {TY_Array,   "t2crt::TY_CXX_Array"},
  {TY_Class,   "t2crt::TY_CXX_Object"},
  {TY_Any,     "t2crt::TY_CXX_Any"},
};

FuncTable hFuncTable;

// Used to build GetProp<xxx> for calls to get Object (class Object in ts2cpp.h) property
std::string hlpGetJSValTypeStr(TypeId typeId) {
  switch(typeId) {
    case TY_Object:
    case TY_Class:
    case TY_Any:
      return "Obj";
    case TY_Function:
      return "Func";
    case TY_Boolean:
      return "bool";
    case TY_Number:
      return "Double";
    case TY_String:
      return "Str";
  }
  return std::string();
}

// Get TypeId info for a treenode.
TypeId hlpGetTypeId(TreeNode* node) {
  // lookup typetable
  if (node->GetTypeIdx()) {
    TypeEntry* te = gTypeTable.GetTypeEntryFromTypeIdx(node->GetTypeIdx());
    if (te) 
      return te->GetTypeId();
  }

  // lookup ast node mTypeId (will be deprecated)
  if (node->GetTypeId() != TY_None) {
    return node->GetTypeId();
  }
  return TY_Any;
}

// Temp. workaround to get func info for var of type function. TODO: replace with TI API when avail.
std::string GetClassOfAssignedFunc(TreeNode* node) {
  if (node->IsIdentifier() &&
      node->GetParent() &&
      node->GetParent()->IsDecl() &&
      static_cast<DeclNode*>(node->GetParent())->GetInit() &&
      static_cast<DeclNode*>(node->GetParent())->GetInit()->IsFunction()) {
    auto n = static_cast<DeclNode*>(node->GetParent())->GetInit();
    return("Cls_"s + static_cast<FunctionNode*>(n)->GetName());
  }
  return std::string();
}

// Generate call to create obj prop with ptr to c++ class fld member
// e.g. obj->AddProp("fdLong", t2crt::ClassFld<long   Foo::*>(&Foo::fdLong).NewProp(this, t2crt::TY_CXX_Long))
//      obj->AddProp("fdAny",  t2crt::ClassFld<JS_Val Foo::*>(&Foo::fdAny).NewProp(this,  t2crt::TY_CXX_Any))
std::string GenClassFldAddProp(std::string objName,
                               std::string clsName,
                               std::string fldName,
                               std::string fldCType,
                               std::string fldJSType) {
  std::string str;
  str = objName + "->AddProp(\"" + fldName + "\", t2crt::ClassFld<" + 
    fldCType + " " + clsName + "::*>(&" +
    clsName + "::" + fldName + ").NewProp(this, " + fldJSType + "))";
  return str;
}

// Each first level function is instantiated from a corresponding class generated with interfaces below:
//   Body  - user defined function code
//   ()    - functor for OrdinaryCallEvaluteBody [9.2.1.3]
//   Ctor  - Call user defined code as constructor (with object from new() op)
//   Call  - Call user defined code with designated 'this'
//   Apply - Call user defined code with designated 'this' and array of argument
//   Bind  - Create and return function object binded to designated this and optional args
// note: the parameter args is expected to be a string that start with "_this"
//       TODO: apply and bind may be moved to ts2cpp.h Fuction class as virtual
// note: TSC prohibits calling non-void constructor func with new(), so in the code generated below
//       for ctor(), it calls _body() but ignores return val from _body(), and instead returns _this
//       per TS/JS spec.

std::string GenFuncClass(std::string retType, std::string funcName, std::string params, std::string args) {
  std::string str;
  std::string clsName = "Cls_" + funcName;
  std::string functorArgs = args;
  std::string functorParams = params;
  std::string thisType;
  functorArgs.replace(0, 5, "_thisArg"); // replace _this with _thisArg
  size_t pos;
  if ((pos = functorParams.find("_this, ")) != std::string::npos)
    functorParams.erase(0, pos+7);
  else if ((pos = functorParams.find("_this")) != std::string::npos)
    functorParams.erase(0, pos+5);
  thisType = params.substr(0, pos-1);

  str = R"""(
class )""" + clsName + R"""( : public t2crt::Function {
  public:
    )""" + clsName + R"""(() : t2crt::Function(&t2crt::Function_ctor,t2crt::Function_ctor.prototype,t2crt::Object_ctor.prototype) {}
    ~)""" + clsName + R"""(() {}

    )""" + retType + R"""(  _body()""" + params + R"""();
    )""" + retType + R"""(  operator()()""" + functorParams + R"""() { return _body(()""" + thisType + R"""())""" + functorArgs + R"""(); }
    t2crt::Object*  ctor ()""" + params + R"""() { _body()""" + args + R"""(); return(_this); }
    )""" + retType + R"""(  call ()""" + params + R"""() { return _body()""" + args + R"""( ); }
    )""" + retType + R"""(  apply(t2crt::Object* _this, t2crt::ArgsT& args) { /* TODO: call _body wtih flatten args  */ }
    )""" + clsName + R"""(* bind (t2crt::Object* _this, t2crt::ArgsT* args) {
      )""" + clsName + R"""(* func = new )""" + clsName + R"""(();
      func->_thisArg = _this;
      func->_args = args;
      return(func);
    }
    virtual const char* __GetClassName() const {return ")""" + funcName + R"""( ";}
};

)""";
  return str;
}

bool IsClassMethod(TreeNode* node) {
  return(node->IsFunction() && node->GetParent() && node->GetParent()->IsClass());
}

std::string tab(int n) {
  return std::string(n*2,' ');
}

std::string GenAnonFuncName(TreeNode* node) {
  return "_anon_func_"s + std::to_string(node->GetNodeId());
}

// Check 1st param of top level function for "this" and do substitution.
void HandleThisParam(unsigned nParams, TreeNode* node, std::string& params, std::string&args) {
  if (nParams == 0) {
    // ts2cpp's C++ mapping for TS func has a "this" obj in the c++ func param list
    // which will be generated from AST if "this" is declared as a TS func parameter
    // as required by TS strict mode. However TS funcs that do not reference 'this'
    // are not required to declare it, so emitter has to check and insert one.
    params = "t2crt::Object* _this"s;
    args = "_this"s;
    return;
  }

  if (node->IsThis()) {
    args = "_this";
    Emitter::Replace(params, "this", "_this"); // change this to _this to avoid c++ keyword
    Emitter::Replace(params, "t2crt::JS_Val", "t2crt::Object*"); // change type any (JS_Val) to Object* per ts2cpp func mapping to C++ interface
  } else {
    // if 1st func param is not "this", insert one to work with c++ mapping for TS func
    args = "_this, "s + args;
    params = "t2crt::Object* _this, "s + params;
  }
}

} // namespace maplefe
