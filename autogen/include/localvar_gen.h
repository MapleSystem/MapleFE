////////////////////////////////////////////////////////////////
//                  Local Variable Generation
////////////////////////////////////////////////////////////////

#ifndef __LOCAL_VAR_GEN_H__
#define __LOCAL_VAR_GEN_H__

#include "base_gen.h"

class LocalvarGen : public BaseGen {
public:
  LocalvarGen(const char *dfile, const char *hfile, const char *cfile) :
    BaseGen(dfile, hfile, cfile) {}
  ~LocalvarGen(){}
};

#endif
