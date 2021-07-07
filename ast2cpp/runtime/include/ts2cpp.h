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

using namespace std::string_literals;

namespace t2crt {

using std::to_string;

inline std::string to_string(std::string t) {return t;}

class BaseObj;
class Ctor;
class Ctor_Function;
class Ctor_Object;

extern BaseObj Object_prototype;
extern BaseObj Function_prototype;
extern Ctor_Function Function_ctor;
extern Ctor_Object   Object_ctor;

// JS types for props
typedef enum JS_Type : uint8_t {
  TY_None = 0,  // Placeholder for non-existing property
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
  JS_Val(BaseObj* o){ x.val_obj = o; type = TY_Object; cxx = false; }

#define OPERATORS(op) \
  JS_Val operator op(const JS_Val &v) { \
    JS_Val res; \
    if(type == v.type) \
      switch(type) { \
        case TY_Long:   return { x.val_long op v.x.val_long }; \
        case TY_Double: return { x.val_double op v.x.val_double }; \
      } \
    else { \
      if(type == TY_Long && v.type == TY_Double) \
        return { (double)x.val_long op v.x.val_double }; \
      if(type == TY_Double && v.type == TY_Long) \
        return { x.val_double op (double)v.x.val_long }; \
    } \
    return res; \
  }

  OPERATORS(+)
  OPERATORS(-)
  OPERATORS(*)

};

typedef struct JS_Prop {
  JS_Val  val;

  // Prop directly generated as class fields when TS is compiled into CPP
  JS_Prop(JS_Type jstype, void* field) { val = { (int64_t)field, jstype, true }; }

  // Prop created at runtime
  JS_Prop(JS_Type jstype, bool v) { val = { (int64_t)v, jstype, false }; }
  JS_Prop(JS_Val v) { val = v; }
  JS_Prop() { val = { 0, t2crt::TY_Undef, false }; }

  bool IsCxxProp() { return val.cxx; }

} JS_Prop;


typedef std::unordered_map<std::string, JS_Prop> JS_PropList;

class BaseObj {
  public:
    JS_PropList propList;
    BaseObj* _proto;    // prototype chain
    Ctor*    _ctor;     // constructor of object
  public:
    BaseObj(): _proto(nullptr) {}
    BaseObj(Ctor* ctor, BaseObj* proto): _ctor(ctor), _proto(proto) {}

    bool HasOwnProp(std::string key) {
      JS_PropList::iterator it;
      it = propList.find(key);
      return (it != propList.end());
    }

    void AddProp(std::string key, JS_Val val) {
      propList[key] = { val };
    }

    bool IsFuncObj() {
      return (this->_ctor == reinterpret_cast<Ctor *>(&Function_ctor));
    }
};


// JavaScript class/function constructor
class Ctor : public BaseObj {
  public:
    BaseObj* _prototype;    // prototype property
  public:
    Ctor(Ctor* ctor, BaseObj* proto, BaseObj* prototype) {
      JS_Val val(this);
      _ctor = ctor;
      _proto = proto;
      _prototype = prototype;
      _prototype->AddProp("constructor", val);
    }
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
    void* Addr()         {return field.addr;}
    T     Offset()       {return field.offset;}
    JS_Prop* NewProp(JS_Type type) {return new JS_Prop(type, field.addr);}
};


// For JS builtins
class Ctor_Function : public Ctor {
  public:
    Ctor_Function(Ctor* ctor, BaseObj* proto, BaseObj* prototype) : Ctor(ctor, proto, prototype) {}
};
class Ctor_Object   : public Ctor {
  public:
    Ctor_Object(Ctor* ctor, BaseObj* proto, BaseObj* prototype) : Ctor(ctor, proto, prototype) {}
};
class Function : public BaseObj {};


template <typename T> std::string __js_typeof(T v) {
  if (std::numeric_limits<T>::is_signed)
    return "number"s;
  if (std::numeric_limits<T>::is_integer)
    return "boolean"s;
  return "unknown"s;
}

template <> inline std::string __js_typeof<std::string>(std::string v) {
  return "string"s;
}

template <> inline std::string __js_typeof<t2crt::JS_Val>(t2crt::JS_Val v) {
  static std::string names[t2crt::TY_LAST] = {
    [t2crt::TY_None]     =  "none"s,
    [t2crt::TY_Undef]    = "undefined"s,
    [t2crt::TY_Null]     = "object"s,
    [t2crt::TY_Bool]     = "boolean"s,
    [t2crt::TY_Long]     = "number"s,
    [t2crt::TY_Double]   = "number"s,
    [t2crt::TY_BigInt]   = "bigint"s,
    [t2crt::TY_String]   = "string"s,
    [t2crt::TY_Symbol]   = "symbol"s,
    [t2crt::TY_Function] = "function"s,
    [t2crt::TY_Object]   = "object"s,
  };
  return names[v.type];
}

} // namespace t2crt

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

extern std::ostream& operator<< (std::ostream& out, const t2crt::JS_Val& v);
extern const t2crt::JS_Val undefined;

#endif
