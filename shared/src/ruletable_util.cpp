#include <cstring>

#include "ruletable_util.h"
#include "lexer.h"
#include "lang_spec.h"
#include "massert.h"
#include "common_header_autogen.h"

////////////////////////////////////////////////////////////////////////////////////
//                Utility Functions to walk tables
////////////////////////////////////////////////////////////////////////////////////

// Return the separator ID, if it's. Or SEP_NA.
//        len : length of matching text.
// Only one of 'str' or 'c' will be valid.
//
// Assumption: Table has been sorted by length.
SepId FindSeparator(const char *str, const char c, unsigned &len) {
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
OprId FindOperator(const char *str, const char c, unsigned &len) {
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
const char* FindKeyword(const char *str, const char c, unsigned &len) {
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
  SepId id = FindSeparator(mLexer->line + mLexer->curidx, 0, len);
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
  OprId id = FindOperator(mLexer->line + mLexer->curidx, 0, len);
  if (id != OPR_NA) {
    mLexer->curidx += len;
    return id;
  }
  return OPR_NA;
}

// Return the keyword name, or else NULL.
const char* RuleTableWalker::TraverseKeywordTable() {
  unsigned len = 0;
  const char *addr = FindKeyword(mLexer->line + mLexer->curidx, 0, len);
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

