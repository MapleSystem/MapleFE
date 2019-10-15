#include "token.h"
#include "massert.h"

//#undef  KEYWORD
//#define KEYWORD(N,I,T) {#N, KW_ID_##I, T},
//
//#include "keywords.def"
//{"", KW_ID_NULL, KW_UN}
//};

#undef  SEPARATOR
#define SEPARATOR(T) case SEP_##T: return #T;
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

void KeywordToken::Dump() {
  DUMP1("Keyword    Token: ", mName);
  return;
}

void IdentifierToken::Dump() {
  DUMP1("Identifier Token: ", mName);
  return;
}
