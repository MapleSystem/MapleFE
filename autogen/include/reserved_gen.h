//////////////////////////////////////////////////////////////////////////////
//  There are many reserved keywords representing certain rule elements, or //
//  operations. This file defines these reservation and necessary operations//
//////////////////////////////////////////////////////////////////////////////
//
#ifndef __RESERVED_OPS_H__
#define __RESERVED_OPS_H__

#include <vector>

#include "rule.h"
#include "base_gen.h"

// ONEOF, ZEROORMORE, ZEROORONE, ...
typedef struct {
  const char *mName;   // The reserved name in the .spec syntax
  RuleOp      mOp;
}ReservedOp;

//////////////////////////////////////////////////////////////////////
//   ReservedGen generates those reserved rules in memory, which are//
//   used by other XxxGen.
//   It also parse the reserved.spec, and output reserved tables in //
//   LANGUAGE/src/gen_reserved.cpp, and the header file too.        //
//                                                                  //
//   It has (1) reserved OPs, in mOps; (2) reserved rules in mRules //
//   inherited from Rule.                                           //
//////////////////////////////////////////////////////////////////////

class ReservedGen : public BaseGen {
private:
  std::vector<ReservedOp>    mOps;

public:
  ReservedGen(const char *dfile, const char *hfile, const char *cppfile);
  ~ReservedGen(){}
  void Run(SPECParser *parser);
  void Generate();
  void GenCppFile();
  void GenHeaderFile();
};

#endif
