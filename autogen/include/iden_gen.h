////////////////////////////////////////////////////////////////
//                  Identifier Generation
////////////////////////////////////////////////////////////////

#ifndef __IDEN_GEN_H__
#define __IDEN_GEN_H__

#include "base_gen.h"

class IdenGen : public BaseGen {
public:
  IdenGen(const char *dfile, const char *hfile, const char *cfile) :
    BaseGen(dfile, hfile, cfile) {}
  ~IdenGen(){}
  void Run(SPECParser *parser);
  void Generate();
  void GenCppFile();
  void GenHeaderFile();
};

#endif
