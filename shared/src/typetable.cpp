/*
* Copyright (C) [2021] Futurewei Technologies, Inc. All rights reverved.
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
//                                                                          //
// This file contains the implementation of string pool.                    //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include <cstdlib>
#include <cstring>

#include "typetable.h"

namespace maplefe {

TypeEntry::TypeEntry() {
  mType = NULL;
  mTypeId = TY_None;
  mTypeKind = NK_Null;
}

TypeEntry::TypeEntry(TreeNode *node) {
  mType = node;
  mTypeId = node->GetTypeId();
  mTypeKind = node->GetKind();
}

TypeTable::TypeTable() {
  // insert a dummy so type index starting from 1
  mTypeTable.push_back(NULL);
}

TypeTable::~TypeTable() {
  mTypeTable.clear();
}

bool TypeTable::AddType(TreeNode *node) {
  unsigned id = node->GetNodeId();
  if (mNodeId2TypeIdxMap.find(id) != mNodeId2TypeIdxMap.end()) {
    return false;
  }
  mNodeId2TypeIdxMap[id] = mTypeTable.size();
  mTypeTable.push_back(node);
  return true;
}

TreeNode *TypeTable::GetTypeFromTypeIdx(unsigned idx) {
  MASSERT(idx < mTypeTable.size() && "type index out of range");
  return mTypeTable[idx];
}

}

