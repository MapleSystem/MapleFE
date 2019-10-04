#include "block_gen.h"

void BlockGen::Generate() {
  GenRuleTables();
  GenHeaderFile();
  GenCppFile();
}

void BlockGen::GenHeaderFile() {
  mHeaderFile.WriteOneLine("#ifndef __BLOCK_GEN_H__", 23);
  mHeaderFile.WriteOneLine("#define __BLOCK_GEN_H__", 23);

  // generate the rule tables
  mHeaderFile.WriteFormattedBuffer(&mRuleTableHeader);

  mHeaderFile.WriteOneLine("#endif", 6);
}

void BlockGen::GenCppFile() {
  mCppFile.WriteOneLine("#include \"common_header_autogen.h\"", 34);
  // generate the rule tables
  mCppFile.WriteFormattedBuffer(&mRuleTableCpp);
}

