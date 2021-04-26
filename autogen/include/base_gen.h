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
/////////////////////////////////////////////////////////////////////
// This is the base functions of auto generation.                  //
// Each individual part is supposed to inherit from this class.    //
/////////////////////////////////////////////////////////////////////

#ifndef __BASE_GEN_H__
#define __BASE_GEN_H__

#include <iostream>
#include <fstream>
#include <stack>
#include <unordered_map>

#include "rule.h"
#include "base_struct.h"
#include "file_write.h"

// headers from ../shared/
#include "fileread.h"
#include "stringpool.h"

namespace maplefe {

class MemPool;
class RuleElemPool;
class ReservedGen;
class SPECParser;;

// rule elements can be categorized into simple one and combo one.
// Element set and Concatenation are the ONLY two combo-s supported so far.
//
// What make things complicated is they can be nested into each other, and
// the handling of special input like ')' and '+' can be messy.
//
// To make the status of element clear, we define a stack which indicates
// the current type of element being read. This helps to read ')' and '+'.
//
// Possibly new syntax in the future also need this help.
typedef enum ST_status {
  ST_Set,         // in ( , , )
  ST_Concatenate, // in E + E + E
  ST_Null
}ST_status;

extern FileWriter *gSummaryHFile;    // summary functions
extern FileWriter *gSummaryCppFile;  // summary functions
extern unsigned    gRuleTableNum;  // rule table number
extern std::vector<std::string> gTopRules; // top rule names.

class BaseGen {
public:
  // The buffer for generated rule tables
  FormattedBuffer mRuleTableCpp;
  FormattedBuffer mRuleTableHeader;

  std::string   mSpecFile;
  FileWriter    mHeaderFile;
  FileWriter    mCppFile;
  StringPool   *mStringPool;

  // .spec could support multiple types of comment,
  // starting with certain pattern
  std::vector<const char *> mComments;

  MemPool      *mMemPool;
  RuleElemPool *mRuleElemPool;

  SPECParser   *mParser;
public:
  // *.spec files are free to define multiple rules.
  std::vector<Rule *> mRules;
  Rule *mCurRule;

  // *.spec files are free to define multiple struct.
  std::vector<StructBase *>mStructs;
  StructBase *mCurStruct;

  // AutoGen reserved certain Elements/OPs, handled by class 'ResGen'.
  // There will be only one instance of 'Reserved' shared by IdenGen,
  // OpGen, SepGen, ...
  ReservedGen *mReserved;

  // Only reading of Combo elements will have status recorded.
  // Simple elements don't need status.
  std::stack<ST_status> mStatus;

  bool NewLine;        // If we are already in a new line.

  // Some rule elements need to be back patched.
  std::vector<RuleElem*> mToBePatched;

  // reuse rule elements for string/char.
  std::unordered_map<std::string, RuleElem *> mElemString;
  std::unordered_map<char, RuleElem *> mElemChar;

public:
  Rule *FindRule(const char *name, int len);
  Rule *FindRule(const std::string name);
  Rule *AddLiteralRule(std::string rulename);
  Rule *AddLiteralRule(std::string rulename, char c);
  Rule *AddLiteralRule(std::string rulename, std::string str);

  Rule *NewRule() { return new Rule(); }
  RuleElem *NewRuleElem();
  RuleElem *NewRuleElem(std::string str);
  RuleElem *NewRuleElem(RuleOp op);
  RuleElem *NewRuleElem(Rule *rule);

public:
  BaseGen(const std::string &d, const std::string &h, const std::string &c);
  BaseGen(const std::string &dfile);
  ~BaseGen();

  void SetReserved(ReservedGen *r) { mReserved = r; }

  // The parsing logic is mostly the same for all .spec, so they are not
  // virtual functions.
  void SetParser(SPECParser *p);
  void SetParser(SPECParser *p, const std::string &d);
  void Parse();
  void ParseOneRule();
  void ParseOneStruct();
  void BackPatch(std::vector<BaseGen*>);

  RuleElem *GetOrCreateRuleElemFromChar(const char c, bool getOnly = false);
  RuleElem *GetRuleElemFromChar(const char c) {return GetOrCreateRuleElemFromChar(c, true);}
  RuleElem *GetOrCreateRuleElemFromString(std::string, bool getOnly = false);
  RuleElem *GetRuleElemFromString(std::string str) { return GetOrCreateRuleElemFromString(str, true); }

  virtual void Run(SPECParser *parser);
  virtual void ProcessStructData() {};
  virtual void GenRuleTables();
  virtual void Generate() { return; }

  // Interfaces for dumping Enum structures in gen_xxx.h/cpp
  // Example of using these interfaces is:
  //
  // EnumBegin(structbase);
  // while (!EnumEnd(structbase)) {
  //   const std::string s = EnumNextElem(structbase);
  //   enum_buffer->Addstring(s);
  //   ...
  // }
  virtual const std::string EnumName() {}
  virtual const std::string EnumNextElem() {}
  virtual const void EnumBegin() {}
  virtual const bool EnumEnd() {}
};

}

#endif
