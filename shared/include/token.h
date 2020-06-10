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
// The lexical translation of the characters creates a sequence of input
// elements -----> white-space (whitespace is finally put into separator)
//            |--> comments (finally treated as a token)
//            |--> tokens |-->identifiers
//                        |-->keywords
//                        |-->literals
//                        |-->separators (including whitespace)
//                        |-->operators
//                        |-->comment
//
// This categorization is shared among all languages. [NOTE] If anything
// in a new language is exceptional, please add to this.
//
// This file defines Tokens and all the sub categories
////////////////////////////////////////////////////////////////////////

#ifndef __Token_H__
#define __Token_H__

#include <vector>
#include "element.h"
#include "stringutil.h"
#include "supported.h"

// TokenText contains the text data of each token, no matter it's a
// number or string. TokenText will be further processed by utility
// functions before creating a real token.
typedef std::vector<std::string> TokenText;

typedef enum {
  TT_ID,    // Identifier
  TT_KW,    // Keyword
  TT_LT,    // Literal
  TT_SP,    // separator
  TT_OP,    // operator
  TT_CM,    // comment
  TT_NA     // N.A.
}TK_Type;

class Token : public Element {
public:
  TK_Type mTkType;
public:
  Token(TK_Type t) : mTkType(t), Element(ET_TK) {}
  Token() : Element(ET_TK) {}

  void SetTkType(TK_Type t) { mTkType = t; }

  bool IsSeparator()  { return mTkType == TT_SP; }
  bool IsOperator()   { return mTkType == TT_OP; }
  bool IsIdentifier() { return mTkType == TT_ID; }
  bool IsLiteral()    { return mTkType == TT_LT; }
  bool IsKeyword()    { return mTkType == TT_KW; }
  bool IsComment()    { return mTkType == TT_CM; }

  virtual const char* GetName(){return "null";}
  virtual void Dump() {}
};

////////////////////////////////////////////////////////////////////////
//                       IdentifierToken                              //
////////////////////////////////////////////////////////////////////////
class IdentifierToken : public Token {
public:
  const char *mName;       // It's put into the Lexer's StringPool
public:
  IdentifierToken(const char *s) : mName(s) {mTkType = TT_ID;}
  IdentifierToken() : mName(NULL) {mTkType = TT_ID;}

  const char* GetName() {return mName;}
  void Dump();
};

////////////////////////////////////////////////////////////////////////
//                            Keywod                                  //
// Keywords fall into different categories:
//   1. data type, including class and interface
//   2. control flow
//   3. attributes, for primitive and Object data
////////////////////////////////////////////////////////////////////////
//
typedef enum {
  KW_DT,     // data type, class, enum, interfaces, are all considered types
  KW_FN,     // Intrinsic call, system call, ...
  KW_CT,     // control flow keyword,
  KW_DA,     // data attribute,
  KW_FA,     // function attribute
  KW_OA,     // object(class,enum) attribute
  KW_SP,     // Language Sepcific
  KW_UN      // Undefined
}KW_Type;

class KeywordToken : public Token {
public:
  const char *mName;   // The text name. During initialization it will be
                       // put into string pool.
public:
  KeywordToken(const char *s) : Token(TT_KW), mName(s){}
  
  void SetName(const char *s){ mName = s; }
  const char *GetName(){ return mName; }
  void Dump();
};

////////////////////////////////////////////////////////////////////////
//                       LiteralToken                                 //
////////////////////////////////////////////////////////////////////////

struct LitData {
  LitId mType;
  union {
    int   mInt;
    float mFloat;
    double mDouble;
    bool  mBool;
    char  mChar;
    char *mStr;   // the string is allocated through lexer's mStringPool
  } mData;

  LitData() {}
  LitData(LitId t) : mType(t) {}
};

class LiteralToken : public Token {
private:
  LitData  mData;
public:
  LiteralToken(LitData data) : Token(TT_LT), mData(data) {}

  LitData GetLitData() {return mData;}
  void Dump();
};

////////////////////////////////////////////////////////////////////////
//                       SeparatorToken                               //
////////////////////////////////////////////////////////////////////////

class SeparatorToken : public Token {
public:
  SepId mSepId;
public:
  SeparatorToken(SepId si) {mTkType = TT_SP; mSepId = si;}
  bool IsWhiteSpace() {return mSepId == SEP_Whitespace;}
  const char* GetName();
  void Dump();
};

////////////////////////////////////////////////////////////////////////
//                              Operator                              //
////////////////////////////////////////////////////////////////////////
class OperatorToken : public Token {
public:
  OprId mOprId;
public:
  OperatorToken(OprId oi) {mTkType = TT_OP; mOprId = oi;}
  const char* GetName();
  void Dump();
};

////////////////////////////////////////////////////////////////////////
//                              Comment                               //
// we dont' want to put any additional information for comments.      //
// The comment string is discarded.                                   //
////////////////////////////////////////////////////////////////////////
class CommentToken : public Token {
public:
  CommentToken() {mTkType = TT_CM;}
  const char* GetName() {return "comment";}
  void Dump();
};

#endif
