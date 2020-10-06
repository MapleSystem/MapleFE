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
#include "token.h"
#include "massert.h"

#undef  SEPARATOR
#define SEPARATOR(N, T) case SEP_##T: return #T;
const char* SeparatorTokenGetName(SepId id) {
  switch (id) {
#include "supported_separators.def"
  default:
    return "NA";
  }
};

void SeparatorTokenDump(SepId id) {
  const char *name = SeparatorTokenGetName(id);
  DUMP1("Separator  Token: ", name);
  return;
}

#undef  OPERATOR
#define OPERATOR(T, D) case OPR_##T: return #T;
const char* OperatorTokenGetName(OprId id) {
  switch (id) {
#include "supported_operators.def"
  default:
    return "NA";
  }
};

void OperatorTokenDump(OprId id) {
  const char *name = OperatorTokenGetName(id);
  DUMP1("Operator   Token: ", name);
  return;
}

void LiteralTokenDump(LitData data) {
  switch (data.mType) {
  case LT_IntegerLiteral:
    DUMP1("Integer Literal Token:", data.mData.mInt);
    break;
  case LT_FPLiteral:
    DUMP1("Floating Literal Token:", data.mData.mFloat);
    break;
  case LT_DoubleLiteral:
    DUMP1("Double Literal Token:", data.mData.mDouble);
    break;
  case LT_BooleanLiteral:
    DUMP1("Boolean Literal Token:", data.mData.mBool);
    break;
  case LT_CharacterLiteral:
    DUMP1("Character Literal Token:", data.mData.mChar);
    break;
  case LT_StringLiteral:
    DUMP1("String Literal Token:", data.mData.mStr);
    break;
  case LT_NullLiteral:
    DUMP0("Null Literal Token:");
    break;
  case LT_NA:
  default:
    DUMP0("NA Token:");
    break;
  }
}

void Token::Dump() {
  switch (mTkType) {
  case TT_SP:
    SeparatorTokenDump(mData.mSepId);
    break;
  case TT_OP:
    OperatorTokenDump(mData.mOprId);
    break;
  case TT_ID:
    DUMP1("Identifier Token: ", mData.mName);
    break;
  case TT_KW:
    DUMP1("Keyword    Token: ", mData.mName);
    break;
  case TT_CM:
    DUMP0("Comment Token: ");
    break;
  case TT_LT:
    LiteralTokenDump(mData.mLitData);
    break;
  default:
    break;
  }
}
