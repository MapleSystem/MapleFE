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
#include <vector>
#include <cstring>

#include "reserved_gen.h"
#include "spec_parser.h"
#include "auto_gen.h"
#include "base_gen.h"
#include "massert.h"
#include "token_table.h"

namespace maplefe {

///////////////////////////////////////////////////////////////////////////////////////
//                       Summary functions during parsing
// During matching phase in the language parser, we often need to print the stack of
// rule table traversing in order to easily find the bug cause. So we need a set of
// functions to help dump the information. As an example, a major request is to dump the
// table name when we step into a new rule table. This request a mapping from the ruletable
// address to the table name.
//
// We generate two files gen_summary.h and gen_summary.cpp to include all the functions we
// may need. These two files are generated by autogen. Don't modify them.
//
// 1. For the rule table name mapping
//
//    In header file, we need
//
//      typedef struct {
//        const RuleTable *mAddr;
//        const char      *mName;
//      }RuleTableSummary;
//      extern RuleTableSummary gRuleTableSummarys[];
//      extern unsigned RuleTableNum;
//      extern std::vector<unsigned> gFailed[621];
//
//    In Cpp file, we need
//
//    #include "gen_summary.h"
//    unsigned RuleTableNum;
//    RuleTableSummary gRuleTableSummarys[] = {
//      {&TblLiteral, "TblLiteral"},
//      ...
//    };
///////////////////////////////////////////////////////////////////////////////////////

FileWriter *gSummaryHFile;
FileWriter *gSummaryCppFile;
unsigned    gRuleTableNum;
std::vector<std::string> gTopRules;

static void WriteSummaryHFile() {
  gSummaryHFile->WriteOneLine("#ifndef __DEBUG_GEN_H__", 23);
  gSummaryHFile->WriteOneLine("#define __DEBUG_GEN_H__", 23);
  gSummaryHFile->WriteOneLine("#include \"ruletable.h\"", 22);
  gSummaryHFile->WriteOneLine("#include \"succ_match.h\"", 23);
  gSummaryHFile->WriteOneLine("#include <vector>", 17);
  gSummaryHFile->WriteOneLine("namespace maplefe {", 19);
  gSummaryHFile->WriteOneLine("typedef struct {", 16);
  gSummaryHFile->WriteOneLine("  const RuleTable *mAddr;", 25);
  gSummaryHFile->WriteOneLine("  const char      *mName;", 25);
  gSummaryHFile->WriteOneLine("  unsigned         mIndex;", 26);
  gSummaryHFile->WriteOneLine("}RuleTableSummary;", 18);
  gSummaryHFile->WriteOneLine("extern RuleTableSummary gRuleTableSummarys[];", 45);
  gSummaryHFile->WriteOneLine("extern unsigned RuleTableNum;", 29);
  gSummaryHFile->WriteOneLine("extern const char* GetRuleTableName(const RuleTable*);", 54);

  std::string s = "extern std::vector<unsigned> gFailed[";
  s += std::to_string(gRuleTableNum);
  s += "];";
  gSummaryHFile->WriteOneLine(s.c_str(), s.size());

  // Write SuccMatch array
  s = "class SuccMatch;";
  gSummaryHFile->WriteOneLine(s.c_str(), s.size());

  s = "extern SuccMatch gSucc[";
  s += std::to_string(gRuleTableNum);
  s += "];";
  gSummaryHFile->WriteOneLine(s.c_str(), s.size());

  // Write Top rules
  s = "extern unsigned gTopRulesNum;";
  gSummaryHFile->WriteOneLine(s.c_str(), s.size());

  s = "extern RuleTable* gTopRules[";
  s += std::to_string(gTopRules.size());
  s += "];";
  gSummaryHFile->WriteOneLine(s.c_str(), s.size());

  gSummaryHFile->WriteOneLine("}", 1);
  gSummaryHFile->WriteOneLine("#endif", 6);
}

// write the beginning part of summary file
static void PrepareSummaryCppFile() {
  gSummaryCppFile->WriteOneLine("#include \"gen_summary.h\"", 24);
  gSummaryCppFile->WriteOneLine("#include \"common_header_autogen.h\"", 34);
  gSummaryCppFile->WriteOneLine("namespace maplefe {", 19);
  gSummaryCppFile->WriteOneLine("RuleTableSummary gRuleTableSummarys[] = {", 41);
}

// write the ending part of summary file
static void FinishSummaryCppFile() {
  gSummaryCppFile->WriteOneLine("};", 2);
  std::string num = std::to_string(gRuleTableNum);
  std::string num_line = "unsigned RuleTableNum = ";
  num_line += num;
  num_line += ";";
  gSummaryCppFile->WriteOneLine(num_line.c_str(), num_line.size());
  gSummaryCppFile->WriteOneLine("const char* GetRuleTableName(const RuleTable* addr) {", 53);
  gSummaryCppFile->WriteOneLine("  for (unsigned i = 0; i < RuleTableNum; i++) {", 47);
  gSummaryCppFile->WriteOneLine("    RuleTableSummary summary = gRuleTableSummarys[i];", 53);
  gSummaryCppFile->WriteOneLine("    if (summary.mAddr == addr)", 30);
  gSummaryCppFile->WriteOneLine("      return summary.mName;", 27);
  gSummaryCppFile->WriteOneLine("  }", 3);
  gSummaryCppFile->WriteOneLine("  return NULL;", 14);
  gSummaryCppFile->WriteOneLine("}", 1);

  std::string s = "std::vector<unsigned> gFailed[";
  s += std::to_string(gRuleTableNum);
  s += "];";
  gSummaryCppFile->WriteOneLine(s.c_str(), s.size());

  s = "SuccMatch gSucc[";
  s += std::to_string(gRuleTableNum);
  s += "];";
  gSummaryCppFile->WriteOneLine(s.c_str(), s.size());

  s = "unsigned gTopRulesNum = ";
  s += std::to_string(gTopRules.size());
  s += ";";
  gSummaryCppFile->WriteOneLine(s.c_str(), s.size());

  s = "RuleTable* gTopRules[";
  s += std::to_string(gTopRules.size());
  s += "] = {";
  unsigned i = 0;
  for (; i < gTopRules.size(); i++) {
    s += "&";
    s += gTopRules[i];
    if (i < gTopRules.size() - 1)
      s += ",";
  }
  s += "};";
  gSummaryCppFile->WriteOneLine(s.c_str(), s.size());
  gSummaryCppFile->WriteOneLine("}", 1);
}

///////////////////////////////////////////////////////////////////////////////////////
//
//                The major implementation  of Autogen
//
///////////////////////////////////////////////////////////////////////////////////////

void AutoGen::Init() {
  std::string lang_path_header("../../java/include/");
  std::string lang_path_cpp("../../java/src/");

  std::string summary_file_name = lang_path_cpp + "gen_summary.cpp";
  gSummaryCppFile = new FileWriter(summary_file_name);
  summary_file_name = lang_path_header + "gen_summary.h";
  gSummaryHFile = new FileWriter(summary_file_name);
  gRuleTableNum = 0;

  PrepareSummaryCppFile();

  std::string hFile = lang_path_header + "gen_reserved.h";
  std::string cppFile = lang_path_cpp + "gen_reserved.cpp";
  mReservedGen = new ReservedGen("reserved.spec", hFile.c_str(), cppFile.c_str());
  mReservedGen->SetReserved(mReservedGen);
  mGenArray.push_back(mReservedGen);

  hFile = lang_path_header + "gen_iden.h";
  cppFile = lang_path_cpp + "gen_iden.cpp";
  mIdenGen  = new IdenGen("identifier.spec", hFile.c_str(), cppFile.c_str());
  mIdenGen->SetReserved(mReservedGen);
  mGenArray.push_back(mIdenGen);

  hFile = lang_path_header + "gen_literal.h";
  cppFile = lang_path_cpp + "gen_literal.cpp";
  mLitGen  = new LiteralGen("literal.spec", hFile.c_str(), cppFile.c_str());
  mLitGen->SetReserved(mReservedGen);
  mGenArray.push_back(mLitGen);

  hFile = lang_path_header + "gen_type.h";
  cppFile = lang_path_cpp + "gen_type.cpp";
  mTypeGen  = new TypeGen("type.spec", hFile.c_str(), cppFile.c_str());
  mTypeGen->SetReserved(mReservedGen);
  mGenArray.push_back(mTypeGen);

  hFile = lang_path_header + "gen_attr.h";
  cppFile = lang_path_cpp + "gen_attr.cpp";
  mAttrGen  = new AttrGen("attr.spec", hFile.c_str(), cppFile.c_str());
  mAttrGen->SetReserved(mReservedGen);
  mGenArray.push_back(mAttrGen);

  hFile = lang_path_header + "gen_block.h";
  cppFile = lang_path_cpp + "gen_block.cpp";
  mBlockGen  = new BlockGen("block.spec", hFile.c_str(), cppFile.c_str());
  mBlockGen->SetReserved(mReservedGen);
  mGenArray.push_back(mBlockGen);

  hFile = lang_path_header + "gen_separator.h";
  cppFile = lang_path_cpp + "gen_separator.cpp";
  mSeparatorGen  = new SeparatorGen("separator.spec", hFile.c_str(), cppFile.c_str());
  mSeparatorGen->SetReserved(mReservedGen);
  mGenArray.push_back(mSeparatorGen);

  hFile = lang_path_header + "gen_operator.h";
  cppFile = lang_path_cpp + "gen_operator.cpp";
  mOperatorGen  = new OperatorGen("operator.spec", hFile.c_str(), cppFile.c_str());
  mOperatorGen->SetReserved(mReservedGen);
  mGenArray.push_back(mOperatorGen);

  hFile = lang_path_header + "gen_keyword.h";
  cppFile = lang_path_cpp + "gen_keyword.cpp";
  mKeywordGen  = new KeywordGen("keyword.spec", hFile.c_str(), cppFile.c_str());
  mKeywordGen->SetReserved(mReservedGen);
  mGenArray.push_back(mKeywordGen);

  hFile = lang_path_header + "gen_expr.h";
  cppFile = lang_path_cpp + "gen_expr.cpp";
  mExprGen  = new ExprGen("expr.spec", hFile.c_str(), cppFile.c_str());
  mExprGen->SetReserved(mReservedGen);
  mGenArray.push_back(mExprGen);

  hFile = lang_path_header + "gen_stmt.h";
  cppFile = lang_path_cpp + "gen_stmt.cpp";
  mStmtGen  = new StmtGen("stmt.spec", hFile.c_str(), cppFile.c_str());
  mStmtGen->SetReserved(mReservedGen);
  mGenArray.push_back(mStmtGen);

  hFile = lang_path_header + "gen_token.h";
  cppFile = lang_path_cpp + "gen_token.cpp";
  mTokenGen  = new TokenGen(hFile.c_str(), cppFile.c_str());
  mTokenGen->SetReserved(mReservedGen);
  mGenArray.push_back(mTokenGen);
}

AutoGen::~AutoGen() {
  std::vector<BaseGen*>::iterator it = mGenArray.begin();
  for (; it != mGenArray.end(); it++){
    BaseGen *gen = *it;
    delete gen;
  }

  if (gSummaryHFile)
    delete gSummaryHFile;
  if (gSummaryCppFile)
    delete gSummaryCppFile;
}

// When parsing a rule, its elements could be rules in the future rules, or
// it could be in another XxxGen. We simply put Pending for these elements.
//
// BackPatch() is the one coming back to solve these Pending. It has to do
// a traversal to solve one by one.
void AutoGen::BackPatch() {
  std::vector<BaseGen*>::iterator it = mGenArray.begin();
  for (; it != mGenArray.end(); it++){
    BaseGen *gen = *it;
    gen->BackPatch(mGenArray);
  }
}

void AutoGen::Run() {
  std::vector<BaseGen*>::iterator it = mGenArray.begin();
  for (; it != mGenArray.end(); it++){
    BaseGen *gen = *it;
    gen->Run(mParser);
  }
}

void AutoGen::Gen() {
  Init();
  Run();
  BackPatch();

  // Prepare the tokens in all rules, by replacing all keyword, operator,
  // separators with tokens. All rules will contain index to the
  // system token.
  gTokenTable.mOperators = &(mOperatorGen->mOperators);
  gTokenTable.mSeparators = &(mSeparatorGen->mSeparators);
  gTokenTable.mKeywords = mKeywordGen->mKeywords;
  gTokenTable.Prepare();

  std::vector<BaseGen*>::iterator it = mGenArray.begin();
  for (; it != mGenArray.end(); it++){
    BaseGen *gen = *it;
    gen->Generate();
  }

  WriteSummaryHFile();
  FinishSummaryCppFile();
}

}
