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

FEOpcode FEOPCode::Token2FEOpcode(TokenKind tk) {
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

