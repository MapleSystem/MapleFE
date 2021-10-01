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

#include "base_gen.h"
#include "reserved_gen.h"
#include "rule_gen.h"
#include "spec_parser.h"

#include "stringpool.h"
#include "mempool.h"
#include "ruleelem_pool.h"
#include "massert.h"
#include "all_supported.h"

namespace maplefe {

//////////////////////////////////////////////////////////////////////////////

void SPECParser::ParserError(std::string msg, std::string str) {
  std::cout << "\n================================= spec file syntax error ============================" << std::endl;
  std::cout << "file " << mFilename << std::endl;
  std::cout << "line " << mLexer->GetLineNum() << std::endl;
  std::cout << mLexer->GetLine() << std::endl;
  if (mLexer->GetCuridx() <= 2) {
    std::cout << "^\n" << std::endl;
  } else {
    std::string s(mLexer->GetCuridx()-1, '-');
    std::cout << s << "^\n" << std::endl;
  }
  MLOC;
  std::cout << msg << " " << str << std::endl;
  std::cout << "=====================================================================================" << std::endl;
  assert(0);
}

// pass spec file
void SPECParser::ResetParser(const std::string &dfile) {
  if (GetVerbose() >= 3)
    MMSG("  >>>> File: ", dfile);
  // init lexer
  mFilename = dfile;
  mLexer->PrepareForFile(dfile);
}

bool SPECParser::Parse() {
  bool atEof = false;
  bool status = true;

  uint32_t lastlnum = 0;
  uint32_t lnum = mLexer->GetLineNum();

  mLexer->NextToken();
  while (!atEof) {
    // print current line
    lnum = mLexer->GetLineNum();
    if (GetVerbose() >= 3 && lastlnum != lnum) {
      lastlnum = lnum;
      MMSG("  >>>> LINE: ", mLexer->GetLine());
    }

    SPECTokenKind tk_prev = mLexer->_thekind;
    SPECTokenKind tk = mLexer->GetToken();
    switch (tk) {
      case SPECTK_Rule:
        status = ParseRule();
        break;
      case SPECTK_Struct:
        status = ParseStruct();
        break;
      case SPECTK_Attr:
        status = ParseAttr();
        break;
      case SPECTK_Eof:
        atEof = true;
        break;
      default:
        ParserError("expect a rule or a struct but get ", mLexer->GetTokenString());
        break;
    }

    if (!status)
      return status;
  }

  if (GetVerbose() >= 3)
    Dump();

  return status;
}

// This function assumes:
// 1) A rule has fixed pattern as:
//      rule NAME : Element
// 2) One element per rule
bool SPECParser::ParseRule() {
  bool status = false;

  // rule
  SPECTokenKind tk = mLexer->GetToken();
  if (tk != SPECTK_Rule)
    ParserError("expect 'rule' but get ", mLexer->GetTokenString());

  // NAME
  tk = mLexer->NextToken();
  std::string name = mLexer->GetTokenString();
  if (tk != SPECTK_Name)
    ParserError("expect a name but get ", name);

  Rule *rule = mBaseGen->FindRule(name);
  if (!rule) {
    rule = mBaseGen->NewRule();
    rule->SetName(mLexer->GetTheName());
    mBaseGen->mRules.push_back(rule);
  }

  // set mCurrrule to be used for attribute parsing
  mCurrrule = rule;

  // :
  tk = mLexer->NextToken();
  if (tk != SPECTK_Colon)
    ParserError("expect ':' but get ", mLexer->GetTokenString());

  // Element
  RuleElem *elem = NULL;
  tk = mLexer->NextToken();
  status = ParseElement(elem, true);
  if (status) {
    rule->SetElement(elem);
  } else {
    ParserError ("Unexpected token: ", mLexer->GetTokenString());
  }

  return status;
}

// This is to read ONE SINGLE element. it could be
//   (1) rule name
//   (2) literal
//   (3) Reserved OP(element)
//   (4) Reserved element
//   (5) concatenation: Elem1 + Elem2
//
bool SPECParser::ParseElement(RuleElem *&elem, bool allowConcat) {
  Rule *rule = NULL;
  bool status = true;

  SPECTokenKind tk = mLexer->GetToken();
  const std::string name = mLexer->GetTheName();
  TypeId t = FindTypeIdLangIndep(name);

  switch (tk) {
    case SPECTK_Name: {
      elem = mBaseGen->NewRuleElem();
      rule = mBaseGen->FindRule(name);
      if (rule) {
        elem->SetRule(rule);
      } else if (t != TY_NA) {
        elem->SetTypeId(t);
      } else {
        const char *str = mBaseGen->mStringPool->FindString(name);
        elem->SetPending(str);
        mBaseGen->mToBePatched.push_back(elem);
        if (GetVerbose() >= 3)
          MMSG("Pending rule: ", name);
      }
      tk = mLexer->NextToken();
      break;
    }
    case SPECTK_Oneof:
      elem = mBaseGen->NewRuleElem(RO_Oneof);
      status = ParseElementSet(elem);
      break;
    case SPECTK_Zeroorone:
      elem = mBaseGen->NewRuleElem(RO_Zeroorone);
      status = ParseElementSet(elem);
      break;
    case SPECTK_Zeroormore:
      elem = mBaseGen->NewRuleElem(RO_Zeroormore);
      status = ParseElementSet(elem);
      break;
    case SPECTK_Char:
    {
      elem = mBaseGen->GetOrCreateRuleElemFromChar(mLexer->thechar);
      mLexer->NextToken();
      break;
    }
    case SPECTK_String:
      elem = mBaseGen->GetOrCreateRuleElemFromString(name);
      mLexer->NextToken();
      break;
    case SPECTK_Actionfunc:
      elem = mBaseGen->NewRuleElem();
      return status;
    case SPECTK_Eof:
      elem = mBaseGen->NewRuleElem();
      break;
    default: {
      ParserError("unexpected token ", mLexer->GetTokenString());
    }
  }

  if (!allowConcat)
    return status;

  // one element is done, check if we have a '+' for concatenation
  tk = mLexer->GetToken();
  if (tk == SPECTK_Concat) {
    RuleElem *sub_elem = elem;
    elem = mBaseGen->NewRuleElem();
    while (sub_elem == elem) {
      ParserError("Should have gotten a new address", name);
      elem = mBaseGen->NewRuleElem();
    }
    elem->SetRuleOp(RO_Concatenate);
    elem->AddSubElem(sub_elem);
    status = ParseConcatenate(elem);
  }

  // check if there is an action
  tk = mLexer->GetToken();
  if (tk == SPECTK_Actionfunc) {
    status = ParseActionFunc(elem);
    if (!status) {
      ParserError ("Error parsing action: ", mLexer->GetTokenString());
    }
  }

  return status;
}

// ==> func funcname (%1, %2, ....)
bool SPECParser::ParseActionFunc(RuleElem *&elem) {
  SPECTokenKind tk = mLexer->GetToken();
  if (tk != SPECTK_Actionfunc)
    ParserError ("expect \"==>\" but get ", mLexer->GetTokenString());

  // func
  tk = mLexer->NextToken();
  if (tk != SPECTK_Func)
    ParserError ("expect func but get ", mLexer->GetTokenString());

  tk = mLexer->NextToken();
  RuleAction *action = GetAction();
  elem->mAttr.AddAction(action);
  return true;
}

// funcname (%1, %2, ....)
RuleAction *SPECParser::GetAction() {
  // funcname
  SPECTokenKind tk = mLexer->GetToken();
  if (tk != SPECTK_Name)
    ParserError ("expect name but get ", mLexer->GetTokenString());
  const std::string str = mLexer->GetTokenString();
  const char *name = mBaseGen->mStringPool->FindString(str);

  RuleAction *action = new RuleAction(name);

  tk = mLexer->NextToken();
  if (tk != SPECTK_Lparen)
    ParserError ("expect '(' but get ", mLexer->GetTokenString());

  tk = mLexer->NextToken();
  while (tk != SPECTK_Rparen) {
    if (tk != SPECTK_Percent) {
      ParserError ("expect '%' but get ", mLexer->GetTokenString());
    }
    tk = mLexer->NextToken();
    if (tk != SPECTK_Intconst) {
      ParserError ("expect a const but get ", mLexer->GetTokenString());
    }
    uint8_t idx = (uint8_t)mLexer->theintval;
    action->mArgs.push_back(idx);
    tk = mLexer->NextToken();
    if (tk == SPECTK_Coma) {
      tk = mLexer->NextToken();
    }
  }
  tk = mLexer->NextToken();

  return action;
}

bool SPECParser::ParseElementSet(RuleElem *elem) {
  SPECTokenKind optk = mLexer->GetToken();
  if (!(optk == SPECTK_Oneof || optk == SPECTK_Zeroorone || optk == SPECTK_Zeroormore))
    ParserError("expect ONEOF/ZEROORONE/ZEROORMORE but get ", mLexer->GetTokenString());

  SPECTokenKind tk = mLexer->NextToken();
  if (tk != SPECTK_Lparen)
    ParserError("expect '(' but get ", mLexer->GetTokenString());

  tk = mLexer->NextToken();
  while (tk != SPECTK_Rparen && tk != SPECTK_Eof) {
    RuleElem *sub_elem = NULL;
    if (ParseElement(sub_elem, true)) {
      elem->AddSubElem(sub_elem);
    } else {
      ParserError("unexpected token ", mLexer->GetTokenString());
    }

    tk = mLexer->GetToken();
    if (optk == SPECTK_Oneof && tk == SPECTK_Coma)
      tk = mLexer->NextToken();
    else if (optk == SPECTK_Zeroorone || optk == SPECTK_Zeroormore)
      // SPECTK_Zeroorone and SPECTK_Zeroormore only allow one element
      break;
  }

  if (tk == SPECTK_Eof)
      return true;

  tk = mLexer->GetToken();
  if (tk != SPECTK_Rparen)
    ParserError("expect ')' but get ", mLexer->GetTokenString());

  mLexer->NextToken();
  return true;
}

bool SPECParser::ParseConcatenate(RuleElem *elem) {
  SPECTokenKind tk = mLexer->GetToken();
  if (tk != SPECTK_Concat)
    ParserError("expect '+' but get ", mLexer->GetTokenString());

  tk = mLexer->NextToken();
  while (tk != SPECTK_Eof) {
    RuleElem *sub_elem = NULL;
    if (ParseElement(sub_elem, false)) {
      elem->AddSubElem(sub_elem);
    } else {
      ParserError("unexpected token ", mLexer->GetTokenString());
    }

    tk = mLexer->GetToken();
    if (tk == SPECTK_Concat)
      tk = mLexer->NextToken();
    else
      break;
  }

  return true;
}

// Struct format:
// STRUCT Name : ( (a1, b1, ...), (a2, b2, ...), ...)
//
bool SPECParser::ParseStruct() {
  // STRUCT
  SPECTokenKind tk = mLexer->GetToken();
  if (tk != SPECTK_Struct)
    ParserError("expect STURCT but get ", mLexer->GetTokenString());

  // NAME
  tk = mLexer->NextToken();
  if (tk != SPECTK_Name)
    ParserError("expect a name but get ", mLexer->GetTokenString());

  mBaseGen->mCurStruct = new StructBase(mLexer->GetTheName().c_str());
  mBaseGen->mStructs.push_back(mBaseGen->mCurStruct);

  // :
  tk = mLexer->NextToken();
  if (tk != SPECTK_Colon)
    ParserError("expect ':' but get ", mLexer->GetTokenString());

  RuleElem *elem = NULL;

  // optional ONEOF(?)
  tk = mLexer->NextToken();
  if (tk == SPECTK_Oneof)
    tk = mLexer->NextToken();

  // leading '('
  tk = mLexer->GetToken();
  if (tk != SPECTK_Lparen)
    ParserError("expect '(' but get ", mLexer->GetTokenString());
  mLexer->NextToken();

  // parse struct elements
  // (a1, b1, ...), (a2, b2, ...), ...
  // ^
  if (!ParseStructElements()) {
    ParserError ("Unexpected token: ", mLexer->GetTokenString());
  }

  // ending ')'
  tk = mLexer->GetToken();
  if (tk != SPECTK_Rparen)
    ParserError("expect ')' but get ", mLexer->GetTokenString());
  mLexer->NextToken();

  return true;
}

// process a tuple (a1, b1, ...)
bool SPECParser::ParseElemData(StructElem *elem) {
  SPECTokenKind tk = mLexer->GetToken();
  if (tk != SPECTK_Lparen)
    ParserError("expect '(' but get ", mLexer->GetTokenString());

  // get all items in the tuple
  tk = mLexer->NextToken();
  while (tk != SPECTK_Rparen) {
    StructData *data = new StructData();
    switch (tk) {
      case SPECTK_String:
        data->SetString(mLexer->GetTheName(), mBaseGen->mStringPool);
        break;
      case SPECTK_Name:
        data->SetName(mLexer->GetTheName(), mBaseGen->mStringPool);
        break;
      case SPECTK_Char:
        data->SetChar(mLexer->thechar);
        break;
      case SPECTK_Intconst:
        data->SetInt(mLexer->theintval);
        break;
      case SPECTK_Floatconst:
        data->SetFloat(mLexer->thefloatval);
        break;
      case SPECTK_Doubleconst:
        data->SetDouble(mLexer->thedoubleval);
        break;
      default:
        ParserError("expect string or name but get ", mLexer->GetTokenString());
        break;
    }

    elem->mDataVec.push_back(data);

    tk = mLexer->NextToken();
    if (tk != SPECTK_Coma && tk != SPECTK_Rparen)
      ParserError("expect ',' or ')' but get ", mLexer->GetTokenString());

    if (tk == SPECTK_Rparen) {
      tk = mLexer->NextToken();
      break;
    } else {
      tk = mLexer->NextToken();
    }
  }

  return true;
}

// (a1, b1, ...), (a2, b2, ...), ...)
// ^                                ^-- terminate
bool SPECParser::ParseStructElements() {
  bool status = false;

  SPECTokenKind tk = mLexer->GetToken();
  while (tk != SPECTK_Rparen) {
    StructElem *elem = new StructElem();
    // (a1, b1, ...)
    // ^
    status = ParseElemData(elem);

    mBaseGen->mCurStruct->mStructElems.push_back(elem);

    // (a1, b1, ...)
    //              ^-- , or )
    tk = mLexer->GetToken();
    if (tk != SPECTK_Coma && tk != SPECTK_Rparen)
      ParserError("expect ',' or ')' but get ", mLexer->GetTokenString());

    if (tk == SPECTK_Rparen)
      break;
    else
      tk = mLexer->NextToken();
  }

  tk = mLexer->GetToken();
  if (tk != SPECTK_Rparen)
    ParserError("expect ')' but get ", mLexer->GetTokenString());

  return status;
}

// attr.xxx
bool SPECParser::ParseAttr() {
  bool status = false;

  SPECTokenKind tk = mLexer->GetToken();
  if (tk != SPECTK_Attr)
    ParserError("expect attr but get ", mLexer->GetTokenString());

  tk = mLexer->NextToken();
  if (tk != SPECTK_Dot)
    ParserError("expect '.' but get ", mLexer->GetTokenString());

  tk = mLexer->NextToken();
  switch (tk) {
    case SPECTK_Datatype:
      status = ParseAttrDatatype();
      break;
    case SPECTK_Tokentype:
      status = ParseAttrTokentype();
      break;
    case SPECTK_Validity:
      status = ParseAttrValidity();
      break;
    case SPECTK_Property:
      status = ParseAttrProperty();
      break;
    case SPECTK_Action:
      status = ParseAttrAction();
      break;
  }
  return status;
}

bool SPECParser::ParseAttrDatatype() {
  bool status = false;

  SPECTokenKind tk = mLexer->GetToken();
  if (tk != SPECTK_Datatype)
    ParserError("expect data type but get ", mLexer->GetTokenString());

  tk = mLexer->NextToken();
  if (tk != SPECTK_Colon)
    ParserError("expect ':' but get ", mLexer->GetTokenString());

  tk = mLexer->NextToken();
  std::string name = mLexer->GetTokenString();

  tk = mLexer->NextToken();
  return true;
}

bool SPECParser::ParseAttrTokentype() {
  bool status = false;

  SPECTokenKind tk = mLexer->GetToken();
  if (tk != SPECTK_Tokentype)
    ParserError("expect token type but get ", mLexer->GetTokenString());

  tk = mLexer->NextToken();
  if (tk != SPECTK_Colon)
    ParserError("expect ':' but get ", mLexer->GetTokenString());

  tk = mLexer->NextToken();
  std::string name = mLexer->GetTokenString();

  tk = mLexer->NextToken();
  return true;
}

bool SPECParser::ParseAttrProperty() {
  bool status = false;

  SPECTokenKind tk = mLexer->GetToken();
  if (tk != SPECTK_Property)
    ParserError("expect token type but get ", mLexer->GetTokenString());

  RuleAttr *attr = &(mCurrrule->mAttr);

  tk = mLexer->NextToken();
  std::vector<RuleAttr *> attrvec;
  if (tk != SPECTK_Dot) {
    attrvec.push_back(attr);
  } else {
    // for specific elements
    tk = mLexer->NextToken();
    if (tk != SPECTK_Percent)
      ParserError("expect '%' but get ", mLexer->GetTokenString());

    while (tk == SPECTK_Percent) {
      tk = mLexer->NextToken();
      if (tk != SPECTK_Intconst)
        ParserError("expect Intconst but get ", mLexer->GetTokenString());
      int i = mLexer->theintval;

      attr = &(mCurrrule->mElement->mSubElems[i-1]->mAttr);
      attrvec.push_back(attr);
      tk = mLexer->NextToken();
      if (tk == SPECTK_Coma)
        tk = mLexer->NextToken();
    }
  }

  if (tk != SPECTK_Colon)
    ParserError("expect ':' but get ", mLexer->GetTokenString());

  while (1) {
    tk = mLexer->NextToken();
    std::string name = mLexer->GetTokenString();
    for (auto attr: attrvec)
      attr->AddProperty(name);
    tk = mLexer->NextToken();
    if (tk != SPECTK_Coma)
      break;
  }

  return true;
}

// attr.validity[.%1[,%3]] : Func(%1, %3)
bool SPECParser::ParseAttrValidity() {
  bool status = false;

  SPECTokenKind tk = mLexer->GetToken();
  if (tk != SPECTK_Validity)
    ParserError("expect validity but get ", mLexer->GetTokenString());

  RuleAttr *attr = &(mCurrrule->mAttr);

  tk = mLexer->NextToken();
  std::vector<RuleAttr *> attrvec;
  if (tk != SPECTK_Dot) {
    attrvec.push_back(attr);
  } else {
    // for specific elements
    tk = mLexer->NextToken();
    if (tk != SPECTK_Percent)
      ParserError("expect '%' but get ", mLexer->GetTokenString());

    while (tk == SPECTK_Percent) {
      tk = mLexer->NextToken();
      if (tk != SPECTK_Intconst)
        ParserError("expect Intconst but get ", mLexer->GetTokenString());
      int i = mLexer->theintval;

      attr = &(mCurrrule->mElement->mSubElems[i-1]->mAttr);
      attrvec.push_back(attr);
      tk = mLexer->NextToken();
      if (tk == SPECTK_Coma)
        tk = mLexer->NextToken();
    }
  }

  if (tk != SPECTK_Colon)
    ParserError("expect ':' but get ", mLexer->GetTokenString());

  while (1) {
    tk = mLexer->NextToken();
    RuleAction *action = GetAction();
    for (auto attr: attrvec)
      attr->AddValidity(action);
    tk = mLexer->GetToken();
    if (tk != SPECTK_Coma)
      break;
  }

  return true;
}

// attr.action[.%1[,%3]] : foo(%1, %3), bar(%1)
bool SPECParser::ParseAttrAction() {
  SPECTokenKind tk = mLexer->GetToken();
  if (tk != SPECTK_Action)
    ParserError("expect action but get ", mLexer->GetTokenString());

  RuleAttr *attr = &(mCurrrule->mAttr);

  tk = mLexer->NextToken();
  std::vector<RuleAttr *> attrvec;
  if (tk != SPECTK_Dot) {
    attrvec.push_back(attr);
  } else {
    // for specific elements
    tk = mLexer->NextToken();
    if (tk != SPECTK_Percent)
      ParserError("expect '%' but get ", mLexer->GetTokenString());

    while (tk == SPECTK_Percent) {
      tk = mLexer->NextToken();
      if (tk != SPECTK_Intconst)
        ParserError("expect Intconst but get ", mLexer->GetTokenString());
      int i = mLexer->theintval;

      attr = &(mCurrrule->mElement->mSubElems[i-1]->mAttr);
      attrvec.push_back(attr);
      tk = mLexer->NextToken();
      if (tk == SPECTK_Coma)
        tk = mLexer->NextToken();
    }
  }

  if (tk != SPECTK_Colon)
    ParserError("expect ':' but get ", mLexer->GetTokenString());

  while (1) {
    tk = mLexer->NextToken();
    RuleAction *action = GetAction();
    for (auto attr: attrvec)
      attr->AddAction(action);
    tk = mLexer->GetToken();
    if (tk != SPECTK_Coma)
      break;
  }

  return true;
}

bool SPECParser::ParseType() {
  SPECTokenKind tk = mLexer->NextToken();
  return true;
}

void SPECParser::DumpStruct() {
  if (mBaseGen->mStructs.size() == 0)
    return;
  std::cout << "\n=========== structs ==========" << std::endl;
  for (auto it: mBaseGen->mStructs)
    it->Dump();
  std::cout << "==================================" << std::endl;
}

void SPECParser::DumpRules() {
  if (mBaseGen->mRules.size() == 0)
    return;
  std::cout << "\n=========== rules ============" << std::endl;
  for (auto it: mBaseGen->mRules)
    if (it->mElement)
      it->Dump();
  std::cout << "==================================" << std::endl;
}

}
