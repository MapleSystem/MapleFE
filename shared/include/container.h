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
//===----------------------------------------------------------------------===//

#ifndef __CONTAINER_H__
#define __CONTAINER_H__

#include "massert.h"
#include "mempool.h"

// The user of containers need explictly call Release() to free
// the memory, if user won't trigger the destructor of the containers. This
// is actually common case, like tree nodes which is maintained by a memory
// pool and not calling destructor, so any memory allocated at runtime won't
// be released. SmallVector is widely used in the tree nodes to save children
// nodes, and they should call Release() explicitly.

class SmallVector {
private:
  MemPool mMemPool;
public:
  SmallVector(MemPool *mp) : mMemPool(mp) {mMemPool.SetBlockSize(256);}
  ~SmallVector(){}

  void Release();
};

#endif
