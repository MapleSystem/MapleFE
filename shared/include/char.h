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

#ifndef __CHAR_H__
#define __CHAR_H__

namespace maplefe {

// This file defines the Char we use in the parser and AST.
// We separate unicode character literal from normal Raw Input character.

struct Char {
  bool mIsUnicode;
  union {
    int  mUniValue; // We simply save integer value of Hex digits, such as D800,
                    // instead of 'code point' which needs special conversion
                    // under unicode specification.
    char mChar;
  }mData;
};

}
#endif
