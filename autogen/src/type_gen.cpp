#include <cstring>

#include "type_gen.h"
#include "massert.h"
#include "all_supported.h"

// Fill Map
void TypeGen::ProcessStructData() {
  for (auto it: mStructs) {
    for (auto eit: it->mStructElems) {
      std::string s(eit->mDataVec[1]->GetString());
      AGTypeId id = FindAGTypeIdLangIndep(s);
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

  // generate the keyword table
  mHeaderFile.WriteOneLine("extern TypeKeyword TypeKeywordTable[TY_NA];", 43);

  // generate the rule tables
  mHeaderFile.WriteFormattedBuffer(&mRuleTableHeader);
  mHeaderFile.WriteOneLine("#endif", 6);
}

void TypeGen::GenCppFile() {
  mCppFile.WriteOneLine("#include \"common_header_autogen.h\"", 34);

  // generate the keyword table
  TableBuffer tb;
  tb.Generate(this, "TypeKeyword TypeKeywordTable[TY_NA] = {");
  mCppFile.WriteFormattedBuffer(&tb);
  mCppFile.WriteOneLine("};", 2);

  // generate the rule tables
  mCppFile.WriteFormattedBuffer(&mRuleTableCpp);
}

