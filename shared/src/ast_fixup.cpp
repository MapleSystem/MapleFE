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

#include "ast_fixup.h"

namespace maplefe {

bool FixUpVisitor::FixUp() {
  for (auto it : mASTModule->mTrees)
    VisitTreeNode(it->mRootNode);
  return mUpdated;
}

// Fix up mOprId of a UnaOperatorNode
UnaOperatorNode *FixUpVisitor::VisitUnaOperatorNode(UnaOperatorNode *node) {
  switch(node->GetOprId()) {
    case OPR_Add:
      node->SetOprId(OPR_Plus);
      mUpdated = true;
      break;
    case OPR_Sub:
      node->SetOprId(OPR_Minus);
      mUpdated = true;
      break;
    case OPR_Inc:
      if(!node->IsPost()) {
        node->SetOprId(OPR_PreInc);
        mUpdated = true;
      }
      break;
    case OPR_Dec:
      if(!node->IsPost()) {
        node->SetOprId(OPR_PreDec);
        mUpdated = true;
      }
  }
  return node;
}

// Fix up mName of UserTypeNode
UserTypeNode *FixUpVisitor::VisitUserTypeNode(UserTypeNode *node) {
  if(auto id = node->GetId()) {
    if(auto n = id->GetStrIdx()) {
      node->SetStrIdx(n);
    }
  }
  return node;
}

}
