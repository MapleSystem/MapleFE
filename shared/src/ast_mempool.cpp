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
#include "ast_mempool.h"
#include "ast.h"

namespace maplefe {

TreePool gTreePool;

TreePool::~TreePool() {
  Release();
}

char* TreePool::NewTreeNode(unsigned size) {
  char *addr = mMP.Alloc(size);
  TreeNode *tree = (TreeNode*)addr;
  mTreeNodes.push_back(tree);
  unsigned id = mTreeNodes.size();
  tree->SetNodeId(id);
  return addr;
}

void TreePool::Release() {
  // step 1. Release the containers in each tree node.
  std::vector<TreeNode*>::iterator it = mTreeNodes.begin();
  for (; it != mTreeNodes.end(); it++) {
    TreeNode *n = *it;
    n->Release();
  }

  mTreeNodes.clear();

  // step 2. Release mMP
  mMP.Release();
}
}
