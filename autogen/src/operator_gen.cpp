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
#include <string>

#include "operator_gen.h"
#include "massert.h"

//////////////  Operators supported /////////////////
#undef  OPERATOR
#define OPERATOR(S) {#S, OPR_##S},
OperatorId OprsSupported[OPR_NA] = {
#include "supported_operators.def"
};

// s : the literal name 
// return the OprId
OprId FindOperatorId(const std::string &s) {
  for (unsigned u = 0; u < OPR_NA; u++) {
    if (!OprsSupported[u].mName.compare(0, s.length(), s))
      return OprsSupported[u].mOprId;
  }
  return OPR_NA;
}

std::string FindOperatorName(OprId id) {
  std::string s("");
  for (unsigned u = 0; u < OPR_NA; u++) {
    if (OprsSupported[u].mOprId == id){
      s = OprsSupported[u].mName;
      break;
    }
  }
  return s;
}

/////////////////////////////////////////////////////////////////////
//                 Generate the Operator table                    //
// The table in the cpp file:
//      OprTableEntry OprTable[OPR_Null] = {
//        {"xxx", OPR_Xxx},
//        ...
//      };
/////////////////////////////////////////////////////////////////////

const void OperatorGen::EnumBegin(){
  mEnumIter = mOperators.begin();
  return;
}

const bool OperatorGen::EnumEnd(){
  return mEnumIter == mOperators.end();
}

// The format is OPR_ appended with the operator name which is from
// the Operator.mText
const std::string OperatorGen::EnumNextElem(){
  Operator opr = *mEnumIter;
  std::string enum_item = "{";
  enum_item = enum_item + "\"" + opr.mText + "\", OPR_" + FindOperatorName(opr.mID);
  enum_item = enum_item + "}";
  mEnumIter++;
  return enum_item; 
}

/////////////////////////////////////////////////////////////////////
//                Parse the Operator .spec file                   //
//                                                                 //
/////////////////////////////////////////////////////////////////////

// Fill Map
void OperatorGen::ProcessStructData() {
  std::vector<StructBase *>::iterator it = mStructs.begin();
  for (; it != mStructs.end(); it++) {
    // Sort
    StructBase *sb = *it;
    sb->Sort(0);

    for (auto eit: sb->mStructElems) {
      const std::string s(eit->mDataVec[1]->GetString());
      OprId id = FindOperatorId(s);
      AddEntry(eit->mDataVec[0]->GetString(), id);
    }
  }
}

/////////////////////////////////////////////////////////////////////
//                   Generate the output files                     //
/////////////////////////////////////////////////////////////////////

void OperatorGen::Generate() {
  GenHeaderFile();
  GenCppFile();
}

void OperatorGen::GenHeaderFile() {
  mHeaderFile.WriteOneLine("#ifndef __OPERATOR_GEN_H__", 26);
  mHeaderFile.WriteOneLine("#define __OPERATOR_GEN_H__", 26);
  mHeaderFile.WriteOneLine("extern OprTableEntry OprTable[OPR_NA];", 38);
  mHeaderFile.WriteOneLine("#endif", 6);
}

void OperatorGen::GenCppFile() {
  mCppFile.WriteOneLine("#include \"ruletable.h\"", 22);
  TableBuffer tb;
  tb.Generate(this, "OprTableEntry OprTable[] = {");
  mCppFile.WriteFormattedBuffer(&tb);
  mCppFile.WriteOneLine("};", 2);
}

