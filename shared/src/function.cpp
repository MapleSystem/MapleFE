#include "function.h"

Function::Function(std::string funcname, Module *module) : mModule(module) {
  mStridx = mModule->mStrTable.GetOrCreateGstridxFromName(funcname);
}

Symbol *Function::GetSymbol(stridx_t stridx) {
  for (auto sb: mSymbolTable) {
    if (sb->mStridx == stridx)
      return sb;
  }
  return NULL;
}

void Function::Dump() {
  std::string str = mModule->mStrTable.GetStringFromGstridx(mStridx);
  std::cout << "func " << str << " (";
  int argSize = mFormals.size();
  for (int i = 0; i < argSize; i++) {
    Symbol *symbol = mFormals[i];
    // symbol type
    std::cout << "int ";
    // symbol name
    std::string str = mModule->mStrTable.GetStringFromGstridx(symbol->mStridx);
    std::cout << str;
    if (i != (argSize - 1)) {
      std::cout << ", ";
    }
  }
  std::cout << ") {" << std::endl;

  std::vector<Stmt *>::iterator it = mBody.begin();
  for (; it != mBody.end(); it++) {
    (*it)->Dump(1, /* withheader */false);
  }

  std::cout << "}" << std::endl;
}

