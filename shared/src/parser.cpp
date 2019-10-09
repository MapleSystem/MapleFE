#include <iostream>
#include <string>
#include <cstring>

#include "module.h"
#include "automata.h"
#include "parser.h"
#include "base_gen.h"
#include "massert.h"

//////////////////////////////////////////////////////////////////////////////

// pass spec file
Parser::Parser(const char *name, Module *m) : mLexer(), filename(name), mModule(m) {
  const std::string file(name);
  // init lexer
  mLexer.PrepareForFile(file);
}

Parser::Parser(const char *name) : mLexer(), filename(name) {
  const std::string file(name);
  // init lexer
  mLexer.PrepareForFile(file);
}


bool Parser::Parse() {
  bool atEof = false;
  bool status = true;

  if (GetVerbose() >= 1) {
    MMSG("\n\n>> Parsing .... ", filename);
  }

  currfunc = NULL;

  TokenKind tk = mLexer.NextToken();
  while (!atEof) {
    tk = mLexer.GetToken();
    switch (tk) {
      case TK_Eof:
        atEof = true;
        break;
      case TK_Int: // int variable
      {
        tk = mLexer.NextToken();
        std::string s = mLexer.GetTokenString();

        tk = mLexer.NextToken();
        if (tk == TK_Lparen) {
          // function
          if (GetVerbose() >= 1) {
            MMSG("func ", s);
          }
          Function *func = new Function(s);
          currfunc = func;
          mAutomata->currfunc = func;
          mModule->mFuncs.push_back(func);
          func->mRetTyidx = inttyidx;
          if (!ParseFunction(func)) {
            MMSG("Parse function error", s);
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
        std::string s = mLexer.GetTokenString();
        stridx_t stridx = mModule->mStrTable.GetOrCreateGstridxFromName(s);
        if (mModule->GetSymbol(stridx)) {
          
        }
      }
      default:
        break;
    }

    if (!status)
      return status;

    tk = mLexer.NextToken();
  }

  if (mLexer.GetVerbose() >= 1)
    Dump();

  return status;
}

TokenKind Parser::GetTokenKind(const char c) {
  char s[2] = {c, 0};
  return GetTokenKind(s);
}

TokenKind Parser::GetTokenKind(const char *str) {
  TokenKind tk = mLexer.GetMappedToken(str);
  if (GetVerbose() >= 2) {
    MLOC;
    std::cout << " GetFEOpcode() str: " << str
              << " \ttoken: " << mLexer.GetTokenKindString(tk)
              <<  std::endl;
  }
  return tk;
}

FEOpcode Parser::GetFEOpcode(const char c) {
  char s[2] = {c, 0};
  return GetFEOpcode(s);
}

FEOpcode Parser::GetFEOpcode(const char *str) {
  TokenKind tk = mLexer.ProcessLocalToken(str);
  TokenKind tk1 = mLexer.GetMappedToken(str);
  assert(tk == tk1);

  FEOPCode *opc = new FEOPCode();
  FEOpcode op = opc->Token2FEOpcode(tk);
  if (GetVerbose() >= 2) {
    MLOC;
    std::cout << "GetFEOpcode() str: " << str
              << " \ttoken: " << mLexer.GetTokenKindString(tk)
              << "   \topcode: " << opc->GetString(op)
              <<  std::endl;
  }
  return op;
}

bool Parser::ParseFunction(Function *func) {
  TokenKind tk = mLexer.GetToken();
  MASSERT(tk == TK_Lparen);
  tk = mLexer.NextToken();

  ParseFuncArgs(func);

  tk = mLexer.NextToken();
  // parse function body
  if (tk == TK_Lbrace) {
    tk = mLexer.NextToken();
    ParseFuncBody(func);
  }
  return true;
}

bool Parser::ParseFuncArgs(Function *func) {
  TokenKind tk = mLexer.GetToken();
  if (tk == TK_Rparen) {
    return true;
  }
  while (tk != TK_Rparen) {
    switch (tk) {
      case TK_Int:
      {
        tk = mLexer.NextToken();
        // check if it is prototype only without variable
        if (tk != TK_Comma || tk != TK_Rparen) {
          std::string s = mLexer.GetTokenString();
          stridx_t stridx = mModule->mStrTable.GetOrCreateGstridxFromName(s);
          Symbol *sb = currfunc->GetSymbol(stridx);
          if (!sb) {
            if (GetVerbose() >= 1) {
              MMSG("arg ", s);
            }
            sb = new Symbol(stridx, inttyidx);
            currfunc->mSymbolTable.push_back(sb);
          }
          func->mFormals.push_back(sb);
          func->mArgTyidx.push_back(inttyidx);
          tk = mLexer.NextToken();
        }
        break;
      }
      default:
        MMSGA("TK_ for args: ", mLexer.GetTokenKindString(tk));
        break;
    }
    if (tk == TK_Comma) {
      tk = mLexer.NextToken();
    }
  }
  return true;
}

bool Parser::ParseFuncBody(Function *func) {
  TokenKind tk = mLexer.GetToken();

  // loop till the end of function body '}'
  while (tk != TK_Rbrace) {
    ParseStmt(func);
    tk = mLexer.GetToken();
  }

  // Consumes '}'
  tk = mLexer.NextToken();
  return true;
}

// ParseStmt consumes ';'
bool Parser::ParseStmt(Function *func) {
  mAutomata->mStack.clear();
  TokenKind tk = mLexer.GetToken();
  bool isDecl = (mAutomata->IsType(tk));
  while (tk != TK_Invalid) {
    switch (tk) {
      case TK_Name:
      {
        std::string s = mLexer.GetTokenString();
        stridx_t stridx = mModule->mStrTable.GetOrCreateGstridxFromName(s);
        Symbol *sb = currfunc->GetSymbol(stridx);
        // either in decl or sb should be exist
        MASSERT(isDecl || sb);
        if (!sb) {
          sb = new Symbol(stridx, inttyidx);
          currfunc->mSymbolTable.push_back(sb);
        }
        // found Identifier
        RuleBase *r = mAutomata->mBaseGen->FindRule("Identifier");
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
        std::string str = GetTokenKindString(tk);
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
    if (tk == TK_Semicolon) {
      break;
    }
    tk = mLexer.NextToken();
  }

  mAutomata->ProcessStack();

  tk = mLexer.NextToken();
  return true;
}

void Parser::Dump() {
  std::cout << "\n================= Code ===========" << std::endl;
  std::cout << "==================================" << std::endl;
}
