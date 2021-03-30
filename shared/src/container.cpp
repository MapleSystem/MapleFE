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

//////////////////////////////////////////////////////////////////////////////
// This file contains the implementation of containters
//////////////////////////////////////////////////////////////////////////////

#include <cstdlib>
#include <cstring>

#include "container.h"
#include "massert.h"

namespace maplefe {

char* ContainerMemPool::AddrOfIndex(unsigned index) {
  unsigned num_in_blk = mBlockSize / mElemSize;
  unsigned blk = index / num_in_blk;
  unsigned index_in_blk = index % num_in_blk;

  Block *block = mBlocks;
  for (unsigned i = 0; i < blk; i++) {
    block = block->next;
  }

  char *addr = block->addr + index_in_blk * mElemSize;
  return addr;
}

////////////////////////////////////////////////////////////////////////////////
//                               Bit Vector
////////////////////////////////////////////////////////////////////////////////

BitVector::BitVector() {
  SetBlockSize(1024);
}

void BitVector::ClearBit(unsigned idx) {
  unsigned byte_idx = idx / 8;
  unsigned blk_idx = byte_idx / mBlockSize;
  Block *block = mBlocks;
  for (unsigned i = 0; i < blk_idx; i++) {
    block = block->next;
    if (!block)
      MERROR("ClearBit at unknown location.");
  }

  unsigned bit_idx = idx % 8;
  char mask = ~(1 << bit_idx);

  char *addr = block->addr + byte_idx % mBlockSize;
  *addr = (*addr) & mask;
}

void BitVector::SetBit(unsigned idx) {
  unsigned byte_idx = idx / 8;
  unsigned blk_idx = byte_idx / mBlockSize;
  Block *block = mBlocks;
  unsigned block_num = 0;
  for (; block && (block_num < blk_idx); block_num++) {
    block = block->next;
  }

  // Out of memory. Need to allocate.
  // For each block allocated, the random data need be wiped off.
  if (!block) {
    unsigned blocks_to_alloc = blk_idx + 1 - block_num;
    for (unsigned i = 0; i < blocks_to_alloc; i++) {
      char *addr = AllocBlock();
      memset((void*)addr, 0, mBlockSize);
    }

    // get the block again
    block = mBlocks;
    for (unsigned i = 0; i < blk_idx; i++)
      block = block->next;
  }

  unsigned bit_idx = idx % 8;
  char *addr = block->addr + byte_idx % mBlockSize;
  *addr = (*addr) | (1 << bit_idx);
}

}
