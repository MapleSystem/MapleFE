#include <iostream>
#include "automata.h"
#include "base_gen.h"
#include "parser.h"
#include "module.h"
#include "massert.h"
#include "rule.h"

Automata::Automata(AutoGen *ag, BaseGen *bg, Parser *p) :
  mAutoGen(ag), mBaseGen(bg), mParser(p), mModule(p->mModule) {
    mParser->mAutomata = this;
}

Automata::~Automata() {
}

std::string Automata::GetTokenString(TK_Kind tk) {
  return mParser->mLexer->GetTokenString(tk);
}

std::string Automata::GetTokenKindString(TK_Kind tk) {
  return mParser->mLexer->GetTokenKindString(tk);
}

int Automata::GetVerbose() {
  return mParser->GetVerbose();
}

// recursively process RuleElems to build mUsedByMap
// note : rule is used in elem
void Automata::ProcessUsedBy(RuleBase *rule, RuleElem *elem) {
  // process op car/string
  RuleBase *rb = NULL;
  switch (elem->mType) {
    case ET_Char:
    {
      elem->mToken = mParser->GetTokenKind(elem->mData.mChar);
      // get rule for the char token
      rb = mTokenRuleMap[elem->mToken];
      break;
    }
    case ET_String:
    {
      elem->mToken = mParser->GetTokenKind(elem->mData.mString);
      // get rule for the string token
      rb = mTokenRuleMap[elem->mToken];
      break;
    }
    case ET_Rule:
      rb = elem->mData.mRule;
      break;
    case ET_Pending:
      MMSG("Pending Rule: ", elem->mData.mString);
      break;
    default:
      break;
  }

  // update mUsedByMap and mSimpleUsedByMap
  if (rb && rb != rule) {
    mUsedByMap[rb].insert(rule);
    mSimpleUsedByMap[rb].insert(rule);
  }

  // recursively process sub elements
  for (auto it: elem->mSubElems) {
    ProcessUsedBy(rule, it);
  }
}

// get the sum of sizes of MapSetType's entries
// used to check if further iterations are needed to build closure
int Automata::GetMapSize(MapSetType map) {
  int count = 0;
  for (auto it: map) {
    count += it.second.size();
  }
  return count;
}

void Automata::BuildUsedByMap() {
  // init processing
  for (auto it: mBaseGen->mRules) {
    ProcessUsedBy(it, it->mElement);
  }

  // make a copy of current mSimpleUsedByMap
  MapSetType mTempUsedByMap;
  for (auto it: mSimpleUsedByMap) {
    for (auto parent: it.second) {
      mTempUsedByMap[it.first].insert(parent);
    }
  }

  // one iteration mSimpleUsedByMap for partial but shorter processing
  for (auto it: mSimpleUsedByMap) {
    for (auto parent: it.second) {
      if (mTempUsedByMap.find(parent) != mTempUsedByMap.end()) {
        for (auto r: mTempUsedByMap[parent]) {
          mSimpleUsedByMap[it.first].insert(r);
        }
      }
    }
  }

  // build Def-Use map closure mUsedByMap
  BuildClosure(mUsedByMap);
}

// build closure of a MapSetType type map
void Automata::BuildClosure(MapSetType &mapset) {
  int count0 = 0;
  int count1 = GetMapSize(mapset);
  int loopcnt = 0;
  while (count0 != count1) {
    for (auto it: mapset) {
      for (auto parent: it.second) {
        if (mapset.find(parent) != mapset.end()) {
          for (auto r: mapset[parent]) {
            mapset[it.first].insert(r);
          }
        }
      }
    }
    count0 = count1;
    count1 = GetMapSize(mapset);
    loopcnt++;
  }
  if (GetVerbose() >= 2) {
    MMSG("Build closure loop count: ", loopcnt);
  }
}

