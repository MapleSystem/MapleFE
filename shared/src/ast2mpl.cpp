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

namespace maplefe {

A2M::A2M(const char *filename) : mFileName(filename) {
  mMirModule = new maple::MIRModule(mFileName);
  mMirBuilder = mMirModule->mirBuilder;
  mDefaultType = GlobalTables::GetTypeTable().GetOrCreateClassType("DEFAULT_TYPE", mMirModule);
  mDefaultType->typeKind = maple::MIRTypeKind::kTypeClass;
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
    ProcessNode(tnode);
  }
}

MIRType *A2M::MapType(TreeNode *type) {
  MIRType *mir_type = mDefaultType;
  if (!type) {
    return mir_type;
  }

  char *name = type->GetName();
  if (mNodeTypeMap.find(name) != mNodeTypeMap.end()) {
    return mNodeTypeMap[name];
  }

  if (type->IsPrimType()) {
    PrimTypeNode *ptnode = static_cast<PrimTypeNode *>(type);
    mir_type = MapPrimType(ptnode);

    // update mNodeTypeMap
    mNodeTypeMap[name] = mir_type;
  } else  if (type->IsUserType()) {
    if (type->IsIdentifier()) {
      IdentifierNode *inode = static_cast<IdentifierNode *>(type);
      mir_type = MapType(inode->GetType());
    } else if (type->IsLiteral()) {
      NOTYETIMPL("MapType IsUserType IsLiteral");
    } else {
      NOTYETIMPL("MapType IsUserType");
    }
    // DimensionNode *mDims
    // unsigned dnum = inode->GetDimsNum();
    mNodeTypeMap[name] = mir_type;
  } else {
    NOTYETIMPL("MapType Unknown");
  }
  return mir_type;
}

MIRSymbol *A2M::MapGlobalSymbol(TreeNode *tnode) {
}

MIRSymbol *A2M::MapLocalSymbol(TreeNode *tnode, maple::MIRFunction *func) {
  const char *name = tnode->GetName();
  MIRType *mir_type;

  if (tnode->IsIdentifier()) {
    IdentifierNode *inode = static_cast<IdentifierNode *>(tnode);
    mir_type = MapType(inode->GetType());
  } else if (tnode->IsLiteral()) {
    NOTYETIMPL("MapLocalSymbol LiteralNode()");
    mir_type = mDefaultType;
  }

  MIRSymbol *symbol = mMirBuilder->CreateLocalDecl(name, mir_type, func);
  return symbol;
}

void A2M::MapAttr(GenericAttrs &attr, const IdentifierNode *inode) {
  // SmallVector<AttrId> mAttrs
  unsigned anum = inode->GetAttrsNum();
  for (int i = 0; i < anum; i++) {
  }
}
}

