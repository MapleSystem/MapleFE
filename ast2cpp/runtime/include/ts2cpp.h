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
#include <cstdarg>
#include <cctype>

using namespace std::string_literals;

namespace t2crt {

using std::to_string;

inline std::string to_string(std::string t) {return t;}

class Object;
class Function;
template <typename T>
class Array;

// JS types for props
#define TY_CXX 0x20
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
  TY_Array,
  TY_LAST,
  TY_CXX_Undef = TY_Undef | TY_CXX,
  TY_CXX_Null,
  TY_CXX_Bool,
  TY_CXX_Long,
  TY_CXX_Double,
  TY_CXX_BigInt,
  TY_CXX_String,
  TY_CXX_Symbol,
  TY_CXX_Function,
  TY_CXX_Object,
  TY_CXX_Array,
  TY_CXX_Any,   //indicate JS_Val::x.field pointing to a JS_Val
  TY_CXX_LAST,
} JS_Type;

struct JS_Val {
  union {
    void*        field;      // used by compiler genereted fields only
    bool         val_bool;
    int64_t      val_long;
    double       val_double;
    void*        val_bigint;
    std::string* val_string; // JS string primitive (not JS String object)
    Object*      val_obj;    // for function, object (incl. String objects)
    Function*    val_func;   // for function
  } x;
  JS_Type type;

  bool IsCxxProp() { return type & TY_CXX; } // true if a cxx field

  JS_Val() { x.val_long = 0l; type = TY_Undef; }
  JS_Val(int64_t l, JS_Type t, bool c) { x.val_long = l; type = t; }
  JS_Val(bool b)    { x.val_bool = b; type = TY_Bool; }
  JS_Val(int64_t l) { x.val_long = l; type = TY_Long; }
  JS_Val(double d)  { x.val_double = d; type = TY_Double; }
  JS_Val(Object* o){ x.val_obj = o; type = TY_Object; }
  JS_Val(Function* o){ x.val_func = o; type = TY_Function; }
  JS_Val(std::string* s) { x.val_string = s; type = TY_String; }
  JS_Val(std::string s) { x.val_string = new std::string(s); type = TY_String; }
  JS_Val(const char* s) { x.val_string = new std::string(s); type = TY_String; }
  JS_Val(int i) { x.val_long = i; type = TY_Long; }
  JS_Val(JS_Type jstype, bool v) { x.val_long = (int64_t)v; type = jstype; }

  // Prop directly generated as class fields when TS is compiled into CPP
  JS_Val(JS_Type jstype, void* field) { x.field = field; type = static_cast<JS_Type>(jstype|TY_CXX); }

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

  // Handle assigning value to a JS_Val obj
#define ASSIGN_OPR(ctype, jstype, jscxxtype) \
    JS_Val& operator=(ctype val)  { \
      if (IsCxxProp()) {            \
        type = jscxxtype;           \
        *(ctype *)x.field = val;    \
      } else {                      \
        type = jstype;              \
        x.val_##ctype = val;        \
      }                             \
      return *this;                 \
    }

  ASSIGN_OPR(double, TY_Double, TY_CXX_Double)
  ASSIGN_OPR(long, TY_Long, TY_CXX_Long)
  ASSIGN_OPR(bool, TY_Bool, TY_CXX_Bool)

  // assign Object ptr to JS_Val
  JS_Val& operator=(Object* val)  {
    if (IsCxxProp()) {
      type = TY_CXX_Object;
      *(Object **)x.field = val;
    } else {
      type = TY_Object;
      x.val_obj = val;
    }
    return *this;
  }

  // convert int to numeric type in target JS_Val
  JS_Val& operator=(int val)  {
    if (IsCxxProp()) {
      if (type == TY_CXX_Long)
        *(long*)x.field = (long)val;
      else if (type == TY_CXX_Double)
        *(double *)x.field = (double)val;
    } else {
      if (type == TY_Long)
        x.val_long = (long)val;
      else if (type == TY_Double)
        x.val_double = (double)val;
      else if (type == TY_Undef) {
        // if target JS_Val is undef, convert to long
        type = TY_Long;
        x.val_long = (long)val;
      }
    }
    return *this;
  }

