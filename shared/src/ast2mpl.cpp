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

#include "ast2mpl.h"
#include "mir_function.h"
#include "constant_fold.h"

A2M::A2M(const char *filename) : mFileName(filename) {
  mMirModule = new maple::MIRModule(mFileName);
}

A2M::~A2M() {
  delete mMirModule;
  mNodeTypeMap.clear();
}

void A2M::ProcessAST(bool trace_a2m) {
  mTraceA2m = trace_a2m;
  if (mTraceA2m) std::cout << "============= in ProcessAST ===========" << std::endl;
  for(auto it: gModule.mTrees) {
    if (mTraceA2m) it->Dump(0);
    TreeNode *tnode = it->mRootNode;
    switch (tnode->GetKind()) {
#undef  NODEKIND
#define NODEKIND(K) case NK_##K: Process##K(tnode); break;
#include "ast_nk.def"
      default:
       break;
    }
  }
}

MIRType *A2M::MapType(TreeNode *type) {
  if (mNodeTypeMap.find(type) != mNodeTypeMap.end()) {
    return mNodeTypeMap[type];
  }

  MIRType *mir_type = nullptr;
  if (type->IsPrimType()) {
    PrimTypeNode *ptnode = static_cast<PrimTypeNode *>(type);
    mir_type = MapPrimType(ptnode);

    // update mNodeTypeMap
    mNodeTypeMap[type] = mir_type;
  }

  if (type->IsUserType()) {
    // DimensionNode *mDims
    // unsigned dnum = inode->GetDimsNum();
    MASSERT("type not set");
  }
  return mir_type;
}

void A2M::MapAttr(GenericAttrs &attr, const IdentifierNode *inode) {
  // SmallVector<AttrId> mAttrs
  unsigned anum = inode->GetAttrsNum();
  for (int i = 0; i < anum; i++) {
  }
}

