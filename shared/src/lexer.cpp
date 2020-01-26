#include <iostream>
#include <cmath>
#include "massert.h"
#include "lexer.h"
#include "token.h"
#include "common_header_autogen.h"
#include "ruletable_util.h"
#include "gen_debug.h"
#include "massert.h"
#include <climits>
#include <cstdlib>

#include "ruletable_util.h"

#define MAX_LINE_SIZE 4096

static unsigned DigitValue(char c) {
  if (c >= '0' && c <= '9')
    return c - '0';
  if (c >= 'A' && c <= 'F')
    return c - 'A' + 10;
  if (c >= 'a' && c <= 'f')
    return c - 'a' + 10;
  std::cout << "Character not a number" << std::endl;
  exit(1);
}

/* Read (next) line from the input file, and return the readed
   number of chars.
   if the line is empty (nothing but a newline), returns 0.
   if EOF, return -1.
   The trailing new-line character has been removed.
 */
int Lexer::ReadALine() {
  if (!srcfile) {
    line[0] = '\0';
    return -1;
  }

  current_line_size = getline(&line, &linebuf_size, srcfile);
  if (current_line_size <= 0) {  // EOF
    fclose(srcfile);
    line[0] = '\0';
    endoffile = true;
  } else {
    if (line[current_line_size - 1] == '\n') {
      line[current_line_size - 1] = '\0';
      current_line_size--;
    }
  }

  curidx = 0;
  return current_line_size;
}

Lexer::Lexer()
  : thename(""),
    theintval(0),
    thefloatval(0.0f),
    thedoubleval(0),
    verboseLevel(0),
    srcfile(nullptr),
    line(nullptr),
    linebuf_size(0),
    current_line_size(0),
    curidx(0),
    endoffile(false),
    mPredefinedTokenNum(0),
    _linenum(0) {
      seencomments.clear();
      keywordmap.clear();

#define KEYWORD(S, T, I)      \
  {                           \
    std::string str(#S);      \
    keywordmap[str] = TK_##T; \
  }
#include "keywords.def"
#undef KEYWORD
#define OPKEYWORD(S, T, I)    \
  {                           \
    std::string str(S);       \
    keywordmap[str] = TK_##I; \
  }
#include "opkeywords.def"
#undef OPKEYWORD
#define SEPARATOR(S, T)     \
  {                           \
    std::string str(S);       \
    keywordmap[str] = TK_##T; \
  }
#include "supported_separators.def"
#undef SEPARATOR
}

void Lexer::PrepareForFile(const std::string filename) {
  // open file
  srcfile = fopen(filename.c_str(), "r");
  if (!srcfile) {
    MASSERT("cannot open file\n");
  }

  // allocate line buffer.
  linebuf_size = (size_t)MAX_LINE_SIZE;
  line = static_cast<char *>(malloc(linebuf_size));  // initial line buffer.
  if (!line) {
    MASSERT("cannot allocate line buffer\n");
  }

  // try to read the first line
  if (ReadALine() < 0) {
    _linenum = 0;
  } else {
    _linenum = 1;
  }
}

void Lexer::GetName(void) {
  int startidx = curidx;
  if ((isalnum(line[curidx]) || line[curidx] == '_' || line[curidx] == '$' ||
       line[curidx] == '@') &&
      (curidx < (size_t)current_line_size)) {
    curidx++;
  }
  if (line[curidx] == '@' && (line[curidx - 1] == 'h' || line[curidx - 1] == 'f')) {
    curidx++;  // special pattern for exception handling labels: catch or finally
  }

  while ((isalnum(line[curidx]) || line[curidx] == '_') && (curidx < (size_t)current_line_size)) {
    curidx++;
  }
  thename = std::string(&line[startidx], curidx - startidx);
}

std::string Lexer::GetTokenString() {
  TK_Kind tk = mToken->mTkKind;
  return GetTokenString(tk);
}

std::string Lexer::GetTokenString(const TK_Kind tk) {
  std::string temp;
  switch (tk) {
    default: {
      temp = "invalid token";
      break;
    }
#define KEYWORD(N, I, T)    \
    case TK_##I: {          \
      temp = #N;            \
      break;                \
    }
#include "keywords.def"
#undef KEYWORD
#define OPKEYWORD(N, I, T)  \
    case TK_##T: {          \
      temp = N;             \
      break;                \
    }
#include "opkeywords.def"
#undef OPKEYWORD
#define SEPARATOR(N, T)   \
    case TK_##T: {          \
      temp = N;             \
      break;                \
    }
