/*
* Copyright (C) [2020] Futurewei Technologies, Inc. All rights reverved.
*
* OpenArkFE is licensed under the Mulan PSL v1.
* You can use this software according to the terms and conditions of the Mulan PSL v1.
* You may obtain a copy of Mulan PSL v1 at:
*
*  http://license.coscl.org.cn/MulanPSL
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR
* FIT FOR A PARTICULAR PURPOSE.
* See the Mulan PSL v1 for more details.
*/
#ifndef __RULE_GEN_H__
#define __RULE_GEN_H__

#include "rule.h"
#include "buffer2write.h"

/////////////////////////////////////////////////////////////////////////
//                     Rule Generation                                 //
// A rule is dumped as an TableEntry. See shared/include/ruletable.h  //
// All tables are defined as global in .cpp file. Their extern declaration
// is in .h file.                                                      //
/////////////////////////////////////////////////////////////////////////

class RuleGen {
private:
  const Rule      *mRule;
  FormattedBuffer *mCppBuffer;
  FormattedBuffer *mHeaderBuffer;

private:
  unsigned mSubTblNum;

  std::string GetTblName(const Rule*);
  std::string GetSubTblName();
  std::string GetEntryTypeName(ElemType, RuleOp);

  std::string Gen4RuleElem(const RuleElem*);
  std::string Gen4TableData(const RuleElem*);

  // RuleAttr can be either in Rule or RuleElem in a Rule
  // This is why there are two parameters, but only one of them will be used.
  void Gen4RuleAttr(std::string rule_table_name, RuleAttr *attr);

  // These are the two major interfaces used by Generate().
  // There are two scenarios they are called:
  //   1. Generate for the current Rule from the .spec
  //   2. Generate for the RuleElem in the Rule, aka. Sub Table.
  // This is why there are two parameters, but only one of them will be used.
  void Gen4Table(const Rule *, const RuleElem*);       // table def in .cpp
  void Gen4TableHeader(const std::string &tablename); // table decl in .h
  void GenDebug(const std::string& tablename);   // Gen debug functions

public:
  RuleGen(const Rule *r, FormattedBuffer *hbuf, FormattedBuffer *cbuf)
    : mRule(r), mHeaderBuffer(hbuf), mCppBuffer(cbuf), mSubTblNum(0) {}
  ~RuleGen() {}

  void Generate(); 
};

#endif
