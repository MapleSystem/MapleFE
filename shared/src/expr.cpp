#include "expr.h"
#include "token.h"

void Expr::Dump(unsigned indent) {
  unsigned i = indent;
  while (i--) {
    std::cout << "  ";
  }
  mElem->Dump();
  std::cout << std::endl;
  std::vector<Expr *>::iterator it = mSubExprs.begin();
  for(; it != mSubExprs.end(); it++) {
    (*it)->Dump(indent+1);
  }

  std::vector<Token *>::iterator tit = mTokens.begin();
  for(; tit != mTokens.end(); tit++) {
    (*tit)->Dump();
  }
}