bool Automata::UpdatemIsAMap(RuleElem *elem, RuleBase *rule) {
  if (elem->mType != ET_Rule &&
      elem->mType != ET_Char &&
      elem->mType != ET_String &&
      elem->mType != ET_Pending) {
    return false;
  }

  RuleBase *rb = NULL;
  switch (elem->mType) {
    case ET_Pending:
      rb = mBaseGen->FindRule(elem->mData.mString);
      break;
    case ET_Rule:
      rb = elem->mData.mRule;
      break;
    case ET_Char:
    {
      TK_Kind tk = mParser->GetTokenKind(elem->mData.mChar);
      std::string tkstr = mParser->GetTokenKindString(tk);
      rb = mBaseGen->FindRule(tkstr);
      break;
    }
    case ET_String:
    {
      TK_Kind tk = mParser->GetTokenKind(elem->mData.mString);
      std::string tkstr = mParser->GetTokenKindString(tk);
      rb = mBaseGen->FindRule(tkstr);
      break;
    }
    default:
      break;
  }

  // during development some rules might not be defined yet
  if (rb) {
    // remove recursive ones
    if (rb != rule) {
      mIsAMap[rb].insert(rule);
    }
  } else {
    // create a temp rule for elem->mData.mString
    std::string str(elem->mData.mString);
    char *rulename = mBaseGen->mStringPool->FindString(str);
    RuleBase *rule = mBaseGen->AddLiteralRule(rulename);
    rule->SetName(rulename);

    if (GetVerbose() >= 2) {
      MMSG("Rule unknown for ", elem->mData.mString);
    }
  }

  return true;
}

void Automata::BuildIsAMap() {
  mIsAMap.clear();
  for (auto rule: mBaseGen->mRules) {
    if (GetVerbose() >= 3) {
      MLOC;
      rule->Dump();
    }
    RuleElem *elem = rule->mElement;
    bool status = UpdatemIsAMap(elem, rule);
    if (!status && elem->mType == ET_Op && elem->mData.mOp == RO_Oneof) {
      for (auto it: elem->mSubElems) {
        status = UpdatemIsAMap(it, rule);
      }
    }
  }

  // build closure
  BuildClosure(mIsAMap);
}

// IsA relation: rule1 is a rule2: IntegerLiteral is a Literal
bool Automata::IsA(RuleBase *rule1, RuleBase *rule2) {
  bool result1 = (mIsAMap.find(rule1) != mIsAMap.end());
  bool result2 = false;
  if (result1)
    result2 = (mIsAMap[rule1].find(rule2) != mIsAMap[rule1].end());
  return result2;
}

// IsUsedBy relation: rule1 is used by rule2
bool Automata::IsUsedBy(RuleBase *rule1, RuleBase *rule2) {
  bool result1 = (mUsedByMap.find(rule1) != mUsedByMap.end());
  bool result2 = false;
  if (result1)
    result2 = (mUsedByMap[rule1].find(rule2) != mUsedByMap[rule1].end());
  return result2;
}

// collect string/char literals in spec file
// char are already converted into string for uniform process
void Automata::CollectTokenInRuleElem(RuleElem *elem) {
  switch (elem->mType) {
    case ET_Char:
      mRuleChars.insert(elem->mData.mChar);
      break;
    case ET_String:
      mRuleStrings.insert(elem->mData.mString);
      break;
    default:
      break;
  }

  // recursively process sub elements
  for (auto it: elem->mSubElems) {
    CollectTokenInRuleElem(it);
  }
}

// create new rules for the string with the Token name as rule name
// rule TK_If : "if"
// rule TK_Land : "&&"
void Automata::CreateTokenRules() {
  for (auto c: mRuleChars) {
    TK_Kind tk = mParser->GetTokenKind(c);
    AddLiteralTokenRule(tk, c);
  }

  for (auto str: mRuleStrings) {
    TK_Kind tk = mParser->GetTokenKind(str.c_str());
    AddLiteralTokenRule(tk, str);
  }
  return;
}

void Automata::AddLiteralTokenRule(TK_Kind tk, char c) {
  if (mTokenRuleMap.find(tk) != mTokenRuleMap.end()) {
    return;
  }

  std::string tkstr = mParser->GetTokenKindString(tk);
  RuleBase *rule = mBaseGen->AddLiteralRule(tkstr, c);
  rule->mElement->mToken = tk;
  mTokenRuleMap[tk] = rule;
}

void Automata::AddLiteralTokenRule(TK_Kind tk, std::string str) {
  if (mTokenRuleMap.find(tk) != mTokenRuleMap.end()) {
    return;
  }

  std::string tkstr = mParser->GetTokenKindString(tk);
  char *name = mBaseGen->mStringPool->FindString(str);
  RuleBase *rule = mBaseGen->AddLiteralRule(tkstr, name);
  rule->mElement->mToken = tk;
  mTokenRuleMap[tk] = rule;
}

