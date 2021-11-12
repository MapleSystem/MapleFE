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
#include "gen_astdump.h"

namespace maplefe {

TypeTable gTypeTable;

TypeEntry::TypeEntry(TreeNode *node) {
  mType = node;
  if (!node->IsTypeIdNone()) {
    mTypeId = node->GetTypeId();
  } else {
    switch (node->GetKind()) {
      case NK_Struct:
      case NK_StructLiteral:
      case NK_Class:
      case NK_Interface:
        mTypeId = TY_Class;
        break;
      case NK_ArrayLiteral:
        mTypeId = TY_Array;
        break;
      case NK_UserType:
        mTypeId = TY_User;
        break;
      default:
        mTypeId = TY_None;
        break;
    }
  }
}

TreeNode *TypeTable::CreatePrimType(std::string name, TypeId tid) {
  unsigned stridx = gStringPool.GetStrIdx(name);
  PrimTypeNode *node = (PrimTypeNode*)gTreePool.NewTreeNode(sizeof(PrimTypeNode));
  new (node) PrimTypeNode();
  node->SetStrIdx(stridx);
  node->SetPrimType(tid);
  node->SetTypeId(tid);
  return node;
}

TreeNode *TypeTable::CreateBuiltinType(std::string name, TypeId tid) {
  unsigned stridx = gStringPool.GetStrIdx(name);
  IdentifierNode *node = (IdentifierNode*)gTreePool.NewTreeNode(sizeof(IdentifierNode));
  new (node) IdentifierNode(stridx);
  node->SetTypeId(tid);

  UserTypeNode *utype = (UserTypeNode*)gTreePool.NewTreeNode(sizeof(UserTypeNode));
  new (utype) UserTypeNode(node);
  utype->SetStrIdx(stridx);
  utype->SetTypeId(TY_Class);
  node->SetParent(utype);
  return utype;
}

bool TypeTable::AddType(TreeNode *node) {
  unsigned id = node->GetNodeId();
  if (mNodeId2TypeIdxMap.find(id) != mNodeId2TypeIdxMap.end()) {
    return false;
  }
  unsigned tyidx = mTypeTable.size();
  mNodeId2TypeIdxMap[id] = tyidx;
  node->SetTypeIdx(tyidx);
  TypeEntry *entry = new TypeEntry(node);
  mTypeTable.push_back(entry);
  return true;
}

#undef  TYPE
#undef  PRIMTYPE
void TypeTable::AddPrimAndBuiltinTypes() {
  // only initialize once
  if (mTypeTable.size() != 0) {
    return;
  }

  TreeNode *node;
  // add a NULL entry so real typeidx starting from 1
  TypeEntry *entry = new TypeEntry();
  mTypeTable.push_back(entry);

  // first are primitive types, and their typeid TY_Xyz is their typeidx as well
#define TYPE(T)
#define PRIMTYPE(T) node = CreatePrimType(#T, TY_##T); AddType(node);
#include "supported_types.def"
  // add additional primitive types for number and string
  PRIMTYPE(Number);
  PRIMTYPE(String);

  mPrimSize = size();

#define TYPE(T)     node = CreateBuiltinType(#T, TY_##T); AddType(node);
#define PRIMTYPE(T)
  // additional usertype Boolean
  TYPE(Boolean);
#include "supported_types.def"

  mPreBuildSize = size();
  return;
}

TypeEntry *TypeTable::GetTypeEntryFromTypeIdx(unsigned idx) {
  MASSERT(idx < mTypeTable.size() && "type index out of range");
  return mTypeTable[idx];
}

TreeNode *TypeTable::GetTypeFromTypeIdx(unsigned idx) {
  MASSERT(idx < mTypeTable.size() && "type index out of range");
  return mTypeTable[idx]->GetType();
}

void TypeTable::Dump() {
  std::cout << "===================== TypeTable =====================" << std::endl;
  std::cout << " tid:type-name: node-kind  node-id" << std::endl;
  std::cout << "--------------------------------" << std::endl;
  unsigned idx = 1;
  for (unsigned idx = 1; idx < mTypeTable.size(); idx++) {
    TypeEntry *entry = mTypeTable[idx];
    TreeNode *node = entry->GetType();
    TypeId tid = node->GetTypeId();
    if (node->IsUserType()) {
      tid = static_cast<UserTypeNode*>(node)->GetId()->GetTypeId();
    }
    std::cout << "  " << idx << " : " << node->GetName() << " : " <<
              AstDump::GetEnumNodeKind(node->GetKind()) << " " <<
              AstDump::GetEnumTypeId(tid) << " " << tid << " " <<
              node->GetNodeId() << std::endl;
  }
  std::cout << "===================== End TypeTable =====================" << std::endl;
}

}

