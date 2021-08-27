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

TypeEntry::TypeEntry() {
  mType = NULL;
  mTypeId = TY_None;
  mTypeKind = NK_Null;
}

TypeEntry::TypeEntry(TreeNode *node) {
  mType = node;
  mTypeKind = node->GetKind();
  if (node->GetTypeId() != TY_None) {
    mTypeId = node->GetTypeId();
  } else {
    switch (mTypeKind) {
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

TypeTable::TypeTable() {
  // insert a dummy so type index starting from 1
  mTypeTable.push_back(NULL);
  AddPrimTypes();
}

TypeTable::~TypeTable() {
  mTypeTable.clear();
}

#undef  TYPE
#undef  PRIMTYPE
#define TYPE(T)
#define PRIMTYPE(T) \
 stridx = gStringPool.GetStrIdx(#T); \
 node = (PrimTypeNode*)gTreePool.NewTreeNode(sizeof(PrimTypeNode)); \
 new (node) PrimTypeNode(); \
 node->SetPrimType(TY_##T); \
 node->SetTypeId(TY_##T); \
 node->SetStrIdx(stridx); \
 AddType(node);
void TypeTable::AddPrimTypes() {
  unsigned stridx;
  PrimTypeNode *node;
  TypeEntry *entry;
#include "supported_types.def"
  return;
}

bool TypeTable::AddType(TreeNode *node) {
  unsigned id = node->GetNodeId();
  if (mNodeId2TypeIdxMap.find(id) != mNodeId2TypeIdxMap.end()) {
    return false;
  }
  mNodeId2TypeIdxMap[id] = mTypeTable.size();
  TypeEntry *entry = new TypeEntry(node);
  mTypeTable.push_back(entry);
  return true;
}

TypeEntry *TypeTable::GetTypeFromTypeIdx(unsigned idx) {
  MASSERT(idx < mTypeTable.size() && "type index out of range");
  return mTypeTable[idx];
}

void TypeTable::Dump() {
  std::cout << "===================== TypeTable =====================" << std::endl;
  unsigned idx = 1;
  for (unsigned idx = 1; idx < mTypeTable.size(); idx++) {
    TypeEntry *entry = mTypeTable[idx];
    TreeNode *node = entry->GetType();
    std::cout << "  " << idx << " : " << node->GetName() << " : " <<
              AstDump::GetEnumTypeId(entry->GetTypeId()) << " " <<
              AstDump::GetEnumNodeKind(node->GetKind()) << " " <<
              node->GetNodeId() << std::endl;
  }
  std::cout << "===================== End TypeTable =====================" << std::endl;
}

}

