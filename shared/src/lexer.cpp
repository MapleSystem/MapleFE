/*
* Copyright (C) [2020] Futurewei Technologies, Inc. All rights reverved.
*
* OpenArkFE is licensed under the Mulan PSL v1.
* You can use this software according to the terms and conditions of the Mulan PSL v1.
* You may obtain a copy of Mulan PSL v1 at:
*
*  http://license.coscl.org.cn/MulanPSL
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR
* FIT FOR A PARTICULAR PURPOSE.
* See the Mulan PSL v1 for more details.
*/
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
      mCheckSeparator = true;

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
  return TraverseSepTable();
}

// Returen the operator ID, if it's. Or OPR_NA.
OprId Lexer::GetOperator() {
  return TraverseOprTable();
}

// keyword string was put into StringPool by walker.TraverseKeywordTable().
const char* Lexer::GetKeyword() {
  const char *addr = TraverseKeywordTable();
  return addr;
}

// identifier string was put into StringPool.
// NOTE: Identifier table is always Hard Coded as TblIdentifier.
const char* Lexer::GetIdentifier() {
  unsigned old_pos = GetCuridx();
  bool found = Traverse(&TblIdentifier);
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
  mCheckSeparator = false;

  unsigned old_pos = GetCuridx();

  LitData ld;
  ld.mType = LT_NA;
  bool found = Traverse(&TblLiteral);
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

  mCheckSeparator = true;

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

//////////////////////////////////////////////////////////////////////////////////////////
//           Traverse The Rule Tables during Lexing
//
// Traverse the rule table by reading tokens through lexer.
// Sub Tables are traversed recursively.
//
// It returns true : if RuleTable is met
//           false : if failed
//
// The curidx will move if we successfully found something. So the string
// of found token can be obtained by the difference of 'new' and 'old' curidx.
//
//  ======================= About Second Try ============================
//
// According to the spec of some languages, e.g. Java. It has productions for Hex
// numerals as below.
//
// rule HexDigitOrUnderscore : ONEOF(HexDigit, '_')
// rule HexDigitsAndUnderscores:HexDigitOrUnderscore + ZEROORMORE(HexDigitOrUnderscore)
// rule HexDigits  : ONEOF(HexDigit,
//             ------->    HexDigit + ZEROORONE(HexDigitsAndUnderscores) + HexDigit)
//
// The problem comes from the line pointed by arrow. Look at the ZEROORONE(...),
// it's easy to see that for a string like "123", ZEROORONE(...) will eat up all the
// characters until the end of string. Which means the third element, or the HexDigit
// at the end will never get a chance to match.
//
// So we come up with a second try. The idea is simple, we want to try a second time.
// [NOTE] we only handle the case where the production has the following patten,
//           ... + ZEROORONE(...) + LASTELEMENT  or
//           ... + ZEROORMOREE(...) + LASTELEMENT
// [NOTE] We handle only one ZEROORMORE or ZEROORONE case.
//
// The algorithm is a loop walking on the line, starting at curidx.
// Suppose it's saved at init_idx.
//   1. We skip ZEROORXXX and try to match LAST_ELEMENT. Suppose the starting index is
//      saved as start_idx
//   2. If success, we continue matching line[init_idx, start_idx] against ZEROORXXX
//   3. if success again, we move start_idx by one, repeat step 1.
// This is just a rough idea. Many border conditions will be checked while walking.
///////////////////////////////////////////////////////////////////////////////////


// The Lexer cursor moves if found target, or restore the original location.
bool Lexer::TraverseTableData(TableData *data) {
  unsigned old_pos = curidx;
  bool found = false;

  switch (data->mType) {

  // The first thinking is I also want to check if the next text after 'curidx' is a separtor
  // or operator.
  //
  // This is the case in parsing a DT_String. However, we have many rules handling DT_Char
  // and they don't expect the following to be a separator. For example, the DecimalNumeral rule
  // specifies its content char by char, so in this case we don't check if the next is a separator.
  case DT_Char:
    if(*(line + curidx) == data->mData.mChar) {
      curidx += 1;
      found = true;
    }
    break;

  case DT_String:
    if( !strncmp(line + curidx, data->mData.mString, strlen(data->mData.mString))) {
      // Need to make sure the following text is a separator
      curidx += strlen(data->mData.mString);
      if (mCheckSeparator && (TraverseSepTable() != SEP_NA) && (TraverseOprTable() != OPR_NA)) {
        // TraverseSepTable() moves 'curidx', need restore it
        curidx = old_pos + strlen(data->mData.mString);
        // Put into StringPool
        mStringPool.FindString(data->mData.mString);
        found = true;
      } else {
        found = true;
      }
    }

    // If not found, restore curidx
    if (!found)
      curidx = old_pos;

    break;

  // Only separator, operator, keywords are planted as DT_Token. During Lexing, these 3 types
  // are processed through traversing the 3 arrays: SeparatorTable, OperatorTable, KeywordTable.
  // So this case won't be hit during Lex. We just ignore it.
  //
  // However, it does hit this case during matching. We will handle this case in matching process.
  case DT_Token:
    break;

  case DT_Type:
    break;

  case DT_Subtable: {
    RuleTable *t = data->mData.mEntry;
    found = Traverse(t);

    if (!found)
      curidx = old_pos;

    break;
  }

  case DT_Null:
  default:
    break;
  }

  return found;
}

static bool IsZeroorxxxTableData (const TableData *td) {
  bool is_zero_xxx = false;
  switch (td->mType) {
  case DT_Subtable: {
    const RuleTable *t = td->mData.mEntry;
    EntryType type = t->mType;
    if ((type == ET_Zeroorone) || (type == ET_Zeroormore))
      is_zero_xxx = true;
    break;
  }
  default:
    break;
  }
  return is_zero_xxx;
}

bool Lexer::TraverseSecondTry(const RuleTable *rule_table) {
  // save the status
  char *old_line = line;
  unsigned old_pos = curidx;

  bool need_try = false;

  EntryType type = rule_table->mType;
  MASSERT((type == ET_Concatenate) && "Wrong entry type of table.");

  // Step 1. We only handle pattens ending with ... + ZEROORXXX(...) + YYY
  //         Anything else will return false.
  //         So I'll check the condition.
  unsigned i = 0;
  for (; i < rule_table->mNum; i++) {
    TableData *data = rule_table->mData + i;
    if (IsZeroorxxxTableData(data))
      break;
  }
  if (i == rule_table->mNum - 2)
    need_try = true;

  if (!need_try)
    return false;

  i = 0;
  bool found = false;

  // step 2. Let the parts before the ending to finish.
  //         If those fail, we don't need go on.
  for (; i < rule_table->mNum - 2; i++) {
    TableData *data = rule_table->mData + i;
    found = TraverseTableData(data);
    if (!found)
      break;
  }
  if (!found)
    return false;

  // step 3. The working loop.
  //         NOTE: 1. We are lucky here since it's lexing, so don't need cross the line.
  //               2. curidx points to the part for the ending two element.
  //               3. We need find the longest match.

  // These four are the final result if success.
  // [NOTE] The reason I use 'the one after' as xxx_end, is to check if TraverseTableData()
  // really moves the curidx. Or in another word, if it matches anything or nother.
  // If it matches something, xxx_end will be greater than xxx_start, or else they are equal.
  unsigned yyy_start = curidx;
  unsigned yyy_end = 0;             // the one after last char
  unsigned zeroxxx_start = curidx;  // A fixed index
  unsigned zeroxxx_end = 0;         // the one after last char

  // These four are the working result.
  unsigned w_yyy_start = curidx;
  unsigned w_yyy_end = 0;             // the one after last char
  unsigned w_zeroxxx_start = curidx;  // A fixed index
  unsigned w_zeroxxx_end = 0;         // the one after last char,

  TableData *zeroxxx = rule_table->mData + i;
  TableData *yyy = rule_table->mData + (i + 1);

  found = false;

  while (1) {
    // step 3.1 try yyy first.
    line = old_line;
    curidx = w_yyy_start;

    bool temp_found = TraverseTableData(yyy);
    if (!temp_found)
      break;
    else
      w_yyy_end = curidx;

    // step 3.2 try zeroxxx
    //          Have to build a line for the lexer.
    unsigned len = w_yyy_start - w_zeroxxx_start;
    if (len) {
      char *newline = (char*)malloc(len);
      strncpy(newline, line + w_zeroxxx_start, len);
      line = newline;
      curidx = 0;

      // 1. It always get true since it's a zeroxxx
      // 2. It may match nothing, meaning curidx never get a chance to move one step.
      temp_found = TraverseTableData(zeroxxx);
      MASSERT(temp_found && "Zeroxxx didn't return true.");
      free(newline);

      w_zeroxxx_end = curidx;
    }

    // step 3.3 Need check if zeroxxx_end is connected to yyy_start
    if ((w_zeroxxx_end + zeroxxx_start) == w_yyy_start) {
      found = true;
      yyy_start = w_yyy_start;
      yyy_end = w_yyy_end;
      //zeroxxx_start = w_zeroxxx_start; It's a fixed number
      zeroxxx_end = w_zeroxxx_end;
    } else
      break;

    // move yyy start one step
    w_yyy_start++;
  }

  // adjust the status
  if (found) {
    line = old_line;
    curidx = yyy_end;
    return true;
  } else {
    line = old_line;
    curidx = old_pos;
    return false;
  }
}

bool Lexer::Traverse(const RuleTable *rule_table) {
  bool matched = false;

  // save the original location
  unsigned old_pos = curidx;

  // Look into rule_table's data
  EntryType type = rule_table->mType;

  switch(type) {
  case ET_Oneof: {
    // We need find the longest match.
    bool found = false;
    unsigned new_pos = curidx;
    for (unsigned i = 0; i < rule_table->mNum; i++) {
      TableData *data = rule_table->mData + i;
      bool temp_found = TraverseTableData(data);
      if (temp_found) {
        found = true;
        if (curidx > new_pos)
          new_pos = curidx;
        // Need restore curidx for the next try.
        curidx = old_pos;
      }
    }
    curidx = new_pos;
    matched = found; 
    break;
  }

  // Lexer moves until hit a NON-target data
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

    if (!found) {
      curidx = old_pos;
      found = TraverseSecondTry(rule_table);
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
    curidx = old_pos;
    return false;
  }
}

// Return the separator ID, if it's. Or SEP_NA.
// Assuming the separator table has been sorted so as to catch the longest separator
//   if possible.
SepId Lexer::TraverseSepTable() {
  unsigned len = 0;
  SepId id = FindSeparator(line + curidx, 0, len);
  if (id != SEP_NA) {
    curidx += len;
    return id;
  }
  return SEP_NA;
}

// Returen the operator ID, if it's. Or OPR_NA.
// Assuming the operator table has been sorted so as to catch the longest separator
//   if possible.
OprId Lexer::TraverseOprTable() {
  unsigned len = 0;
  OprId id = FindOperator(line + curidx, 0, len);
  if (id != OPR_NA) {
    curidx += len;
    return id;
  }
  return OPR_NA;
}

// Return the keyword name, or else NULL.
const char* Lexer::TraverseKeywordTable() {
  unsigned len = 0;
  const char *addr = FindKeyword(line + curidx, 0, len);
  if (addr) {
    unsigned saved_curidx = curidx;

    // It's a keyword only if the following is a separator
    curidx += len;
    if (TraverseSepTable() != SEP_NA) {
      // TraverseSepTable() moves 'curidx', need move it back to after keyword
      curidx = saved_curidx + len;
      addr = mStringPool.FindString(addr);
      return addr;
    } else {
      // failed, restore curidx
      curidx = saved_curidx;
    }
  }
  return NULL;
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

