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
/////////////////////////////////////////////////////////////////////////////////
//            Language Specific Implementations                                //
/////////////////////////////////////////////////////////////////////////////////

#ifndef __LANG_SPEC_H__
#define __LANG_SPEC_H__

#include "stringutil.h"
#include "token.h"
#include "lexer.h"
#include "parser.h"

namespace maplefe {

class StringToValueImpl : public StringToValue {
public:
  float  StringToFloat(std::string &s);
  double StringToDouble(std::string &s);
  bool   StringToBool(std::string &s);
  Char   StringToChar(std::string &s);
  bool   StringIsNull(std::string &s);
};

extern LitData ProcessLiteral(LitId type, const char *str);

////////////////////////////////////////////////////////////////////////////////////
//                              Typescript Lexer
////////////////////////////////////////////////////////////////////////////////////

class TypescriptLexer : public Lexer {
public:
  bool CharIsSeparator(const char);
  void AppendSemiColon();

  TempLitData* GetTempLit();
  bool FindNextTLFormat(unsigned start, std::string& s, unsigned& end);
  bool FindNextTLPlaceHolder(unsigned start, std::string& s, unsigned& end);
};

////////////////////////////////////////////////////////////////////////////////////
//                              Typescript Parser
////////////////////////////////////////////////////////////////////////////////////

class TypescriptParser : public Parser {
public:
  TypescriptParser(const char *f) : Parser(f) {}
  Token* GetRegExpr(Token *t);
};

}
#endif
