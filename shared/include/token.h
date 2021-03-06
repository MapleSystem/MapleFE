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
////////////////////////////////////////////////////////////////////////
// The lexical translation of the characters creates a sequence of tokens.
// tokens |-->identifiers
//        |-->keywords
//        |-->literals
//        |-->separators (whitespace is skipped by lexer)
//        |-->operators
//        |-->comment
//
// This categorization is shared among all languages. [NOTE] If anything
// in a new language is exceptional, please add to this.
//
// This file defines Tokens and all the sub categories
////////////////////////////////////////////////////////////////////////

#ifndef __Token_H__
#define __Token_H__

#include <vector>
#include "char.h"
#include "stringutil.h"
#include "supported.h"

namespace maplefe {

typedef enum {
  TT_ID,    // Identifier
  TT_KW,    // Keyword
  TT_LT,    // Literal
  TT_SP,    // separator
  TT_OP,    // operator
  TT_CM,    // comment
  TT_NA     // N.A.
}TK_Type;

// There are quite a few concerns regarding literals.
//
// 1. One of the concern here is we are using c++ type to store java
//    data which could mess the precision. Need look into it in the future.
//    Possibly will keep the text string of literal and let compiler to decide.
// 2. We also treat 'this' and NULL/null as a literal, see supported_literals.def
// 3. About character literal. We separate unicode character literal from normal
//    Raw Input character.

struct LitData {
  LitId mType;
  union {
    int    mInt;
    float  mFloat;
    double mDouble;
    bool   mBool;
    Char   mChar;
    const char  *mStr;     // the string is allocated in gStringPool
  }mData;
};

// This is about a complicated situation. In some languages, an operator
// could be combination of some other operators. e.g., in Java, operator >>
// can be two continous GT, greater than. It could also happen in other
// types of tokens, like separator.
//
// However, lexer will always take the longest possible match, so >> is always
// treated as ShiftRight, instead of two continuous GT. This causes problem
// during parsing. e.g., List<Class<?>>, it actually is two continuous GT.
//
// To solve this problem, we add additional field in Token to indicate whether
// it could be combinations of other smaller tokens.
//
// We now only handle the simplest case where alternative tokens are the SAME.
// e.g., >> has alternative of two >
//       >>> has alternative of three >
//
// [NOTE] Alt tokens are system tokens.
struct AltToken {
  unsigned mNum;
  unsigned mAltTokenId;
};

struct Token {
  TK_Type mTkType;
  union {
    const char *mName; // Identifier, Keyword. In the gStringPool
    LitData     mLitData;
    SepId       mSepId;
    OprId       mOprId;
  }mData;

  AltToken     *mAltTokens;

  bool IsSeparator()  const { return mTkType == TT_SP; }
  bool IsOperator()   const { return mTkType == TT_OP; }
  bool IsIdentifier() const { return mTkType == TT_ID; }
  bool IsLiteral()    const { return mTkType == TT_LT; }
  bool IsKeyword()    const { return mTkType == TT_KW; }
  bool IsComment()    const { return mTkType == TT_CM; }

  void SetIdentifier(const char *name) {mTkType = TT_ID; mData.mName = name;}
  void SetLiteral(LitData data)        {mTkType = TT_LT; mData.mLitData = data;}

  const char* GetName() const;
  LitData     GetLitData()   const {return mData.mLitData;}
  OprId       GetOprId()     const {return mData.mOprId;}
  SepId       GetSepId()     const {return mData.mSepId;}
  bool        IsWhiteSpace() const {return mData.mSepId == SEP_Whitespace;}
  void Dump();
};

}
#endif
