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
#define OPERATOR(T) case OPR_##T: return #T;
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

//#undef  KEYWORD
//#define KEYWORD(N,I,T) {#N, KW_ID_##I, T},
//
//#include "keywords.def"
//{"", KW_ID_NULL, KW_UN}
//};

void KeywordToken::Dump() {
  DUMP1("Keyword    Token: ", mName);
  return;
}

void IdentifierToken::Dump() {
  DUMP1("Identifier Token: ", mName);
  return;
}
