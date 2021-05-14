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
// This file contains the Memory Pool for many place.
//
// [NOTE]
//
// There is one assumption when using this memory pool. The object in the pool
// doesn't rely on destructor to free memory. All the memory
// usage involved in the object is allocated by the memory pool at the first
// place. So the destructor of the memory pool cleans everything related to
// the object.
//
// Here are example of good candidates, Tokens, string.
//////////////////////////////////////////////////////////////////////////////

// So far it supports:
//    1. Request new memory allocation.
//    2. Release from the end of a certain bytes of space. So it supports
//       a very simple reuse of memory space.

#ifndef __MEMPOOL_H__
#define __MEMPOOL_H__

#include <cstdlib>

namespace maplefe {

//  Each time when extra memory is needed, a fixed size BLOCK will be allocated.
//  It's defined by mBlockSize. Anything above this size will not
//  be supported.
#define DEFAULT_BLOCK_SIZE 4096

struct Block {
  char *addr;        // starting address
  unsigned int used; // bytes used
  Block *next;       // next block
  Block *prev;       // prev block
};

class MemPool{
protected:
  Block    *mCurrBlock; // Currently available block
  Block    *mBlocks;    // all blocks, with the ending blocks could be free.
  unsigned  mBlockSize;
public:
  MemPool() : mCurrBlock(NULL), mBlocks(NULL), mBlockSize(DEFAULT_BLOCK_SIZE) {}
  ~MemPool();

  void  SetBlockSize(unsigned i) {mBlockSize = i;}
  char* AllocBlock();
  char* Alloc(unsigned);
  void  Release(unsigned i);  // release the last occupied i bytes.

  void  WipeOff(int c = 0);   // wipe off all data with c
  void  Clear();   // free up all block, dont wipe data, Keep memory.
  void  Release(); // Allow users to free memory explicitly.
};

}
#endif  // __MEMPOOL_H__
