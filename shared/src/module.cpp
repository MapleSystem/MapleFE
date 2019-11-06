#include "module.h"
#include "symbol.h"

Symbol *Module::GetSymbol(stridx_t stridx) {
  for (auto sb: mSymbolTable) {
    if (sb->mStridx == stridx)
      return sb;
  }
  return NULL;
}

GlobalTables GlobalTables::globalTables;

