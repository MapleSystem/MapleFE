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