#include "supported_separators.def"
#undef SEPARATOR
    case TK_Intconst: {
      temp = "intconst";
      break;
    }
    case TK_Floatconst: {
      temp = "floatconst";
      break;
    }
    case TK_Doubleconst: {
      temp = "doubleconst";
      break;
    }
    case TK_Name: {
      temp.append(thename);
      break;
    }
    case TK_Newline: {
      temp = "\\n";
      break;
    }
    case TK_Achar: {
      temp = "\'";
      temp += thechar;
      temp.append("\'");
      break;
    }
    case TK_String: {
      temp = "\"";
      temp.append(thename);
      temp.append("\"");
      break;
    }
    case TK_Eof: {
      temp = "EOF";
      break;
    }
  }
  return temp;
}

std::string Lexer::GetTokenKindString() {
  TK_Kind tk = mToken->mTkKind;
  return GetTokenKindString(tk);
}

std::string Lexer::GetTokenKindString(const TK_Kind tkk) {
  std::string temp;
  switch (tkk) {
    default: {
      temp = "Invalid";
      break;
    }
#define KEYWORD(N, I, T)    \
    case TK_##I: {          \
      temp = #I;            \
      break;                \
    }
#include "keywords.def"
#undef KEYWORD
#define OPKEYWORD(N, I, T)  \
    case TK_##T: {          \
      temp = #T;            \
      break;                \
    }
#include "opkeywords.def"
#undef OPKEYWORD
#define SEPARATOR(N, T)   \
    case TK_##T: {          \
      temp = #T;            \
      break;                \
    }
#include "supported_separators.def"
#undef SEPARATOR
#define TOKEN(N, T)         \
    case TK_##T: {          \
      temp = #T;            \
      break;                \
    }
#include "tokens.def"
#undef TOKEN
  }
  return "TK_" + temp;
}

///////////////////////////////////////////////////////////////////////////
//                Utilities for finding predefined tokens
///////////////////////////////////////////////////////////////////////////

Token* Lexer::FindSeparatorToken(SepId id) {
  for (unsigned i = 0; i < mPredefinedTokenNum; i++) {
    Token *token = mTokenPool.mTokens[i];
    if ((token->mTkType == TT_SP) && (((SeparatorToken*)token)->mSepId == id))
      return token;
  }
}

Token* Lexer::FindOperatorToken(OprId id) {
  for (unsigned i = 0; i < mPredefinedTokenNum; i++) {
    Token *token = mTokenPool.mTokens[i];
    if ((token->mTkType == TT_OP) && (((OperatorToken*)token)->mOprId == id))
      return token;
  }
}

// The caller of this function makes sure 'key' is already in the
// string pool of Lexer.
Token* Lexer::FindKeywordToken(char *key) {
  for (unsigned i = 0; i < mPredefinedTokenNum; i++) {
    Token *token = mTokenPool.mTokens[i];
    if ((token->mTkType == TT_KW) && (((KeywordToken*)token)->mName == key))
      return token;
  }
}

// CommentToken is the last predefined token
Token* Lexer::FindCommentToken() {
  Token *token = mTokenPool.mTokens[mPredefinedTokenNum - 1];
  MASSERT((token->mTkType == TT_CM) && "Last predefined token is not a comment token.");
  return token;
}

/////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////

// Read a token until end of file.
// If no remaining tokens in current line, we move to the next line.
Token* Lexer::LexToken(void) {
  return LexTokenNoNewLine();
}

