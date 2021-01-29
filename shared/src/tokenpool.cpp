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
#include "tokenpool.h"
#include "token.h"
#include "mempool.h"
#include "massert.h"

namespace maplefe {

char* TokenPool::NewToken(unsigned size) {
  char *addr = mMemPool.Alloc(size);
  MASSERT(addr && "MemPool failed to alloc a token.");
  Token *token = (Token*)addr;
  token->mAltTokens = NULL;
  mTokens.PushBack((Token*)addr);
  return addr;
}
}
