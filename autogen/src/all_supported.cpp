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
#include "all_supported.h"

//////////////   Types supported /////////////////
#undef  TYPE
#define TYPE(T) {#T, TY_##T},
TypeMapping TypesSupported[TY_NA] = {
#include "supported_types.def"
};

TypeId FindTypeIdLangIndep(const std::string &s) {
  for (unsigned u = 0; u < TY_NA; u++) {
    if (!TypesSupported[u].mName.compare(0, s.length(), s))
      return TypesSupported[u].mType;
  }
  return TY_NA;
}

#undef  TYPE
#define TYPE(T) case TY_##T: return #T;
char *GetTypeString(TypeId tid) {
  switch (tid) {
#include "supported_types.def"
  default:
    return "NA";
  }
};

//////////////   literals supported /////////////////

#undef  LITERAL
#define LITERAL(S) {#S, LIT_##S},
LiteralSuppStruct LiteralsSupported[LIT_NA] = {
#include "supported_literals.def"
};

// s : the name 
// return the LiteralId
LiteralId FindLiteralId(const std::string &s) {
  for (unsigned u = 0; u < LIT_NA; u++) {
    if (!LiteralsSupported[u].mName.compare(0, s.length(), s))
      return LiteralsSupported[u].mLiteralId;
  }
  return LIT_NA;
}

std::string FindLiteralName(LiteralId id) {
  std::string s("");
  for (unsigned u = 0; u < LIT_NA; u++) {
    if (LiteralsSupported[u].mLiteralId == id){
      s = LiteralsSupported[u].mName;
      break;
    }
  }
  return s;
}
