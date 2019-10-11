
#include "keyword_gen.h"
#include "massert.h"

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
  for (auto it: mStructs) {
    for (auto eit: it->mStructElems) {
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
  std::string s = "extern KeywordTableEntry KeywordTable[";
  s = s + std::to_string(mKeywords.size());
  s = s + "];";
  mHeaderFile.WriteOneLine(s.c_str(), s.size());
  mHeaderFile.WriteOneLine("#endif", 6);
}

void KeywordGen::GenCppFile() {
  mCppFile.WriteOneLine("#include \"ruletable.h\"", 22);
  TableBuffer tb;
  std::string s = "KeywordTableEntry KeywordTable[";
  s = s + std::to_string(mKeywords.size());
  s = s + "] = {";
  tb.Generate(this, s);
  mCppFile.WriteFormattedBuffer(&tb);
  mCppFile.WriteOneLine("};", 2);
}

