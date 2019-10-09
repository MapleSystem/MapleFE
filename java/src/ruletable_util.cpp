#include "ruletable_util.h"
#include "lexer.h"
#include "common_header_autogen.h"

RuleTableWalker::RuleTableWalker(const RuleTable *t, Lexer *l) {
  mTable = t;
  mLexer = l;
  mTokenNum = 0;
  mMatched = false;
}

// Traverse the rule table by reading tokens through lexer.
// If the rule is matched with the upcoming tokens, mMatched is set to true.
void RuleTableWalker::Traverse() {

}

// Returen the separator ID, if it's. Or SEP_NA.
// Assuming the separator table has been sorted so as to catch the longest separator
//   if possible.
SepId RuleTableWalker::TraverseSepTable() {
  unsigned i = 0;
  for (; i < SEP_NA; i++) {
    SepTableEntry e = SepTable[i];
    if (!strncmp(mLexer->line + mLexer->curidx, e.mText, strlen(e.mText))) {
      mLexer->curidx += strlen(e.mText); 
      return e.mId;
    }
  }
  return SEP_NA;
}

////////////////////////////////////////////////////////////////////////////////
//                 Implementation of External Interfaces                      //
////////////////////////////////////////////////////////////////////////////////

// Returen the separator ID, if it's. Or SEP_NA.
SepId GetSeparator(Lexer *lex) {
  RuleTableWalker walker(NULL, lex);
  return walker.TraverseSepTable();
}
