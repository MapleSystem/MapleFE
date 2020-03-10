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
// This file contains the Memory Pool for AST node which are dynamically    //
// allocated.                                                               //
//////////////////////////////////////////////////////////////////////////////

#ifndef __AST_MEMPOOL_H__
#define __AST_MEMPOOL_H__

#include <map>
#include <vector>

#include "mempool.h"

class TreeNode;

class TreePool {
private:
  MemPool mMP;
public:
  std::vector<TreeNode*> mTreeNodes; // only TreeNode* is stored, no matter what's
public:
  TreePool(){}
  ~TreePool(){}

  char* NewTreeNode(unsigned);
};

#endif
