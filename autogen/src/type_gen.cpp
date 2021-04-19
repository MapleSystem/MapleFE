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

#include "type_gen.h"
#include "massert.h"
#include "all_supported.h"

namespace maplefe {

// Fill Map
void TypeGen::ProcessStructData() {
  for (auto it: mStructs) {
    for (auto eit: it->mStructElems) {
      std::string s(eit->mDataVec[1]->GetString());
      TypeId id = FindTypeIdLangIndep(s);
      AddEntry(eit->mDataVec[0]->GetString(), id);
    }
  }
}

/////////////////////////////////////////////////////////////////////
//                 Generate the Type Keyword table                 //
// The table in the cpp file:
//      TypeKeyword TypeKeywordTable[TY_NA] = {
//        {"xxx", TY_Xxx},
//        ...
//      };
// And in .h file there is an external decl                        //
//      extern TypeKeyword TypeKeywordTable[TY_NA];                //
/////////////////////////////////////////////////////////////////////

const void TypeGen::EnumBegin(){
  mEnumIter = mTypes.begin();
  return;
}

const bool TypeGen::EnumEnd(){
  return mEnumIter == mTypes.end();
}

const std::string TypeGen::EnumNextElem(){
  Keyword2Type entry = *mEnumIter;
  std::string enum_item = "{";
  enum_item = enum_item + "\"" + entry.mKeyword + "\", TY_" + GetTypeString(entry.mId);
  enum_item = enum_item + "}";
  mEnumIter++;
  return enum_item;
}

/////////////////////////////////////////////////////////////////////
//                   Generate the output files                     //
/////////////////////////////////////////////////////////////////////

void TypeGen::Generate() {
  GenRuleTables();
  GenHeaderFile();
  GenCppFile();
}

void TypeGen::GenHeaderFile() {
  mHeaderFile.WriteOneLine("#ifndef __TYPE_GEN_H__", 22);
  mHeaderFile.WriteOneLine("#define __TYPE_GEN_H__", 22);
  mHeaderFile.WriteOneLine("namespace maplefe {", 19);

  // generate the keyword table
  mHeaderFile.WriteOneLine("extern TypeKeyword TypeKeywordTable[];", 38);

  // generate the rule tables
  mHeaderFile.WriteFormattedBuffer(&mRuleTableHeader);
  mHeaderFile.WriteOneLine("}", 1);
  mHeaderFile.WriteOneLine("#endif", 6);
}

void TypeGen::GenCppFile() {
  mCppFile.WriteOneLine("#include \"common_header_autogen.h\"", 34);
  mCppFile.WriteOneLine("namespace maplefe {", 19);

  // generate the keyword table
  TableBuffer tb;
  std::string s = "TypeKeyword TypeKeywordTable[";
  std::string num = std::to_string(mTypes.size());
  s += num;
  s += "] = {";
  tb.Generate(this, s);
  mCppFile.WriteFormattedBuffer(&tb);
  mCppFile.WriteOneLine("};", 2);

  // generate the table size
  s = "unsigned TypeKeywordTableSize = ";
  s += num;
  s += ";";
  mCppFile.WriteOneLine(s.c_str(), s.size());

  // generate the rule tables
  mCppFile.WriteFormattedBuffer(&mRuleTableCpp);

  mCppFile.WriteOneLine("}", 1);
}

}
