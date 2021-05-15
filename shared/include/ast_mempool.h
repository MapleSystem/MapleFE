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
// This file contains the Memory Pool for AST node which are dynamically    //
// allocated.                                                               //
//////////////////////////////////////////////////////////////////////////////

#ifndef __AST_MEMPOOL_H__
#define __AST_MEMPOOL_H__

#include <vector>

#include "mempool.h"

namespace maplefe {

class TreeNode;

// TreePool contains two types of dynamic memory.
// (1) Those managed by mMP. This is where all the TreeNode come from.
// (2) Those managed by some containers in some TreeNodes. For example, the
//     SmallVector of children nodes. These are maintained by containers.
//
// To accomodate to the two different scenarios, we add Release() to allow
// each tree node explicitly release container-managed memory.

class TreePool {
private:
  MemPool mMP;
public:
  std::vector<TreeNode*> mTreeNodes; // only TreeNode* is stored, no matter what's
public:
  TreePool(){}
  ~TreePool();

  void  SetBlockSize(unsigned s) {mMP.SetBlockSize(s);}

  char* NewTreeNode(unsigned);
  void  Release();  // Allow user to explicitly
                    // (1) release memory by mMP
                    // (2) release memory allocated inside each TreeNode, which
                    //     is out of the control of mMP.
};

extern TreePool gTreePool;
}
#endif
