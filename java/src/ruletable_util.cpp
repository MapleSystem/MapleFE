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
}

// The Lexer cursor moves if found target, or restore the original location.
bool RuleTableWalker::TraverseTableData(TableData *data) {
  unsigned old_pos = mLexer->curidx;
  bool found = false;

  switch (data->mType) {

  case DT_Char:
    if(*(mLexer->line + mLexer->curidx) == data->mData.mChar) {
      found = true;
      mLexer->curidx++;
    }
    break;

  case DT_String:
    if( !strncmp(mLexer->line + mLexer->curidx, data->mData.mString, strlen(data->mData.mString))) { 
      // Need to make sure the following text is a separator
      mLexer->curidx += strlen(data->mData.mString); 
      if (TraverseSepTable() != SEP_NA) {
        // TraverseSepTable() moves 'curidx', need restore it
        mLexer->curidx = old_pos + strlen(data->mData.mString); 
        // Put into StringPool
        mLexer->mStringPool.FindString(data->mData.mString);
        found = true;
      }
     }

     // If not found, restore curidx
     if (!found)
       mLexer->curidx = old_pos;

    break;

  // Only separator, operator, keywords are saved as DT_Token. During Lexing, these 3 types
  // are processed through traversing the 3 arrays: SeparatorTable, OperatorTable, KeywordTable.
  // So this case won't be hit during Lex.
  //
  // However, it does hit this case during matching.
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
// !!!!!!!!! right now, This just travese for one single token  !!!!!!!!!!!!!!!
///////////////////////////////////////////////////////////////////////////////////

bool RuleTableWalker::Traverse(const RuleTable *rule_table) {
  bool matched = false;

  // save the original location
  unsigned old_pos = mLexer->curidx;

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
// NOTE: Identifier table is always Hard Coded as TblIDENTIFIER.
const char* GetIdentifier(Lexer *lex) {
  RuleTableWalker walker(&TblIDENTIFIER, lex);
  unsigned old_pos = lex->GetCuridx();
  bool found = walker.Traverse(&TblIDENTIFIER);
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
LitData GetLiteral(Lexer *lex) {
  RuleTableWalker walker(&TblLiteral, lex);
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
      return;
    }
    // 2. Try operator.
    OprId oid = TraverseOprTable_core(NULL, data->mData.mChar, len);
    if (oid != OPR_NA) {
      Token *token = FindOperatorToken(lex, oid);
      data->mType = DT_Token;
      data->mData.mToken = token;
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
      return;
    }
    // 2. Try operator.
    OprId oid = TraverseOprTable_core(data->mData.mString, 0, len);
    if (oid != OPR_NA) {
      Token *token = FindOperatorToken(lex, oid);
      data->mType = DT_Token;
      data->mData.mToken = token;
      return;
    }
    // 3. Try keyword.
    //    Need to make sure string is put in Lexer::StringPool, a request of
    //    FindKeywordToken(lex, key);
    char *str = lex->mStringPool.FindString(data->mData.mString);
    char *key = TraverseKeywordTable_core(str, 0, len);
    if (key) {
      Token *token = FindKeywordToken(lex, key);
      data->mType = DT_Token;
      data->mData.mToken = token;
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

void PlantTokensInRuleTables(Lexer *lex) {
  PlantTraverseRuleTable(&TblStatement, lex);
}

