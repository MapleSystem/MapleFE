//////////////////////////////////////////////////////////////////////////
// This file contains all the information to describe the tables that
// autogen creates. 
//////////////////////////////////////////////////////////////////////////

#ifndef __RULE_TABLE_H__
#define __RULE_TABLE_H__

// The list of all supported types. This covers all the languages.
// NOTE: autogen also relies on this set of supported separators
#undef  TYPE
#define TYPE(T) TY_##T,
typedef enum {
#include "supported_types.def"
TY_NA
}TypeId;

// The list of all supported separators. This covers all the languages.
// NOTE: autogen also relies on this set of supported separators
#undef  SEPARATOR
#define SEPARATOR(N, T) SEP_##T,
typedef enum {
#include "supported_separators.def"
SEP_NA
}SepId;

// The list of all supported operators. This covers all the languages.
// NOTE: autogen also relies on this set of supported operators.
#undef  OPERATOR
#define OPERATOR(T) OPR_##T,
typedef enum {
#include "supported_operators.def"
OPR_NA
}OprId;

///////////////////////////////////////////////////////////////////////////
//                       Rule Table                                      //
//
// The most important thing to know about RuleTable is it takes two rounds
// of generation.
//   (1) Autogen generates .h/.cpp files with rule tables in
//       there. In the TableData there is DT_Token because tokens are created
//       when the language parser is running.
//   (2) When the parser starts, we replace TableData entries which are
//       keyword, separator, operator with tokens. This happens in memory.
// The reason we need token is to save the time of matching a rule. Lexer
// returns a set of tokens, so it's faster if parts of a rule are tokens
// to compare. 
///////////////////////////////////////////////////////////////////////////

// The list of RuleTable Entry types
typedef enum {
  ET_Oneof,      // one of (...)
  ET_Zeroormore, // zero or more of (...)
  ET_Zeroorone,  // zero or one ( ... )
  ET_Concatenate,// Elem + Elem + Elem
  ET_Data,       // data, further categorized into DT_Char, DT_String, ...
  ET_Null
}EntryType;

// List of data types
typedef enum {
  DT_Char,       // It's a literal elements, char, 'c'.
  DT_String,     // It's a literal elements, string "abc".
  DT_Type,       // It's a type id
  DT_Token,      // It's a token
  DT_Subtable,   // sub-table
  DT_Null
}DataType;

struct RuleTable;

class Token;

struct TableData {
  DataType mType;
  union {
    RuleTable  *mEntry;  // sub-table
    char        mChar;
    const char *mString;
    TypeId      mTypeId;
    Token      *mToken;
  }mData;
};

// TODO: The action id will come from both the shared part and language specific part.
//       For now I just put everything together in order to expediate the overall
//       progress. Will come back.

#undef  ACTION
#define ACTION(T) ACT_##T,
typedef enum {
#include "supported_actions.def"
ACT_NA
}ActionId;

// We give the biggest number of elements in an action to 12
// Please keep this number the same as the one in Autogen. We verify this number
// in Autogen to make sure it doesn't exceed.
#define MAX_ACT_ELEM_NUM 12

struct RuleAction {
  ActionId  mId;
  unsigned  mNumElem;
  unsigned  mElems[MAX_ACT_ELEM_NUM]; // the index of elements involved in the action
                                      // As mentioned in README.spec, the index
                                      // starts from 1. So 0 means nothing.
};

// Struct of the table entry
struct RuleTable{
  EntryType   mType;
  unsigned    mNum;        // Num of TableData entries
  TableData  *mData;
  unsigned    mNumAction;  // Num of actions
  RuleAction *mActions;
};

//////////////////////////////////////////////////////////////////////
//                    Literal   Table                               //
//////////////////////////////////////////////////////////////////////

extern const RuleTable LiteralTable;

//////////////////////////////////////////////////////////////////////
//                       Type   Table                               //
// Type tables contain two parts. The keyword tables and rule tables//
//////////////////////////////////////////////////////////////////////

// The declaration and definition of TYPE KEYWORD TABLE are generated
// in LANGUAGE/include/gen_type.h and LANGUAGE/src/gen_type.cpp
struct TypeKeyword {
  const char *mText;
  TypeId      mId;
};

// The rule table of types
extern const RuleTable TypeTable;

//////////////////////////////////////////////////////////////////////
//                    Separator Table                               //
// separator tables are merely generated from STRUCT defined in     //
// separator.spec, so they are NOT rule table.                      //
//////////////////////////////////////////////////////////////////////

struct SepTableEntry {
  const char *mText;
  SepId       mId;
};

//////////////////////////////////////////////////////////////////////
//                    Operator  Table                               //
// operator tables are merely generated from STRUCT defined in      //
// operator.spec, so they are NOT rule table.                       //
//////////////////////////////////////////////////////////////////////

struct OprTableEntry {
  const char *mText;
  OprId       mId;
};

//////////////////////////////////////////////////////////////////////
//                    Keyword   Table                               //
// keyword tables are merely generated from STRUCT defined in       //
// keyword.spec.                                                    //
//////////////////////////////////////////////////////////////////////

struct KeywordTableEntry {
  const char *mText;
};

#endif
