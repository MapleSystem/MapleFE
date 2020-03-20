/*
* Copyright (C) [2020] Futurewei Technologies, Inc. All rights reverved.
*
* OpenArkFE is licensed under the Mulan PSL v1.
* You can use this software according to the terms and conditions of the Mulan PSL v1.
* You may obtain a copy of Mulan PSL v1 at:
*
*  http://license.coscl.org.cn/MulanPSL
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR
* FIT FOR A PARTICULAR PURPOSE.
* See the Mulan PSL v1 for more details.
*/

#include <cstring>

#include "attr_gen.h"
#include "massert.h"
#include "all_supported.h"

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
  GenHeaderFile();
  GenCppFile();
}

void AttrGen::GenHeaderFile() {
  mHeaderFile.WriteOneLine("#ifndef __ATTR_GEN_H__", 22);
  mHeaderFile.WriteOneLine("#define __ATTR_GEN_H__", 22);
  mHeaderFile.WriteOneLine("extern AttrKeyword AttrKeywordTable[ATTR_NA];", 45);
  mHeaderFile.WriteOneLine("#endif", 6);
}

void AttrGen::GenCppFile() {
  mCppFile.WriteOneLine("#include \"common_header_autogen.h\"", 34);

  TableBuffer tb;
  tb.Generate(this, "AttrKeyword AttrKeywordTable[ATTR_NA] = {");
  mCppFile.WriteFormattedBuffer(&tb);
  mCppFile.WriteOneLine("};", 2);
}

