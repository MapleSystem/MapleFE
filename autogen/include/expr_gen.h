////////////////////////////////////////////////////////////////////////
//                     Expr Generation                                //
////////////////////////////////////////////////////////////////////////

#ifndef __EXPR_GEN_H__
#define __EXPR_GEN_H__

#include "base_gen.h"
#include "all_supported.h"

class ExprGen : public BaseGen {
public:
  ExprGen(const char *dfile, const char *hfile, const char *cfile)
      : BaseGen(dfile, hfile, cfile) {}
  ~ExprGen(){}

  void Generate();
  void GenCppFile();
  void GenHeaderFile();
};

#endif
