#include "../include/ts2cpp.h"

namespace t2crt {

Ctor_Function Function_ctor(&Function_ctor, Function_ctor.prototype, Object_ctor.prototype);
Ctor_Object   Object_ctor  (&Function_ctor, Function_ctor.prototype, nullptr);

ARRAY_CTOR_DEF(int)
ARRAY_CTOR_DEF(long)
ARRAY_CTOR_DEF(double)
ARRAY_CTOR_DEF(JS_Val)
ARRAY_CTOR_DEF(Object)
ARRAY_CTOR_DEF(ObjectP)

} // namepsace t2crt

