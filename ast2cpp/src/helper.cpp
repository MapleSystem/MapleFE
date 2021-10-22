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

// Generate call to create obj prop with ptr to c++ class fld member
// e.g. obj->AddProp("fdLong", t2crt::ClassFld<long   Foo::*>(&Foo::fdLong).NewProp(this, t2crt::TY_CXX_Long))
//      obj->AddProp("fdAny",  t2crt::ClassFld<JS_Val Foo::*>(&Foo::fdAny).NewProp(this,  t2crt::TY_CXX_Any))
std::string hlpClassFldAddProp(std::string objName,
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

std::string indent(int n) {
  return std::string(n*2,' ');
}

} // namespace maplefe
