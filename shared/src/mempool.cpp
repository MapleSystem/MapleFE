//////////////////////////////////////////////////////////////////////////////
// This file contains the implementation of memory pool of static objects   //
// in a module.                                                             //
//                                                                          //
// Original Author: Handong Ye                                              //
// Date:   08/20/2016                                                       //
// Email:  ye.handong@huawei.com                                            //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include <cstdlib>


#include "mempool.h"
#include "massert.h"

// Release the mBlocks
MemPool::~MemPool() {
  std::vector<Block>::iterator it;
  for (it = mBlocks.begin(); it != mBlocks.end(); it++) {
    Block block = *it;
    free(block.addr);
  }
}


char* MemPool::AllocBlock() {
  char *addr = (char*)malloc(BLOCK_SIZE);
  Block block = {addr, 0};
  mBlocks.push_back(block);
  return addr;
}

// The searching for available block is going forward in the vector of 'mBlocks',
// and if one block is not big enough to hold 'size', it will be skipped and
// we search the next block.
//
// the 'firstavil' is always incremental, so that means even if a block before
// 'mFirstAvail' contains enough space for 'size', we still won't allocate
// from that block. So, Yes, we are wasting space.
//
// TODO: will come back to remove this wasting.
//
char* MemPool::Alloc(unsigned int size) {
  char *addr = NULL;

  if (size > BLOCK_SIZE) {
    MERROR ("Requsted size is bigger than block size");
  }

  if (mFirstAvail == -1) {
    addr = AllocBlock(); 
    MASSERT (addr && "StaticMemPool failed to alloc a block");
    mFirstAvail = 0;
  }

  int avail = BLOCK_SIZE - mBlocks[mFirstAvail].used;

  if (avail < size) { 
    mFirstAvail++;
    if (mFirstAvail >= mBlocks.size()) {
      addr = AllocBlock(); 
      MASSERT (addr && "StaticMemPool failed to alloc a block");
    }
  }
    
  // At this point, it's guaranteed that 'b' is able to hold 'size'
  Block *b = &mBlocks[mFirstAvail];
  addr = b->addr + b->used;
  b->used += size;
  return addr;
}
