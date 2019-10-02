////////////////////////////////////////////////////////////////////////
// The lexical translation of the characters creates a sequence of input
// elements -----> white-space
//            |--> comments
//            |--> tokens --->identifiers
//                        |-->keywords
//                        |-->literals
//                        |-->separators
//                        |-->operators
//
// This categorization is shared among all languages. [NOTE] If anything
// in a new language is exceptional, please add to this.
// This file defines Elements
//
////////////////////////////////////////////////////////////////////////

#ifndef __Element_H__
#define __Element_H__

#include "stringmap.h"

typedef enum {
  ET_WS,    // White Space
  ET_CM,    // Comment
  ET_TK,    // Token
  ET_NA     // Null
}ELMT_Type;

class Element {
public:
  ELMT_Type       EType;
  StringMapEntry *Text;  // It saves the 'string' data

  bool IsToken()   {return EType == ET_TK;}
  bool IsComment() {return EType == ET_CM;}

  Element(ELMT_Type t, const std::string &s);
  Element() {EType = ET_NA; Text = NULL;}
};

#endif
