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
//                     Attribute Generation                           //
// This module generates a table of mapping language keywords to the  //
// supported attributes.                                              //
////////////////////////////////////////////////////////////////////////

#ifndef __ATTR_GEN_H__
#define __ATTR_GEN_H__

#include <vector>

#include "base_struct.h"
#include "base_gen.h"
#include "all_supported.h"

namespace maplefe {

struct Keyword2Attr {
  std::string mKeyword;
  AttrId    mId;
};

class AttrGen : public BaseGen {
public:
  std::vector<Keyword2Attr> mAttrs;
public:
  AttrGen(const char *dfile, const char *hfile, const char *cfile)
      : BaseGen(dfile, hfile, cfile) {}
  ~AttrGen(){}

  void ProcessStructData();
  void AddEntry(std::string s, AttrId t) { mAttrs.push_back({s, t}); }

  void Generate();
  void GenCppFile();
  void GenHeaderFile();

  // Functions to dump Keyword Enum.
  std::vector<Keyword2Attr>::iterator mEnumIter;
  const std::string EnumName(){return "";}
  const std::string EnumNextElem();
  const void EnumBegin();
  const bool EnumEnd();
};

}
#endif
