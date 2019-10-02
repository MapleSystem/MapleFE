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
//
// This file defines Tokens and all the sub categories
//
////////////////////////////////////////////////////////////////////////

#ifndef __COMMENT_H__
#define __COMMENT_H__

#include "element.h"

typedef enum {
  COMM_EOL,   //End of Line, //
  COMM_TRA    //Traditional, /* ... */
}COMM_Type;

class Comment : public Element {
private:
  COMM_Type CommType;
public:
  Comment(COMM_Type ct) : CommType(ct) {EType = ET_CM;}
  
  bool IsEndOfLine()   {return CommType == COMM_EOL;}
  bool IsTraditional() {return CommType == COMM_TRA;}
};

#endif
