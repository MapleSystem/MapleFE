#ifndef __FUNCTION_H__
#define __FUNCTION_H__

#include <iostream>
#include <vector>

#include "massert.h"
#include "base_gen.h"
#include "module.h"
#include "symbol.h"
#include "stmt.h"

class Function {
 public:
  Module *mModule;
  stridx_t mStridx;
  std::vector<tyidx_t> mArgTyidx;
  std::vector<Symbol *> mFormals;
  tyidx_t mRetTyidx;
  std::vector<Stmt *> mBody;

  std::vector<Symbol *> mSymbolTable;

  Function(std::string funcname, Module *module);
  ~Function() {}

  Symbol *GetSymbol(stridx_t stridx);

  void Dump();
};

#endif
