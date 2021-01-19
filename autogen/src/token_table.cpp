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

namespace maplefe {

void TokenTable::Prepare() {
  PrepareOperators();
  PrepareSeparators();
  PrepareKeywords();
  // Need a comment token in the end
  Token t;
  t.mTkType = TT_CM;
  mTokens.push_back(t);
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

// Returns true if found 'c' as a token, and set id to token id.
bool TokenTable::FindCharTokenId(char c, unsigned &id) {
  id = 0;
  std::list<Operator>::iterator oit = mOperators->begin();
  for (; oit != mOperators->end(); oit++, id++) {
    Operator opr = *oit;
    const char *syntax = opr.mText;
    if ((strlen(syntax) == 1) && (*syntax == c))
      return true;
  }

  std::list<Separator>::iterator sit = mSeparators->begin();
  for (; sit != mSeparators->end(); sit++, id++) {
    Separator sep = *sit;
    const char *syntax = sep.mKeyword;
    if ((strlen(syntax) == 1) && (*syntax == c))
      return true;
  }

  std::vector<std::string>::iterator kit = mKeywords.begin();
  for (; kit != mKeywords.end(); kit++, id++) {
    std::string keyword = *kit;
    if (keyword.size() == 1 && keyword[0] == c)
      return true;
  }

  return false;
}

bool TokenTable::FindStringTokenId(const char *str, unsigned &id) {
  id = 0;
  std::list<Operator>::iterator oit = mOperators->begin();
  for (; oit != mOperators->end(); oit++, id++) {
    Operator opr = *oit;
    const char *syntax = opr.mText;
    if (strlen(syntax) == strlen(str) &&
        strncmp(syntax, str, strlen(str)) == 0)
      return true;
  }

  std::list<Separator>::iterator sit = mSeparators->begin();
  for (; sit != mSeparators->end(); sit++, id++) {
    Separator sep = *sit;
    const char *syntax = sep.mKeyword;
    if (strlen(syntax) == strlen(str) &&
        strncmp(syntax, str, strlen(str)) == 0)
      return true;
  }

  std::vector<std::string>::iterator kit = mKeywords.begin();
  for (; kit != mKeywords.end(); kit++, id++) {
    std::string keyword = *kit;
    if (keyword.size() == strlen(str) &&
        strncmp(keyword.c_str(), str, strlen(str)) == 0)
      return true;
  }

  return false;
}

TokenTable gTokenTable;
}


