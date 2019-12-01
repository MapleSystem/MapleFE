#include "driver.h"
#include "parser.h"
#include "token.h"
#include "common_header_autogen.h"
#include "ruletable_util.h"
#include "gen_debug.h"

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

// Read a token until end of file.
// If no remaining tokens in current line, we move to the next line.
Token* Lexer::LexToken_autogen(void) {
  return LexTokenNoNewLine();
}

// Read a token until end of line.
// Return NULL if no token read.
Token* Lexer::LexTokenNoNewLine(void) {
  SepId sep = GetSeparator(this);
  if (sep != SEP_NA) {
    Token *t = FindSeparatorToken(this, sep);
    t->Dump();
    return t;
  }

  OprId opr = GetOperator(this);
  if (opr != OPR_NA) {
    Token *t = FindOperatorToken(this, opr);
    t->Dump();
    return t;
  }

  const char *keyword = GetKeyword(this);
  if (keyword != NULL) {
    Token *t = FindKeywordToken(this, keyword);
    t->Dump();
    return t;
  }

  const char *identifier = GetIdentifier(this);
  if (identifier != NULL) {
    IdentifierToken *t = (IdentifierToken*)mTokenPool.NewToken(sizeof(IdentifierToken)); 
    new (t) IdentifierToken(identifier);
    t->Dump();
    return t;
  }

  LitData ld = GetLiteral(this);
  if (ld.mType != LT_NA) {
    LiteralToken *t = (LiteralToken*)mTokenPool.NewToken(sizeof(LiteralToken)); 
    new (t) LiteralToken(TK_Invalid, ld);
    t->Dump();
    return t;
  }

  return NULL;
}
