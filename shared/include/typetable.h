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
// This file contains the Memory Pool for String pool.                      //
//                                                                          //
// A String is stored in the pool with an ending '\0'.                      //
//////////////////////////////////////////////////////////////////////////////

#ifndef __TYPETABLE_H__
#define __TYPETABLE_H__

#include <unordered_map>
#include <vector>
#include <string>
#include "massert.h"
#include "ast.h"

namespace maplefe {

class TypeEntry {
 private:
  TreeNode *mType;
  TypeId    mTypeId;
  NodeKind  mTypeKind;

 public:
  TypeEntry();
  TypeEntry(TreeNode *node);
  ~TypeEntry(){};

  NodeKind  GetTypeKind() { return mTypeKind; }
  TypeId    GetTypeId()   { return mTypeId; }
  TreeNode *GetType()     { return mType; }
  
  void SetTypeKind(NodeKind k) { mTypeKind = k; }
  void SetTypeId(TypeId i)     { mTypeId = i; }
  void SetType(TreeNode *n)    { mType = n; }
};

class TypeTable {
private:
  std::vector<TypeEntry *> mTypeTable;
  std::unordered_map<unsigned, unsigned> mNodeId2TypeIdxMap;

public:
  TypeTable();
  ~TypeTable();

  bool AddType(TreeNode *node);
  TypeEntry *GetTypeFromTypeIdx(unsigned idx);
  void Dump();
};

}
#endif  // __TYPETABLE_H__
