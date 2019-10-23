#ifndef __EXPR_H__
#define __EXPR_H__

#include <iostream>
#include <fstream>

#include "massert.h"
#include "base_gen.h"

class Token;
class Expr {
 public:
  RuleElem *mElem;
  std::vector<Expr *> mSubExprs;
  std::vector<Token*> mTokens;

  Expr() : mElem(NULL) {}
  Expr(RuleElem *elem) : mElem(elem) {}

  void Dump(unsigned indent);
};

#endif
