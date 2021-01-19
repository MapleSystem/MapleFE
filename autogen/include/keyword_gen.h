/*
* Copyright (C) [2020] Futurewei Technologies, Inc. All rights reverved.
*
* OpenArkFE is licensed under the Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*
*  http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR
* FIT FOR A PARTICULAR PURPOSE.
* See the Mulan PSL v2 for more details.
*/
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

namespace maplefe {

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

}

#endif
