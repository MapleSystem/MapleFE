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

  // group the rules by string literal used as key (TokenKind)
  std::unordered_map<TokenKind, RuleBase *, EnumHash> mTokenRuleMap;

  typedef std::unordered_map<RuleBase *, std::unordered_set<RuleBase *>> MapSetType;

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
  void AddLiteralTokenRule(TokenKind tk, char c);
  void AddLiteralTokenRule(TokenKind tk, std::string str);
  void CollectTokenInRuleElem(RuleElem *elem);
  void CreateTokenRules();

  void ProcessUsedBy(RuleBase *rule, RuleElem *elem);
  void BuildUsedByMap();
  void ProcessRules();

  bool IsType(TokenKind tk);
  bool IsA(RuleBase *rule1, RuleBase *rule2);
  bool IsUsedBy(RuleBase *rule1, RuleBase *rule2);
  void BuildIsAMap();
  void BuildClosure(MapSetType &mapset);
  bool UpdatemIsAMap(RuleElem *elem, RuleBase *rule);

  void DumpStack();
  void ProcessStack();
  bool MatchStack(Expr *&expr, RuleBase *rule, unsigned stackstart, unsigned idx, TokenKind tk);
  bool MatchStackOp(Expr *&expr, RuleBase *rule, unsigned stackstart, unsigned idx, TokenKind tk);
  bool MatchStackRule(Expr *&expr, RuleBase *rule, unsigned stackstart, unsigned idx, TokenKind tk);
  bool MatchStackVec(Expr *&expr, std::vector<RuleElem *> vec, unsigned stackstart, unsigned idx, TokenKind tk);
  bool MatchStackVecRange(Expr *&expr, std::vector<RuleElem *> vec, unsigned start, unsigned end,
                          unsigned stackstart, unsigned stackend);
  bool MatchStackWithExpectation(Expr *&expr, RuleBase *rule, unsigned stackstart, unsigned stackend);
  bool ProcessDecls();

  int GetVerbose();
  int GetMapSize(MapSetType map);
  TokenKind GetFirstStackOpcode(unsigned start, unsigned end, unsigned &idx);
  void DumpTokenRuleMap();
  void DumpSetMap(MapSetType setmap);
  void DumpSimpleUsedByMap();
  void DumpUsedByMap();
  void DumpIsAMap();

  std::string GetTokenString(TokenKind tk);
  std::string GetTokenKindString(TokenKind tk);
};

#endif

