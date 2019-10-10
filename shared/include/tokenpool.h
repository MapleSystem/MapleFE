//////////////////////////////////////////////////////////////////////////////
// This file contains the Memory Pool for Tokens which are dynamically      //
// allocated. The management of Token memory is as below:                   //
//  1) Tokens are used during parsing. Once a complete language structure   //
//     is done, all the tokens created can be released. So is the memory    //
//  2) All the Tokens are stored as in a stack                              //
//  3) A start tag is inserted at the beginning of a language structure     //
//  4) Once it's done parsing, the Tokens will be popped out until the one  //
//     before the tag.
//  5) NOTE: Althoug it's like a stack, but the memory is actually a continuous
//           block.
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#ifndef __TOKEN_POOL_H__
#define __TOKEN_POOL_H__

#include <map>
#include <vector>

#include "mempool.h"

// TokenPool will request/release memory on the Block level.
// So far it only request new Block and keep (re)using it. It won't release
// any memory right now, or it even doesn't let MemPool know a Block is free.
// TODO: We will come back to this.

class TokenPool {
private:
  MemPool               mMP;       //
  std::vector<char *>   mBlocks;
  std::vector<unsigned> mTags;
  unsigned              mCurBlock;
  unsigned              mCurPos;   // current available position in mCurBlock.
                                   // Use index since Tokens have same size

private:
  char* NewBlock();

public:
  TokenPool();
  ~TokenPool(){};   // memory is freed in destructor of mMP.

  char* NewToken(unsigned);
};

#endif
