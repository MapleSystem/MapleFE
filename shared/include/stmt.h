#ifndef __STMT_H__
#define __STMT_H__

#include <iostream>
#include <fstream>

#include "massert.h"
#include "base_gen.h"
#include "expr.h"

class Stmt {
 public:
  std::vector<Expr *> mExprs;

  Stmt() {}
  void Dump(unsigned indent, bool withheader = true);
};

#endif