// add extra rules for string/char literals
// "+" --> rule TK_Add : "+"
void Automata::AddTokenRules() {

  for (auto rule: mBaseGen->mRules) {
    CollectTokenInRuleElem(rule->mElement);
  }

  CreateTokenRules();

  // add default token rules
#define TOKEN(N,T) AddLiteralTokenRule(TK_##T, #N);
#include "tokens.def"
#undef TOKEN

  // case TK_Assign:
#define OPKEYWORD(N,I,T) AddLiteralTokenRule(TK_##T, #N);
#include "opkeywords.def"
#undef OPKEYWORD

  // case TK_Return:
#define KEYWORD(N,I,T) AddLiteralTokenRule(TK_##I, #N);
#include "keywords.def"
#undef EYWORD

#define SEPARATOR(N,T) AddLiteralTokenRule(TK_##T, #N);
#include "supported_separators.def"
#undef SEPARATOR

}

// main routine to additional process of rules for automata
void Automata::ProcessRules() {
  AddTokenRules();

  // build mUsedByMap
  BuildUsedByMap();

  // build mIsA relation for relations like IntegerLiteral IsA Literal
  BuildIsAMap();

  if (GetVerbose() >= 3) {
    DumpTokenRuleMap();
    DumpIsAMap();
    DumpSimpleUsedByMap();
    DumpUsedByMap();
  }
}

// get the first string/char literal as token in stack
TK_Kind Automata::GetFirstStackOpcode(unsigned start, unsigned end, unsigned &idx) {
  // try to match token to start from a shorter list to match
  TK_Kind tk = TK_Invalid;
  for (idx = start; idx < end; idx++) {
    RuleElem *elem = mStack[idx].first;
    if (elem->mType == ET_Char || elem->mType == ET_String) {
      tk = elem->mToken;
      if (GetVerbose() >= 2) {
        MMSG("found token: ", GetTokenString(tk));
      }
      break;
    }
  }
  return tk;
}

bool Automata::IsType(const TK_Kind tk) {
  if (tk == TK_Boolean ||
      tk == TK_Byte ||
      tk == TK_Char ||
      tk == TK_Class ||
      tk == TK_Double ||
      tk == TK_Enum ||
      tk == TK_Float ||
      tk == TK_Int ||
      tk == TK_Interface ||
      tk == TK_Long ||
      tk == TK_Package ||
      tk == TK_Short ||
      tk == TK_Void ||
      tk == TK_Var)
    return true;
  else
    return false;
}

// process stack to match and reduce to a statement
void Automata::ProcessStack() {
  if (mStack.size() == 0) {
    return;
  }

  if (GetVerbose() >= 2) {
    DumpStack();
  }

  bool status = false;

  // get opcode index and token
  unsigned idx = 0;
  TK_Kind tk = GetFirstStackOpcode(0, mStack.size(), idx);

  stmtroot = new Stmt();;

  // add statement to function body
  currfunc->mBody.push_back(stmtroot);

  // Declarations
  if (IsType(tk)) {
    status = ProcessDecls();
  } else if (tk != TK_Invalid) {
    // go through related rules to match
    RuleBase *rule = mTokenRuleMap[tk];

    MASSERT(rule && "expect non-NULL rule");

    // check a shorter list but more directly related rules
    for (auto it: mSimpleUsedByMap[rule]) {
      if (GetVerbose() >= 3) {
        MLOC;
        it->Dump();
      }
      RuleElem *elem = mBaseGen->NewRuleElem(it);
      Expr *expr = new Expr(elem);
      status = MatchStack(expr, it, 0, idx, tk);
      if (status) {
        stmtroot->mExprs.push_back(expr);
        if (GetVerbose() >= 2) {
          MLOC;
          it->Dump();
          stmtroot->Dump(0);
          MLOC;
        }
        MMSGNOLOC("!!! Stack match result: ", status);
        return;
      } else {
        delete(expr);
      }
    }

    // check full list of used by rules
    for (auto it: mUsedByMap[rule]) {
      if (GetVerbose() >= 3) {
        MLOCENDL;
        it->Dump();
      }
      Expr *expr = new Expr();
      status = MatchStack(expr, it, 0, idx, tk);
      if (status) {
        stmtroot->mExprs.push_back(expr);
        if (GetVerbose() >= 2) {
          stmtroot->Dump(0);
          MLOC;
        }
        MMSGNOLOC("!!! Stack match result: ", status);
        return;
      } else {
        delete(expr);
      }
    }
  }

  if (GetVerbose() >= 2) {
    stmtroot->Dump(0);
    MLOC;
  }
  MMSGNOLOC("!!! Stack match result: ", status);
  return;
}

