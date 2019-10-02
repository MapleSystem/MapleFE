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
  FEOpcode Token2FEOpcode(TokenKind tk);
};

#endif
