////////////////////////////////////////////////////////////////
//                  Block Generation
////////////////////////////////////////////////////////////////

#ifndef __BLOCK_GEN_H__
#define __BLOCK_GEN_H__

#include "base_gen.h"

class BlockGen : public BaseGen {
public:
  BlockGen(const char *dfile, const char *hfile, const char *cfile) :
    BaseGen(dfile, hfile, cfile) {}
  ~BlockGen(){}

  void Generate();
  void GenCppFile();
  void GenHeaderFile();
};

#endif
