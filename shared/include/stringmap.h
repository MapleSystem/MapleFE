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

namespace maplefe {

class StringPool;

// The StringMapEntry-s in the same bucket form a linked list
class StringMapEntry {
public:
  char           *Addr;   // Addr in the string pool
  unsigned        StrIdx; // String index
  StringMapEntry *Next;
public:
  StringMapEntry() { Addr = NULL; StrIdx = 0; Next = NULL; }
  StringMapEntry(char *A, unsigned idx) { Addr = A; StrIdx = idx; Next = NULL; }
  StringMapEntry(char *A, unsigned idx, StringMapEntry *E) { Addr = A; StrIdx = idx; Next = E; }

  ~StringMapEntry() {}

  char *GetAddr() { return Addr; }
  unsigned GetStrIdx() { return StrIdx; }
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
  StringMapEntry *LookupEntryFor(const std::string &s);
  StringMapEntry *InsertEntry(char *, unsigned, unsigned);
};

}
#endif
