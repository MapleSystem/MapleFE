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

#include <cstring>

#include "attr_gen.h"
#include "massert.h"
#include "all_supported.h"

namespace maplefe {

// Fill Map
void AttrGen::ProcessStructData() {
  for (auto it: mStructs) {
    for (auto eit: it->mStructElems) {
      std::string s(eit->mDataVec[1]->GetString());
      AttrId id = FindAttrId(s);
      AddEntry(eit->mDataVec[0]->GetString(), id);
    }
  }
}

/////////////////////////////////////////////////////////////////////
//              Generate the Attribute Keyword table               //
// The table in the cpp file:
//      AttrKeyword AttrKeywordTable[ATTR_NA] = {
//        {"xxx", ATTR_Xxx},
//        ...
//      };
// And in .h file there is an external decl                        //
//      extern AttrKeyword AttrKeywordTable[ATTR_NA];                //
/////////////////////////////////////////////////////////////////////

const void AttrGen::EnumBegin(){
  mEnumIter = mAttrs.begin();
  return;
}

const bool AttrGen::EnumEnd(){
  return mEnumIter == mAttrs.end();
}

const std::string AttrGen::EnumNextElem(){
  Keyword2Attr entry = *mEnumIter;
  std::string enum_item = "{";
  enum_item = enum_item + "\"" + entry.mKeyword + "\", ATTR_" + GetAttrString(entry.mId);
  enum_item = enum_item + "}";
  mEnumIter++;
  return enum_item;
}

/////////////////////////////////////////////////////////////////////
//                   Generate the output files                     //
/////////////////////////////////////////////////////////////////////

void AttrGen::Generate() {
  GenCppFile();
}

void AttrGen::GenCppFile() {
  mCppFile.WriteOneLine("#include \"common_header_autogen.h\"", 34);
  mCppFile.WriteOneLine("namespace maplefe {", 19);

  TableBuffer tb;
  std::string s = "AttrKeyword AttrKeywordTable[";
  std::string num = std::to_string(mAttrs.size());
  s += num;
  s += "] = {";
  tb.Generate(this, s);
  mCppFile.WriteFormattedBuffer(&tb);
  mCppFile.WriteOneLine("};", 2);

  // generate the table size
  s = "unsigned AttrKeywordTableSize = ";
  s += num;
  s += ";";
  mCppFile.WriteOneLine(s.c_str(), s.size());

  mCppFile.WriteOneLine("}", 1);
}
}
