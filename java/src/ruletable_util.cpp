#include "ruletable_util.h"
#include "lexer.h"
#include "massert.h"
#include "common_header_autogen.h"

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

  // The next data should be either non-target or target. The lexer will
  // always stop.
  case ET_Zeroorone:
    break;

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
  case ET_Data:
    break;

  case ET_Null:
  default:
    break;
  }

  if(matched) {
    return true;
  } else {
    mLexer->curidx = old_pos;
    return false;
  }
}

// Returen the separator ID, if it's. Or SEP_NA.
// Assuming the separator table has been sorted so as to catch the longest separator
//   if possible.
SepId RuleTableWalker::TraverseSepTable() {
  unsigned i = 0;
  for (; i < SEP_NA; i++) {
    SepTableEntry e = SepTable[i];
    if (!strncmp(mLexer->line + mLexer->curidx, e.mText, strlen(e.mText))) {
      mLexer->curidx += strlen(e.mText); 
      return e.mId;
    }
  }
  return SEP_NA;
}

// Returen the operator ID, if it's. Or OPR_NA.
// Assuming the operator table has been sorted so as to catch the longest separator
//   if possible.
OprId RuleTableWalker::TraverseOprTable() {
  unsigned i = 0;
  for (; i < OPR_NA; i++) {
    OprTableEntry e = OprTable[i];
    if (!strncmp(mLexer->line + mLexer->curidx, e.mText, strlen(e.mText))) {
      mLexer->curidx += strlen(e.mText); 
      return e.mId;
    }
  }
  return OPR_NA;
}

// Return the keyword name, or else NULL.
const char* RuleTableWalker::TraverseKeywordTable() {
  const char *addr = NULL;
  unsigned i = 0;
  for (; i < KeywordTableSize; i++) {
    KeywordTableEntry e = KeywordTable[i];
    if (!strncmp(mLexer->line + mLexer->curidx, e.mText, strlen(e.mText))) {
      unsigned saved_curidx = mLexer->curidx;

      // Also need to make sure the following text is a separator
      mLexer->curidx += strlen(e.mText); 
      if (TraverseSepTable() != SEP_NA) {

        // TraverseSepTable() moves 'curidx', need move it back to after keyword
        mLexer->curidx = saved_curidx + strlen(e.mText); 

        // Put into StringPool
        addr = mLexer->mStringPool.FindString(e.mText);
        return addr;
      }

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

// keyword string was put into StringPool.
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
    return NULL;
  }
}
