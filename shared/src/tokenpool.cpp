#include "tokenpool.h"
#include "token.h"
#include "mempool.h"

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
  return vAddr;
}
  
