#include <iostream>
#include "base_struct.h"
#include "stringpool.h"

void StructData::SetString(std::string name, const StringPool *pool) {
  mKind = DK_String;
  char *str = pool->FindString(name);
  mValue.mString = str;
}

void StructData::SetName(std::string name, const StringPool *pool) {
  mKind = DK_Name;
  char *str = pool->FindString(name);
  mValue.mName = str;
}

void StructBase::Dump() {
  std::cout << "STRUCT " << mName << " : (";
  for (auto it: mStructElems) {
    std::cout << "\n  (";
    it->Dump();
    std::cout << ")";
    if (it != mStructElems.back())
      std::cout << ", ";
  }
  std::cout << ")" << std::endl;
}

void StructElem::Dump() {
  for (auto it: mDataVec) {
    it->Dump();
    if (it != mDataVec.back())
      std::cout << ", ";
  }
}

void StructData::Dump() {
  switch (mKind) {
    case DK_Char:
      std::cout << "'" << mValue.mChar << "'";
      break;
    case DK_Int:
      std::cout << mValue.mInt;
      break;
    case DK_Float:
      std::cout << mValue.mFloat;
      break;
    case DK_Double:
      std::cout << mValue.mDouble;
      break;
    case DK_Name:
      std::cout << mValue.mName;
      break;
    case DK_String:
      std::cout << "\"" << mValue.mString << "\"";
      break;
  }
}
