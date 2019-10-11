////////////////////////////////////////////////////////////////////////
//                      Keyword   Generation
// KeywordGen just dump the keywords into a table.
////////////////////////////////////////////////////////////////////////

#ifndef __KEYWORD_GEN_H__
#define __KEYWORD_GEN_H__

#include <vector>
#include <string>

#include "base_struct.h"
#include "base_gen.h"

// The class of SeparatorGen
class KeywordGen : public BaseGen {
public:
  std::vector<std::string> mKeywords;
public:
  KeywordGen(const char *dfile, const char *hfile, const char *cfile)
      : BaseGen(dfile, hfile, cfile) {}
  ~KeywordGen(){}

  void ProcessStructData();
  void Generate();
  void GenCppFile();
  void GenHeaderFile();

  void AddEntry(std::string s) { mKeywords.push_back(s); }

  // Functions to dump Enum.
  std::vector<std::string>::iterator mEnumIter;
  const std::string EnumName(){return "";}
  const std::string EnumNextElem();
  const void EnumBegin();
  const bool EnumEnd();
};

#endif
