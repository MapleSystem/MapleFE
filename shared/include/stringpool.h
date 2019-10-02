//////////////////////////////////////////////////////////////////////////////
// This file contains the Memory Pool for String pool.                      //
//                                                                          //
// A String is stored in the pool with an ending '\0'.                      //
//////////////////////////////////////////////////////////////////////////////

#ifndef __STRINGPOOL_H__
#define __STRINGPOOL_H__

#include <map>
#include <vector>
#include <string>

//  Each time when extra memory is needed, a fixed size BLOCK will be allocated.
//  It's defined by BLOCK_SIZE. Anything above this size will not
//  be supported.
#define BLOCK_SIZE 4096

class StringMap;

struct SPBlock {
  char *Addr;        // starting address
  unsigned int Used; // bytes used
};

class StringPool {
private:
  StringMap            *mMap;
  std::vector<SPBlock>  mBlocks;
  int                   mFirstAvail; // -1 means no available.

public:
  char* AllocBlock();
  char* Alloc(const size_t);
  char* Alloc(const std::string&);
  char* Alloc(const char*);

public:
  StringPool();
  ~StringPool();

  // If not found, add into StringPool
  char* FindString(const std::string&);
  char* FindString(const char*);
  char* FindString(const char*, size_t);
};


#endif  // __STRINGPOOL_H__
