#include "stringmap.h"
#include "stringutil.h"
#include "stringpool.h"

#include "massert.h"

#define DEFAULT_BUCKETS_NUM 256

StringMap::StringMap(unsigned BucketNum) {
  Init(BucketNum);
}

StringMap::StringMap() {
  Init(DEFAULT_BUCKETS_NUM);
}

StringMap::~StringMap() {
  // iterate to delete all entries.
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
char* StringMap::LookupAddrFor(const std::string &S) { 
  unsigned BucketNo = BucketNoFor(S);
  StringMapEntry *E = &mBuckets[BucketNo];
  while (E && E->Addr) {
    if (S.compare(E->Addr) == 0)
      return E->Addr;
    E = E->Next;
  }
   
  // We cannot find an existing string for 'S'. Need to allocate
  char *Addr = mPool->Alloc(S);
  InsertEntry(Addr, BucketNo);

  return Addr;
} 

// Add a new entry in 'bucket'.
// 'addr' is the address in the string pool
void StringMap::InsertEntry(char *addr, unsigned bucket) {
  StringMapEntry *E = &mBuckets[bucket];
  while (E->Next) {
    E = E->Next;
  }

  StringMapEntry *NewEnt = new StringMapEntry(addr);
  E->Next = NewEnt;
}

