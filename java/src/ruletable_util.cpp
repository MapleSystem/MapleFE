#include <cstring>

#include "ruletable_util.h"
#include "lexer.h"
#include "lang_spec.h"
#include "massert.h"
#include "common_header_autogen.h"

////////////////////////////////////////////////////////////////////////////////////
//                Utility Functions to walk tables
////////////////////////////////////////////////////////////////////////////////////

extern Token* FindSeparatorToken(Lexer*, SepId);
extern Token* FindOperatorToken(Lexer*, OprId);
extern Token* FindKeywordToken(Lexer*, char*);

// Return the separator ID, if it's. Or SEP_NA.
//        len : length of matching text.
// Only one of 'str' or 'c' will be valid.
//
// Assumption: Table has been sorted by length.
static SepId TraverseSepTable_core(const char *str, const char c, unsigned &len) {
  std::string text;
  if (str)
    text = str;
  else
    text = c;

  unsigned i = 0;
  for (; i < SEP_NA; i++) {
    SepTableEntry e = SepTable[i];
    if (!strncmp(text.c_str(), e.mText, strlen(e.mText))) {
      len = strlen(e.mText);
      return e.mId;
    }
  }

  return SEP_NA;
}

// Returen the operator ID, if it's. Or OPR_NA.
// Only one of 'str' or 'c' will be valid.
//
// Assumption: Table has been sorted by length.
static OprId TraverseOprTable_core(const char *str, const char c, unsigned &len) {
  std::string text;
  if (str)
    text = str;
  else
    text = c;

  unsigned i = 0;
  for (; i < OPR_NA; i++) {
    OprTableEntry e = OprTable[i];
    if (!strncmp(text.c_str(), e.mText, strlen(e.mText))) {
      len = strlen(e.mText);
      return e.mId;
    }
  }

  return OPR_NA;
}

// Return the keyword name, or else NULL.
// Only one of 'str' or 'c' will be valid.
//
// Assumption: Table has been sorted by length.
static const char* TraverseKeywordTable_core(const char *str, const char c, unsigned &len) {
  std::string text;
  if (str)
    text = str;
  else
    text = c;

  const char *addr = NULL;
  unsigned i = 0;
  for (; i < KeywordTableSize; i++) {
    KeywordTableEntry e = KeywordTable[i];
    if (!strncmp(text.c_str(), e.mText, strlen(e.mText))) {
      len = strlen(e.mText);
      return e.mText;
    }
  }

  return NULL;
}

//////////////////////////////////////////////////////////////////////////////////////////

RuleTableWalker::RuleTableWalker(const RuleTable *t, Lexer *l) {
  mTable = t;
  mLexer = l;
  mTokenNum = 0;
  mCheckSeparator = true;
}

// The Lexer cursor moves if found target, or restore the original location.
bool RuleTableWalker::TraverseTableData(TableData *data) {
  unsigned old_pos = mLexer->curidx;
  bool found = false;

  switch (data->mType) {

  // The first thinking is I also want to check if the next text after 'curidx' is a separtor.
  // This is the case in parsing a DT_String. However, we have many rules handling DT_Char
  // and they don't expect the following to be a separator. For example, the DecimalNumeral rule
  // specifies its content char by char, so in this case we don't check if the next is a separator.
  case DT_Char:
    if(*(mLexer->line + mLexer->curidx) == data->mData.mChar) {
      mLexer->curidx += 1;
      found = true;
    }
    break;

  case DT_String:
    if( !strncmp(mLexer->line + mLexer->curidx, data->mData.mString, strlen(data->mData.mString))) { 
      // Need to make sure the following text is a separator
      mLexer->curidx += strlen(data->mData.mString); 
      if (mCheckSeparator && (TraverseSepTable() != SEP_NA)) {
        // TraverseSepTable() moves 'curidx', need restore it
        mLexer->curidx = old_pos + strlen(data->mData.mString); 
        // Put into StringPool
        mLexer->mStringPool.FindString(data->mData.mString);
        found = true;
      } else {
        found = true;
      }
    }

    // If not found, restore curidx
    if (!found)
      mLexer->curidx = old_pos;

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
      mLexer->curidx = old_pos;

    break;
  }

  case DT_Null:
  default:
    break;
  }

  return found;
}

