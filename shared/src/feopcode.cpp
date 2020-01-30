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
#include "feopcode.h"

std::string FEOPCode::GetString(FEOpcode op) {
  std::string str("FEOP_");
  switch (op) {
    default:
    str += "Invalid";
    break;
#define KEYWORD(N, I, T)    \
    case FEOP_##I: {        \
      str += #I;            \
      break;                \
    }
#include "keywords.def"
#undef KEYWORD
#define OPKEYWORD(N, I, T)  \
    case FEOP_##T: {        \
      str += #T;            \
      break;                \
    }
#include "opkeywords.def"
#undef OPKEYWORD
  }
  return str;
}

FEOpcode FEOPCode::Token2FEOpcode(TK_Kind tk) {
  FEOpcode op = FEOP_Invalid;
  switch (tk) {
    default:
      break;
#define KEYWORD(N, I, T)    \
    case TK_##I: {          \
      op = FEOP_##I;        \
      break;                \
    }
#include "keywords.def"
#undef KEYWORD
#define OPKEYWORD(N, I, T)  \
    case TK_##T: {          \
      op = FEOP_##T;        \
      break;                \
    }
#include "opkeywords.def"
#undef OPKEYWORD
  }
  return op;
}

