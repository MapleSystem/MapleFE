/////////////////////////////////////////////////////////////////////
// this contains all the data from parser
/////////////////////////////////////////////////////////////////////

#ifndef __MODULE_H__
#define __MODULE_H__

#include <iostream>
#include <fstream>
#include <stack>
#include <vector>

#include "massert.h"
#include "base_gen.h"

class Symbol;
class Function;

typedef uint32_t stridx_t;
typedef uint32_t tyidx_t;

class StrPtrHash {
 public:
  size_t operator()(const std::string *str) const {
    return std::hash<std::string>{}(*str);
  }
};

class StrPtrEqual {
 public:
  size_t operator()(const std::string *str1, const std::string *str2) const {
    return *str1 == *str2;
  }
};

template<typename T, typename U>
class StringTable {
 public:
  std::vector<const T*> string_table_;  // index is stridx_t
  std::unordered_map<const T*, U, StrPtrHash, StrPtrEqual> string_table_map_;

 public:
  StringTable() {
    // initialize 0th entry of string_table_ with an empty string
    T *ptr = new (std::nothrow) T;
    string_table_.push_back(ptr);
  }

  virtual ~StringTable() {}

  U GetStridxFromName(T &str) const {
    auto it = string_table_map_.find(&str);
    if (it == string_table_map_.end()) {
      return U(0);
    }   
    return it->second;
  }

  U GetOrCreateStridxFromName(T &str) {
    U strIdx = GetStridxFromName(str);
    if (strIdx == 0) {
      strIdx = string_table_.size();
      T *newStr = new (std::nothrow) T(str);
      string_table_.push_back(newStr);
      string_table_map_[newStr] = strIdx;
    }   
    return strIdx;
  }

  const T &GetStringFromStridx(U stridx) const {
    MASSERT(stridx < string_table_.size());
    return *string_table_[stridx];
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

class Module {
public:
  std::vector<Symbol *> mSymbolTable;
  std::vector<Function *> mFuncs;

public:
  Module() {};
  ~Module() {};

  Symbol *GetSymbol(stridx_t stridx);
  void Dump();
};

class GlobalTables {
 public:
  GlobalTables() = default;
  ~GlobalTables() = default;

  static StringTable<std::string, stridx_t> &GetStringTable() { return globalTables.mStrTable; }

 private:
  StringTable<std::string, stridx_t> mStrTable;
  static GlobalTables globalTables;
};

#endif
