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
#include "token.h"
#include "massert.h"

#undef  SEPARATOR
#define SEPARATOR(N, T) case SEP_##T: return #T;
const char* SeparatorToken::GetName() {
  switch (mSepId) {
#include "supported_separators.def"
  default:
    return "NA";
  }
};

void SeparatorToken::Dump() {
  const char *name = GetName();
  DUMP1("Separator  Token: ", name);
  return;
}

#undef  OPERATOR
#define OPERATOR(T, D) case OPR_##T: return #T;
const char* OperatorToken::GetName() {
  switch (mOprId) {
#include "supported_operators.def"
  default:
    return "NA";
  }
};

void OperatorToken::Dump() {
  const char *name = GetName();
  DUMP1("Operator   Token: ", name);
  return;
}

void KeywordToken::Dump() {
  DUMP1("Keyword    Token: ", mName);
  return;
}

void IdentifierToken::Dump() {
  DUMP1("Identifier Token: ", mName);
  return;
}

void CommentToken::Dump() {
  DUMP0("Comment Token: ");
  return;
}

void LiteralToken::Dump() {
  switch (mData.mType) {
  case LT_IntegerLiteral:
    DUMP1("Integer Literal Token:", mData.mData.mInt);
    break;
  case LT_FPLiteral:
    if (mData.mIsDouble)
      DUMP1("Double Literal Token:", mData.mData.mDouble);
    else
      DUMP1("Floating Literal Token:", mData.mData.mFloat);
    break;
  case LT_BooleanLiteral:
    DUMP1("Boolean Literal Token:", mData.mData.mBool);
    break;
  case LT_CharacterLiteral:
    DUMP1("Character Literal Token:", mData.mData.mChar);
    break;
  case LT_StringLiteral:
    DUMP1("String Literal Token:", mData.mData.mStr);
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
