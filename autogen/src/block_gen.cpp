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

