#include "iden_gen.h"

// Needs to override BaseGen::Run, since we dont need process
// STRUCT data
void IdenGen::Run(SPECParser *parser) {
  SetParser(parser);
  Parse();
}

/////////////////////////////////////////////////////////////////////
//                   Generate the output files                     //
/////////////////////////////////////////////////////////////////////

void IdenGen::Generate() {
  GenRuleTables();
  GenHeaderFile();
  GenCppFile();
}

void IdenGen::GenHeaderFile() {
  mHeaderFile.WriteOneLine("#ifndef __IDEN_GEN_H__", 22);
  mHeaderFile.WriteOneLine("#define __IDEN_GEN_H__", 22);
  mHeaderFile.WriteOneLine("#include \"temp_table.h\"", 23);
  mHeaderFile.WriteFormattedBuffer(&mRuleTableHeader);
  mHeaderFile.WriteOneLine("#endif", 6);
}

void IdenGen::GenCppFile() {
  mCppFile.WriteOneLine("#include \"common_header_autogen.h\"", 34);
  mCppFile.WriteFormattedBuffer(&mRuleTableCpp);
}

