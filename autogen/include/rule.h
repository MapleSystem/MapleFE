/////////////////////////////////////////////////////////////////////////
//   This file defines the rules parsed.                               //
/////////////////////////////////////////////////////////////////////////

#ifndef __RULE_H__
#define __RULE_H__

#include <string>
#include <vector>
#include <queue>

#include "feopcode.h"
#include "all_supported.h"

class Rule;
class TypeRule;

typedef enum {
  RO_Oneof,      // one of (...)
  RO_Zeroormore, // zero or more of (...)
  RO_Zeroorone,  // zero or one ( ... )
  RO_Concatenate,// Elem + Elem + Elem
  RO_Null
}RuleOp;

typedef enum {
  ET_Char,       // It's a literal elements, char, 'c'.
  ET_String,     // It's a literal elements, string "abc".
  ET_Rule,       // It's rule
  ET_Op,         // It's an Op
  ET_Type,       // It's a AutoGen recoganized type
  ET_Token,      // It's a source language Token, updated later
  ET_Pending,    // It's a pending element, to be patched in BackPatch().
                 // pending elements are unknown rule names, constituted by
                 // the 26 regular letters.
  ET_NULL
}ElemType;

// an action is the behavior performed for a matched rule
// rule addexpr : ONEOF( expr '+' expr,
//                       expr '-' expr)
//    attr.action.%2 foo(%1, %3)
// where %2 means applys to element #2: "expr '-' expr"
//       %1 and %3 are the sub elements in element #2
// mName  foo
// mArgs  1, 3
// mOp    OP_Add
class RuleAction {
public:
  const char          *mName;
  std::vector<uint8_t> mArgs;
  FEOpcode             mOpcode;
public:
  RuleAction(){}
  RuleAction(const char *name) : mName(name), mOpcode(FEOP_Invalid) {}
  RuleAction(const char *name, FEOpcode op) : mName(name), mOpcode(op) {}
  ~RuleAction();

  void SetName(const char *name) { mName = name; }
  void AddArg(uint8_t idx) { mArgs.push_back(idx); }

  const char *GetName() { return mName; } const char *GetArg(uint8_t i) { return mArgs[i]; }

  void Dump();
};

// Attributes for Rule and RuleElem
class RuleAttr {
public:
  TypeRule *mType;
  std::vector<RuleAction*> mValidity;
  std::vector<RuleAction*> mAction;

  RuleAttr() : mType(NULL) {}
  ~RuleAttr() { delete mType; mValidity.clear(); mAction.clear(); }

  void SetType(TypeRule *r) { mType = r; }
  void AddValidity(RuleAction *a) { mValidity.push_back(a); }
  void AddAction(RuleAction *a) { mValidity.push_back(a); }

  void DumpType(int i);
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
  union {
    RuleOp      mOp;       // It could be a OP
    Rule       *mRule;     // It could be another defined rule
                           // Here only put the base type: Rule
    char        mChar;
    const char *mString;   // Pending elem. string is NULL ended in string pool
    AGTypeId    mTypeId;
  }mData;

  TK_Kind       mToken;    // record the token for rule like '(' ')' '[' ']' ';' ...
  std::vector<RuleElem *> mSubElems;  // Sub elements. It's empty if mType
                                      // is ET_Rule;
  RuleAttr     *mAttr;
public:
  RuleElem() { mAttr = new RuleAttr(); }
  RuleElem(Rule *rule) : mType(ET_Rule) {
    mData.mRule = rule;
    mAttr = new RuleAttr();
  }
  ~RuleElem();

  void SetRuleOp(RuleOp op) {mType = ET_Op; mData.mOp = op;}
  void SetRule(Rule *r) {mType = ET_Rule; mData.mRule = r;}
  void SetChar(char c)      {mType = ET_Char; mData.mChar = c;}
  void SetString(const char *s) {mType = ET_String; mData.mString = s;}
  void SetTypeId(AGTypeId t)  {mType = ET_Type; mData.mTypeId = t;}
  void SetPending(const char *s) {mType = ET_Pending; mData.mString = s;}

  const char *GetPendingName() {return mData.mString;}
  const char *GetElemTypeName();
  const char *GetRuleOpName();

  void AddSubElem(RuleElem *e) {mSubElems.push_back(e);}

  void Dump(bool newline = false);
};

// a rule can contain multiple elements with actions
class Rule {
public:
  const std::string mName;
  RuleElem         *mElement;
  RuleAttr         *mAttr;
public:
  Rule(const std::string &s) : mName(s), mElement(NULL) {
    mAttr = new RuleAttr();
  }
  Rule() : mElement(NULL) {
    mAttr = new RuleAttr();
  }
  ~Rule();

  void SetName(const std::string &n) {mName = n;}
  void SetElement(RuleElem *e)       {mElement = e;}
  void Dump();
  void DumpAttr();
};

//////////////////////////////////////////////////////////////////////
//                   IdentifierRule                                 //
// Defines the legal wording sequence of an identifier              //
//                                                                  //
// Each rule elements in identifierRule will be concatenate together//
// to form a complete identifier syntax.                            //
//////////////////////////////////////////////////////////////////////

class IdentifierRule : public Rule {
public:
  IdentifierRule(const std::string &s) : Rule(s) {}
  IdentifierRule() {}
  ~IdentifierRule() {}
};

//////////////////////////////////////////////////////////////////////
//                   LiteralRule                                    //
//////////////////////////////////////////////////////////////////////

class LiteralRule : public Rule {
public:
  LiteralRule(const std::string &s) : Rule(s) {}
  LiteralRule() {}
  ~LiteralRule() {}
};

//////////////////////////////////////////////////////////////////////
//                   TypeRule                                       //
//////////////////////////////////////////////////////////////////////

class TypeRule : public Rule {
public:
  TypeRule(const std::string &s) : Rule(s) {}
  TypeRule() {}
  ~TypeRule() {}
};

#endif
