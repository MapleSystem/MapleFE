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
  char *vAddr = NULL;
  if (tokensize > BLOCK_SIZE - (tokensize * mCurPos)) {
    // Go to the next block
    if (mCurBlock >= mBlocks.size()) {
      vAddr = NewBlock();
    } else {
      mCurBlock ++;
      vAddr = mBlocks[mCurBlock];
    }
    mCurPos = 0;
  } else {
    mCurPos++;
    vAddr = mBlocks[mCurBlock] + mCurPos * tokensize;
  }

  // Put into mTokens;
  mTokens.push_back((Token*)vAddr);

  return vAddr;
}
  
