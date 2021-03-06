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
}
