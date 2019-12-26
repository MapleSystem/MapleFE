#include <iostream>
#include <string>
#include <cstring>

#include "module.h"
#include "automata.h"
#include "parser.h"
#include "base_gen.h"
#include "massert.h"
#include "token.h"

//////////////////////////////////////////////////////////////////////////////

// pass spec file
Parser::Parser(const char *name, Module *m) : filename(name), mModule(m) {
  mLexer = new Lexer();
  const std::string file(name);
  // init lexer
  mLexer->PrepareForFile(file);
  mCurToken = 0;

  mTraceTable = false;
  mTraceAppeal = false;
  mTraceSecondTry = false;
  mTraceVisited = false;
  mTraceFailed = false;
  mIndentation = 0;
}

Parser::Parser(const char *name) : filename(name) {
  mLexer = new Lexer();
  const std::string file(name);
  // init lexer
  mLexer->PrepareForFile(file);
  mCurToken = 0;
  mPending = 0;

  mTraceTable = false;
  mTraceAppeal = false;
  mTraceSecondTry = false;
  mTraceVisited = false;
  mTraceFailed = false;
  mIndentation = 0;
}


TK_Kind Parser::GetTokenKind(const char c) {
  char s[2] = {c, 0};
  return GetTokenKind(s);
}

TK_Kind Parser::GetTokenKind(const char *str) {
  TK_Kind tkk = mLexer->GetMappedTokenKind(str);
  if (GetVerbose() >= 3) {
    MLOC;
    std::cout << " GetFEOpcode() str: " << str
              << " \ttoken: " << mLexer->GetTokenKindString(tkk)
              <<  std::endl;
  }
  return tkk;
}

void Parser::Dump() {
  std::cout << "\n================= Code ===========" << std::endl;
  if (GetVerbose() >= 3) {
    for (auto it: mModule->mFuncs) {
      it->EmitCode();
    }
  }
  std::cout << "==================================" << std::endl;
}


// Utility function to handle visited rule tables.
bool Parser::IsVisited(RuleTable *table) {
  std::map<RuleTable*, bool>::iterator it;
  it = mVisited.find(table);
  if (it == mVisited.end())
    return false;
  if (it->second == false)
    return false;
  return true;
}

void Parser::SetVisited(RuleTable *table) {
  //std::cout << " set visited " << table;
  mVisited[table] = true;
}

void Parser::ClearVisited(RuleTable *table) {
  //std::cout << " clear visited " << table;
  mVisited[table] = false;
}

// Push the current position into stack, as we are entering the table again.
void Parser::VisitedPush(RuleTable *table) {
  //std::cout << " push " << mCurToken << " from " << table;
  mVisitedStack[table].push_back(mCurToken);
}

// Pop the last position in stack, as we are leaving the table again.
void Parser::VisitedPop(RuleTable *table) {
  //std::cout << " pop " << " from " << table;
  mVisitedStack[table].pop_back();
}

// Add one fail case for the table
void Parser::AddFailed(RuleTable *table, unsigned token) {
  //std::cout << " push " << mCurToken << " from " << table;
  mFailed[table].push_back(token);
}

// Remove one fail case for the table
void Parser::ResetFailed(RuleTable *table, unsigned token) {
  std::vector<unsigned>::iterator it = mFailed[table].begin();;
  for (; it != mFailed[table].end(); it++) {
    if (*it == token)
      break;
  }

  if (it != mFailed[table].end())
    mFailed[table].erase(it);
}

bool Parser::WasFailed(RuleTable *table, unsigned token) {
  std::vector<unsigned>::iterator it = mFailed[table].begin();
  for (; it != mFailed[table].end(); it++) {
    if (*it == token)
      return true;
  }
  return false;
}
