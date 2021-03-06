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
#include <iostream>
#include <string>
#include <cstring>

#include "spec_parser.h"
#include "base_gen.h"
#include "reserved_gen.h"
#include "rule_gen.h"

#include "stringpool.h"
#include "mempool.h"
#include "ruleelem_pool.h"
#include "massert.h"

namespace maplefe {

// The delimiter when parsing rule elements, during identifier generation
static const char *delimiter = " ()+,";

// The comment syntax in identifer.spec
static const char *CommentSyntax = "#";

// Rule name is valid iff it's constituted by the 26 letters.
static bool IsValidRuleName(const std::string &s) {
  for (int i = 0; i < s.size(); i++) {
    if ((s[i] >= 'a' && s[i] <= 'z') || (s[i] >= 'A' && s[i] <= 'Z'))
      continue;
    else
      return false;
  }
  return true;
}

//////////////////////////////////////////////////////////////////////////////

BaseGen::BaseGen(const std::string &dfile,
                 const std::string &hfile,
                 const std::string &cfile)
          : mSpecFile(dfile), mHeaderFile(hfile), mCppFile(cfile) {
  mStringPool = new StringPool();

  mComments.push_back(CommentSyntax);

  mMemPool = new MemPool();
  mRuleElemPool = new RuleElemPool(mMemPool);

  NewLine = false;
  mStatus.push(ST_Null);  // init mStatus
}

// only one def file. This is for ReservedGen
BaseGen::BaseGen(const std::string &dfile) : mSpecFile(dfile)  {
  mStringPool = new StringPool();

  mComments.push_back(CommentSyntax);

  mMemPool = new MemPool();
  mRuleElemPool = new RuleElemPool(mMemPool);

  NewLine = false;
}

BaseGen::~BaseGen() {
  delete mStringPool;
  delete mRuleElemPool;
  delete mMemPool;
}

Rule *BaseGen::AddLiteralRule(std::string rulename) {
  Rule *rule = NULL;
  if (rule = FindRule(rulename)) {
    return rule;
  }

  rule = NewRule();
  rule->SetName(rulename);
  mRules.push_back(rule);

  RuleElem *elem = GetOrCreateRuleElemFromString(rulename);
  rule->mElement = elem;
  mElemString[rulename] = elem;
  return rule;
}

Rule *BaseGen::AddLiteralRule(std::string rulename, char c) {
  Rule *rule = AddLiteralRule(rulename);
  RuleElem *elem = GetOrCreateRuleElemFromChar(c);
  rule->mElement = elem;
  mElemString[rulename] = elem;
  mElemChar[c] = rule->mElement;
  return rule;
}

Rule *BaseGen::AddLiteralRule(std::string rulename, std::string str) {
  Rule *rule = AddLiteralRule(rulename);
  RuleElem *elem = GetOrCreateRuleElemFromString(str);
  rule->mElement = elem;
  mElemString[rulename] = elem;
  mElemString[str] = rule->mElement;
  return rule;
}

// Find a defined rule 'name'.
// Return it if found, or NULL if not found.
Rule* BaseGen::FindRule(const std::string name) {
  Rule *ret = NULL;
  // search in mReserved first
  if (mReserved && mReserved != this) {
    ret = mReserved->FindRule(name);
  }
  if (ret)
    return ret;

  std::vector<Rule *>::iterator it = mRules.begin();
  for (; it != mRules.end(); it++) {
    Rule *r = *it;
    if (r->mName == name) {
      ret = r;
      break;
    }
  }
  return ret;
}

// Allocate a RuleElem from the pool
RuleElem* BaseGen::NewRuleElem() {
  return mRuleElemPool->NewRuleElem();
}

RuleElem* BaseGen::NewRuleElem(RuleOp op) {
  RuleElem *elem = mRuleElemPool->NewRuleElem();
  elem->SetRuleOp(op);
  return elem;
}

RuleElem* BaseGen::NewRuleElem(Rule *rule) {
  RuleElem *elem = mRuleElemPool->NewRuleElem();
  elem->SetRule(rule);
  return elem;
}

RuleElem* BaseGen::NewRuleElem(std::string str) {
  RuleElem *elem = mRuleElemPool->NewRuleElem();
  const char *name = mStringPool->FindString(str);
  elem->SetString(name);
  return elem;
}

////////////////////////////////////////////////////////////////////////
//              Major work flow functions                             //
////////////////////////////////////////////////////////////////////////

// make the parser to be used for the spec file
void BaseGen::SetParser(SPECParser *parser) {
  mParser = parser;
  mParser->mBaseGen = this;

  // set up to use the given spec file
  mParser->ResetParser(mSpecFile);
}

void BaseGen::SetParser(SPECParser *parser, const std::string &dfile) {
  mSpecFile = dfile;
  SetParser(parser);
}


void BaseGen::Parse() {
  mParser->Parse();
}

// BackPatch all the un-finished items.
void BaseGen::BackPatch(std::vector<BaseGen*> vec) {
  if (mToBePatched.size() == 0)
    return;
  for (auto eit: mToBePatched) {
    const char *name = eit->GetPendingName();
    // go through all XxxGen to find the rule. However, most of the time
    // the pending rule is in the same XxxGen. So we try itself first.
    Rule *r = FindRule(name);
    if (!r) {
      for (auto g: vec) {
        r = g->FindRule(name);
        if (r) {
          break;
        }
      }
    }

    if (r) {
      eit->SetRule(r);
      //  std::cout << "BackPatched " << name << std::endl;
    } else {
      MMSG("Pending Rule element cannot be patched: ", name);
    }
  }
}

void BaseGen::Run(SPECParser *parser) {
  SetParser(parser);
  Parse();
  ProcessStructData();
}

// After the parsing of rules, this function generates the rule tables
void BaseGen::GenRuleTables() {
  std::vector<Rule *>::iterator it;
  for (it = mRules.begin(); it != mRules.end(); it++) {
    Rule *rule = *it;
    RuleGen gen(rule, &mRuleTableHeader, &mRuleTableCpp);
    gen.Generate();
  }
}

RuleElem *BaseGen::GetOrCreateRuleElemFromChar(const char c, bool getOnly) {
  if (mElemChar.find(c) != mElemChar.end()) {
    return mElemChar[c];
  }

  if (getOnly) {
    return NULL;
  }

  RuleElem *elem = NewRuleElem();
  elem->SetChar(c);
  mElemChar[c] = elem;
  return elem;
}

//
RuleElem *BaseGen::GetOrCreateRuleElemFromString(std::string str, bool getOnly) {
  if (mElemString.find(str) != mElemString.end()) {
    return mElemString[str];
  }

  if (getOnly) {
    return NULL;
  }

  RuleElem *elem = NewRuleElem();
  const char *newstr = mStringPool->FindString(str);
  elem->SetString(newstr);
  mElemString[str] = elem;

  return elem;
}
}


