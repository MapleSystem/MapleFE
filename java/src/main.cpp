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

Token* Lexer::LexToken_autogen(void) {
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
    return t;
  }

  LitData ld = GetLiteral(this);
  if (ld.mType != LT_NA) {
    LiteralToken *t = (LiteralToken*)mTokenPool.NewToken(sizeof(LiteralToken)); 
    new (t) LiteralToken(TK_Invalid, ld);
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
  bool succ = TraverseStmt();
}

// return true : if all tokens in mTokens are matched.
//       false : if faled.
bool Parser::TraverseStmt() {
  bool succ = TraverseRuleTable(&TblStatement);
  if (mTokens.size() != mCurToken)
    MASSERT(0 && "some tokens left unmatched!");
  else
    std::cout << "Matched " << mCurToken << " tokens." << std::endl;

  return succ;
}

// return true : if all tokens in mTokens are matched.
//       false : if faled.
bool Parser::TraverseRuleTable(RuleTable *rule_table) {
  bool matched = false;
  unsigned old_pos = mCurToken;
  Token *curr_token = mTokens[mCurToken];

  // We don't go into Identifier table.
  if ((rule_table == &TblIdentifier)) {
    if (curr_token->IsIdentifier()) {
      mCurToken++;
      //std::cout << "Matched identifier token: " << curr_token << std::endl;
      return true;
    } else {
      return false;
    }
  }

  // We don't go into Literal table.
  if ((rule_table == &TblLiteral)) {
    if (curr_token->IsLiteral()) {
      mCurToken++;
      //std::cout << "Matched literal token: " << curr_token << std::endl;
      return true;
    } else {
      return false;
    }
  }

  // Look into rule_table's data
  EntryType type = rule_table->mType;

  switch(type) {
  case ET_Oneof: {
    bool found = false;
    for (unsigned i = 0; i < rule_table->mNum; i++) {
      TableData *data = rule_table->mData + i;
      found = found | TraverseTableData(data);
      if (found)
        break;
    }
    matched = found; 
    break;
  }

  // moves until hit a NON-target data
  // It always return true because it doesn't matter how many target hit.
  case ET_Zeroormore: {
    matched = true;
    while(1) {
      bool found = false;
      for (unsigned i = 0; i < rule_table->mNum; i++) {
        TableData *data = rule_table->mData + i;
        found = found | TraverseTableData(data);
        // The first element is hit, then we restart the loop.
        if (found)
          break;
      }
      if (!found)
        break;
    }
    break;
  }

  // It always matched. The lexer will stop after it zeor or at most one target
  case ET_Zeroorone: {
    matched = true;
    bool found = false;
    for (unsigned i = 0; i < rule_table->mNum; i++) {
      TableData *data = rule_table->mData + i;
      found = TraverseTableData(data);
      // The first element is hit, then stop.
      if (found)
        break;
    }
    break;
  }

  // Lexer needs to find all elements, and in EXACTLY THE ORDER as defined.
  case ET_Concatenate: {
    bool found = false;
    for (unsigned i = 0; i < rule_table->mNum; i++) {
      TableData *data = rule_table->mData + i;
      found = TraverseTableData(data);
      // The first element missed, then we stop.
      if (!found)
        break;
    }
    matched = found;
    break;
  }

  // Next table
  case ET_Data: {
    break;
  }

  case ET_Null:
  default: {
    break;
  }
  }

  if(matched) {
    return true;
  } else {
    mLexer->curidx = old_pos;
    return false;
  }
}

// The mCurToken moves if found target, or restore the original location.
bool Parser::TraverseTableData(TableData *data) {
  unsigned old_pos = mCurToken;
  bool     found = false;
  Token   *curr_token = mTokens[mCurToken];

  switch (data->mType) {
  case DT_Char:
  case DT_String:
    MASSERT(0 && "Hit Char/String in TableData during matching!");
    break;
  // separator, operator, keywords are planted as DT_Token.
  // just need check the pointer of token
  case DT_Token:
    if (data->mData.mToken == curr_token) {
      found = true;
      mCurToken++;
      //std::cout << "Matched a token: " << curr_token << std::endl;
    }
    break;
  case DT_Type:
    break;
  case DT_Subtable: {
    RuleTable *t = data->mData.mEntry;
    found = TraverseRuleTable(t);
    if (!found)
      mCurToken = old_pos;
    break;
  }
  case DT_Null:
  default:
    break;
  }

  return found;
}



// Initialized the predefined tokens.
void Parser::InitPredefinedTokens() {
  // 1. create separator Tokens.
  for (unsigned i = 0; i < SEP_NA; i++) {
    Token *t = (Token*)mLexer->mTokenPool.NewToken(sizeof(SeparatorToken));
    new (t) SeparatorToken(i);
    //std::cout << "init a pre token " << t << std::endl;
    //t->Dump();
  }
  mLexer->mPredefinedTokenNum += SEP_NA;

  // 2. create operator Tokens.
  for (unsigned i = 0; i < OPR_NA; i++) {
    Token *t = (Token*)mLexer->mTokenPool.NewToken(sizeof(OperatorToken));
    new (t) OperatorToken(i);
    //std::cout << "init a pre token " << t << std::endl;
    //t->Dump();
  }
  mLexer->mPredefinedTokenNum += OPR_NA;

  // 3. create keyword Tokens.
  for (unsigned i = 0; i < KeywordTableSize; i++) {
    Token *t = (Token*)mLexer->mTokenPool.NewToken(sizeof(KeywordToken));
    char *s = mLexer->mStringPool.FindString(KeywordTable[i].mText);
    new (t) KeywordToken(s);
    //std::cout << "init a pre token " << t << std::endl;
    //t->Dump();
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
