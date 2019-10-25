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
    if (mCurBlock >= mBlocks.size()) {
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
  
