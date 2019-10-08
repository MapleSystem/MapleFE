/////////////////////////////////////////////////////////////////////
// this contains all the data from parser
/////////////////////////////////////////////////////////////////////

#ifndef __MODULE_H__
#define __MODULE_H__

#include <iostream>
#include <fstream>
#include <stack>

#include "massert.h"
#include "base_gen.h"

typedef uint32_t stridx_t;
typedef uint32_t tyidx_t;

class StringTable {
 public:
  std::vector<std::string> string_table_;  // index is stridx_t
  std::unordered_map<std::string, stridx_t> string_table_map_;

 public:
  StringTable() {
    // initialize 0th entry of string_table_ with an empty string
    std::string temp;
    string_table_.push_back(temp);
  }

  virtual ~StringTable() {}

  stridx_t GetGstridxFromName(const std::string &str) const {
    auto it = string_table_map_.find(str);
    if (it == string_table_map_.end()) {
      return stridx_t(0);
    }   
    return it->second;
  }

  stridx_t GetOrCreateGstridxFromName(const std::string &str) {
    stridx_t strIdx = GetGstridxFromName(str);
    if (strIdx == 0) {
      strIdx = string_table_.size();
      string_table_.push_back(str);
      string_table_map_[str] = strIdx;
    }   
    return strIdx;
  }

  const std::string &GetStringFromGstridx(stridx_t stridx) const {
    MASSERT(stridx < string_table_.size());
    return string_table_.at(stridx);
  }
};

class Type {
  stridx_t mStridx;
};

#define TYPE_HASH_LENGTH 10993
class TypeTable {
 public:
  std::vector<Type *> type_table_;
  Type *type_hash_table[TYPE_HASH_LENGTH];  // the hash table that hash the number value
  static Type *voidPtrType;

 public:
  TypeTable();
  ~TypeTable();
  uint32_t GetHashIndex(Type *) const;
  void PutToHashTable(uint32_t hidx, Type *mirtype);
  Type *GetTypeFromTyIdx(tyidx_t tyidx) const {
    MASSERT(tyidx < type_table_.size());
    return type_table_.at(tyidx);
  }
};

class Symbol {
 public:
  stridx_t mStridx;
  tyidx_t  mTyidx;

  Symbol(stridx_t s, tyidx_t t) : mStridx(s), mTyidx(t) {}

  void Dump();
};

class Expr {
 public:
  RuleElem *mElem;
  std::vector<Expr *> mSubExprs;

  Expr() : mElem(NULL) {}
  Expr(RuleElem *elem) : mElem(elem) {}

  void Dump(unsigned indent) {
    unsigned i = indent;
    while (i--) {
      std::cout << "  ";
    }
    mElem->Dump();
    std::cout << std::endl;
    std::vector<Expr *>::iterator it = mSubExprs.begin();
    for(; it != mSubExprs.end(); it++) {
      (*it)->Dump(indent+1);
    }
  }
};

class Stmt {
 public:
  std::vector<Expr *> mExprs;

  Stmt() {}
  void Dump(unsigned indent) {
    std::cout << "============ stmt tree ============" << std::endl;
    std::vector<Expr *>::iterator it = mExprs.begin();
    for(; it != mExprs.end(); it++) {
      (*it)->Dump(indent+1);
    }
    std::cout << "===================================" << std::endl;
  }
};

class Function {
 public:
  stridx_t mStridx;
  std::vector<tyidx_t> mArgTyidx;
  std::vector<Symbol *> mFormals;
  tyidx_t mRetTyidx;
  std::vector<Stmt *> mBody;

  std::vector<Symbol *> mSymbolTable;

  Function(std::string funcname) {};
  ~Function() {}

  Symbol *GetSymbol(stridx_t stridx) {
    for (auto sb: mSymbolTable) {
      if (sb->mStridx == stridx)
        return sb;
    }
    return NULL;
  }
};

class Module {
public:
  StringTable mStrTable;
  std::vector<Symbol *> mSymbolTable;
  std::vector<Function *> mFuncs;

public:
  Module() {};
  ~Module() {};

  Symbol *GetSymbol(stridx_t stridx) {
    for (auto sb: mSymbolTable) {
      if (sb->mStridx == stridx)
        return sb;
    }
    return NULL;
  }

  void Dump();
};

#endif
