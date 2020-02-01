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
#include "mempool.h"
#include "rule.h"
#include "ruleelem_pool.h"

RuleElemPool::RuleElemPool(MemPool *mp) {
  mMP = mp;
  mCurBlock = 0;
  mCurIndex = 0;
  NewBlock();
}

char* RuleElemPool::NewBlock() {
  char *addr = mMP->AllocBlock();
  mBlocks.push_back(addr);
  return addr;
}

// [NOTE]: size of all kinds of rule elements must be the same
RuleElem* RuleElemPool::NewRuleElem() {
  unsigned elemSize = sizeof(RuleElem);
  char *addr = NULL;

  // as mCurIndex starting from 0, need to increment it first
  // to calculate address offset using "elemSize * mCurIndex"
  mCurIndex++;
  if (elemSize > BLOCK_SIZE - (elemSize * mCurIndex)) {
    // Go to the next block
    mCurBlock++;
    if (mCurBlock >= mBlocks.size()) {
      addr = NewBlock();
    } else {
      addr = mBlocks[mCurBlock];
    }
    mCurIndex = 0;
  } else {
    addr = mBlocks[mCurBlock] + mCurIndex * elemSize;
  }

  RuleElem *elem = new (addr) RuleElem();
  return elem;
}

// push the tag
void RuleElemPool::AddTag(unsigned block, unsigned index) {
  MemPoolTag tag = {block, index};
  mTags.push_back(tag);
}

// Releasing means moving the current block/current pos back until the Tag.
// There is no real free of memory.
void RuleElemPool::ReleaseTopPhase() {
  MemPoolTag topTag = mTags.back();
  mCurBlock = topTag.mBlock;
  mCurIndex = topTag.mIndex;

 // TODO: call the destructors. Right now it's ok to skip since destructors
 // do nothing :-)
}
