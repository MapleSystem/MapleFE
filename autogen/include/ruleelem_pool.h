//////////////////////////////////////////////////////////////////////////////
// This file contains the Memory Pool for RuleElem which are dynamically    //
// allocated. The management of Token memory is as below:                   //
//  1) RuleElem are used during parsing. Once a .spec file is done,         //
//     all the tokens created can be released. So is the memory             //
//  2) A start tag is inserted at the beginning of a .spec parsing          //
//  4) Once it's done parsing, the RuleElems will be popped out until the   //
//     one before the tag.                                                  //
//  4) RESERVED rule elements are those shared across multiple .spec files, //
//     and they are saved at the beginning of the pool, they will be freed  //
//     when the program is over.                                            //
//  NOTE: Althoug it's like a stack, but the memory is actually a continuous//
//           block.                                                         //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#ifndef __RULEELEM_POOL_H__
#define __RULEELEM_POOL_H__

#include <map>
#include <vector>

#include "rule.h"

class MemPool;

// RuleElemPool will request/release memory on the Block level.
// So far it only request new Block and keep (re)using it. It won't release
// any memory right now. It even doesn't let MemPool know a Block is free.
//
// The fullly release of memory is done by MemPool when AutoGen is done.

// [NOTE]: The size of different types of RuleElem should be the same,
//         so that the memory is organized as a vector and elements are
//         identified using index.

struct MemPoolTag {
  unsigned mBlock;   //
  unsigned mIndex;   // the index of starting of a new Phase
};

class RuleElemPool {
private:
  MemPool                *mMP;
  std::vector<char *>     mBlocks;
  std::vector<MemPoolTag> mTags;
  unsigned                mCurBlock;
  unsigned                mCurIndex; // current available position in mCurBlock.
                                     // Use index since Tokens have same size

private:
  char* NewBlock();

public:
  RuleElemPool(MemPool *mp);
  ~RuleElemPool(){}

  RuleElem* NewRuleElem();

  void AddTag(unsigned block, unsigned index);
  void ReleaseTopPhase();
};

#endif
