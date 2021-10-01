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

#ifndef __SPEC_PARSER_H__
#define __SPEC_PARSER_H__

#include <iostream>
#include <fstream>
#include <stack>

#include "rule.h"
#include "base_struct.h"
#include "spec_lexer.h"

#include "stringpool.h"

namespace maplefe {

class MemPool;
class RuleElemPool;
class ReservedGen;
class AutoGen;
class BaseGen;

class SPECParser {
public:
  SPECLexer    *mLexer;
  BaseGen      *mBaseGen;
  Rule         *mCurrrule;
  std::string   mFilename;

public:
  SPECParser() { mLexer = new SPECLexer(); }
  SPECParser(const std::string &dfile) : mFilename(dfile) {
    mLexer = new SPECLexer();
    ResetParser(dfile);
  }
  ~SPECParser() { delete mLexer; }

  // for all ParseXXX routines
  // Return true  : succeed
  //        false : failed
  bool Parse();
  void ResetParser(const std::string &dfile);

  bool ParseRule();
  RuleAction *GetAction();
  bool ParseActionFunc(RuleElem *&elem);
  bool ParseElement(RuleElem *&elem, bool allowConcat);
  bool ParseElementSet(RuleElem *parent);
  bool ParseConcatenate(RuleElem *parent);

  bool ParseStruct();
  bool ParseStructElements();
  bool ParseElemData(StructElem *elem);

  bool ParseType();

  bool ParseAttr();
  bool ParseAttrDatatype();
  bool ParseAttrTokentype();
  bool ParseAttrValidity();
  bool ParseAttrProperty();
  bool ParseAttrAction();

  void SetVerbose(int i) { mLexer->SetVerbose(i); }
  int GetVerbose() { return mLexer->GetVerbose(); }

  void ParserError(std::string msg, std::string str);
  void DumpRules();
  void DumpStruct();
  void Dump() { DumpStruct(); DumpRules(); }
};

}

#endif
