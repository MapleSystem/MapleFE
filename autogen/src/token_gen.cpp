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
#include <cstring>
#include <string>

#include "token_gen.h"
#include "token_table.h"
#include "massert.h"

namespace maplefe {

/////////////////////////////////////////////////////////////////////
//                   Generate the output files                     //
/////////////////////////////////////////////////////////////////////

void TokenGen::Generate() {
  ProcessAltTokens();
  GenHeaderFile();
  GenCppFile();
}

// For the header file, we are gonna generate
//
// extern unsigned gSystemTokensNum;
// extern Token gSystemTokens[];

void TokenGen::GenHeaderFile() {
  mHeaderFile.WriteOneLine("#ifndef __TOKEN_GEN_H__", 23);
  mHeaderFile.WriteOneLine("#define __TOKEN_GEN_H__", 23);
  mHeaderFile.WriteOneLine("#include \"token.h\"", 18);
  mHeaderFile.WriteOneLine("namespace maplefe {", 19);
  mHeaderFile.WriteOneLine("extern unsigned gSystemTokensNum;", 33);
  mHeaderFile.WriteOneLine("extern unsigned gOperatorTokensNum;", 35);
  mHeaderFile.WriteOneLine("extern unsigned gSeparatorTokensNum;", 36);
  mHeaderFile.WriteOneLine("extern unsigned gKeywordTokensNum;", 34);
  mHeaderFile.WriteOneLine("extern Token gSystemTokens[];", 29);
  mHeaderFile.WriteOneLine("extern unsigned gAltTokensNum;", 30);
  mHeaderFile.WriteOneLine("extern AltToken gAltTokens[];", 29);
  mHeaderFile.WriteOneLine("}", 1);
  mHeaderFile.WriteOneLine("#endif", 6);
}

extern std::string FindOperatorName(OprId id);

static AlternativeToken alt_tokens[] = {
#include "alt_tokens.spec"
};

// Replace the names in alt_tokens to index, and save them in
// mAltTokens.
void TokenGen::ProcessAltTokens() {
  unsigned mAltTokensNum = sizeof(alt_tokens) / sizeof(AlternativeToken);
  for (unsigned i = 0; i < mAltTokensNum; i++) {
    AlternativeToken at = alt_tokens[i];
    unsigned orig_id;
    bool found = gTokenTable.FindStringTokenId(at.mName, orig_id);
    MASSERT(found);
    unsigned alt_id;
    found = gTokenTable.FindStringTokenId(at.mAltName, alt_id);
    MASSERT(found);

    ProcessedAltToken pat;
    pat.mId = orig_id;
    pat.mNum = at.mNum;
    pat.mAltId = alt_id;
    mAltTokens.push_back(pat);
  }
}

void TokenGen::GenCppFile() {
  mCppFile.WriteOneLine("#include \"token.h\"", 18);
  mCppFile.WriteOneLine("namespace maplefe {", 19);

  // Write alt tokens
  //   unsigned gAltTokensNum=xxx;
  //   AltToken gAltTokens[xxx] = {
  //     {2, 40},
  //     {...  },
  //   };
  std::string s = "unsigned gAltTokensNum=";
  std::string num = std::to_string(mAltTokens.size());
  s += num;
  s += ";";
  mCppFile.WriteOneLine(s.c_str(), s.size());

  s = "AltToken gAltTokens[";
  s += num;
  s += "] = {";
  mCppFile.WriteOneLine(s.c_str(), s.size());

  for (unsigned i = 0; i < mAltTokens.size(); i++) {
    ProcessedAltToken pat = mAltTokens[i];
    s = "  {";
    num = std::to_string(pat.mNum);
    s += num;
    s += ", ";
    num = std::to_string(pat.mAltId);
    s += num;
    s += "},";
    mCppFile.WriteOneLine(s.c_str(), s.size());
  }

  s = "};";
  mCppFile.WriteOneLine(s.c_str(), s.size());

  // write:
  //   unsigned gSystemTokensNum=xxx;
  //   unsigned gOperatorTokensNum=xxx;
  //   unsigned gSeparatorTokensNum=xxx;
  //   unsigned gKeywordTokensNum=xxx;
  s = "unsigned gSystemTokensNum=";
  num = std::to_string(gTokenTable.mTokens.size());
  s += num;
  s += ";";
  mCppFile.WriteOneLine(s.c_str(), s.size());

  s = "unsigned gOperatorTokensNum=";
  num = std::to_string(gTokenTable.mNumOperators);
  s += num;
  s += ";";
  mCppFile.WriteOneLine(s.c_str(), s.size());

  s = "unsigned gSeparatorTokensNum=";
  num = std::to_string(gTokenTable.mNumSeparators);
  s += num;
  s += ";";
  mCppFile.WriteOneLine(s.c_str(), s.size());

  s = "unsigned gKeywordTokensNum=";
  num = std::to_string(gTokenTable.mNumKeywords);
  s += num;
  s += ";";
  mCppFile.WriteOneLine(s.c_str(), s.size());

  // write:
  //   Token gSystemTokens[] = {
  s = "Token gSystemTokens[] = {";
  mCppFile.WriteOneLine(s.c_str(), s.size());

  // write all tokens

  unsigned overall_index = 0;

  std::list<Operator>::iterator oit = gTokenTable.mOperators->begin();
  for (; oit != gTokenTable.mOperators->end(); oit++, overall_index++) {
    std::string output = "  {.mTkType = TT_OP, {.mOprId = ";
    Operator opr = *oit;
    std::string opr_name = "OPR_";
    opr_name += FindOperatorName(opr.mID);
    output += opr_name;

    // find the alt tokens
    unsigned at_idx = 0;
    bool found = false;
    for (; at_idx < mAltTokens.size(); at_idx++) {
      ProcessedAltToken pat = mAltTokens[at_idx];
      if (overall_index == pat.mId) {
        found = true;
        break;
      }
    }

    if (found) {
      output += "}, .mAltTokens = &gAltTokens[";
      std::string idx_str = std::to_string(at_idx);
      output += idx_str;
      output += "]},";
    } else {
      output += "}, .mAltTokens = NULL},";
    }

    mCppFile.WriteOneLine(output.c_str(), output.size());
  }

  std::list<Separator>::iterator sit = gTokenTable.mSeparators->begin();
  for (; sit != gTokenTable.mSeparators->end(); sit++, overall_index++) {
    std::string output = "  {.mTkType = TT_SP, {.mSepId = ";
    Separator sep = *sit;
    std::string sep_name = "SEP_";
    sep_name += FindSeparatorName(sep.mID);
    output += sep_name;

    // find the alt tokens
    unsigned at_idx = 0;
    bool found = false;
    for (; at_idx < mAltTokens.size(); at_idx++) {
      ProcessedAltToken pat = mAltTokens[at_idx];
      if (overall_index == pat.mId) {
        found = true;
        break;
      }
    }

    if (found) {
      output += "}, .mAltTokens = &gAltTokens[";
      std::string idx_str = std::to_string(at_idx);
      output += idx_str;
      output += "]},";
    } else {
      output += "}, .mAltTokens = NULL},";
    }

    mCppFile.WriteOneLine(output.c_str(), output.size());
  }

  unsigned kw_size = gTokenTable.mKeywords.size();
  for (unsigned index = 0; index < kw_size; index++, overall_index++) {
    std::string output = "  {.mTkType = TT_KW, {.mName = ";
    std::string keyword = gTokenTable.mKeywords[index];
    output += "\"";
    output += keyword;
    output += "\"";

    // find the alt tokens
    unsigned at_idx = 0;
    bool found = false;
    for (; at_idx < mAltTokens.size(); at_idx++) {
      ProcessedAltToken pat = mAltTokens[at_idx];
      if (overall_index == pat.mId) {
        found = true;
        break;
      }
    }

    if (found) {
      output += "}, .mAltTokens = &gAltTokens[";
      std::string idx_str = std::to_string(at_idx);
      output += idx_str;
      output += "]},";
    } else {
      output += "}, .mAltTokens = NULL},";
    }

    mCppFile.WriteOneLine(output.c_str(), output.size());
  }

  // Write the comment token
  std::string output = "  {.mTkType = TT_CM, {.mName = NULL}, .mAltTokens = NULL}";
  mCppFile.WriteOneLine(output.c_str(), output.size());
  mCppFile.WriteOneLine("};", 2);
  mCppFile.WriteOneLine("}", 1);
}
}


