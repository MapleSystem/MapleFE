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
  for (auto it: mStructs) {
    for (auto eit: it->mStructElems) {
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

