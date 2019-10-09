////////////////////////////////////////////////////////////////////////
//                     Type Generation                                //
// A same type may have different keyword in different languages, e.g.//
// Boolean is "boolean" in Java, and "bool" in C/C++.                 //
////////////////////////////////////////////////////////////////////////

#ifndef __TYPE_GEN_H__
#define __TYPE_GEN_H__

#include <vector>

#include "base_struct.h"
#include "base_gen.h"
#include "all_supported.h"

// The types in Autogen has no implication of physical representation.
// It's just a virtual type with a NAME. Only MapleIR types have physical
// representatioin.
//

struct Keyword2Type {
  std::string mKeyword;
  AGTypeId    mId;
};

class TypeGen : public BaseGen {
public:
  std::vector<Keyword2Type> mTypes;
public:
  TypeGen(const char *dfile, const char *hfile, const char *cfile)
      : BaseGen(dfile, hfile, cfile) {}
  ~TypeGen(){}

  void ProcessStructData();
  void AddEntry(std::string s, AGTypeId t) { mTypes.push_back({s, t}); }

  void Generate();
  void GenCppFile();
  void GenHeaderFile();

  // Functions to dump Keyword Enum.
  std::vector<Keyword2Type>::iterator mEnumIter;
  const std::string EnumName(){return "";}
  const std::string EnumNextElem();
  const void EnumBegin();
  const bool EnumEnd();
};

#endif
