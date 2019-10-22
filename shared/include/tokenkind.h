////////////////////////////////////////////////////////////////////////
// This file defines TokenKind
////////////////////////////////////////////////////////////////////////

#ifndef __TokenKind_H__
#define __TokenKind_H__

struct EnumHash {
  template <typename T>
    int operator()(T t) const {
      return static_cast<int>(t);
    }
};

typedef enum {
  // non-keywords
#define TOKEN(N,T) TK_##T,
  #include "tokens.def"
#undef TOKEN
  // keywords
#define KEYWORD(N,I,T) TK_##I,
  #include "keywords.def"
#undef KEYWORD
#define OPKEYWORD(N,I,T) TK_##T,
  #include "opkeywords.def"
#undef OPKEYWORD
#define SEPARATOR(N,T) TK_##T,
  #include "supported_separators.def"
#undef SEPARATOR
} TK_Kind;
#endif
