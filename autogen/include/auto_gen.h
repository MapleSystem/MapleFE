//////////////////////////////////////////////////////////////////////////
//                          class AutoGen                               //
//////////////////////////////////////////////////////////////////////////

#ifndef __AUTO_GEN_H_
#define __AUTO_GEN_H_

#include <vector>

#include "reserved_gen.h"
#include "iden_gen.h"
#include "literal_gen.h"
#include "type_gen.h"
#include "localvar_gen.h"
#include "block_gen.h"
#include "separator_gen.h"
#include "operator_gen.h"

class AutoGen {
private:
  IdenGen      *mIdenGen;
  ReservedGen  *mReservedGen;
  LiteralGen   *mLitGen;
  TypeGen      *mTypeGen;
  LocalvarGen  *mLocalvarGen;
  BlockGen     *mBlockGen;
  SeparatorGen *mSeparatorGen;
  OperatorGen  *mOperatorGen;

  std::vector<BaseGen*> mGenArray;
  SPECParser   *mParser;

public:
  AutoGen(SPECParser *p) : mParser(p) {}
  ~AutoGen();

  void Init();
  void Run();
  void BackPatch();
  void Gen();
  ReservedGen *GetReservedGen() { return mReservedGen; }

  std::vector<BaseGen*> GetGenArray() { return mGenArray; }
};

#endif
