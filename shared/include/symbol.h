#ifndef __SYMBOL_H__
#define __SYMBOL_H__

#include <iostream>
#include <fstream>

#include "massert.h"
#include "base_gen.h"
#include "module.h"

class Symbol {
 public:
  stridx_t mStridx;
  tyidx_t  mTyidx;

  Symbol(stridx_t s, tyidx_t t) : mStridx(s), mTyidx(t) {}

  void Dump();
};

#endif
