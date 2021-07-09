#include "../include/ts2cpp.h"

namespace t2crt {

Object    Object_prototype  ((Function *)&Object_ctor,   nullptr);
Object    Function_prototype((Function *)&Function_ctor, &Object_prototype);
Object    Array_prototype   ((Function *)&Array_ctor,    &Object_prototype);
Ctor_Function Function_ctor(&Function_ctor, &Function_prototype, &Function_prototype);
Ctor_Object   Object_ctor  (&Function_ctor, &Function_prototype, &Object_prototype);
Ctor_Array    Array_ctor   (&Function_ctor, &Function_prototype, &Array_prototype);
} // namepsace t2crt