// Read a token until end of line.
// Return NULL if no token read.
Token* Lexer::LexTokenNoNewLine(void) {
  bool is_comment = GetComment();
  if (is_comment) {
    Token *t = FindCommentToken();
    t->Dump();
    return t;
  }

  SepId sep = GetSeparator();
  if (sep != SEP_NA) {
    Token *t = FindSeparatorToken(sep);
    t->Dump();
    return t;
  }

  OprId opr = GetOperator();
  if (opr != OPR_NA) {
    Token *t = FindOperatorToken(opr);
    t->Dump();
    return t;
  }

  const char *keyword = GetKeyword();
  if (keyword != NULL) {
    Token *t = FindKeywordToken(keyword);
    t->Dump();
    return t;
  }

  const char *identifier = GetIdentifier();
  if (identifier != NULL) {
    IdentifierToken *t = (IdentifierToken*)mTokenPool.NewToken(sizeof(IdentifierToken)); 
    new (t) IdentifierToken(identifier);
    t->Dump();
    return t;
  }

  LitData ld = GetLiteral();
  if (ld.mType != LT_NA) {
    LiteralToken *t = (LiteralToken*)mTokenPool.NewToken(sizeof(LiteralToken)); 
    new (t) LiteralToken(TK_Invalid, ld);
    t->Dump();
    return t;
  }

  return NULL;
}

// Returen the separator ID, if it's. Or SEP_NA.
SepId Lexer::GetSeparator() {
  RuleTableWalker walker(NULL, this);
  return walker.TraverseSepTable();
}

// Returen the operator ID, if it's. Or OPR_NA.
OprId Lexer::GetOperator() {
  RuleTableWalker walker(NULL, this);
  return walker.TraverseOprTable();
}

// keyword string was put into StringPool by walker.TraverseKeywordTable().
const char* Lexer::GetKeyword() {
  RuleTableWalker walker(NULL, this);
  const char *addr = walker.TraverseKeywordTable();
  return addr;
}

// identifier string was put into StringPool.
// NOTE: Identifier table is always Hard Coded as TblIdentifier.
const char* Lexer::GetIdentifier() {
  RuleTableWalker walker(&TblIdentifier, this);
  unsigned old_pos = GetCuridx();
  bool found = walker.Traverse(&TblIdentifier);
  if (found) {
    unsigned len = GetCuridx() - old_pos;
    MASSERT(len > 0 && "found token has 0 data?");
    std::string s(GetLine() + old_pos, len);
    const char *addr = mStringPool.FindString(s);
    return addr;
  } else {
    SetCuridx(old_pos);
    return NULL;
  }
}

// NOTE: Literal table is TblLiteral.
//
// Literal rules are special, an element of the rules may be a char, or a string, and they
// are not followed by separators. They may be followed by another char or string. So we
// don't check if the following is a separator or not.
LitData Lexer::GetLiteral() {
  RuleTableWalker walker(&TblLiteral, this);
  walker.mCheckSeparator = false;

  unsigned old_pos = GetCuridx();

  LitData ld;
  ld.mType = LT_NA;
  bool found = walker.Traverse(&TblLiteral);
  if (found) {
    unsigned len = GetCuridx() - old_pos;
    MASSERT(len > 0 && "found token has 0 data?");
    std::string s(GetLine() + old_pos, len);
    const char *addr = mStringPool.FindString(s);
    // We just support integer token right now. Value is put in LitData.mData.mStr
    ld = ProcessLiteral(LT_IntegerLiteral, addr);
  } else {
    SetCuridx(old_pos);
  }

  return ld;
}

