#include "stmt.h"

void Stmt::Dump(unsigned indent, bool withheader) {
  if (withheader) {
    std::cout << "============ stmt tree ============" << std::endl;
  }
  std::vector<Expr *>::iterator it = mExprs.begin();
  for(; it != mExprs.end(); it++) {
    (*it)->Dump(indent+1);
  }
  if (withheader) {
    std::cout << "===================================" << std::endl;
  }
}

void Stmt::EmitCode(unsigned indent) {
  std::vector<Expr *>::iterator it = mExprs.begin();
  for(; it != mExprs.end(); it++) {
    (*it)->EmitAction(indent, NULL);
  }
}
