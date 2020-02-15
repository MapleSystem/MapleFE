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

// TreePool will request/release memory on the Block level.
// So far it only request new Block and keep (re)using it. It won't release
// any memory right now, or it even doesn't let MemPool know a Block is free.
// TODO: We will come back to this.

class TreePool {
private:
  MemPool               mMP;       //
  std::vector<char *>   mBlocks;
  std::vector<unsigned> mTags;
  unsigned              mCurBlock;
  unsigned              mCurPos;   // current available position in mCurBlock.
                                   // It's offset from starting of mCurBlock
public:
  std::vector<TreeNode*>   mTreeNodes; // only TreeNode* is stored, no matter what's
                                       // the exact derived class.

private:
  char* NewBlock();

public:
  TreePool();
  ~TreePool();   // memory is freed in destructor of mMP.

  char* NewTreeNode(unsigned);
};

#endif
