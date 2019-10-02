////////////////////////////////////////////////////////////////////////
//                     Literal Generation                             //
////////////////////////////////////////////////////////////////////////

#ifndef __LITERAL_GEN_H__
#define __LITERAL_GEN_H__

#include "base_struct.h"
#include "base_gen.h"
#include "rule.h"

class LiteralGen : public BaseGen {
public:
  LiteralGen(const char *dfile, const char *hfile, const char *cfile)
      : BaseGen(dfile, hfile, cfile) {}
  ~LiteralGen(){}

  void ProcessStructData(){}
  void Generate();
  void GenCppFile();
  void GenHeaderFile();
};

#endif
