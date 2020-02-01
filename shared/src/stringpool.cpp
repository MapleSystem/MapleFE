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
//                                                                          //
// This file contains the implementation of string pool.                    //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include <cstdlib>
#include <cstring>

#include "stringpool.h"
#include "stringmap.h"
#include "massert.h"

StringPool::StringPool() {
  mMap = new StringMap();
  mMap->SetPool(this);
  mFirstAvail = -1;
}

StringPool::~StringPool() {
  // Release the mBlocks
  std::vector<SPBlock>::iterator it;
  for (it = mBlocks.begin(); it != mBlocks.end(); it++) {
    SPBlock block = *it;
    char *addr = block.Addr;
    free(addr);
  }
  
  // Release the StringMap
  delete mMap;
}

char* StringPool::Alloc(const std::string &s) {
  return Alloc(s.c_str());
}

// 's' must guarantee to end with NULL
char* StringPool::Alloc(const char *s) {
  size_t size = strlen(s) + 1;
  if (size > BLOCK_SIZE) {
    MERROR ("Requsted size is bigger than block size");
  }

  char *addr = Alloc(size);
  MASSERT (addr && "StringPool failed to alloc for string");

  strncpy(addr, s, size - 1);
  *(addr + size -1) = '\0';
  return addr;
}

// The searching for available block is going forward in the vector of 'mBlocks',
// and if one block is not big enough to hold 'size', it will be skipped and
// we search the next block.
//
// the 'firstavil' is always incremental, so it means even if a block before
// 'mFirstAvail' contains enough space for 'size', we still won't allocate
// from that block. So, Yes, we are wasting space.
//
// TODO: will come back to remove this wasting.
//
char* StringPool::Alloc(const size_t size) {
  char *addr = NULL;

  if (mFirstAvail == -1) {
    addr = AllocBlock();
    MASSERT (addr && "StringPool failed to alloc a block");
    mFirstAvail = 0;
  }

  int avail = BLOCK_SIZE - mBlocks[mFirstAvail].Used;

  if (avail < size) {
    mFirstAvail++;
    if (mFirstAvail >= mBlocks.size()) {
      addr = AllocBlock();
      MASSERT (addr && "StringPool failed to alloc a block");
    }
  }

  // At this point, it's guaranteed that 'b' is able to hold 'size'
  SPBlock *b = &mBlocks[mFirstAvail];
  addr = b->Addr + b->Used;
  b->Used += size;

  return addr;
}

char* StringPool::AllocBlock() {
  char *addr = (char*)malloc(BLOCK_SIZE);
  SPBlock block = {addr, 0};
  mBlocks.push_back(block);
  return addr;
}

// This is the public interface to find a string in the pool.
// If not found, add it.
char* StringPool::FindString(const std::string &s) {
  return mMap->LookupAddrFor(s);
}

// This is the public interface to find a string in the pool.
// If not found, add it.
char* StringPool::FindString(const char *str) {
  std::string s(str);
  return mMap->LookupAddrFor(s);
}

// This is the public interface to find a string in the pool.
// If not found, add it.
char* StringPool::FindString(const char *str, size_t len) {
  std::string s;
  s.assign(str, len);
  return mMap->LookupAddrFor(s);
}

