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
// This file contains the Memory Pool for AppealNode which are dynamically    //
// allocated.                                                               //
//////////////////////////////////////////////////////////////////////////////

#ifndef __APPNODE_POOL_H__
#define __APPNODE_POOL_H__

#include "mempool.h"

namespace maplefe {

class AppealNode;

// this Pool contains two types of dynamic memory.
// (1) Those managed by mMP. This is where all the AppealNode come from.
// (2) Those managed by some containers in some AppealNode. For example, the
//     SmallVector of children nodes. These are maintained by containers.
//
class AppealNodePool {
private:
  MemPool mMP;
public:
  AppealNodePool(){}
  ~AppealNodePool() {mMP.Release();}

  void  SetBlockSize(unsigned s) {mMP.SetBlockSize(s);}
  AppealNode* NewAppealNode();

  // Clear all data, keep the memory
  void  Clear() {mMP.Clear();}
};

}
#endif
