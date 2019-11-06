#include <sstream>
#include "function.h"

Function::Function(std::string funcname, Module *module) : mModule(module) {
  mStridx = GlobalTables::GetStringTable().GetOrCreateStridxFromName(funcname);
  symbolIdx = 0;
}

Symbol *Function::GetSymbol(stridx_t stridx) {
  for (auto sb: mSymbolTable) {
    if (sb->mStridx == stridx)
      return sb;
  }
  return NULL;
}

Symbol *Function::GetNewSymbol(tyidx_t t) {
  std::stringstream ss;
  ss << "var__" << symbolIdx++;
  std::string str(ss.str());
  stridx_t stridx = GlobalTables::GetStringTable().GetOrCreateStridxFromName(str);
  //std::cout << "NewSymbol " << str << " " << stridx << 
  //                      " " << GlobalTables::GetStringTable().GetStringFromStridx(stridx) << " ttt" << std::endl;
  return new Symbol(stridx, t);
}

void Function::Dump() {
  std::string str = GlobalTables::GetStringTable().GetStringFromStridx(mStridx);
  std::cout << "\n\n============================== function ==============================" << std::endl;
  std::cout << "func " << str << " (";
  int argSize = mFormals.size();
  for (int i = 0; i < argSize; i++) {
    Symbol *symbol = mFormals[i];
    // symbol type
    std::cout << "int ";
    // symbol name
    std::string str = GlobalTables::GetStringTable().GetStringFromStridx(symbol->mStridx);
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
  std::cout << "======================================================================\n\n" << std::endl;
}

void Function::EmitCode() {
  std::string str = GlobalTables::GetStringTable().GetStringFromStridx(mStridx);
  std::cout << "\n\nfunc " << str << " (";
  int argSize = mFormals.size();
  for (int i = 0; i < argSize; i++) {
    Symbol *symbol = mFormals[i];
    // symbol type
    std::cout << "int ";
    // symbol name
    std::string str = GlobalTables::GetStringTable().GetStringFromStridx(symbol->mStridx);
    std::cout << str;
    if (i != (argSize - 1)) {
      std::cout << ", ";
    }
  }
  std::cout << ") {" << std::endl;

  std::vector<Stmt *>::iterator it = mBody.begin();
  for (; it != mBody.end(); it++) {
    (*it)->EmitCode(1);
  }

  std::cout << "}\n" << std::endl;
}
