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
//////////////////////////////////////////////////////////////////////////
// This file contains all the functions that are used when we traverse the
// rule tables. Most of the functions are designed to do the following
// 1. Validity check
//    e.g. if an identifier is a type name, variable name, ...
// 2. Traverse the rule tables
//////////////////////////////////////////////////////////////////////////

#ifndef __RULE_TABLE_UTIL_H__
#define __RULE_TABLE_UTIL_H__

#include "ruletable.h"
#include "token.h"

namespace maplefe {

// This class contains all the functions that a language could use during parsing.
// The mechansim behind this design has three major parts.
//
//   1. ValidityCheck is the parent class of the children classes, say JavaValidityCheck.
//      ValidityCheck has some common implementations, and children can have their own.
//
//      A language can have its own special syntax checking, e.g., Java Type Argument has
//      wildcard synatx, such as ? extend SomeClass, and we need check if two type
//      arguments are equal, containing, contained, or distinct.
//
//   2. Children's validity checking is language specific, eg Java has a requirement to
//      check if an identifier is a packagename. The package names are specific in each
//      language.
//
//   3. The hard problem is how to invoke the correct functions when a rule is matched.
//      The most straightforward idea is to put the function pointers in the rule table
//      with additional information. However, the signatures of a function makes the idea
//      complicated.
//
//      So I finally decided to have an Id for each check function, and Id is written into
//      the rule tables by Autogen.

class ValidityCheck {
public:
  virtual bool IsPackageName();
  virtual bool IsTypeName();
  virtual bool IsVariable();
};

extern SepId FindSeparator(const char *str, const char c, unsigned &len);
extern OprId FindOperator(const char *str, const char c, unsigned &len);
extern const char* FindKeyword(const char *str, const char c, unsigned &len);

extern bool RuleActionHasElem(RuleTable*, unsigned);
extern bool RuleFindChildIndex(RuleTable *p, RuleTable *c, unsigned&);
extern RuleTable* RuleFindChild(RuleTable *p, unsigned index);

extern bool RuleReachable(RuleTable *from, RuleTable *to);

// If two lookaheads are equal.
extern bool LookAheadEqual(LookAhead, LookAhead);

}
#endif
