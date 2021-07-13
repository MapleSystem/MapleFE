#include "../include/ts2cpp.h"

namespace t2crt {

Ctor_Function Function_ctor(&Function_ctor, Function_ctor.prototype, Object_ctor.prototype);
Ctor_Object   Object_ctor  (&Function_ctor, Function_ctor.prototype, nullptr);
Ctor_Array    Array_ctor   (&Function_ctor, Function_ctor.prototype, Object_ctor.prototype);

} // namepsace t2crt

