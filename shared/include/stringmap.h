///////////////////////////////////////////////////////////////////////////////
//
// StringMap is heavily used in the StringPool. It's used to locate the address
// of string in the StringPool. Here is how it works:
//   (1) std::string ---> Hash value, --> Locate the Bucket in StringMap 
//       --> get the addr from StringMapEntry
//   (2) If conflicted in the Bucket, iterate to find the right StringMapEntry
//   (3) If not found, a) Add std::string to the StringPool
//                     b) Insert the corresponding StringMapEntry
//
// StringMap is (de)allocated through Mempool. String's are in StringPool.
//===----------------------------------------------------------------------===//

#ifndef __STRINGMAP_H__
#define __STRINGMAP_H__

#include <string>

class StringPool;

// The StringMapEntry-s in the same bucket form a linked list
class StringMapEntry {
public:
  char           *Addr;   // Addr in the string pool
  StringMapEntry *Next; 
public:
  StringMapEntry() { Addr = NULL; Next = NULL; }
  StringMapEntry(char *A) { Addr = A; Next = NULL; }
  StringMapEntry(char *A, StringMapEntry *E) { Addr = A; Next = E; }

  ~StringMapEntry() {}
};

class StringMap {
protected:
  StringPool     *mPool;
  StringMapEntry *mBuckets = nullptr;
  unsigned        mNumBuckets = 0;

public:
  explicit StringMap(unsigned numbuckets);
  StringMap();
  ~StringMap();

  void     Init(unsigned numbuckets);
  void     SetPool(StringPool *p) {mPool = p;}

  unsigned BucketNoFor(const std::string &s);
  char*    LookupAddrFor(const std::string &s);
  void     InsertEntry(char *, unsigned);
};

#endif
