#include <vector>
#include <cstring>

#include "reserved_gen.h"
#include "spec_parser.h"
#include "auto_gen.h"
#include "massert.h"

void AutoGen::Init() {
  std::string lang_path_header("../../java/include/");
  std::string lang_path_cpp("../../java/src/");

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

  hFile = lang_path_header + "gen_local_var.h";
  cppFile = lang_path_cpp + "gen_local_var.cpp";
  mLocalvarGen  = new LocalvarGen("local_var.spec", hFile.c_str(), cppFile.c_str());
  mLocalvarGen->SetReserved(mReservedGen);
  mGenArray.push_back(mLocalvarGen);

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
}

AutoGen::~AutoGen() {
  if (mReservedGen)
    delete mReservedGen;
  if (mIdenGen)
    delete mIdenGen;
  if (mLitGen)
    delete mLitGen;
  if (mTypeGen)
    delete mTypeGen;
  if (mLocalvarGen)
    delete mLocalvarGen;
  if (mBlockGen)
    delete mBlockGen;
  if (mSeparatorGen)
    delete mSeparatorGen;
  if (mOperatorGen)
    delete mOperatorGen;
  if (mKeywordGen)
    delete mKeywordGen;
  if (mExprGen)
    delete mExprGen;
}

// When parsing a rule, its elements could be rules in the future rules, or
// it could be in another XxxGen. We simply put Pending for these elements.
//
// BackPatch() is the one coming back to solve these Pending. It has to do
// a traversal to solve one by one.
void AutoGen::BackPatch() {
  std::vector<BaseGen*>::iterator it = mGenArray.begin();
  //  std::cout << "mGenArray.size " << mGenArray.size() << std::endl;
  for (; it != mGenArray.end(); it++){
    BaseGen *gen = *it;
    gen->BackPatch(mGenArray);
  }
}

void AutoGen::Run() {
  mReservedGen->Run(mParser);
  mIdenGen->Run(mParser);
  mLitGen->Run(mParser);
  mTypeGen->Run(mParser);

  mLocalvarGen->Run(mParser);
  mBlockGen->Run(mParser);
  mSeparatorGen->Run(mParser);
  mOperatorGen->Run(mParser);
  mKeywordGen->Run(mParser);
  mExprGen->Run(mParser);
}

void AutoGen::Gen() {
  Init();
  Run();
  BackPatch();

  mReservedGen->Generate();
  mIdenGen->Generate();
  mLitGen->Generate();
  mTypeGen->Generate();

  mLocalvarGen->Generate();
  mBlockGen->Generate();
  mSeparatorGen->Generate();
  mOperatorGen->Generate();
  mKeywordGen->Generate();
  mExprGen->Generate();
}

