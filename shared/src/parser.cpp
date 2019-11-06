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
}

Parser::Parser(const char *name) : filename(name) {
  mLexer = new Lexer();
  const std::string file(name);
  // init lexer
  mLexer->PrepareForFile(file);
  mCurToken = 0;
}


bool Parser::Parse() {
  bool atEof = false;
  bool status = true;

  if (GetVerbose() >= 3) {
    MMSG("\n\n>> Parsing .... ", filename);
  }

  currfunc = NULL;

  Token *tk = mLexer->NextToken();
  while (!atEof) {
    tk = mLexer->GetToken();
    switch (tk->mTkKind) {
      case TK_Eof:
        atEof = true;
        break;
      case TK_Int: // int variable
      {
        tk = mLexer->NextToken();
        std::string s = mLexer->GetTokenString();

        tk = mLexer->NextToken();
        if (tk->mTkKind == TK_Lparen) {
          // function
          if (GetVerbose() >= 2) {
            MMSG("func ", s);
          }
          Function *func = new Function(s, mModule);
          currfunc = func;
          mAutomata->currfunc = func;
          mModule->mFuncs.push_back(func);
          func->mRetTyidx = inttyidx;
          if (!ParseFunction(func)) {
            MMSG("Parse function error", s);
          }
          if (GetVerbose() >= 0) {
            func->Dump();
          }
        } else {
          // global symbol
          stridx_t stridx = mModule->mStrTable.GetOrCreateGstridxFromName(s);
          if (!mModule->GetSymbol(stridx)) {
            MMSG("global var ", s);
            Symbol *sb = new Symbol(stridx, inttyidx);
            mModule->mSymbolTable.push_back(sb);
          }
        }
        break;
      }
      case TK_Name:
      {
        std::string s = mLexer->GetTokenString();
        stridx_t stridx = mModule->mStrTable.GetOrCreateGstridxFromName(s);
        if (mModule->GetSymbol(stridx)) {
          
        }
      }
      default:
        break;
    }

    if (!status)
      return status;

    tk = mLexer->NextToken();
  }

  if (mLexer->GetVerbose() >= 3)
    Dump();

  return status;
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

FEOpcode Parser::GetFEOpcode(const char c) {
  char s[2] = {c, 0};
  return GetFEOpcode(s);
}

FEOpcode Parser::GetFEOpcode(const char *str) {
  Token *tk = mLexer->ProcessLocalToken(str);
  TK_Kind tkk = mLexer->GetMappedTokenKind(str);
  assert(tk->mTkKind == tkk);

  FEOPCode *opc = new FEOPCode();
  FEOpcode op = opc->Token2FEOpcode(tk->mTkKind);
  if (GetVerbose() >= 3) {
    MLOC;
    std::cout << "GetFEOpcode() str: " << str
              << " \ttoken: " << mLexer->GetTokenKindString(tk->mTkKind)
              << "   \topcode: " << opc->GetString(op)
              <<  std::endl;
  }
  return op;
}

bool Parser::ParseFunction(Function *func) {
  Token *tk = mLexer->GetToken();
  MASSERT(tk->mTkKind == TK_Lparen);
  tk = mLexer->NextToken();

  ParseFuncArgs(func);

  tk = mLexer->NextToken();
  // parse function body
  if (tk->mTkKind == TK_Lbrace) {
    tk = mLexer->NextToken();
    ParseFuncBody(func);
  }
  return true;
}

bool Parser::ParseFuncArgs(Function *func) {
  Token *tk = mLexer->GetToken();
  if (tk->mTkKind == TK_Rparen) {
    return true;
  }
  while (tk->mTkKind != TK_Rparen) {
    switch (tk->mTkKind) {
      case TK_Int:
      {
        tk = mLexer->NextToken();
        // check if it is prototype only without variable
        if (tk->mTkKind != TK_Comma || tk->mTkKind != TK_Rparen) {
          std::string s = mLexer->GetTokenString();
          stridx_t stridx = mModule->mStrTable.GetOrCreateGstridxFromName(s);
          Symbol *sb = currfunc->GetSymbol(stridx);
          if (!sb) {
            if (GetVerbose() >= 3) {
              MMSG("arg ", s);
            }
            sb = new Symbol(stridx, inttyidx);
            currfunc->mSymbolTable.push_back(sb);
          }
          func->mFormals.push_back(sb);
          func->mArgTyidx.push_back(inttyidx);
          tk = mLexer->NextToken();
        }
        break;
      }
      default:
        MMSGA("TK_ for args: ", mLexer->GetTokenKindString(tk->mTkKind));
        break;
    }
    if (tk->mTkKind == TK_Comma) {
      tk = mLexer->NextToken();
    }
  }
  return true;
}

bool Parser::ParseFuncBody(Function *func) {
  Token *tk = mLexer->GetToken();

  // loop till the end of function body '}'
  while (tk->mTkKind != TK_Rbrace) {
    ParseStmt(func);
    tk = mLexer->GetToken();
  }

  // Consumes '}'
  tk = mLexer->NextToken();
  return true;
}

// ParseStmt consumes ';'
bool Parser::ParseStmt(Function *func) {
  mAutomata->mStack.clear();
  Token *tk = mLexer->GetToken();
  bool isDecl = (mAutomata->IsType(tk->mTkKind));
  while (tk->mTkKind != TK_Invalid) {
    switch (tk->mTkKind) {
      case TK_Name:
      {
        std::string s = mLexer->GetTokenString();
        stridx_t stridx = mModule->mStrTable.GetOrCreateGstridxFromName(s);
        Symbol *sb = currfunc->GetSymbol(stridx);
        // either in decl or sb should be exist
        MASSERT(isDecl || sb);
        if (!sb) {
          sb = new Symbol(stridx, inttyidx);
          currfunc->mSymbolTable.push_back(sb);
        }
        // found Identifier
        Rule *r = mAutomata->mBaseGen->FindRule("Identifier");
        RuleElem *e = mAutomata->mBaseGen->NewRuleElem();
        e->mType = ET_Rule;
        e->mData.mRule = r;
        std::pair<RuleElem *, Symbol *> p(e, sb);
        mAutomata->mStack.push_back(p);
        break;
      }
      // case TK_Assign:
      #define OPKEYWORD(N,I,T) case TK_##T:
        #include "opkeywords.def"
      #undef OPKEYWORD
      // case TK_Return:
      #define KEYWORD(N,I,T) case TK_##I:
        #include "keywords.def"
      #undef KEYWORD
      case TK_Semicolon: // end of Stmt
      {
        std::string str = GetTokenKindString(tk->mTkKind);
        RuleElem *e = NULL;
        // "=" is considered as '='
        if (str.size() == 1) {
          e = mAutomata->mBaseGen->GetRuleElemFromChar(str[0]);
        } else {
          e = mAutomata->mBaseGen->GetRuleElemFromString(str);
        }
        std::pair<RuleElem *, Symbol *> p(e, NULL);
        mAutomata->mStack.push_back(p);
        break;
      }
      default:
        break;
    }

    // ';' marks the end of statement
    if (tk->mTkKind == TK_Semicolon) {
      break;
    }
    tk = mLexer->NextToken();
  }

  mAutomata->ProcessStack();

  tk = mLexer->NextToken();
  return true;
}

void Parser::Dump() {
  std::cout << "\n================= Code ===========" << std::endl;
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

bool Parser::WasFailed(RuleTable *table, unsigned token) {
  std::vector<unsigned>::iterator it = mFailed[table].begin();
  for (; it != mFailed[table].end(); it++) {
    if (*it == token)
      return true;
  }
  return false;
}
