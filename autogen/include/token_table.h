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

#ifndef __TOKEN_TABLE_H__
#define __TOKEN_TABLE_H__

#include "token.h"
#include "operator_gen.h"
#include "separator_gen.h"

namespace maplefe {

class TokenTable {
public:
  std::vector<Token> mTokens;

  // We will organize tokens in the order of: operators, separators and keywords.
  // This order should be kept in (1) token table generation in autogen
  //                              (2) token table searching in Lexer/Parser
  unsigned mNumOperators;
  unsigned mNumSeparators;
  unsigned mNumKeywords;

  std::list<Operator>      *mOperators;
  std::list<Separator>     *mSeparators;
  std::vector<std::string>  mKeywords;   // using an object instead of pointer
                                         // to save the string data.

public:
  TokenTable(){}
  ~TokenTable(){}

  void Prepare();
  void PrepareOperators();
  void PrepareSeparators();
  void PrepareKeywords();

  bool FindCharTokenId(char c, unsigned&);
  bool FindStringTokenId(const char *s, unsigned&);
};

extern TokenTable gTokenTable;
}

#endif
