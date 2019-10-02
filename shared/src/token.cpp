#include "token.h"

#undef  KEYWORD
#define KEYWORD(N,I,T) {#N, KW_ID_##I, T},

KwInfo KeywordsInAll[] = {
#include "keywords.def"
{"", KW_ID_NULL, KW_UN}
};

