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

#include "mempool.h"
#include "massert.h"

// We define a ContainerMemPool, which is slightly different than the other MemPool.
// There are two major differences.
// (1) Size of each allocation will be the same.
// (2) Support indexing.

class ContainerMemPool : public MemPool {
public:
  unsigned mElemSize;
public:
  char* AddrOfIndex(unsigned index);
  void  SetElemSize(unsigned i) {mElemSize = i;}
  char* AllocElem() {return Alloc(mElemSize);}
};

// SmallVector is widely used in the tree nodes to save children nodes, and
// they should call Release() explicitly, if the destructor of SmallVector
// won't be called.

// NOTE: When we locate an element in the memory pool, we don't check if it's
//       out of boundary. It's the user of SmallVector to ensure element index
//       is valid.

template <class T> class SmallVector {
private:
  ContainerMemPool mMemPool;
  unsigned         mNum;     // element number

public:
  SmallVector() {
    mNum = 0;
    SetBlockSize(128);
    mMemPool.SetElemSize(sizeof(T));
  }
  ~SmallVector() {Release();}

  void SetBlockSize(unsigned i) {mMemPool.SetBlockSize(i);}
  void Release() {mMemPool.Release();}

  void PushBack(T t) {
    char *addr = mMemPool.AllocElem();
    *(T*)addr = t;
    mNum++;
  }

  void PopBack(T);

  unsigned GetNum() {return mNum;}

  T Back();

  T ValueAtIndex(unsigned i) {
    char *addr = mMemPool.AddrOfIndex(i);
    return *(T*)addr;
  }

  T* RefAtIndex(unsigned i) {
    char *addr = mMemPool.AddrOfIndex(i);
    return (T*)addr;
  }
};

#endif
