#include "stmt_gen.h"

void StmtGen::Generate() {
  GenRuleTables();
  GenHeaderFile();
  GenCppFile();
}

void StmtGen::GenHeaderFile() {
  mHeaderFile.WriteOneLine("#ifndef __STMT_GEN_H__", 22);
  mHeaderFile.WriteOneLine("#define __STMT_GEN_H__", 22);

  // generate the rule tables
  mHeaderFile.WriteFormattedBuffer(&mRuleTableHeader);

  mHeaderFile.WriteOneLine("#endif", 6);
}

void StmtGen::GenCppFile() {
  mCppFile.WriteOneLine("#include \"common_header_autogen.h\"", 34);

  // generate the rule tables
  mCppFile.WriteFormattedBuffer(&mRuleTableCpp);
}

