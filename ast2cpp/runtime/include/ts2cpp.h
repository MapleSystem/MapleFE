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

#ifndef __TS2CPP_RT_HEADER__
#define __TS2CPP_RT_HEADER__

#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <cmath>

namespace t2crt {

using std::to_string;
inline std::string to_string(std::string t) {return t;}

class BaseObj;

// JS types for props
typedef enum JS_Type : uint8_t {
  TY_None,      // Placeholder for non-existing property
  TY_Undef,     // "undefined"
  TY_Null,      // "object"
  TY_Bool,      // "boolean"
  TY_Long,      // "number"
  TY_Double,    // "number"
  TY_BigInt,    // "bigint"
  TY_String,    // "string"
  TY_Symbol,    // "symbol"
  TY_Function,  // "function"
  TY_Object,    // "object"
  TY_LAST,
} JS_Type;

struct JS_Val {
  union {
    void*        field;      // used by compiler genereted fields only
    bool         val_bool;
    int64_t      val_long;
    double       val_double;
    void*        val_bigint;
    std::string* val_string; // JS string primitive (not JS String object)
    BaseObj*     val_obj;    // for function, object (incl. String objects)
  } x;
  JS_Type type;
  bool    cxx;  // if it is a cxx field

  JS_Val() { x.val_long = 0l; type = TY_Undef; cxx = false; }
  JS_Val(int64_t l, JS_Type t, bool c) { x.val_long = l; type = t; cxx = c; }
  JS_Val(bool b)    { x.val_bool = b; type = TY_Bool; cxx = false; }
  JS_Val(int64_t l) { x.val_long = l; type = TY_Long; cxx = false; }
  JS_Val(double d)  { x.val_double = d; type = TY_Double; cxx = false; }
};

typedef struct JS_Prop {
  JS_Val  val;

  // Prop directly generated as class fields when TS is compiled into CPP
  JS_Prop(JS_Type jstype, void* field) {
    val = { (int64_t)field, jstype, true };
  }

  // Prop created at runtime
  JS_Prop(JS_Type jstype, bool v) {
    val = { (int64_t)v, jstype, false };
  }

  bool IsCxxProp() { return val.cxx; }

} JS_Prop;


typedef std::unordered_map<std::string, JS_Prop*> JS_PropList;

class BaseObj {
  public:
    JS_PropList propList;
    BaseObj* __proto__;    // link to prototype chain of object
  public:
    bool hasOwnProp(std::string key) {
      JS_PropList::iterator it;
      it = propList.find(key);
      return (it != propList.end());
    }
};


// JavaScript class/function constructor
class CtorObj : public BaseObj {
  public:
    BaseObj* prototype;    // prototype property of constructor
};

template <class T>
class ClassFld {
  // convert between class member offset and void ptr
  typedef union {
    void* addr;
    T     offset;
  } FldAddr;

  public:
    FldAddr field;
  public:
    ClassFld(void* addr) {field.addr = addr;}
    ClassFld(T offset)   {field.offset = offset;}
    void* addr()         {return field.addr;}
    T     offset()       {return field.offset;}
    JS_Prop* newProp(JS_Type type) {return new JS_Prop(type, field.addr);}
};

extern CtorObj* Function;
extern CtorObj* Object;

} // namespace t2crt

using namespace std::string_literals;

template <typename T>
std::ostream& operator<< (std::ostream& out, const std::vector<T>& v) {
  if(v.empty())
    out << "[]";
  else {
    out << "[ ";
    auto i = v.begin(), e = v.end();
    out << *i++;
    for (; i != e; ++i)
        std::cout << ", " << *i;
    out << " ]";
  }
  return out;
}

std::ostream& operator<< (std::ostream& out, const t2crt::JS_Val& v) {
  switch(v.type) {
    case t2crt::TY_None: out << "None"; break;
    case t2crt::TY_Undef: out << "undefined"; break;
    case t2crt::TY_Null: out << "null"; break;
    case t2crt::TY_Bool: out << v.x.val_bool; break;
    case t2crt::TY_Long: out << v.x.val_long; break;
    case t2crt::TY_Double: out << v.x.val_double; break;
    case t2crt::TY_BigInt: out << "bigint"; break;
    case t2crt::TY_String: out << "string"; break;
    case t2crt::TY_Symbol: out << "symbol"; break;
    case t2crt::TY_Function: out << "function"; break;
    case t2crt::TY_Object: out << "object"; break;
  }
  return out;
}

const t2crt::JS_Val undefined = { 0, t2crt::TY_Undef, false };

#endif
