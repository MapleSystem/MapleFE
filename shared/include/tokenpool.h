/*
* Copyright (C) [2020] Futurewei Technologies, Inc. All rights reverved.
*
* OpenArkFE is licensed under the Mulan PSL v1.
* You can use this software according to the terms and conditions of the Mulan PSL v1.
* You may obtain a copy of Mulan PSL v1 at:
*
*  http://license.coscl.org.cn/MulanPSL
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR
* FIT FOR A PARTICULAR PURPOSE.
* See the Mulan PSL v1 for more details.
*/
//////////////////////////////////////////////////////////////////////////////
// This file contains the Memory Pool for Tokens which are dynamically      //
// allocated. The management of Token memory is as below:                   //
//  1) Tokens are used during parsing. Once a complete language structure   //
//     is done, all the tokens created can be released. So is the memory    //
//  2) All the Tokens are stored as in a stack                              //
//  3) A start tag is inserted at the beginning of a language structure     //
//  4) Once it's done parsing, the Tokens will be popped out until the one  //
//     before the tag.
//  5) NOTE: Althoug it's like a stack, but the memory is actually a continuous
//           block.
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#ifndef __TOKEN_POOL_H__
#define __TOKEN_POOL_H__

#include <map>
#include <vector>

#include "mempool.h"

class Token;

class TokenPool {
private:
  MemPool mMemPool;
public:
  std::vector<Token*>   mTokens;

public:
  TokenPool() {}
  ~TokenPool(){}   // memory is freed in destructor of mMP.

  char* NewToken(unsigned);
};

#endif
