////////////////////////////////////////////////////////////////////////
// This is the automata using rules.
////////////////////////////////////////////////////////////////////////

#ifndef __AUTOMATA_H__
#define __AUTOMATA_H__

#include <unordered_map>
#include <unordered_set>
#include <utility>
#include "tokenkind.h"
#include "module.h"
#include "expr.h"
#include "stmt.h"
#include "symbol.h"
#include "function.h"
#include "rule.h"

class Lexer;
class Parser;
class AutoGen;
class BaseGen;
class Module;
class Symbol;
class Function;

class Automata {
 public:
  AutoGen    *mAutoGen;
  BaseGen    *mBaseGen;
  Parser     *mParser;
  Module     *mModule;

  Function   *currfunc;
  Stmt       *stmtroot;

  // temp
  std::vector<std::pair<RuleElem *, Symbol *>> mStack;

  // char tokens rules
  std::unordered_set<char> mRuleChars;

  // string tokens rules
  std::unordered_set<std::string> mRuleStrings;

  // group the rules by string literal used as key (TK_Kind)
  std::unordered_map<TK_Kind, Rule *, EnumHash> mTokenRuleMap;

  typedef std::unordered_map<Rule *, std::unordered_set<Rule *>> MapSetType;

  // Def-Use of rules
  MapSetType mUsedByMap;
  // a shorter mapped list for quick match
  MapSetType mSimpleUsedByMap;

  // IsA relation: <RULE, {parent rules}>  <IntegerLiteral, {Literal}>
  MapSetType mIsAMap;

public:
  Automata(AutoGen *ag, BaseGen *bg, Parser *p);
  ~Automata();

  AutoGen *GetAutogen() { return mAutoGen; }
  BaseGen *GetBasegen() { return mBaseGen; }
  Parser *GetParser() { return mParser; }
  void SetAutogen(AutoGen *ag) { mAutoGen = ag; }
  void SetBasegen(BaseGen *bg) { mBaseGen = bg; }

  FEOpcode GetFEOpcode(char c);
  FEOpcode GetFEOpcode(const char *str);

  // add addition rules like
  // rule TK_Land : "&&"
  void AddTokenRules();
  void AddLiteralTokenRule(TK_Kind tk, char c);
  void AddLiteralTokenRule(TK_Kind tk, std::string str);
  void CollectTokenInRuleElem(RuleElem *elem);
  void CreateTokenRules();

  void ProcessUsedBy(Rule *rule, RuleElem *elem);
  void BuildUsedByMap();
  void ProcessRules();

  bool IsType(TK_Kind tk);
  bool IsA(Rule *rule1, Rule *rule2);
  bool IsUsedBy(Rule *rule1, Rule *rule2);
  void BuildIsAMap();
  void BuildClosure(MapSetType &mapset);
  bool UpdatemIsAMap(RuleElem *elem, Rule *rule);

  void DumpStack();
  void ProcessStack();
  bool MatchStackRule(Expr *&expr, Rule *rule, unsigned stkstart, unsigned stkend, TK_Kind tk);
  bool MatchStackOp(Expr *&expr, Rule *rule, unsigned stkstart, unsigned stkend, TK_Kind tk);
  bool MatchStackVec(Expr *&expr, std::vector<RuleElem *> vec, unsigned stkstart, unsigned stkend, TK_Kind tk);
  bool MatchStackVecRange(Expr *&expr, std::vector<RuleElem *> vec,
                          unsigned vecstart, unsigned vecend, unsigned stkstart, unsigned stkend);

  bool ProcessDecls();

  int GetVerbose();
  int GetMapSize(MapSetType map);
  TK_Kind GetFirstStackOpcode(unsigned start, unsigned end, unsigned &idx);
  void DumpTokenRuleMap();
  void DumpSetMap(MapSetType setmap);
  void DumpSimpleUsedByMap();
  void DumpUsedByMap();
  void DumpIsAMap();

  std::string GetTokenString(TK_Kind tk);
  std::string GetTokenKindString(TK_Kind tk);
};

#endif

