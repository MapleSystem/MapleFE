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
#include "stringmap.h"
#include "stringutil.h"
#include "stringpool.h"

#include "massert.h"

namespace maplefe {

#define DEFAULT_BUCKETS_NUM 256

StringMap::StringMap(unsigned BucketNum) {
  Init(BucketNum);
}

StringMap::StringMap() {
  Init(DEFAULT_BUCKETS_NUM);
}

StringMap::~StringMap() {
  StringMapEntry *E = mBuckets;
  for (unsigned i = 0; i < mNumBuckets; i++, E++) {
    // iterate to delete all entries.
    StringMapEntry *entry = E->Next;
    StringMapEntry *temp = NULL;
    while(entry) {
      temp = entry->Next;
      delete entry;
      entry = temp;
    }
  }

  delete [] mBuckets;
}

void StringMap::Init(unsigned Num) {
  MASSERT((Num & (Num-1)) == 0 &&
        "Init Size must be a power of 2 or zero!");
  mNumBuckets = Num ? Num : DEFAULT_BUCKETS_NUM;

  mBuckets = new StringMapEntry[mNumBuckets];
  StringMapEntry *E = mBuckets;
  for (unsigned i = 0; i < mNumBuckets; i++, E++) {
    E->Addr = NULL;
    E->Next = NULL;
  }
}

// Get the bucket no for 'S'.
unsigned StringMap::BucketNoFor(const std::string &S) {
  unsigned HTSize = mNumBuckets;
  if (HTSize == 0) {  // Hash table unallocated so far?
    Init(DEFAULT_BUCKETS_NUM);
    HTSize = mNumBuckets;
  }
  unsigned FullHashValue = HashString(S);
  unsigned BucketNo = FullHashValue & (HTSize-1);
  return BucketNo;
}

// Look up to find the address in the string pool of 'S'.
// If 'S' is not in the string pool, insert it.
StringMapEntry *StringMap::LookupEntryFor(const std::string &S) {
  unsigned BucketNo = BucketNoFor(S);
  StringMapEntry *E = &mBuckets[BucketNo];

  // Since we add an empty StringMapEntry as the first entry of this
  // bucket, we can use it at the first time.
  if (E && !E->Addr && !E->Next) {
    char *addr = mPool->Alloc(S);
    E->Addr = addr;
    E->StrIdx = mPool->mStringTable.size();
    mPool->mStringTable.push_back(addr);
    return E;
  }

  while (E && E->Addr) {
    if (S.compare(E->Addr) == 0) {
      return E;
    }
    E = E->Next;
  }

  // We cannot find an existing string for 'S'. Need to allocate
  char *addr = mPool->Alloc(S);
  unsigned idx = mPool->mStringTable.size();
  mPool->mStringTable.push_back(addr);
  E = InsertEntry(addr, idx, BucketNo);
  return E;
}

// Add a new entry in 'bucket'.
// 'addr' is the address in the string pool
StringMapEntry *StringMap::InsertEntry(char *addr, unsigned idx, unsigned bucket) {
  StringMapEntry *E = &mBuckets[bucket];
  while (E->Next) {
    E = E->Next;
  }

  StringMapEntry *NewEnt = new StringMapEntry(addr, idx);
  E->Next = NewEnt;
  return NewEnt;
}
}

