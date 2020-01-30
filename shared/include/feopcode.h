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
#ifndef __FEOPCODE_H__
#define __FEOPCODE_H__

#include "tokenkind.h"
#include <iostream>

enum FEOpcode {
#define KEYWORD(N, I, T) FEOP_##I,
#include "keywords.def"
#undef KEYWORD
#define OPKEYWORD(N, I, T) FEOP_##T,
#include "opkeywords.def"
#undef OPKEYWORD
  FEOP_Invalid
};

class FEOPCode {
  FEOpcode  op;

 public:
  FEOPCode() : op(FEOP_Invalid) {}
  FEOPCode(FEOpcode o) : op(o) {}

  std::string GetString(FEOpcode op);
  FEOpcode Token2FEOpcode(TK_Kind tk);
};

#endif
