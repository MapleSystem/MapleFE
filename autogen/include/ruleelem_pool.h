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
// This file contains the Memory Pool for RuleElem which are dynamically
// allocated.
//////////////////////////////////////////////////////////////////////////////

#ifndef __RULEELEM_POOL_H__
#define __RULEELEM_POOL_H__

#include <map>
#include <vector>
#include "rule.h"

namespace maplefe {

// RuleElemPool will request/release memory on the Block level.
// So far it only request new Block and keep (re)using it. It won't release
// any memory right now. It even doesn't let MemPool know a Block is free.
//
// The fullly release of memory is done by MemPool when AutoGen is done.
//
// [NOTE]: The size of different types of RuleElem should be the same,
//         so that the memory is organized as a vector and elements are
//         identified using index.

class MemPool;
class RuleElemPool {
private:
  MemPool *mMP;
public:
  RuleElemPool(MemPool *mp) : mMP(mp){}
  ~RuleElemPool(){}
  RuleElem* NewRuleElem();
};
}

#endif
