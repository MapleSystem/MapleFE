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
#include "tokenpool.h"
#include "token.h"
#include "mempool.h"

TokenPool::TokenPool() {
  mCurBlock = 0;
  mCurPos = 0;
  NewBlock();  // Need to allocate a block at beginning
}

char* TokenPool::NewBlock() {
  char *addr = mMP.AllocBlock();
  mBlocks.push_back(addr);
  return addr;
}

char* TokenPool::NewToken(unsigned tokensize) {
  char *addr = NULL;
  // If current block doesn't have enough space, just go to the next block
  if (tokensize > BLOCK_SIZE - mCurPos) {
    if (mCurBlock >= (mBlocks.size()-1)) {
      addr = NewBlock();
      mCurBlock++;
    } else {
      // In current implementation, the blocks are working as a stack, meaning
      // the top blocks are alwasy available.
      mCurBlock++;
      addr = mBlocks[mCurBlock];
    }
    // mCurPos is always set to 0.
    mCurPos = 0;
  } else {
    addr = mBlocks[mCurBlock] + mCurPos;
    mCurPos += tokensize;
  }

  // Put into mTokens;
  mTokens.push_back((Token*)addr);

  return addr;
}
  
