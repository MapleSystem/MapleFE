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

/////////////////////////////////////////////////////////////////////
//                   Generate the output files                     //
/////////////////////////////////////////////////////////////////////

void TokenGen::Generate() {
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
  mHeaderFile.WriteOneLine("extern unsigned gSystemTokensNum;", 33);
  mHeaderFile.WriteOneLine("extern unsigned gOperatorTokensNum;", 35);
  mHeaderFile.WriteOneLine("extern unsigned gSeparatorTokensNum;", 36);
  mHeaderFile.WriteOneLine("extern unsigned gKeywordTokensNum;", 34);
  mHeaderFile.WriteOneLine("extern Token gSystemTokens[];", 29);
  mHeaderFile.WriteOneLine("#endif", 6);
}

extern std::string FindOperatorName(OprId id);

void TokenGen::GenCppFile() {
  mCppFile.WriteOneLine("#include \"token.h\"", 18);

  // write:
  //   unsigned gSystemTokensNum=xxx;
  //   unsigned gOperatorTokensNum=xxx;
  //   unsigned gSeparatorTokensNum=xxx;
  //   unsigned gKeywordTokensNum=xxx;
  std::string s = "unsigned gSystemTokensNum=";
  std::string num = std::to_string(gTokenTable.mTokens.size());
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

  std::list<Operator>::iterator oit = gTokenTable.mOperators->begin();
  for (unsigned index = 0; oit != gTokenTable.mOperators->end(); oit++, index++) {
    std::string output = "  {.mTkType = TT_OP, {.mOprId = ";
    Operator opr = *oit;
    std::string opr_name = "OPR_";
    opr_name += FindOperatorName(opr.mID);
    output += opr_name;
    output += "}},";
    mCppFile.WriteOneLine(output.c_str(), output.size());
  }

  std::list<Separator>::iterator sit = gTokenTable.mSeparators->begin();
  for (unsigned index = 0; sit != gTokenTable.mSeparators->end(); sit++, index++) {
    std::string output = "  {.mTkType = TT_SP, {.mSepId = ";
    Separator sep = *sit;
    std::string sep_name = "SEP_";
    sep_name += FindSeparatorName(sep.mID);
    output += sep_name;
    output += "}},";
    mCppFile.WriteOneLine(output.c_str(), output.size());
  }

  unsigned kw_size = gTokenTable.mKeywords.size();
  for (unsigned index = 0; index < kw_size; index++) {
    std::string output = "  {.mTkType = TT_KW, {.mName = ";
    std::string keyword = gTokenTable.mKeywords[index];
    output += "\"";
    output += keyword;
    output += "\"";
    output += "}},";
    mCppFile.WriteOneLine(output.c_str(), output.size());
  }

  // Write the comment token
  std::string output = "  {.mTkType = TT_CM}";
  mCppFile.WriteOneLine(output.c_str(), output.size());
  mCppFile.WriteOneLine("};", 2);
}

