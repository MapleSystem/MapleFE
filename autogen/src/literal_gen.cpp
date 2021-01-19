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
#include <cstring>

#include "literal_gen.h"
#include "massert.h"

namespace maplefe {

void LiteralGen::Generate() {
  GenRuleTables();
  GenHeaderFile();
  GenCppFile();
}

void LiteralGen::GenHeaderFile() {
  mHeaderFile.WriteOneLine("#ifndef __LITERAL_GEN_H__", 25);
  mHeaderFile.WriteOneLine("#define __LITERAL_GEN_H__", 25);
  mHeaderFile.WriteOneLine("#include \"ruletable.h\"", 22);
  mHeaderFile.WriteOneLine("namespace maplefe {", 19);
  mHeaderFile.WriteFormattedBuffer(&mRuleTableHeader);
  mHeaderFile.WriteOneLine("}", 1);
  mHeaderFile.WriteOneLine("#endif", 6);
}

void LiteralGen::GenCppFile() {
  mCppFile.WriteOneLine("#include \"common_header_autogen.h\"", 34);
  mCppFile.WriteOneLine("namespace maplefe {", 19);
  mCppFile.WriteFormattedBuffer(&mRuleTableCpp);
  mCppFile.WriteOneLine("}", 1);
}

}
