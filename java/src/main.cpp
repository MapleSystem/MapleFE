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

//////////////////////////////////////////////////////////////////////////////////
//                           Parsing System
//
// 1. Cycle Reference
//
// The most important thing in parsing is how to HANDLE CYCLES in the rules. Take
// rule additiveExpression for example. It calls itself in the second element.
// Should we keep parsing when we find a cycle? If yes, is there any concern we
// need to address?
//
// rule MultiplicativeExpression : ONEOF(
//   UnaryExpression,  ---------------------> it can parse a variable name.
//   MultiplicativeExpression + '*' + UnaryExpression,
//   MultiplicativeExpression + '/' + UnaryExpression,
//   MultiplicativeExpression + '%' + UnaryExpression)
//
// rule AdditiveExpression : ONEOF(
//   MultiplicativeExpression,
//   AdditiveExpression + '+' + MultiplicativeExpression,
//   AdditiveExpression + '-' + MultiplicativeExpression)
//   attr.action.%2,%3 : GenerateBinaryExpr(%1, %2, %3)
//
// The answer is yes, cycle is useful in real life. It needs to keep looping.
// a + b + c + d + ... is a good example showing loops of AdditiveExpression.
//
// Another good example is Block. There are nested blocks. So loop exists.
//
// However, there is a concern which is how to avoid ENDLESS LOOP, and how to
// figure out Valuable Loop vs. Endless Loop. If it's a valuable loop we keep
// parsing, if it's an endless loop, we stop parsing.
//
// A loop is valuable if the mCurToken moves after an iteration of the loop.
// A loop is endless if mCurToken doesn't move between iterations.
// This is determined through the mVisitedStack which records the mCurToken each
// time we hit the ruletable once we are in a loop.
//
// Border conditions:  (1) The first time a table is hit, it's set Visited. We
//                         don's push to the stack since no loop so far.
//                     (2) The second time a table is hit, current token position
//                         is pushed. Now we are forming a loop, but we are not
//                         comparing its current position with the previous one since
//                         it's just a one iteration loop. We allow it go
//                         on until in third time where we can prove it's endless.
//                     (3) From second time on, each time a rule is done, its token
//                         position is popped from the stack.
//                     (4) From the third time on, start checking if the position
//                         has moved compared to the previous one.
//
// Parsing Time Issue
// 
// The rules are referencing each other and could increase the parsing time extremely.
// In order to save time, the first thing is avoiding entering a rule for the second
// time with the same token position if that rule has failed before. This is the
// origin of mFailed.
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
  // 1. clear mVisited for parsing every statement.
  //    TODO: Need think about when there are compound statement. Sometimes we
  //          should have more than one mVisited.
  //          Like block, which crosses multiple statements.
  mVisited.clear();

  // clear the failed info.
  ClearFailed();

  // clear the tokens.
  mTokens.clear();
  mCurToken = 0;

  // 2. Lex tokens in a line
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

  // 3. Match the tokens against the rule tables.
  //    In a rule table there are : (1) separtaor, operator, keyword, are already in token
  //                                (2) Identifier Table won't be traversed any more since
  //                                    lex has got the token from source program and we only
  //                                    need check if the table is &TblIdentifier.
  bool succ = TraverseStmt();
}

