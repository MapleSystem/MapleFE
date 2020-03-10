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

// [NOTE]: size of all kinds of rule elements must be the same
RuleElem* RuleElemPool::NewRuleElem() {
  unsigned elemSize = sizeof(RuleElem);
  char *addr = mMP->Alloc(elemSize);
  RuleElem *elem = new (addr) RuleElem();
  return elem;
}