  JS_Val& operator=(std::string &val)  {
    if (IsCxxProp()) {
      type = TY_CXX_String;
      // x.field already pt to a str fd in a c++ obj
      *(std::string *)x.field = val;
    } else {
      type = TY_String;
      x.val_string = new std::string(val);
    }
    return *this;
  }

  JS_Val& operator=(char* val)  {
    if (IsCxxProp()) {
      type = TY_CXX_String;
      *(std::string *)x.field = std::string(val);
    } else {
      type = TY_String;
      x.val_string = new std::string(val);
    }
    return *this;
  }


};

typedef std::unordered_map<std::string, JS_Val> JS_PropList;
typedef std::pair<std::string, JS_Val> ObjectProp;

class Object {
  public:
    JS_PropList propList;
    Object*   __proto__;       // prototype chain
    Function* constructor;     // constructor of object

  public:
    Object(): __proto__(nullptr) {}
    Object(Function* ctor, Object* proto): constructor(ctor), __proto__(proto) {}
    Object(Function* ctor, Object* proto, std::vector<ObjectProp> props): constructor(ctor), __proto__(proto)
    {
      for (int i=0; i<props.size(); ++i)
        this->AddProp(props[i].first, props[i].second);
    }
    virtual ~Object() {}
    class Ctor;

    JS_Val& operator[] (std::string key)
    {
      if (!HasOwnProp(key)) AddProp(key, JS_Val());
      return GetPropVal(key);
    }


    bool HasOwnProp(std::string key) {
      JS_PropList::iterator it;
      it = propList.find(key);
      return (it != propList.end());
    }

    void AddProp(std::string key, JS_Val val) {
      propList[key] = val;
    }

    JS_Val GetProp(std::string key) {
      return propList[key];
    }

    JS_Val& GetPropVal(std::string key) {
      return propList[key];
    }

    void* GetPropField(std::string key) {
      return propList[key].x.field;
    }

    bool GetPropBool(std::string key) {
      if (propList[key].IsCxxProp())
        return *(bool*)GetPropField(key);
      return propList[key].x.val_bool;
    }
    long GetPropLong(std::string key) {
      if (propList[key].IsCxxProp())
        return *(long*)GetPropField(key);
      return propList[key].x.val_long;
    }
    double GetPropDouble(std::string key) {
      if (propList[key].IsCxxProp())
        return *(double*)GetPropField(key);
      return propList[key].x.val_double;
    }
    void* GetPropBigInt(std::string key) {
      if (propList[key].IsCxxProp())
        return *(void**)GetPropField(key);
      return propList[key].x.val_bigint;
    }
    std::string GetPropStr(std::string key) {
      if (propList[key].IsCxxProp())
        return *(*(std::string**)GetPropField(key));
      return *propList[key].x.val_string;
    }
    Object* GetPropObj(std::string key) {
      if (propList[key].IsCxxProp())
        return *(Object**)GetPropField(key);
      return propList[key].x.val_obj;
    }
    Function* GetPropFunc(std::string key) {
      if (propList[key].IsCxxProp())
        return *(Function**)GetPropField(key);
      return propList[key].x.val_func;
    }

    virtual bool IsFuncObj() {
      return false;
    }

    bool IsEmpty() const {
      return propList.empty();
    }

    virtual const char* __GetClassName() const {
      return "";
    }

    // Put code for JS Object.prototype props as static fields and methods in this class
    // and add to propList of Object_ctor.prototype object on system init.

    virtual std::string TypeId() {
      return "object"s;
    }
};


using ArgsT = Array<JS_Val>;

class Function : public Object {
  public:
    Object* prototype;    // prototype property
    Object* _thisArg;     // from bind()
    ArgsT*  _args;        // from bind()

