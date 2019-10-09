#include "driver.h"
#include "parser.h"
#include "common_header_autogen.h"
#include "ruletable_util.h"

/////////////////////////////////////
// Testing autogen
/////////////////////////////////////

// This is for testing autogen
TokenKind Lexer::LexToken_autogen(void) {
  SepId sep = GetSeparator(this);
  MMSG("sep id: ", sep);
}

bool Parser::Parse_autogen() {
  if (GetVerbose() >= 1) {
    MMSG("\n\n>> Parsing .... ", filename);
  }

  mLexer.ReadALine();
  while (!mLexer.EndOfLine()) {
    TokenKind tk = mLexer.LexToken_autogen();
  }
} 

int main (int argc, char *argv[]) {
  Parser *parser = new Parser(argv[1]);
  parser->Parse_autogen();
  delete parser;
  return 0;
}
