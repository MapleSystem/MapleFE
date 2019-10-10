#include "driver.h"
#include "parser.h"
#include "token.h"
#include "common_header_autogen.h"
#include "ruletable_util.h"

/////////////////////////////////////
// Testing autogen
/////////////////////////////////////

// This is for testing autogen
Token* Lexer::LexToken_autogen(void) {
  SepId sep = GetSeparator(this);
  if (sep != SEP_NA) {
    SeparatorToken *t = (SeparatorToken*)mTokenPool.NewToken(sizeof(SeparatorToken)); 
    new (t) SeparatorToken(sep);
    return t;
  }

  return NULL;
}

bool Parser::Parse_autogen() {
  if (GetVerbose() >= 1) {
    MMSG("\n\n>> Parsing .... ", filename);
  }

  mLexer.ReadALine();
  while (!mLexer.EndOfLine()) {
    Token* t = mLexer.LexToken_autogen();
    if (t)
      t->Dump();
    else
      return;
  }
} 

int main (int argc, char *argv[]) {
  Parser *parser = new Parser(argv[1]);
  parser->Parse_autogen();
  delete parser;
  return 0;
}
