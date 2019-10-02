//////////////////////////////////////////////////////////////////////////
// This file is temporarily contains all the type information to describe
// the tables autogen create. Later on it will be re-organized and merged
// into different module.
//////////////////////////////////////////////////////////////////////////

#ifndef __TEMP_TABLE_H__
#define __TEMP_TABLE_H__

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
#define SEPARATOR(T) SEP_##T,
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
  DT_Subtable,   // sub-table
  DT_Null
}DataType;

struct RuleTable;

struct TableData {
  DataType mType;
  union {
    RuleTable  *mEntry;  // sub-table
    char        mChar;
    const char *mString;
    TypeId      mTypeId;
  }mData;
};

// Struct of the table entry
struct RuleTable{
  EntryType  mType;    
  unsigned   mNum;
  TableData *mData;
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

#endif
