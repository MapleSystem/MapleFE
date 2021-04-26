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
/////////////////////////////////////////////////////////////////////////
//   This file defines the rules parsed.                               //
/////////////////////////////////////////////////////////////////////////

#ifndef __RULE_H__
#define __RULE_H__

#include <string>
#include <vector>
#include <queue>

#include "all_supported.h"

namespace maplefe {

class Rule;
class StructData;

typedef enum RuleOp {
  RO_Oneof,      // one of (...)
  RO_Zeroormore, // zero or more of (...)
  RO_Zeroorone,  // zero or one ( ... )
  RO_Concatenate,// Elem + Elem + Elem
  RO_Null
} RuleOp;

typedef enum ElemType {
  ET_Char,       // It's a literal elements, char, 'c'.
  ET_String,     // It's a literal elements, string "abc".
  ET_Rule,       // It's rule
  ET_Op,         // It's an Op
  ET_Type,       // It's a AutoGen recoganized type
  ET_Token,      // It's a sytem token including operators, separators, and keywords
  ET_Pending,    // It's a pending element, to be patched in BackPatch().
                 // pending elements are unknown rule names, constituted by
                 // the 26 regular letters.
  ET_NULL
} ElemType;

// an action is the behavior performed for a matched rule
// rule addexpr : ONEOF( expr '+' expr,
//                       expr '-' expr)
//    attr.action.%2 foo(%1, %3)
// where %2 means applys to element #2: "expr '-' expr"
//       %1 and %3 are the sub elements in element #2
// mName  foo
// mArgs  1, 3
class RuleAction {
public:
  const char          *mName;
  std::vector<uint8_t> mArgs;
public:
  RuleAction(){}
  RuleAction(const char *name) : mName(name){}
  ~RuleAction() {}

  void SetName(const char *name) { mName = name; }
  void AddArg(uint8_t idx);

  const char* GetName() { return mName; }
  uint8_t GetArg(uint8_t i) { return mArgs[i]; }

  void Dump();
};

// Attributes for Rule and RuleElem
class RuleAttr {
public:
  std::vector<RuleAction*> mValidity;
  std::vector<RuleAction*> mAction;
  std::vector<std::string> mProperty; // I'm using a std::string for property

  RuleAttr(){}
  ~RuleAttr();

  void AddValidity(RuleAction *a) { mValidity.push_back(a); }
  void AddAction(RuleAction *a) { mAction.push_back(a); }
  void AddProperty(std::string s) { mProperty.push_back(s); }

  bool Empty() { return mValidity.size() == 0 &&
                        mAction.size() == 0 &&
                        mProperty.size() == 0; }

  void DumpDataType(int i);
  void DumpTokenType(int i);
  void DumpValidity(int i);
  void DumpAction(int i);
};

// [NOTE] RuleElem is allocated in a pool, through placement new operation.
//        So, Don't inherit RuleElem, because we are calling its destructor
//        directly, i.e. e->~RuleElem(). children destructor won't be
//        called at all.
//
// The element in a rule.
// It can be a nested structures, e.g.
//    RO_ONEOF ( RO_PLAIN, RO_PLAIN, RO_ONEOF (...))
// The element contained in another element is called sub-element
//
class RuleElem {
public:
  ElemType mType;          // type
  RuleAttr mAttr;
  union {
    RuleOp      mOp;       // It could be a OP
    Rule       *mRule;     // It could be another defined rule
                           // Here only put the base type: Rule
    char        mChar;
    const char *mString;   // Pending elem. string is NULL ended in string pool
    TypeId      mTypeId;
    unsigned    mTokenId;  // index of system token.
  } mData;

  std::vector<RuleElem *> mSubElems;  // Sub elements. It's empty if mType
                                      // is ET_Rule;
public:
  RuleElem() {}
  RuleElem(Rule *rule) : mType(ET_Rule) { mData.mRule = rule; }
  ~RuleElem();

  void SetRuleOp(RuleOp op)     {mType = ET_Op; mData.mOp = op;}
  void SetRule(Rule *r)         {mType = ET_Rule; mData.mRule = r;}
  void SetChar(char c)          {mType = ET_Char; mData.mChar = c;}
  void SetString(const char *s) {mType = ET_String; mData.mString = s;}
  void SetTypeId(TypeId t)      {mType = ET_Type; mData.mTypeId = t;}
  void SetTokenId(unsigned t)   {mType = ET_Token; mData.mTokenId = t;}
  void SetPending(const char *s){mType = ET_Pending; mData.mString = s;}

  const char *GetPendingName() {return mData.mString;}
  const char *GetElemTypeName();
  const char *GetRuleOpName();

  void AddSubElem(RuleElem *e) {mSubElems.push_back(e);}

  bool IsIdentifier();
  bool IsLeaf();

  void Dump(bool newline = false);
  void DumpAttr();
  void EmitAction();
};

// a rule can contain multiple elements with actions
class Rule {
public:
  std::string mName;
  RuleElem         *mElement;
  RuleAttr          mAttr;
public:
  Rule(const std::string &s) : mName(s), mElement(NULL) {}
  Rule() : mElement(NULL) {}
  ~Rule();

  void SetName(const std::string &n) {mName = n;}
  void SetElement(RuleElem *e)       {mElement = e;}
  void Dump();
  void DumpAttr();
};

}

#endif