// match stack to a rule
bool Automata::MatchStack(Expr *&expr, RuleBase *rule, unsigned stackstart, unsigned stkidx, TK_Kind tk) {
  RuleBase *tkrule = mTokenRuleMap[tk];
  if (!IsUsedBy(tkrule, rule)) {
    return false;
  }

  bool status = false;
  switch (rule->mElement->mType) {
    case ET_Op:
    {
      status = MatchStackOp(expr, rule, stackstart, stkidx, tk);
      if (status) {
        if (GetVerbose() >= 3) {
          MMSG0("rule matched:");
          rule->Dump();
        }
      }
      break;
    }
    case ET_Rule:
    {
      status = MatchStackRule(expr, rule, stackstart, stkidx, tk);
      if (status) {
        if (GetVerbose() >= 3) {
          MMSG0("rule matched:");
          rule->Dump();
        }
      }
      break;
    }
    default:
      MMSG("To do", rule->mElement->GetElemTypeName());
      break;
  }

  return status;
}

// match rule which is of ET_Op
bool Automata::MatchStackOp(Expr *&expr, RuleBase *rule, unsigned stackstart, unsigned stkidx, TK_Kind tk) {
  bool status = false;

  if (rule->mElement->mType != ET_Op) {
    return status;
  }

  RuleBase *tkrule = mTokenRuleMap[tk];
  switch (rule->mElement->mData.mOp) {
    case RO_Oneof:
    {
      // check to match each sub element
      for (auto it: rule->mElement->mSubElems) {
        switch (it->mType) {
          case ET_Rule:
          {
            // check if rule is compatible with token
            if (IsUsedBy(tkrule, it->mData.mRule)) {
              Expr *newexpr = new Expr(it);
              status = MatchStack(newexpr, it->mData.mRule, stackstart, stkidx, tk);
              if (status) {
                expr->mSubExprs.push_back(newexpr);
                if (GetVerbose() >= 2) {
                  MMSG0("rule matched:");
                  it->mData.mRule->Dump();
                }
                return status;
              } else {
                delete(newexpr);
              }
            }
            break;
          }
          case ET_Op:
          {
            if (it->mData.mOp == RO_Concatenate) {
              Expr *newexpr = new Expr(it);
              status = MatchStackVec(newexpr, it->mSubElems, stackstart, stkidx, tk);
              if (status) {
                expr->mSubExprs.push_back(newexpr);

                if (GetVerbose() >= 2) {
                  MMSG0("rule matched:");
                  it->Dump(true);
                }

                return status;
              } else {
                delete(newexpr);
              }
            }
            break;
          }
          case ET_Pending:
          case ET_Char:
          case ET_String:
          default:
          {
            if (GetVerbose() >= 2) {
              MMSG("To do", it->GetElemTypeName());
              it->Dump(true);
            }
            break;
          }
        }
      }
      break;
    }
    case RO_Concatenate:
    {
      Expr *newexpr = new Expr(rule->mElement);
      status = MatchStackVec(newexpr, rule->mElement->mSubElems, stackstart, stkidx, tk);
      if (status) {
        expr->mSubExprs.push_back(newexpr);

        if (GetVerbose() >= 2) {
          MMSG0("rule matched:");
          rule->Dump();
        }
      } else {
        delete(newexpr);
      }
      break;
    }
    case RO_Zeroormore:
    case RO_Zeroorone:
    {
      MMSG("To do", rule->mElement->GetRuleOpName());
      break;
    }
    default:
      break;
  }

  return status;
}

// match rule which is of ET_Rule
bool Automata::MatchStackRule(Expr *&expr, RuleBase *rule, unsigned stackstart, unsigned stkidx, TK_Kind tk) {
  bool status = false;

  // check if rule is compatible with token
  RuleBase *tkrule = mTokenRuleMap[tk];
  if (!IsUsedBy(tkrule, rule)) {
    return false;
  }

  // check if can be reduced to rule

  if (status) {
    expr->mElem = mBaseGen->NewRuleElem(rule);
  }

  return status;
}

