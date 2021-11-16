/*
* Copyright (C) [2020] Futurewei Technologies, Inc. All rights reverved.
*
* OpenArkFE is licensed under the Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*
*  http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR
* FIT FOR A PARTICULAR PURPOSE.
* See the Mulan PSL v2 for more details.
*/
//////////////////////////////////////////////////////////////////////////
// This file contains all the information to describe the tables that
// autogen creates.
//////////////////////////////////////////////////////////////////////////

#ifndef __RULE_TABLE_H__
#define __RULE_TABLE_H__

#include "supported.h"

namespace maplefe {

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
typedef enum EntryType {
  ET_Oneof,      // one of (...)
  ET_Zeroormore, // zero or more of (...)
  ET_Zeroorone,  // zero or one ( ... )
  ET_Concatenate,// Elem + Elem + Elem
  ET_Data,       // data, further categorized into DT_Char, DT_String, ...
  ET_Null
}EntryType;

// List of data types
typedef enum DataType {
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
    RuleTable  *mEntry;   // sub-table
    char        mChar;
    const char *mString;
    TypeId      mTypeId;
    unsigned    mTokenId; // index of a system token
  }mData;
};

// We give the biggest number of elements in an action to 12
// Please keep this number the same as the one in Autogen. We verify this number
// in Autogen to make sure it doesn't exceed.
#define MAX_ACT_ELEM_NUM 12

struct Action {
  ActionId  mId;
  unsigned  mNumElem;
  unsigned  mElems[MAX_ACT_ELEM_NUM]; // the index of elements involved in the action
                                      // As mentioned in README.spec, the index
                                      // starts from 1. So 0 means nothing.
};

enum RuleProp {
  RP_NA = 0,
  RP_Single  = 1, // For a ONEOF rule, there is one and only one children valid.
  RP_ZomFast = 2, // For a Zeroormore rule, it can be a fast parsing.
  RP_Top     = 4, // A top rule
  RP_SecondTry = 8, // A usually lexer rule wanting second try. This is specific
                    // for lex rules. For most lexing rules we just want the longest
                    // match, and it's ok. However, some concatenate rules do require
                    // certain sub-rules NOT to match the longest so that the later
                    // sub-rules can match, and so the whole rule.
  RP_NoAltToken = 16, // don't do alternative token matching for some special tokens
                      // inside the current rule.
};

// A rule has a limited set of beginning tokens. These are called LookAhead.
// During parsing we can check if it matches lookahead. If not, we skip traversal
// so as to expedite the process.
//
// When checking the lookahead, we stop if it's an identifier or literal, in order
// to avoid checking each character of an identifier.
enum LAType {
  LA_Char,
  LA_String,
  LA_Token,    // system token generated by autogen.
  LA_Identifier,
  LA_Literal,
  LA_NA
};

struct LookAhead {
  LAType mType;   // the type of look ahead
  union{
    unsigned char  mChar;
    const char *mString;
    unsigned    mTokenId;
  }mData;
};

struct LookAheadTable {
  unsigned   mNum;
  LookAhead *mData;
};

// This will be an array with NumOfRules elements.
// Generated in gen_lookahead.cpp.
extern LookAheadTable *gLookAheadTable;

// Struct of the table entry
struct RuleTable{
  EntryType   mType;
  unsigned    mProperties; // properties of the rule table.
  unsigned    mNum;        // Num of TableData entries
  TableData  *mData;
  unsigned    mNumAction;  // Num of actions
  Action     *mActions;
  unsigned    mIndex;      // a global index of rule table.
                           // Many places use this index for arrays.
};

//////////////////////////////////////////////////////////////////////
//                       Type   Table                               //
// Tells the keyword of each supported type in language.            //
//////////////////////////////////////////////////////////////////////

struct TypeKeyword {
  const char *mText;
  TypeId      mId;
};
extern TypeKeyword TypeKeywordTable[];
extern unsigned TypeKeywordTableSize;

//////////////////////////////////////////////////////////////////////
//                     Attribute  Table                             //
// Tells the keyword of each supported attribute in language.       //
//////////////////////////////////////////////////////////////////////

struct AttrKeyword {
  const char *mText;
  AttrId      mId;
};
extern AttrKeyword AttrKeywordTable[];
extern unsigned AttrKeywordTableSize;

//////////////////////////////////////////////////////////////////////
//                    Separator Table                               //
// separator tables are merely generated from STRUCT defined in     //
// separator.spec, so they are NOT rule table.                      //
//////////////////////////////////////////////////////////////////////

struct SepTableEntry {
  const char *mText;
  SepId       mId;
};
extern SepTableEntry SepTable[];
extern unsigned SepTableSize;

//////////////////////////////////////////////////////////////////////
//                    Operator  Table                               //
// operator tables are merely generated from STRUCT defined in      //
// operator.spec, so they are NOT rule table.                       //
//////////////////////////////////////////////////////////////////////

struct OprTableEntry {
  const char *mText;
  OprId       mId;
};
extern OprTableEntry OprTable[];
extern unsigned OprTableSize;

//////////////////////////////////////////////////////////////////////
//                    Keyword   Table                               //
// keyword tables are merely generated from STRUCT defined in       //
// keyword.spec.                                                    //
//////////////////////////////////////////////////////////////////////

struct KeywordTableEntry {
  const char *mText;
};
extern KeywordTableEntry KeywordTable[];
extern unsigned KeywordTableSize;

}
#endif
