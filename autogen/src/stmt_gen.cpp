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
#include "stmt_gen.h"

namespace maplefe {

void StmtGen::Generate() {
  GenRuleTables();
  GenHeaderFile();
  GenCppFile();
}

void StmtGen::GenHeaderFile() {
  mHeaderFile.WriteOneLine("#ifndef __STMT_GEN_H__", 22);
  mHeaderFile.WriteOneLine("#define __STMT_GEN_H__", 22);
  mHeaderFile.WriteOneLine("namespace maplefe {", 19);

  // generate the rule tables
  mHeaderFile.WriteFormattedBuffer(&mRuleTableHeader);

  mHeaderFile.WriteOneLine("}", 1);
  mHeaderFile.WriteOneLine("#endif", 6);
}

void StmtGen::GenCppFile() {
  mCppFile.WriteOneLine("#include \"common_header_autogen.h\"", 34);
  mCppFile.WriteOneLine("namespace maplefe {", 19);

  // generate the rule tables
  mCppFile.WriteFormattedBuffer(&mRuleTableCpp);
  mCppFile.WriteOneLine("}", 1);
}
}


