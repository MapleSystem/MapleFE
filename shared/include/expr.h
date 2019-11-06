#ifndef __EXPR_H__
#define __EXPR_H__

#include <iostream>
#include <fstream>

#include "massert.h"
#include "base_gen.h"
#include "symbol.h"

class Token;
class Expr {
 public:
  RuleElem *mElem;
  std::vector<Expr *> mSubExprs;
  std::vector<Token*> mTokens;
  Symbol             *mSymbol;

  Expr() : mElem(NULL), mSymbol(NULL) {}
  Expr(RuleElem *elem) : mElem(elem), mSymbol(NULL) {}

  void Dump(unsigned indent);
  void EmitAction(unsigned indent, Symbol *symbol);
};

#endif
