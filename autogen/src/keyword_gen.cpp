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

#include "keyword_gen.h"
#include "massert.h"

namespace maplefe {

/////////////////////////////////////////////////////////////////////
//                 Generate the Separator table                    //
// The table in the cpp file:
//      KeywordTableEntry KeywordTable[18] = {"xxx", ..., "yyy"};  //
/////////////////////////////////////////////////////////////////////

const void KeywordGen::EnumBegin(){
  mEnumIter = mKeywords.begin();
  return;
}

const bool KeywordGen::EnumEnd(){
  return mEnumIter == mKeywords.end();
}

const std::string KeywordGen::EnumNextElem(){
  std::string keyword = "\"" + *mEnumIter + "\"";
  mEnumIter++;
  return keyword;
}

/////////////////////////////////////////////////////////////////////
//                Parse the keyword .spec file                     //
/////////////////////////////////////////////////////////////////////

void KeywordGen::ProcessStructData() {
  std::vector<StructBase *>::iterator it = mStructs.begin();
  for (; it != mStructs.end(); it++) {
    StructBase *sb = *it;
    sb->Sort(0);

    for (auto eit: sb->mStructElems) {
      AddEntry(eit->mDataVec[0]->GetString());
    }
  }
}

/////////////////////////////////////////////////////////////////////
//                   Generate the output files                     //
/////////////////////////////////////////////////////////////////////

void KeywordGen::Generate() {
  GenHeaderFile();
  GenCppFile();
}

void KeywordGen::GenHeaderFile() {
  mHeaderFile.WriteOneLine("#ifndef __KEYWORD_GEN_H__", 25);
  mHeaderFile.WriteOneLine("#define __KEYWORD_GEN_H__", 25);
  mHeaderFile.WriteOneLine("namespace maplefe {", 19);

  // table decl
  std::string s = "extern KeywordTableEntry KeywordTable[";
  s = s + std::to_string(mKeywords.size());
  s = s + "];";
  mHeaderFile.WriteOneLine(s.c_str(), s.size());

  // table size decl
  s = "extern unsigned KeywordTableSize;";
  mHeaderFile.WriteOneLine(s.c_str(), s.size());

  mHeaderFile.WriteOneLine("}", 1);
  mHeaderFile.WriteOneLine("#endif", 6);
}

void KeywordGen::GenCppFile() {
  mCppFile.WriteOneLine("#include \"ruletable.h\"", 22);
  mCppFile.WriteOneLine("namespace maplefe {", 19);
  TableBuffer tb;
  std::string s = "KeywordTableEntry KeywordTable[";
  s = s + std::to_string(mKeywords.size());
  s = s + "] = {";
  tb.Generate(this, s);
  mCppFile.WriteFormattedBuffer(&tb);
  mCppFile.WriteOneLine("};", 2);

  // table size;
  s = "unsigned KeywordTableSize = ";
  s = s + std::to_string(mKeywords.size());
  s = s + ";";
  mCppFile.WriteOneLine(s.c_str(), s.size());
  mCppFile.WriteOneLine("}", 1);
}
}


