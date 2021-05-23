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

///////////////////////////////////////////////////////////////////////////////
// This is about the attributes of symbols, or maybe a block of code. I decided
// to give each supported attribute a tree node for two reasons. (1) During
// AST generation (aka the parsing procedure), it's easier to give each attribe
// a tree node, so we don't need handle it as a special case. (2) When the attribute
// is applied to a symbol, it won't be part of the AST. The symbol only takes
// the AttrId. Attribute tree node is still sitting in its pool, no one reference
// them from the tree.
//
// We have a small pool for all these attribute nodes. The number is very small
// since there are not so many attributes in the languages.
///////////////////////////////////////////////////////////////////////////////

#ifndef __AST_ATTR_H__
#define __AST_ATTR_H__

#include "ruletable.h"
#include "container.h"
#include "ast.h"
#include "supported.h"

namespace maplefe {

class AttrNode : public TreeNode {
private:
  AttrId mId;
public:
  AttrNode() : TreeNode(NK_Attr), mId(ATTR_NA) {}
  ~AttrNode(){}

  AttrId GetId()          {return mId;}
  void   SetId(AttrId id) {mId = id;}
};

extern const char* FindAttrKeyword(AttrId id);
extern AttrId FindAttrId(const char *keyword);

///////////////////////////////////////////////////////////////////////////////
//                      AttrPool
// The size of AttrNode is fixed, so it's good to use MemPool for the storage.
// The AttrNode pool is global, across all modules.
///////////////////////////////////////////////////////////////////////////////

class AttrPool {
private:
  SmallVector<AttrNode> mAttrs;
  void InitAttrs();
public:
  AttrPool();
  ~AttrPool();

  AttrNode* GetAttrNode(AttrId);
  AttrNode* GetAttrNode(const char *keyword);
  AttrId    GetAttrId(const char *keyword);
};

// A global pool for all AttrNodes.
extern AttrPool gAttrPool;

}
#endif
