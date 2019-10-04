#include "ruletable_util.h"
#include "lexer.h"

RuleTableWalker::RuleTableWalker(const RuleTable *t, const Lexer *l) {
  mTable = t;
  mLexer = l;
  mTokenNum = 0;
  mMatched = false;
}

// Traverse the rule table by reading tokens through lexer.
// If the rule is matched with the upcoming tokens, mMatched is set to true.
void RuleTableWalker::Traverse() {

}
