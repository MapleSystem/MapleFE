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

// Lexing, Parsing, AST Building and IR Building all share one global
// StringPool for their symbols or necessary strings.
extern StringPool gStringPool;

#endif  // __STRINGPOOL_H__
