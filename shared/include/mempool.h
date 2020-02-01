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
// This file contains the Memory Pool for String pool.                      //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#ifndef __MEMPOOL_H__
#define __MEMPOOL_H__

#include <map>
#include <vector>

//  Each time when extra memory is needed, a fixed size BLOCK will be allocated.
//  It's defined by BLOCK_SIZE. Anything above this size will not
//  be supported.
#define BLOCK_SIZE 4096

struct Block {
  char *addr;        // starting address
  unsigned int used; // bytes used
};

// So far there is nothing like free list. Everything will be released when
// StaticMemPool is destructed.
//
class MemPool {
private:
  std::vector<Block> mBlocks;
  int                 mFirstAvail;// first block available; -1 means no available
public:
  MemPool() {mFirstAvail = -1;}
  ~MemPool();

  char* AllocBlock();
  char* Alloc(unsigned int);
};

#endif  // __MEMPOOL_H__
