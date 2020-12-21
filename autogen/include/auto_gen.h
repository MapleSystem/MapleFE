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
//////////////////////////////////////////////////////////////////////////
//                          class AutoGen                               //
//////////////////////////////////////////////////////////////////////////

#ifndef __AUTO_GEN_H_
#define __AUTO_GEN_H_

#include <vector>

#include "reserved_gen.h"
#include "iden_gen.h"
#include "literal_gen.h"
#include "type_gen.h"
#include "block_gen.h"
#include "separator_gen.h"
#include "operator_gen.h"
#include "expr_gen.h"
#include "stmt_gen.h"
#include "keyword_gen.h"
#include "attr_gen.h"
#include "token_gen.h"

class AutoGen {
private:
  IdenGen      *mIdenGen;
  ReservedGen  *mReservedGen;
  LiteralGen   *mLitGen;
  TypeGen      *mTypeGen;
  AttrGen      *mAttrGen;
  BlockGen     *mBlockGen;
  SeparatorGen *mSeparatorGen;
  OperatorGen  *mOperatorGen;
  KeywordGen   *mKeywordGen;
  ExprGen      *mExprGen;
  StmtGen      *mStmtGen;
  TokenGen     *mTokenGen;

  std::vector<BaseGen*> mGenArray;
  std::vector<Rule*>    mTopRules;
  SPECParser   *mParser;

public:
  AutoGen(SPECParser *p) : mParser(p) {}
  ~AutoGen();

  void Init();
  void Run();
  void BackPatch();
  void CollectTopRules(){}
  void Gen();
};

#endif
