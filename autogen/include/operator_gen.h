////////////////////////////////////////////////////////////////////////
//                      Operator Generation
// The output of this Operator Generation is a table in gen_operator.cpp 
//
//   OprTableEntry OprTable[OPR_Null] = {
//     {"xxx", OPR_Xxx},
//     ...
//   };
//
////////////////////////////////////////////////////////////////////////

#ifndef __OPERATOR_GEN_H__
#define __OPERATOR_GEN_H__

#include <list>

#include "base_struct.h"
#include "base_gen.h"

// For each operator, it has three parts involved in the generation.
//   1. OprId: Used inside autogen, connection between LANGUAGE and
//             supported_operators.spec files
//   2. Name:  Name of OPR ID, to be generated in gen_operator.cpp 
//   4. Text:  LANGUAGE syntax text, to be in gen_operator.cpp

// The operators supported in autogen, super set of all languages.
#undef  OPERATOR
#define OPERATOR(T) OPR_##T,
enum OprId {
#include "supported_operators.def"
OPR_NA
};

// The SUPPORTED operator and their name.
// 'name' is used as known word in operator.spec
struct OperatorId {
  std::string mName;   // This will be output in gen_operator.cpp
  OprId       mOprId;  // ID linked to LANGUAGE operator defined below.
};

// The LANGUAGE operator
struct Operator {
  const char *mText;   // Syntax text in LANGUAGE, output in gen_separator.cpp
  OprId       mID;     // ID linked to SUPPORTED OperatorId defined above.
};

extern OprId       FindOperatorId(const std::string &s);
extern std::string FindOperatorName(OprId id);

// The class of OperatorGen
class OperatorGen : public BaseGen {
public:
  std::list<Operator> mOperators;
public:
  OperatorGen(const char *dfile, const char *hfile, const char *cfile)
      : BaseGen(dfile, hfile, cfile) {}
  ~OperatorGen(){}

  bool Empty();

  // Generate the output files: gen_separator.h/.cpp
  void ProcessStructData();
  void Generate();
  void GenCppFile();
  void GenHeaderFile();

  void AddEntry(const char *s, OprId i) { mOperators.push_back({s, i}); }

  // Generation functions for Enum output.
  std::list<Operator>::iterator mEnumIter;
  const std::string EnumName(){return "";}
  const std::string EnumNextElem();
  const void EnumBegin();
  const bool EnumEnd();
};

#endif
