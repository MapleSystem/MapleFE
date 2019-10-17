////////////////////////////////////////////////////////////////////////
//                     Stmt Generation                                //
////////////////////////////////////////////////////////////////////////

#ifndef __STMT_GEN_H__
#define __STMT_GEN_H__

#include "base_gen.h"
#include "all_supported.h"

class StmtGen : public BaseGen {
public:
  StmtGen(const char *dfile, const char *hfile, const char *cfile)
      : BaseGen(dfile, hfile, cfile) {}
  ~StmtGen(){}

  void Generate();
  void GenCppFile();
  void GenHeaderFile();
};

#endif
