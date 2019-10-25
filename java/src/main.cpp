#include "driver.h"
#include "parser.h"
#include "token.h"
#include "common_header_autogen.h"
#include "ruletable_util.h"

Token* FindSeparatorToken(Lexer * lex, SepId id) {
  for (unsigned i = 0; i < lex->mPredefinedTokenNum; i++) {
    Token *token = lex->mTokenPool.mTokens[i];
    if ((token->mTkType == TT_SP) && (((SeparatorToken*)token)->mSepId == id))
      return token;
  }
}

Token* FindOperatorToken(Lexer * lex, OprId id) {
  for (unsigned i = 0; i < lex->mPredefinedTokenNum; i++) {
    Token *token = lex->mTokenPool.mTokens[i];
    if ((token->mTkType == TT_OP) && (((OperatorToken*)token)->mOprId == id))
      return token;
  }
}

// The caller of this function makes sure 'key' is already in the
// string pool of Lexer.
Token* FindKeywordToken(Lexer * lex, char *key) {
  for (unsigned i = 0; i < lex->mPredefinedTokenNum; i++) {
    Token *token = lex->mTokenPool.mTokens[i];
    if ((token->mTkType == TT_KW) && (((KeywordToken*)token)->mName == key))
      return token;
  }
}

// This is for testing autogen
Token* Lexer::LexToken_autogen(void) {
  SepId sep = GetSeparator(this);
  if (sep != SEP_NA) {
    return FindSeparatorToken(this, sep);
  }

  OprId opr = GetOperator(this);
  if (opr != OPR_NA) {
    return FindOperatorToken(this, opr);
  }

  const char *keyword = GetKeyword(this);
  if (keyword != NULL) {
    return FindKeywordToken(this, keyword);
  }

  const char *identifier = GetIdentifier(this);
  if (identifier != NULL) {
    IdentifierToken *t = (IdentifierToken*)mTokenPool.NewToken(sizeof(IdentifierToken)); 
    new (t) IdentifierToken(identifier);
    return t;
  }

  LitData ld = GetLiteral(this);
  if (ld.mType != LT_NA) {
    LiteralToken *t = (LiteralToken*)mTokenPool.NewToken(sizeof(LiteralToken)); 
    // dump keywordmap to show the memory issue for t2.java
    std::cout << "\nnew LiteralToken *t = " << std::hex << t << std::endl;
    DumpKeywordMap();
    new (t) LiteralToken(ld);
    return t;
  }

  return NULL;
}

//////////////////////////////////////////////////////////////////////////////////
//          Framework based on Autogen + Token
//////////////////////////////////////////////////////////////////////////////////

bool Parser::Parse_autogen() {
  if (GetVerbose() >= 1) {
    MMSG("\n\n>> Parsing .... ", filename);
  }

  // mLexer->ReadALine();
  // In Lexer::PrepareForFile() already did one ReadALine().

  while (!mLexer->EndOfFile()) {
    while (!mLexer->EndOfLine()) {
      Token* t = mLexer->LexToken_autogen();
      if (t)
        t->Dump();
      else
        return;
    }
    mLexer->ReadALine();
  }
} 

// return true : if successful
//       false : if failed
bool Parser::ParseStmt_autogen() {
}

// Initialized the predefined tokens.
void Parser::InitPredefinedTokens() {
  // 1. create separator Tokens.
  for (unsigned i = 0; i < SEP_NA; i++) {
    Token *t = (Token*)mLexer->mTokenPool.NewToken(sizeof(SeparatorToken));
    new (t) SeparatorToken(i);
  }
  mLexer->mPredefinedTokenNum += SEP_NA;

  // 2. create operator Tokens.
  for (unsigned i = 0; i < OPR_NA; i++) {
    Token *t = (Token*)mLexer->mTokenPool.NewToken(sizeof(OperatorToken));
    new (t) OperatorToken(i);
  }
  mLexer->mPredefinedTokenNum += OPR_NA;

  // 3. create keyword Tokens.
  for (unsigned i = 0; i < KeywordTableSize; i++) {
    Token *t = (Token*)mLexer->mTokenPool.NewToken(sizeof(KeywordToken));
    char *s = mLexer->mStringPool.FindString(KeywordTable[i].mText);
    new (t) KeywordToken(s);
  }
  mLexer->mPredefinedTokenNum += KeywordTableSize;
}

int main (int argc, char *argv[]) {
  Parser *parser = new Parser(argv[1]);
  parser->InitPredefinedTokens();
  parser->Parse_autogen();
  delete parser;
  return 0;
}
