#include "expr_gen.h"

void ExprGen::Generate() {
  GenRuleTables();
  GenHeaderFile();
  GenCppFile();
}

void ExprGen::GenHeaderFile() {
  mHeaderFile.WriteOneLine("#ifndef __EXPR_GEN_H__", 22);
  mHeaderFile.WriteOneLine("#define __EXPR_GEN_H__", 22);

  // generate the rule tables
  mHeaderFile.WriteFormattedBuffer(&mRuleTableHeader);

  mHeaderFile.WriteOneLine("#endif", 6);
}

void ExprGen::GenCppFile() {
  mCppFile.WriteOneLine("#include \"common_header_autogen.h\"", 34);

  // generate the rule tables
  mCppFile.WriteFormattedBuffer(&mRuleTableCpp);
}

