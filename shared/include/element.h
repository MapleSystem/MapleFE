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
// This file defines Elements
//
////////////////////////////////////////////////////////////////////////

#ifndef __Element_H__
#define __Element_H__

namespace maplefe {

typedef enum ELMT_Type {
  ET_WS,    // White Space
  ET_CM,    // Comment
  ET_TK,    // Token
  ET_NA     // Null
}ELMT_Type;

class Element {
public:
  ELMT_Type       EType;

  bool IsToken()   {return EType == ET_TK;}
  bool IsComment() {return EType == ET_CM;}

  Element(ELMT_Type t) {EType = t;}
  Element() {EType = ET_NA;}
};

}
#endif
