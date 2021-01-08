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
#include <string>

#include "separator_gen.h"
#include "massert.h"
#include "all_supported.h"

//////////////   Separators supported /////////////////
#undef  SEPARATOR
#define SEPARATOR(N, S) {#S, SEP_##S},
SuppSepId SepsSupported[SEP_NA] = {
#include "supported_separators.def"
};

// s : the literal name 
// return the SepId
SepId FindSeparatorId(const std::string &s) {
  for (unsigned u = 0; u < SEP_NA; u++) {
    if (!SepsSupported[u].mName.compare(0, s.length(), s))
      return SepsSupported[u].mSepId;
  }
  return SEP_NA;
}

std::string FindSeparatorName(SepId id) {
  std::string s("");
  for (unsigned u = 0; u < SEP_NA; u++) {
    if (SepsSupported[u].mSepId == id){
      s = SepsSupported[u].mName;
      break;
    }
  }
  return s;
}

/////////////////////////////////////////////////////////////////////
//                 Generate the Separator table                    //
// The table in the cpp file:
//      SepTableEntry SepTable[SEP_Null] = {
//        {"xxx", SEP_Xxx},
//        ...
//      };
/////////////////////////////////////////////////////////////////////

const void SeparatorGen::EnumBegin(){
  mEnumIter = mSeparators.begin();
  return;
}

const bool SeparatorGen::EnumEnd(){
  return mEnumIter == mSeparators.end();
}

// The format is SEP_ appended with the separator name which is from
// the Separator.mKeyword
const std::string SeparatorGen::EnumNextElem(){
  Separator sep = *mEnumIter;
  std::string enum_item = "{";
  enum_item = enum_item + "\"" + sep.mKeyword + "\", SEP_" + FindSeparatorName(sep.mID);
  enum_item = enum_item + "}";
  mEnumIter++;
  return enum_item; 
}

/////////////////////////////////////////////////////////////////////
//                Parse the Separator .spec file                   //
//                                                                 //
/////////////////////////////////////////////////////////////////////

// Fill Map
void SeparatorGen::ProcessStructData() {
  std::vector<StructBase *>::iterator it = mStructs.begin();
  for (; it != mStructs.end(); it++) {
    // Sort
    StructBase *sb = *it;
    sb->Sort(0);

    for (auto eit: sb->mStructElems) {
      const std::string s(eit->mDataVec[1]->GetString());
      SepId id = FindSeparatorId(s);
      AddEntry(eit->mDataVec[0]->GetString(), id);
    }
  }
}

/////////////////////////////////////////////////////////////////////
//                   Generate the output files                     //
/////////////////////////////////////////////////////////////////////

void SeparatorGen::Generate() {
  GenHeaderFile();
  GenCppFile();
}

void SeparatorGen::GenHeaderFile() {
  mHeaderFile.WriteOneLine("#ifndef __SEPARATOR_GEN_H__", 27);
  mHeaderFile.WriteOneLine("#define __SEPARATOR_GEN_H__", 27);
  mHeaderFile.WriteOneLine("extern SepTableEntry SepTable[SEP_NA];", 38);
  mHeaderFile.WriteOneLine("#endif", 6);
}

void SeparatorGen::GenCppFile() {
  mCppFile.WriteOneLine("#include \"ruletable.h\"", 22);
  TableBuffer tb;
  tb.Generate(this, "SepTableEntry SepTable[SEP_NA] = {");
  mCppFile.WriteFormattedBuffer(&tb);
  mCppFile.WriteOneLine("};", 2);
}

