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

#include "token_table.h"

void TokenTable::Prepare() {
  PrepareOperators();
  PrepareSeparators();
  PrepareKeywords();
}

void TokenTable::PrepareOperators() {
  mNumOperators = mOperators->size();
  std::list<Operator>::iterator it = mOperators->begin();
  for (; it != mOperators->end(); it++) {
    Operator op = *it;
    Token t;
    t.mTkType = TT_OP;
    t.mData.mOprId = op.mID;
    mTokens.push_back(t);
  }
}

void TokenTable::PrepareSeparators() {
  mNumSeparators = mSeparators->size();
  std::list<Separator>::iterator it = mSeparators->begin();
  for (; it != mSeparators->end(); it++) {
    Separator sp = *it;
    Token t;
    t.mTkType = TT_SP;
    t.mData.mSepId = sp.mID;
    mTokens.push_back(t);
  }
}

void TokenTable::PrepareKeywords() {
  mNumKeywords = mKeywords.size();
  for (unsigned i = 0; i < mNumKeywords; i++) {
    Token t;
    t.mTkType = TT_KW;
    t.mData.mName = mKeywords[i].c_str();
    mTokens.push_back(t);
  }
}

TokenTable gTokenTable;

