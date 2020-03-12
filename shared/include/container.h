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

///////////////////////////////////////////////////////////////////////////////
// This file contains the popular container for most data structures.
//
// [NOTE] For containers, there is one basic rule that is THE CONTAINTER ITSELF
//        SHOULD BE SELF CONTAINED. In another word, all the memory should be
//        contained in the container itself. Once the destructor or Release()
//        is called, all memory should be freed.
//
// The user of containers need explictly call Release() to free
// the memory, if he won't trigger the destructor of the containers. This
// is actually common case, like tree nodes which is maintained by a memory
// pool and won't call destructor, so any memory allocated at runtime won't
// be released.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __CONTAINER_H__
#define __CONTAINER_H__

#include "massert.h"

// We define a ContainerMemPool, which is slightly different than the other MemPool.
// There are three major differences.
// (1) Size of each allocation will be the same.
// (2) Support indexing.
// (3) No STL involved, meaning self contained.
struct ContainerBlock {
  char           *mAddr;
  unsigned int    mUsed;    // bytes used
  ContainerBlock *mNext;
};

// SmallVector is widely used in the tree nodes to save children nodes, and
// they should call Release() explicitly, if the destructor of SmallVector
// won't be called.

template <class T> class SmallVector {
private:
  // To make sure SmallVector is self contained, we manage the memory allocated
  // by a linked list.
  ContainerBlock *mBlocks;

  // the current block which is available to use.
  ContainerBlock *mCurrBlock;

  // size of a block. Most of data is small, like a pointer. Usually 128 is good
  // enough for block size.
  unsigned mBlockSize;

private:
  ContainerBlock* AllocBlock();
  char* Alloc(unsigned);

public:
  SmallVector() {mBlockSize = 128; mBlocks = NULL; mCurrBlock = NULL;}
  ~SmallVector(){Release();}

  void SetBlockSize(unsigned i) {mBlockSize = i;}

  void Release();

  void PushBack(T);
  void PopBack(T);
  T    AtIndex(unsigned);
};

#endif
