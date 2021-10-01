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
//                                                                          //
// This file contains the implementation of string pool.                    //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include <cstdlib>
#include <cstring>

#include "stringpool.h"
#include "stringmap.h"

namespace maplefe {

// The global string pool for lexing, parsing, ast building and ir building
// for the symbols, etc.
StringPool gStringPool;

StringPool::StringPool() {
  mMap = new StringMap();
  mMap->SetPool(this);
  mFirstAvail = -1;
  // make string idx starting from 1
  mStringTable.push_back("");
  // empty string idx is 1
  mStringTable.push_back("");
}

StringPool::~StringPool() {
  // Release the mBlocks
  std::vector<SPBlock>::iterator it;
  for (it = mBlocks.begin(); it != mBlocks.end(); it++) {
    SPBlock block = *it;
    char *addr = block.Addr;
    free(addr);
  }
  mStringTable.clear();

  // Release the long strings
  std::vector<char*>::iterator long_it;
  for (long_it = mLongStrings.begin(); long_it != mLongStrings.end(); long_it++) {
    char *addr = *long_it;
    free(addr);
  }
  mLongStrings.clear();

  // Release the StringMap
  delete mMap;
}

char* StringPool::Alloc(const std::string &s) {
  return Alloc(s.c_str());
}

// 's' must guarantee to end with NULL
char* StringPool::Alloc(const char *s) {
  size_t size = strlen(s) + 1;
  char *addr = NULL;
  if (size > BLOCK_SIZE) {
    addr = (char*)malloc(size);
    mLongStrings.push_back(addr);
  } else {
    addr = Alloc(size);
  }

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
// If not found, allocate in the pool and save in the map.
const char* StringPool::FindString(const std::string &s) {
  return mMap->LookupEntryFor(s)->GetAddr();
}

// This is the public interface to find a string in the pool.
// If not found, allocate in the pool and save in the map.
const char* StringPool::FindString(const char *str) {
  std::string s(str);
  return mMap->LookupEntryFor(s)->GetAddr();
}

// This is the public interface to find a string in the pool.
// If not found, allocate in the pool and save in the map.
const char* StringPool::FindString(const char *str, size_t len) {
  std::string s;
  s.assign(str, len);
  return mMap->LookupEntryFor(s)->GetAddr();
}

// This is the public interface to find a string in the pool.
// If not found, allocate in the pool and save in the map.
unsigned StringPool::GetStrIdx(const std::string &s) {
  if (s.empty()) return 1;
  return mMap->LookupEntryFor(s)->GetStrIdx();
}

// This is the public interface to find a string in the pool.
// If not found, allocate in the pool and save in the map.
unsigned StringPool::GetStrIdx(const char *str) {
  if (strlen(str) == 0) return 1;
  std::string s(str);
  return mMap->LookupEntryFor(s)->GetStrIdx();
}

// This is the public interface to find a string in the pool.
// If not found, allocate in the pool and save in the map.
unsigned StringPool::GetStrIdx(const char *str, size_t len) {
  if (len == 0) return 1;
  std::string s;
  s.assign(str, len);
  return mMap->LookupEntryFor(s)->GetStrIdx();
}
}

