#include <cstring>
#include <string>

#include "separator_gen.h"
#include "massert.h"
#include "all_supported.h"

//////////////   Separators supported /////////////////
#undef  SEPARATOR
#define SEPARATOR(S) {#S, SEP_##S},
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
  for (auto it: mStructs) {
    for (auto eit: it->mStructElems) {
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
  mCppFile.WriteOneLine("#include \"temp_table.h\"", 23);
  TableBuffer tb;
  tb.Generate(this, "SepTableEntry SepTable[SEP_NA] = {");
  mCppFile.WriteFormattedBuffer(&tb);
  mCppFile.WriteOneLine("};", 2);
}

