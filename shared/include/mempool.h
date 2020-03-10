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
// This file contains the Memory Pool for many place.
//
// [NOTE]
//
// There is one assumption when using this memory pool. The object in the pool
// doesn't rely on destructor to free any additional memory. All the memory
// usage involved in the object is allocated by the memory pool at the first
// place. So the destructor of the memory pool cleans everything related to
// the object.
//
// Here are example of good candidates, Tokens, string.
//////////////////////////////////////////////////////////////////////////////

// So far it only request new Block and keep using it. It won't release
// any memory right now, or it even doesn't let MemPool know a Block is free.
// TODO: We will come back to this.

#ifndef __MEMPOOL_H__
#define __MEMPOOL_H__

#include <map>
#include <vector>

//  Each time when extra memory is needed, a fixed size BLOCK will be allocated.
//  It's defined by mBlockSize. Anything above this size will not
//  be supported.
#define DEFAULT_BLOCK_SIZE 4096

struct Block {
  char *addr;        // starting address
  unsigned int used; // bytes used
};

// So far there is nothing like free list. Everything will be released when
// StaticMemPool is destructed.
//
class MemPool{
private:
  std::vector<Block> mBlocks;
  int                mFirstAvail;// first block available; -1 means no available
  unsigned           mBlockSize;

public:
  MemPool() {mFirstAvail = -1; mBlockSize = DEFAULT_BLOCK_SIZE;}
  ~MemPool();

  void  SetBlockSize(unsigned i) {mBlockSize = i;}
  char* AllocBlock();
  char* Alloc(unsigned int);
};

#endif  // __MEMPOOL_H__
