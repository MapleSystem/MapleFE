#include "expr.h"
#include "token.h"
#include "module.h"
#include <set>


void Expr::Dump(unsigned indent) {
  unsigned i = indent;
  while (i--) {
    std::cout << "  ";
  }
  mElem->Dump();
#if 0
  std::cout << "\t\t-- expr0 " << std::hex << this << " mElem " << mElem << std::dec;
  if (mSymbol)
    std::cout << " symbol " << GlobalTables::GetStringTable().GetStringFromStridx(mSymbol->mStridx);
#endif
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

void Expr::EmitAction(unsigned indent, Symbol *symbol) {
#if 0
  if (mSymbol) {
    std::cout << "emit -- expr " << std::hex << this << std::dec;
    std::cout << " name " << GlobalTables::GetStringTable().GetStringFromStridx(mSymbol->mStridx);
    std::cout  << std::endl;
  }
#endif

  std::set<unsigned> doneAction;
  if (!mElem->mAttr->Empty()) {
    if (mElem->mAttr->mValidity.size()) {
    }
    if (mElem->mAttr->mAction.size()) {
      for (int i = 0; i < mElem->mAttr->mAction.size(); i++) {
        unsigned size = mElem->mAttr->mAction[i]->mArgs.size();
        for (int j = 0; j < size; j++) {
          int k = mElem->mAttr->mAction[i]->mArgs[j];
          // recursively search for defined symbol
          Expr *e = mSubExprs[k-1];
          while (!e->mSymbol) {
            if (e->mSubExprs.size() == 1)
              e = e->mSubExprs[0];
            else
              break;
          }

          // check if the sub expression needs to be defined first
          if (e->mSymbol && doneAction.find(k-1) == doneAction.end()) {
            // emit the action with defined symbol
            e->EmitAction(indent+1, e->mSymbol);
            // collect emitted actions
            doneAction.insert(k-1);
          }

          std::cout << "var" << j+1 << " = ";
          if (e->mSymbol) {
            // std::cout << "symbol ";
            stridx_t stridx = e->mSymbol->mStridx;
            std::cout << GlobalTables::GetStringTable().GetStringFromStridx(stridx) << std::endl;
          } else if (mSubExprs[k-1]->mElem->IsLeaf()) {
            // std::cout << "leaf ";
            mSubExprs[k-1]->Dump(3);
          } else {
            // std::cout << "else ";
            mSubExprs[k-1]->EmitAction(3, NULL);
          }
        }
        if (symbol) 
          std::cout << GlobalTables::GetStringTable().GetStringFromStridx(symbol->mStridx) << " = ";
        else
          std::cout << "var" << "0 = ";
        std::cout << mElem->mAttr->mAction[i]->mName << "(";
        for (int j = 0; j < size; j++) {
          std::cout << "var" << j+1;
          if (j != size-1)
            std::cout << ", ";
        }
        std::cout << ")\n\n";
      }
    }
  }

#if 0
  if (!mSymbol) {
    std::cout << "emit -- expr " << std::hex << this << std::dec;
    std::cout  << std::endl;
  }
#endif

  unsigned size = mSubExprs.size();
  for (int j = 0; j < size; j++) {
    // skip emitting aciton if already done
    if (doneAction.find(j) != doneAction.end())
      continue;

    // recursively search for defined symbol
    Expr *e = mSubExprs[j];
    while (!e->mSymbol) {
      if (e->mSubExprs.size() == 1)
        e = e->mSubExprs[0];
      else
        break;
    }
    if (e->mSymbol)
      e->EmitAction(indent+1, e->mSymbol);
    else
      mSubExprs[j]->EmitAction(indent+1, NULL);
  }

}

