#include "../include/ts2cpp.h"

namespace t2crt {

Object::Ctor   Object_ctor  (&Function_ctor, Function_ctor.prototype);
Function::Ctor Function_ctor(&Function_ctor, Function_ctor.prototype, Object_ctor.prototype);
Function::Ctor Number_ctor  (&Function_ctor, Function_ctor.prototype, Object_ctor.prototype);

ARRAY_CTOR_DEF(int)
ARRAY_CTOR_DEF(long)
ARRAY_CTOR_DEF(double)
ARRAY_CTOR_DEF(JS_Val)
ARRAY_CTOR_DEF(Object)
ARRAY_CTOR_DEF(ObjectP)

} // namepsace t2crt

