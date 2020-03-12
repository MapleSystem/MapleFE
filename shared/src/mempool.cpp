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
  Release();
}

void MemPool::Release() {
  Block *block = mBlocks;
  while (block) {
    Block *next_block = block->next;
    free(block->addr);
    delete block;
    block = next_block;
  }
  mBlocks = NULL;
  mCurrBlock = NULL;
}

char* MemPool::AllocBlock() {
  Block *block = new Block;
  block->addr = (char*)malloc(mBlockSize);
  block->used = 0;
  block->next = NULL;

  if (!mBlocks) {
    mBlocks = block;
    mCurrBlock = block;
  } else {
    MASSERT(mCurrBlock && "No mCurrBlock while memory already allocated.");
    mCurrBlock->next = block;
    mCurrBlock = block;
  }

  return block->addr;
}

// The searching for available block is going forward in the list of 'mBlocks',
// and if one block is not big enough to hold 'size', it will be skipped and
// we search the next block.
//
// The mCurrBlock is always moving forward, so that means even if a block before
// mCurrBlock contains enough space for 'size', we still won't allocate
// from that block. So, Yes, we are wasting space.
//
// TODO: will come back to remove this wasting.
//
char* MemPool::Alloc(unsigned size) {
  char *addr = NULL;

  if (size > mBlockSize)
    MERROR ("Requsted size is bigger than block size");

  if (!mCurrBlock) {
    addr = AllocBlock(); 
    MASSERT (addr && "MemPool failed to alloc a block");
  } else {
    int avail = mBlockSize - mCurrBlock->used;
    if (avail < size) {
      addr = AllocBlock(); 
      MASSERT (addr && "MemPool failed to alloc a block");
    }
  }
    
  // At this point, it's guaranteed that 'b' is able to hold 'size'
  addr = mCurrBlock->addr + mCurrBlock->used;
  mCurrBlock->used += size;
  return addr;
}