// For comments we are not going to generate rule tables since the most common comment
// grammar are used widely in almost every language. So we decided to have a common
// implementation here. In case any language has specific un-usual grammar, they can
// have their own implementation.
//
// The two common comments are
//  (1) //
//      This is the end of line
//  (2) /* .. */
//      This is the traditional comments
//
// Return true if a comment is read. The contents are ignore.
bool Lexer::GetComment() {
  if (line[curidx] == '/' && line[curidx+1] == '/') {
    curidx = current_line_size;
    return true;
  }

  // Handle comments in /* */
  // If there is a /* without ending */, the rest of code until the end of the current
  // source file will be treated as comment.
  if (line[curidx] == '/' && line[curidx+1] == '*') {
    curidx += 2; // skip /*
    bool get_ending = false;  // if we get the ending */

    // the while loop stops only at either (1) end of file (2) finding */
    while (1) {
      if (curidx == current_line_size) {
        if (ReadALine() < 0)
          return true;
        _linenum++;  // a new line read.
      }
      if ((line[curidx] == '*' && line[curidx+1] == '/')) {
        get_ending = true;
        curidx += 2;
        break;
      }
      curidx++;
    }
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////////
//                       Plant Tokens in Rule Tables
//
// RuleTable's are generated by Autogen, at that time there is no Token.
// However, during language parsing we are using tokens from lexer. So it's
// more efficient if the rule tables can have tokens embeded during the traversing
// and matching process.
//
// One point to notice is this Planting process is a traversal of rule tables. It's
// better that we start from the root of the trees. Or there many be several roots.
// This has to be compliant with the Parser::Parse().
//
// NOTE: Here and right now, we take TblStatement as the only root. This is subject
//       to change.
//
////////////////////////////////////////////////////////////////////////////////////

// All rules form many cycles. Don't want to visit them for the second time.
static std::vector<RuleTable *> visited;
static bool IsVisited(RuleTable *table) {
  std::vector<RuleTable *>::iterator it;
  for (it = visited.begin(); it != visited.end(); it++) {
    if (table == *it)
      return true;
  }
  return false;
}

void Lexer::PlantTraverseTableData(TableData *data) {
  switch (data->mType) {
  case DT_Char: {
    unsigned len = 0;
    // 1. Try separator.
    SepId sid = FindSeparator(NULL, data->mData.mChar, len);
    if (sid != SEP_NA) {
      Token *token = FindSeparatorToken(sid);
      data->mType = DT_Token;
      data->mData.mToken = token;
      //std::cout << "In Plant 1, plant token " << token << std::endl;
      return;
    }
    // 2. Try operator.
    OprId oid = FindOperator(NULL, data->mData.mChar, len);
    if (oid != OPR_NA) {
      Token *token = FindOperatorToken(oid);
      data->mType = DT_Token;
      data->mData.mToken = token;
      //std::cout << "In Plant 2, plant token " << token << std::endl;
      return;
    }
    // 3. Try keyword.
    //    Don't need try keyword since there is no one-character keyword
    break;
  }

  //
  case DT_String: {
    unsigned len = 0;
    // 1. Try separator.
    SepId sid = FindSeparator(data->mData.mString, 0, len);
    if (sid != SEP_NA) {
      Token *token = FindSeparatorToken(sid);
      data->mType = DT_Token;
      data->mData.mToken = token;
      //std::cout << "In Plant 3, plant token " << token << std::endl;
      return;
    }
    // 2. Try operator.
    OprId oid = FindOperator(data->mData.mString, 0, len);
    if (oid != OPR_NA) {
      Token *token = FindOperatorToken(oid);
      data->mType = DT_Token;
      data->mData.mToken = token;
      //std::cout << "In Plant 4, plant token " << token << std::endl;
      return;
    }
    // 3. Try keyword.
    //    Need to make sure string is put in Lexer::StringPool, a request of
    //    FindKeywordToken(key);
    char *key = FindKeyword(data->mData.mString, 0, len);
    if (key) {
      key = mStringPool.FindString(key);
      Token *token = FindKeywordToken(key);
      data->mType = DT_Token;
      data->mData.mToken = token;
      //std::cout << "In Plant 5, plant token " << token << std::endl;
      return;
    }
    break;
  }

  case DT_Subtable: {
    RuleTable *t = data->mData.mEntry;
    PlantTraverseRuleTable(t);
  }

  case DT_Type:
  case DT_Token:
  case DT_Null:
  default:
    break;
  }
}

// The traversal is very simple depth first.
void Lexer::PlantTraverseRuleTable(RuleTable *table) {
  if (IsVisited(table))
    return;
  else
    visited.push_back(table);

  switch (table->mType) {
  case ET_Data:
  case ET_Oneof:
  case ET_Zeroormore:
  case ET_Zeroorone:
  case ET_Concatenate: {
    for (unsigned i = 0; i < table->mNum; i++) {
      TableData *data = table->mData + i;
      PlantTraverseTableData(data);
    }
    break;
  }
  case ET_Null:
  default:
    break;
  }
}

void Lexer::PlantTokens() {
  PlantTraverseRuleTable(&TblStatement);
}

