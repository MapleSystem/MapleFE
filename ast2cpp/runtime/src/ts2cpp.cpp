#include "../include/ts2cpp.h"

const t2crt::JS_Val undefined = { 0, t2crt::TY_Undef, false };

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