// match stack to a vector of rules consistent with given token
bool Automata::MatchStackVec(Expr *&expr, std::vector<RuleElem *> vec, unsigned stackstart, unsigned stkidx, TK_Kind tk) {
  RuleBase *tkrule = mTokenRuleMap[tk];
  unsigned tkidx = 0xffff;
  for (unsigned i = 0; i < vec.size(); i++) {
    ElemType et = vec[i]->mType;
    if ((et == ET_Rule && IsA(tkrule, vec[i]->mData.mRule)) ||
        ((et == ET_Char || et == ET_String) && vec[i]->mToken == tk)) {
      tkidx = i;
      break;
    }
  }

  // check if token matched
  if (tkidx == 0xffff) {
    return false;
  }

  // match subvec with substack before the token
  bool status = MatchStackVecRange(expr, vec, 0, tkidx, stackstart, stkidx);

  if (status) {
    RuleElem *elem = mStack[stkidx].first;
    Expr *newexpr = new Expr(elem);
    expr->mSubExprs.push_back(newexpr);

    // match after the token
    // adjust the match for trailing ';'
    unsigned vecsize = vec.size();
    unsigned stacksize = mStack.size();
    if (vec[vecsize-1]->mToken != TK_Semicolon &&
        mStack[stacksize-1].first->mToken == TK_Semicolon) {
      stacksize--;
    }
    if (vec[vecsize-1]->mToken == TK_Semicolon &&
        mStack[stacksize-1].first->mToken != TK_Semicolon) {
      vecsize--;
    }
    status  = MatchStackVecRange(expr, vec, tkidx + 1, vecsize, stkidx + 1, stacksize);
  }

  return status;
}

// match reset of stack to the range of a vector of rules
bool Automata::MatchStackVecRange(Expr *&expr, std::vector<RuleElem *> vec, unsigned start, unsigned end,
                               unsigned stackstart, unsigned stackend) {
  bool status = false;
  unsigned size = end - start;
  unsigned stacksize = stackend - stackstart;

  // 1-1 match
  if (size == stacksize) {
    // first pass check if match
    for (unsigned i = start, j = stackstart; i < end, j < stackend; i++, j++) {
      RuleElem *elem = mStack[j].first;
      RuleElem *e = vec[i];
      // RO_Zeroorone
      if (e->mType == ET_Op && e->mData.mOp == RO_Zeroorone) {
        e = e->mSubElems[0];
      }
      if (e->mType == ET_Rule && elem->mType == ET_Rule) {
        if (!IsA(elem->mData.mRule, e->mData.mRule)) {
          return false;
        }
      } else {
        if (GetVerbose() >= 1) {
          MMSG("not Rules?", vec[j]->GetElemTypeName());
          MMSG("not Rules?", elem->GetElemTypeName());
        }
      }
    }
    // handle stmt tree once match
    for (unsigned i = start, j = stackstart; i < end, j < stackend; i++, j++) {
      RuleElem *elem = mStack[j].first;
      RuleElem *e = vec[i];
      // RO_Zeroorone
      if (e->mType == ET_Op && e->mData.mOp == RO_Zeroorone) {
        e = e->mSubElems[0];
      }
      if (e->mType == ET_Rule && elem->mType == ET_Rule) {
        if (mStack[j].second) {
          Expr *newexpr = new Expr(elem);
          expr->mSubExprs.push_back(newexpr);

          MASSERT(elem->mType == ET_Rule && "expect a rule");

          // symbol expr which is a subexpr
          stridx_t stridx = mStack[j].second->mStridx;
          std::string str = mModule->mStrTable.GetStringFromGstridx(stridx);
          RuleElem *symbolelem = mBaseGen->NewRuleElem(str);
          Expr *symbolexpr = new Expr(symbolelem);
          newexpr->mSubExprs.push_back(symbolexpr);
        }
      }
    }
    return true;
  }

  // size == 1
  if (size == 1 && vec[start]->mType == ET_Rule) {
    unsigned idx = 0;
    TK_Kind tk = GetFirstStackOpcode(stackstart, stackend, idx);
    RuleElem *elem = mBaseGen->NewRuleElem(vec[start]->mData.mRule);
    Expr *newexpr = new Expr(elem);
    if (tk != TK_Invalid) {
      status = MatchStack(newexpr, vec[start]->mData.mRule, stackstart, idx, tk);
    } else {
      status = MatchStackWithExpectation(newexpr, vec[start]->mData.mRule, stackstart, stackend);
    }

    if (status) {
      expr->mSubExprs.push_back(newexpr);
    } else {
      delete(newexpr);
    }
  }

  return status;
}

