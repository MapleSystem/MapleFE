#include "ast_mempool.h"
#include "ast.h"

TreePool::TreePool() {
  mCurBlock = 0;
  mCurPos = 0;
  NewBlock();  // Need to allocate a block at beginning
}

char* TreePool::NewBlock() {
  char *addr = mMP.AllocBlock();
  mBlocks.push_back(addr);
  return addr;
}

char* TreePool::NewTreeNode(unsigned tokensize) {
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

  // Put into mTreeNodes;
  mTreeNodes.push_back((TreeNode*)addr);

  return addr;
}
  
