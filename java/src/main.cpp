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
    new (t) LiteralToken(ld);
    return t;
  }

  return NULL;
}

//////////////////////////////////////////////////////////////////////////////////
//          Framework based on Autogen + Token
//////////////////////////////////////////////////////////////////////////////////

bool Parser::Parse_autogen() {
  // TODO: Right now assume a statement has only one line. Will come back later.
  while (!mLexer->EndOfFile()) {
    ParseStmt_autogen();
    mLexer->ReadALine();
  }
} 

// return true : if successful
//       false : if failed
// This parses just one single statement.
bool Parser::ParseStmt_autogen() {
  // 1. Lex tokens in a line
  //    In Lexer::PrepareForFile() already did one ReadALine().
  while (!mLexer->EndOfLine()) {
    Token* t = mLexer->LexToken_autogen();
    if (t) {
      bool is_whitespace = false;
      t->Dump();
      if (t->IsSeparator()) {
        SeparatorToken *sep = (SeparatorToken *)t;
        if (sep->IsWhiteSpace())
          is_whitespace = true;
      }
      if (!is_whitespace)
        mTokens.push_back(t);
    } else {
      MASSERT(0 && "Non token got? Problem here!");
      break;
    }
  }

  // 2. Match the tokens against the rule tables.
  //    In a rule table there are : (1) separtaor, operator, keyword, are already in token
  //                                (2) Identifier Table won't be traversed any more since
  //                                    lex has got the token from source program and we only
  //                                    need check if the table is &TblIdentifier.
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
  PlantTokens(parser->mLexer);
  parser->Parse_autogen();
  delete parser;
  return 0;
}
