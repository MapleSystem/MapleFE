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

namespace maplefe {

typedef enum COMM_Type {
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

}
#endif
