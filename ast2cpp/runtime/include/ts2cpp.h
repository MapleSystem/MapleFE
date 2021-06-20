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

#include <vector>
#include <string>
#include <unordered_map>

namespace t2crt {

using std::to_string;
inline std::string to_string(std::string t) {return t;}

// __JSVAL is set to be double for now
// Should be a JS value type
typedef double __JSVAL;

class BaseObj;

// JS types for props
typedef enum JS_Type : uint8_t {
  CG_Undef,     // "undefined"
  CG_Null,      // "object"
  CG_Bool,      // "boolean"
  CG_Long,      // "number"
  CG_Double,    // "number"
  CG_BigInt,    // "bigint"
  CG_String,    // "string"
  CG_Symbol,    // "symbol"
  CG_Function,  // "function"
  CG_Object,    // "object"
  CG_LAST,
  RT_Undef = 16,
  RT_Null,
  RT_Bool,
  RT_Long,
  RT_Double,
  RT_BigInt,
  RT_String,
  RT_Symbol,
  RT_Function,
  RT_Object,
  RT_LAST
} JS_Type;

typedef union JS_Val {
  void*    field;      // used by compiler genereted fields only
  bool     val_bool;
  long     val_long;
  double   val_double;
  long     val_bigint;
  std::string* val_string; // JS string primitive (not JS String object)
  BaseObj* val_obj;    // for function, object (incl. String objects)
} JS_Val;

typedef struct JS_Prop {
  JS_Val  val;
  JS_Type type;

  // Prop directly generated as class fields when TS is compiled into CPP
  JS_Prop(JS_Type jstype, void* field) {
    type = jstype;
    val.field = field;
  }

  // Prop created at runtime
  JS_Prop(JS_Type jstype, JS_Val jsval) {
    type = jstype;
    val  = jsval;
  }

  bool IsCompilerGenProp() { return !(type & 0x10); }

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
#endif