    Function(Function* ctor, Object* proto, Object* prototype_proto) : Object(ctor, proto) {
      JS_Val val(this);
      prototype = new Object(this, prototype_proto);
      prototype->AddProp("constructor", val);
    }
    class Ctor;

    bool IsFuncObj() {
      return true;
    }

    Object* bind(Object* obj, ArgsT* argv);
    virtual JS_Val func(Object* obj, ArgsT& args) {JS_Val res; return res;}

    // Put code for JS Function.prototype props as static fields and methods in this class
    // and add to propList of Function_ctor.prototype object on system init.

    std::string TypeId() override {
      return "function"s;
    }
};



class Object::Ctor : public Function {
  public:
    Ctor(Function* ctor, Object* proto) : Function(ctor, proto, nullptr) {}

    Object* operator()(Object* obj) {
      return(obj);
    }
    Object* _new() {
      return new Object(this, this->prototype);
    }
    Object* _new(std::vector<ObjectProp> props) {
      return new Object(this, this->prototype, props);
    }
};

class Function::Ctor : public Function {
  public:
    Ctor(Function* ctor, Object* proto, Object* prototype_proto) : Function(ctor, proto, prototype_proto) {}
};

template <class T>
class ClassFld {
  // Helper class for converting between class field offset, void ptr and integer val
  // NewProp() creates a JS_Val of type TY_CXX_<xxx> with pointer to objet member field.
  typedef union {
//  void* addr;
    T     offset;
    int   fld_offset;
  } FldAddr;

  public:
    FldAddr field;
  public:
//  JS_Val NewProp(JS_Type type) {return JS_Val(type, field.addr);}
    ClassFld(T offset)   {field.offset = offset;}
    T     Offset()       {return field.offset;}
    JS_Val NewProp(void* obj, JS_Type type) {return JS_Val(type, (void*)((char*)obj+field.fld_offset));}
};

template <typename T> std::string __js_typeof(T* v) {
  return v->TypeId();
}

template <typename T> std::string __js_typeof(T v) {
  if (std::is_signed<T>::value)
    return "number"s;
  if (std::is_integral<T>::value)
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

// TSC restricts Lhs of instanceof operator to either type any or an object type.
bool InstanceOf(JS_Val val, Function* ctor);

// Our implementation returns true if the prototype property of the func/class
// constructor appers in the proto chain of the object.
template <class T>
bool InstanceOf(T* val, Function* ctor) {
  if (ctor == nullptr)
    return false;

  Object* p = val->__proto__;
  while (p) {
    if (p == ctor->prototype)
      return true;
    else
      p = p->__proto__;
  }
  return false;
}

// TODO: To be implemented
inline bool StrictEqu(JS_Val lhs, JS_Val rhs) { return false; }
inline bool StrictNotEqu(JS_Val lhs, JS_Val rhs) { return true; }

void GenerateDOTGraph( std::vector<Object *>&obj, std::vector<std::string>&name);

} // namespace t2crt

#include "builtins.h"

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

template <typename T>
std::ostream& operator<< (std::ostream& out, const t2crt::Array<T>* v) {
  if(v->elements.empty())
    out << "[]";
  else {
    out << "[ ";
    auto i = v->elements.begin(), e = v->elements.end();
    out << *i++;
    for (; i != e; ++i)
        std::cout << ", " << *i;
    out << " ]";
  }
  return out;
}

template <typename T>
std::ostream& operator<< (std::ostream& out, const t2crt::Array<T>& v) {
  if(v.elements.empty())
    out << "[]";
  else {
    out << "[ ";
    auto i = v.elements.begin(), e = v.elements.end();
    out << *i++;
    for (; i != e; ++i)
        std::cout << ", " << *i;
    out << " ]";
  }
  return out;
}
extern std::ostream& operator<< (std::ostream& out, const t2crt::JS_Val& v);
extern std::ostream& operator<< (std::ostream& out, const t2crt::Object* obj);
extern const t2crt::JS_Val undefined;
extern const t2crt::JS_Val null;
#define debugger (0)

using t2crt::Object;
using t2crt::Function;
using t2crt::Array;

#endif