// return true : if all tokens in mTokens are matched.
//       false : if faled.
bool Parser::TraverseStmt() {
  // right now assume statement is just one line.
  // I'm doing a simple separation of one-line class declaration.
  bool succ = false;

  // Go through the top level construct, find the right one.
  std::vector<RuleTable*>::iterator it = mTopTables.begin();
  for (; it != mTopTables.end(); it++) {
    RuleTable *t = *it;
    succ = TraverseRuleTable(t);
    if (succ)
      break;
  }

  //Token *curr_token = mTokens[mCurToken];
  //bool is_class = false;
  //if (curr_token->IsKeyword()) {
  //  KeywordToken *kw = (KeywordToken*)curr_token;
  //  const char *name = kw->GetName();
  //  if (name && !strncmp(name, "class", 5))
  //    is_class = true;
  //}

  //if (is_class)
  //  succ = TraverseRuleTable(&TblClassDeclaration);
  //else
  //  succ = TraverseRuleTable(&TblStatement);

  if (mTokens.size() != mCurToken)
    std::cout << "Illegal syntax detected!" << std::endl;
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

  //
  if (WasFailed(rule_table, mCurToken))
    return false;

  if (IsVisited(rule_table)) {
    //std::cout << " ==== " << std::endl;
    //std::cout << "In table " << rule_table << " mCurToken " << mCurToken;
    // If there is already token position in stack, it means we are at at least the
    // 3rd instance. So need check if we made any progress between the two instances.
    //
    // This is not a failure case, don't need put into mFailed.
    //
    if (mVisitedStack[rule_table].size() > 0) {
      unsigned prev = mVisitedStack[rule_table].back();
      if (mCurToken == prev) {
        //std::cout << " An endless loop." << std::endl;
        return false; 
      }
    }
    // push the current token position
    VisitedPush(rule_table);
  } else {
    SetVisited(rule_table);
  }
    
  // We don't go into Identifier table.
  // No mVisitedStack invovled for identifier table.
  if ((rule_table == &TblIdentifier)) {
    ClearVisited(rule_table);
    if (curr_token->IsIdentifier()) {
      mCurToken++;
      //std::cout << "Matched identifier token: " << curr_token << std::endl;
      return true;
    } else {
      AddFailed(rule_table, mCurToken);
      return false;
    }
  }

  // We don't go into Literal table.
  // No mVisitedStack invovled for literal table.
  if ((rule_table == &TblLiteral)) {
    ClearVisited(rule_table);
    if (curr_token->IsLiteral()) {
      mCurToken++;
      //std::cout << "Matched literal token: " << curr_token << std::endl;
      return true;
    } else {
      AddFailed(rule_table, mCurToken);
      return false;
    }
  }

  // Look into rule_table's data
  EntryType type = rule_table->mType;

  switch(type) {

  // There is an issue in this case. Any rule which can consume one or more tokens is
  // considered 'found'. However, we need find the one which consume the most tokens.
  // [Question, TODO]: It this right? ...
  
  case ET_Oneof: {
    bool found = false;
    unsigned new_mCurToken = mCurToken; // position after most tokens eaten
    unsigned old_mCurToken = mCurToken;
    for (unsigned i = 0; i < rule_table->mNum; i++) {
      TableData *data = rule_table->mData + i;
      bool temp_found = TraverseTableData(data);
      found = found | temp_found;
      if (temp_found) {
        if (mCurToken > new_mCurToken)
          new_mCurToken = mCurToken;
        // Need restore the position of original mCurToken,
        // in order to catch the most tokens.
        mCurToken = old_mCurToken;
      }
    }
    // move position after most tokens are eaten
    mCurToken = new_mCurToken;
    matched = found; 
    break;
  }

  // It always return true.
  // Moves until hit a NON-target data
  // [Note]
  //   1. Every iteration we go through all table data, and pick the one eating most tokens.
  //   2. If noone of table data can read the token. It's the end.
  //   3. The final mCurToken needs to be updated.
  case ET_Zeroormore: {
    matched = true;
    while(1) {
      bool found = false;
      unsigned old_pos = mCurToken;
      unsigned new_pos = mCurToken;
      for (unsigned i = 0; i < rule_table->mNum; i++) {
        // every table entry starts from the old_pos
        mCurToken = old_pos;
        TableData *data = rule_table->mData + i;
        found = found | TraverseTableData(data);
        if (mCurToken > new_pos)
          new_pos = mCurToken;
      }

      // If hit the first non-target, stop it.
      if (!found)
        break;

      // Sometimes 'found' is true, but actually nothing was read becauser the 'true'
      // is coming from a Zeroorone or Zeroormore. So need check this.
      if (new_pos == old_pos)
        break;
      else
        mCurToken = new_pos;
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
  // There is only one data table in this case
  case ET_Data: {
    matched = TraverseTableData(rule_table->mData);
    break;
  }

  case ET_Null:
  default:
    break;
  }

  // If we are leaving the first instance of this rule_table, so clear the flag.
  // Or we pop out the last token position.
  if (mVisitedStack[rule_table].size() == 0)
    ClearVisited(rule_table);
  else
    VisitedPop(rule_table);

  if(matched) {
    //std::cout << " matched." << std::endl;
    return true;
  } else {
    //std::cout << " unmatched." << std::endl;
    mCurToken = old_pos;
    AddFailed(rule_table, mCurToken);
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
    //MASSERT(0 && "Hit Char/String in TableData during matching!");
    //TODO: Need compare literal. But so far looks like it's impossible to
    //      have a literal token able to match a string/char in rules.
    break;
  // separator, operator, keywords are planted as DT_Token.
  // just need check the pointer of token
  case DT_Token:
    //std::cout << "Matching a token: " << data->mData.mToken << std::endl;
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

// Set up the top level rule tables.
void Parser::SetupTopTables() {
  mTopTables.push_back(&TblStatement);
  mTopTables.push_back(&TblClassDeclaration);
}

int main (int argc, char *argv[]) {
  Parser *parser = new Parser(argv[1]);
  parser->InitPredefinedTokens();
  PlantTokens(parser->mLexer);
  parser->SetupTopTables();
  parser->Parse_autogen();
  delete parser;
  return 0;
}