///////////////////////////////////////////////////////////////////////////////////
//                       This is a general interface                             //
//
// Traverse the rule table by reading tokens through lexer.
// Sub Tables are traversed recursively.
//
// It returns true : if RuleTable is met
//           false : if failed
//
// The mLexer->curidx will move if we successfully found something. So the string
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
// The algorithm is a loop walking on the mLexer->line, starting at mLexer->curidx.
// Suppose it's saved at init_idx.
//   1. We skip ZEROORXXX and try to match LAST_ELEMENT. Suppose the starting index is
//      saved as start_idx
//   2. If success, we continue matching line[init_idx, start_idx] against ZEROORXXX
//   3. if success again, we move start_idx by one, repeat step 1.
// This is just a rough idea. Many border conditions will be checked while walking.
///////////////////////////////////////////////////////////////////////////////////

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

bool RuleTableWalker::TraverseSecondTry(const RuleTable *rule_table) {
  // save the status
  char *old_line = mLexer->line;
  unsigned old_pos = mLexer->curidx;

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
  //               2. mLexer->curidx points to the part for the ending two element.
  //               3. We need find the longest match.

  // These four are the final result if success.
  // [NOTE] The reason I use 'the one after' as xxx_end, is to check if TraverseTableData()
  // really moves the curidx. Or in another word, if it matches anything or nother.
  // If it matches something, xxx_end will be greater than xxx_start, or else they are equal.
  unsigned yyy_start = mLexer->curidx;
  unsigned yyy_end = 0;                     // the one after last char
  unsigned zeroxxx_start = mLexer->curidx;  // A fixed index
  unsigned zeroxxx_end = 0;                 // the one after last char

  // These four are the working result.
  unsigned w_yyy_start = mLexer->curidx;
  unsigned w_yyy_end = 0;                     // the one after last char
  unsigned w_zeroxxx_start = mLexer->curidx;  // A fixed index
  unsigned w_zeroxxx_end = 0;                 // the one after last char, 

  TableData *zeroxxx = rule_table->mData + i;
  TableData *yyy = rule_table->mData + (i + 1);

  found = false;

  while (1) {
    // step 3.1 try yyy first.
    mLexer->line = old_line;
    mLexer->curidx = w_yyy_start;

    bool temp_found = TraverseTableData(yyy);
    if (!temp_found)
      break;
    else
      w_yyy_end = mLexer->curidx;

    // step 3.2 try zeroxxx
    //          Have to build a line for the lexer.
    unsigned len = w_yyy_start - w_zeroxxx_start;
    if (len) {
      char *newline = (char*)malloc(len);
      strncpy(newline, mLexer->line + w_zeroxxx_start, len);
      mLexer->line = newline;
      mLexer->curidx = 0;

      // 1. It always get true since it's a zeroxxx
      // 2. It may match nothing, meaning mLexer->curidx never get a chance to move one step.
      temp_found = TraverseTableData(zeroxxx);
      MASSERT(temp_found && "Zeroxxx didn't return true.");
      free(newline);

      w_zeroxxx_end = mLexer->curidx;
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

  // adjust the status of mLexer
  if (found) {
    mLexer->line = old_line;
    mLexer->curidx = yyy_end;
    return true;
  } else {
    mLexer->line = old_line;
    mLexer->curidx = old_pos;
    return false;
  }
}

bool RuleTableWalker::Traverse(const RuleTable *rule_table) {
  bool matched = false;

  // save the original location
  unsigned old_pos = mLexer->curidx;

  // Look into rule_table's data
  EntryType type = rule_table->mType;

  switch(type) {
  case ET_Oneof: {
    // We need find the longest match.
    bool found = false;
    unsigned new_pos = mLexer->curidx;
    for (unsigned i = 0; i < rule_table->mNum; i++) {
      TableData *data = rule_table->mData + i;
      bool temp_found = TraverseTableData(data);
      if (temp_found) {
        found = true;
        if (mLexer->curidx > new_pos)
          new_pos = mLexer->curidx;
        // Need restore curidx for the next try.
        mLexer->curidx = old_pos;
      }
    }
    mLexer->curidx = new_pos;
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
      mLexer->curidx = old_pos;
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
    mLexer->curidx = old_pos;
    return false;
  }
}

// Return the separator ID, if it's. Or SEP_NA.
// Assuming the separator table has been sorted so as to catch the longest separator
//   if possible.
SepId RuleTableWalker::TraverseSepTable() {
  unsigned len = 0;
  SepId id = TraverseSepTable_core(mLexer->line + mLexer->curidx, 0, len);
  if (id != SEP_NA) {
    mLexer->curidx += len;
    return id;
  }
  return SEP_NA;
}

// Returen the operator ID, if it's. Or OPR_NA.
// Assuming the operator table has been sorted so as to catch the longest separator
//   if possible.
OprId RuleTableWalker::TraverseOprTable() {
  unsigned len = 0;
  OprId id = TraverseOprTable_core(mLexer->line + mLexer->curidx, 0, len);
  if (id != OPR_NA) {
    mLexer->curidx += len;
    return id;
  }
  return OPR_NA;
}

// Return the keyword name, or else NULL.
const char* RuleTableWalker::TraverseKeywordTable() {
  unsigned len = 0;
  const char *addr = TraverseKeywordTable_core(mLexer->line + mLexer->curidx, 0, len);
  if (addr) {
    unsigned saved_curidx = mLexer->curidx;

    // It's a keyword only if the following is a separator
    mLexer->curidx += len;
    if (TraverseSepTable() != SEP_NA) {
      // TraverseSepTable() moves 'curidx', need move it back to after keyword
      mLexer->curidx = saved_curidx + len;
      addr = mLexer->mStringPool.FindString(addr);
      return addr;
    } else {
      // failed, restore curidx
      mLexer->curidx = saved_curidx;
    }
  }
  return NULL;
}

////////////////////////////////////////////////////////////////////////////////
//                 Implementation of External Interfaces                      //
////////////////////////////////////////////////////////////////////////////////

// Returen the separator ID, if it's. Or SEP_NA.
SepId GetSeparator(Lexer *lex) {
  RuleTableWalker walker(NULL, lex);
  return walker.TraverseSepTable();
}

// Returen the operator ID, if it's. Or OPR_NA.
OprId GetOperator(Lexer *lex) {
  RuleTableWalker walker(NULL, lex);
  return walker.TraverseOprTable();
}

// keyword string was put into StringPool by walker.TraverseKeywordTable().
const char* GetKeyword(Lexer *lex) {
  RuleTableWalker walker(NULL, lex);
  const char *addr = walker.TraverseKeywordTable();
  return addr;
}

// identifier string was put into StringPool.
// NOTE: Identifier table is always Hard Coded as TblIdentifier.
const char* GetIdentifier(Lexer *lex) {
  RuleTableWalker walker(&TblIdentifier, lex);
  unsigned old_pos = lex->GetCuridx();
  bool found = walker.Traverse(&TblIdentifier);
  if (found) {
    unsigned len = lex->GetCuridx() - old_pos;
    MASSERT(len > 0 && "found token has 0 data?");
    std::string s(lex->GetLine() + old_pos, len);
    const char *addr = lex->mStringPool.FindString(s);
    return addr;
  } else {
    lex->SetCuridx(old_pos);
    return NULL;
  }
}

// NOTE: Literal table is TblLiteral.
//
// Literal rules are special, an element of the rules may be a char, or a string, and they
// are not followed by separators. They may be followed by another char or string. So we
// don't check if the following is a separator or not.
LitData GetLiteral(Lexer *lex) {
  RuleTableWalker walker(&TblLiteral, lex);
  walker.mCheckSeparator = false;

  unsigned old_pos = lex->GetCuridx();

  LitData ld;
  ld.mType = LT_NA;
  bool found = walker.Traverse(&TblLiteral);
  if (found) {
    unsigned len = lex->GetCuridx() - old_pos;
    MASSERT(len > 0 && "found token has 0 data?");
    std::string s(lex->GetLine() + old_pos, len);
    const char *addr = lex->mStringPool.FindString(s);
    // We just support integer token right now. Value is put in LitData.mData.mStr
    ld = ProcessLiteral(LT_IntegerLiteral, addr);
  } else {
    lex->SetCuridx(old_pos);
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
bool GetComment(Lexer *lex) {
  if (lex->line[lex->curidx] == '/' && lex->line[lex->curidx+1] == '/') {
    lex->curidx = lex->current_line_size;
    return true;
  }

  // Handle comments in /* */
  // If there is a /* without ending */, the rest of code until the end of the current
  // source file will be treated as comment.
  if (lex->line[lex->curidx] == '/' && lex->line[lex->curidx+1] == '*') {
    lex->curidx += 2; // skip /*
    bool get_ending = false;  // if we get the ending */

    // the while loop stops only at either (1) end of file (2) finding */
    while (1) {
      if (lex->curidx == lex->current_line_size) {
        if (lex->ReadALine() < 0)
          return true;
        lex->_linenum++;  // a new line read.
      }
      if ((lex->line[lex->curidx] == '*' && lex->line[lex->curidx+1] == '/')) {
        get_ending = true;
        lex->curidx += 2;
        break;
      }
      lex->curidx++;
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

// The traversal is very simple depth first.
static void PlantTraverseRuleTable(RuleTable *table, Lexer *lex);

static void PlantTraverseTableData(TableData *data, Lexer *lex) {
  switch (data->mType) {
  case DT_Char: {
    unsigned len = 0;
    // 1. Try separator.
    SepId sid = TraverseSepTable_core(NULL, data->mData.mChar, len);
    if (sid != SEP_NA) {
      Token *token = FindSeparatorToken(lex, sid);
      data->mType = DT_Token;
      data->mData.mToken = token;
      //std::cout << "In Plant 1, plant token " << token << std::endl;
      return;
    }
    // 2. Try operator.
    OprId oid = TraverseOprTable_core(NULL, data->mData.mChar, len);
    if (oid != OPR_NA) {
      Token *token = FindOperatorToken(lex, oid);
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
    SepId sid = TraverseSepTable_core(data->mData.mString, 0, len);
    if (sid != SEP_NA) {
      Token *token = FindSeparatorToken(lex, sid);
      data->mType = DT_Token;
      data->mData.mToken = token;
      //std::cout << "In Plant 3, plant token " << token << std::endl;
      return;
    }
    // 2. Try operator.
    OprId oid = TraverseOprTable_core(data->mData.mString, 0, len);
    if (oid != OPR_NA) {
      Token *token = FindOperatorToken(lex, oid);
      data->mType = DT_Token;
      data->mData.mToken = token;
      //std::cout << "In Plant 4, plant token " << token << std::endl;
      return;
    }
    // 3. Try keyword.
    //    Need to make sure string is put in Lexer::StringPool, a request of
    //    FindKeywordToken(lex, key);
    char *key = TraverseKeywordTable_core(data->mData.mString, 0, len);
    if (key) {
      key = lex->mStringPool.FindString(key);
      Token *token = FindKeywordToken(lex, key);
      data->mType = DT_Token;
      data->mData.mToken = token;
      //std::cout << "In Plant 5, plant token " << token << std::endl;
      return;
    }
    break;
  }

  case DT_Subtable: {
    RuleTable *t = data->mData.mEntry;
    PlantTraverseRuleTable(t, lex);
  }

  case DT_Type:
  case DT_Token:
  case DT_Null:
  default:
    break;
  }
}

// The traversal is very simple depth first.
static void PlantTraverseRuleTable(RuleTable *table, Lexer *lex) {
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
      PlantTraverseTableData(data, lex);
    }
    break;
  }
  case ET_Null:
  default:
    break;
  }
}

void PlantTokens(Lexer *lex) {
  PlantTraverseRuleTable(&TblStatement, lex);
}