// match range of entries on stack to a rule
bool Automata::MatchStackWithExpectation(Expr *&expr, RuleBase *rule, unsigned stackstart, unsigned stackend) {
  MMSG("MatchStackWithExpectation", 0);
  for (unsigned j = stackstart; j < stackend; j++) {
    RuleElem *elem = mStack[j].first;
    if (elem == NULL)
      continue;

    if (GetVerbose() >= 3) {
      MLOC;
      elem->Dump();
    }

    RuleElem *lookupelem = NULL;
    if (j < mStack.size() -1) {
      lookupelem = mStack[j+1].first;
    }
  }

  return true;
}

// need rework to be consistent with other statements
bool Automata::ProcessDecls() {
  tyidx_t tyidx = 0;
  RuleBase *declrule = mBaseGen->FindRule("LocalVariableDeclarationStatement");
  RuleElem *declelem = new RuleElem(declrule);
  Expr *declexpr = new Expr(declelem);
  stmtroot->mExprs.push_back(declexpr);

  for (unsigned j = 0; j < mStack.size() - 1; j++) { // skip ';'
    RuleElem *elem = mStack[j].first;

    // build tree
    Expr *expr = new Expr(elem);
    declexpr->mSubExprs.push_back(expr);
    if (mStack[j].second) {
      MASSERT(elem->mType == ET_Rule && "expect a rule");
      // symbol expr which is a subexpr
      stridx_t stridx = mStack[j].second->mStridx;
      std::string str = mModule->mStrTable.GetStringFromGstridx(stridx);
      RuleElem *symbolelem = mBaseGen->NewRuleElem(str);
      Expr *symbolexpr = new Expr(symbolelem);
      expr->mSubExprs.push_back(symbolexpr);
    }

    // update symbol table
    TK_Kind tk = elem->mToken;
    if (IsType(tk)) {
      tyidx = inttyidx;
    } else if (elem->mType == ET_Rule) {
      stridx_t stridx = mStack[j].second->mStridx;
      std::string s = mModule->mStrTable.GetStringFromGstridx(stridx);
      currfunc = mParser->currfunc;
      if (currfunc) {
        // local symbol
        if (!currfunc->GetSymbol(stridx)) {
          if (GetVerbose() >= 3) {
            MMSG("local var ", s);
          }
          Symbol *sb = new Symbol(stridx, inttyidx);
          currfunc->mSymbolTable.push_back(sb);
        }
      }
    }
  }
  return true;
}

void Automata::DumpStack() {
  std::cout << "\n\n============= Dump Stack =========" << std::endl;
  for (auto it: mStack) {
    std::cout << "Elem: ";
    it.first->Dump();
    if (it.second) {
      std::string str = mModule->mStrTable.GetStringFromGstridx(it.second->mStridx);
      std::cout << "\t symbol: " << str;
    }
    std::cout << std::endl;
  }
  std::cout << "========= end Dump Stack =========\n" << std::endl;
}

void Automata::DumpTokenRuleMap() {
  std::cout << "\n========== DumpTokenRuleMap ========" << std::endl;
  for (auto it: mTokenRuleMap) {
    std::cout << GetTokenKindString(it.first) << " :";
    if (it.second) {
      std::cout << "  " << it.second->mName;
    }
    std::cout << std::endl;
  }
  std::cout << "====== end DumpTokenRuleMap ========" << std::endl;
}

void Automata::DumpSetMap(MapSetType setmap) {
  for (auto it: setmap) {
    std::cout << it.first->mName << " :";
    for (auto rit: it.second) {
      std::cout << " " << rit->mName;
    }
    std::cout << std::endl;
  }
}

void Automata::DumpSimpleUsedByMap() {
  std::cout << "\n========== DumpSimpleUsedByMap =========" << std::endl;
  DumpSetMap(mSimpleUsedByMap);
  std::cout << "====== end DumpSimpleUsedByMap =========" << std::endl;
}

void Automata::DumpUsedByMap() {
  std::cout << "\n========== DumpUsedByMap =========" << std::endl;
  DumpSetMap(mUsedByMap);
  std::cout << "====== end DumpUsedByMap =========" << std::endl;
}

void Automata::DumpIsAMap() {
  std::cout << "\n=========== DumpIsAMap ===========" << std::endl;
  DumpSetMap(mIsAMap);
  std::cout << "======= end DumpIsAMap ===========" << std::endl;
}
