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

/////////////////////////////////////////////////////////////////////////////////////
// This module is to provide a common place to maintain the information of recursion
// during parsing. It computes the fundamental information of a recursion like
// LeadFronNode, FronNode. It also provides some query functions useful.
/////////////////////////////////////////////////////////////////////////////////////

#include "recursion.h"
#include "rule_summary.h"
#include "token.h"

namespace maplefe {

Rule2Recursion* GetRule2Recursion(RuleTable *rule_table){
  for (unsigned i = 0; i < gRule2RecursionNum; i++) {
    Rule2Recursion *r2r = gRule2Recursion[i];
    if (r2r->mRuleTable == rule_table)
      return r2r;
  }
  return NULL;
}

// Find the RecursionGroup 'rt' belongs to
bool FindRecursionGroup(RuleTable *rt, unsigned &id) {
  int index = gRule2Group[rt->mIndex];
  if (index == -1) {
    return false;
  } else {
    id = index;
    MASSERT(id < gRecursionGroupsNum);
    return true;
  }
}

// Find the index-th child and return it.
// This function only returns Token or RuleTable, it won't take of FNT_Concat.
FronNode RuleFindChildAtIndex(RuleTable *parent, unsigned index) {
  FronNode node;
  EntryType type = parent->mType;

  switch(type) {
  // Concatenate and Oneof, can be handled the same
  case ET_Concatenate:
  case ET_Oneof: {
    TableData *data = parent->mData + index;
    switch (data->mType) {
    case DT_Subtable:
      node.mType = FNT_Rule;
      node.mData.mTable = data->mData.mEntry;
      break;
    case DT_Token:
      node.mType = FNT_Token;
      node.mData.mToken = &gSystemTokens[data->mData.mTokenId];
      break;
    default:
      MERROR("Unkown type in table data");
      break;
    }
    break;
  }

  // Zeroorone, Zeroormore and Data can be handled the same way.
  case ET_Data:
  case ET_Zeroorone:
  case ET_Zeroormore: {
    MASSERT((index == 0) && "zeroormore node has more than one elements?");
    TableData *data = parent->mData;
    switch (data->mType) {
    case DT_Subtable:
      node.mType = FNT_Rule;
      node.mData.mTable = data->mData.mEntry;
      break;
    case DT_Token:
      node.mType = FNT_Token;
      node.mData.mToken = &gSystemTokens[data->mData.mTokenId];
      break;
    default:
      MERROR("Unkown type in table data");
      break;
    }
    break;
  }

  default:
    MERROR("Unkown type of table");
    break;
  }

  return node;
}

/////////////////////////////////////////////////////////////////////////////////////
//               Implementation of class Recursion
/////////////////////////////////////////////////////////////////////////////////////

Recursion::Recursion(LeftRecursion *lr) {
  mNum = 0;
  mLeadNode = NULL;
  mCircles = NULL;

  Init(lr);
}

void Recursion::Release() {
  mRecursionNodes.Release();
  mLeadFronNodes.Release();

  for (unsigned i = 0; i <mFronNodes.GetNum(); i++) {
    SmallVector<FronNode> *p_fnodes = mFronNodes.ValueAtIndex(i);
    delete p_fnodes;
  }
  mFronNodes.Release();

  for (unsigned i = 0; i <mRecursionNodes.GetNum(); i++) {
    SmallVector<RuleTable*> *rec_nodes = mRecursionNodes.ValueAtIndex(i);
    delete rec_nodes;
  }
  mRecursionNodes.Release();
}

// 1. Collect all info from LeftRecursion.
// 2. Calculate RecursionNods, LeadFronNode and FronNode.
void Recursion::Init(LeftRecursion *lr) {
  // Collect info
  mLeadNode = lr->mRuleTable;
  mNum = lr->mNum;
  mCircles = lr->mCircles;
}

bool Recursion::IsRecursionNode(RuleTable *rt) {
  for (unsigned k = 0; k < mRecursionNodes.GetNum(); k++) {
    SmallVector<RuleTable*> *rec_nodes = mRecursionNodes.ValueAtIndex(k);
    if (rec_nodes->Find(rt))
      return true;
  }
  return false;
}

// Find all the nodes in all circles of the recursion.
// Each node should be a RuleTable* since it forms a circle.
// A node could be shared among multiple circles.
void Recursion::FindRecursionNodes() {
  for (unsigned i = 0; i < mNum; i++) {
    unsigned *circle = mCircles[i];
    unsigned len = circle[0];
    RuleTable *prev = mLeadNode;

    SmallVector<RuleTable*> *rec_nodes = mRecursionNodes.ValueAtIndex(i);
    rec_nodes->PushBack(mLeadNode);

    for (unsigned j = 1; j <= len; j++) {
      unsigned child_index = circle[j];
      FronNode node = RuleFindChildAtIndex(prev, child_index);
      MASSERT(node.mType == FNT_Rule);
      RuleTable *rt = node.mData.mTable;

      if (j == len)
        MASSERT(rt == mLeadNode);
      else
        rec_nodes->PushBack(rt);

      prev = rt;
    }
  }
}

// cir_idx : the index of circle.
// pos     : the position of the rule on the circle.
//           0 is the length of circle.
RuleTable* Recursion::FindRuleOnCircle(unsigned cir_idx, unsigned pos) {
  unsigned *circle = mCircles[cir_idx];
  unsigned len = circle[0];
  MASSERT(pos <= len);
  RuleTable *prev = mLeadNode;
  for (unsigned j = 1; j <= pos; j++) {
    unsigned child_index = circle[j];
    FronNode node = RuleFindChildAtIndex(prev, child_index);
    MASSERT(node.mType == FNT_Rule);
    RuleTable *rt = node.mData.mTable;
    // The last one should be the back edge.
    if (j == len)
      MASSERT(rt == mLeadNode);
    prev = rt;
  }
  MASSERT(prev);
  return prev;
}

// Find the FronNode along one of the circles.
// We also record the position of each FronNode at the circle. 1 means the first
// node after LeadNode.
// FronNode: Nodes directly accessible from 'circle' but not in RecursionNodes.
//
// [NOTE] Concatenate rule is complicated. Here is an example.
//
//  rule Add: ONEOF(Id, Add '+' Id)
//
// The recursion graph of this Add looks like below
//
//           Add ------>Id
//            ^    |
//            |    |-->Add  '+'   Id
//            |         |
//            |----------
//
// For a input expresion like a + b + c, the Appeal Tree looks like below
//
//                 Add
//                  |
//                 Add-------> '+'
//                  |   |----> Id --> c
//                  |
//                 Add-------> '+'
//                  |   |----> Id --> b
//                  |
//                 Add
//                  |
//                  |
//                 Id
//                  |
//                  a
// It's clear that the last two nodes of (Add '+' Id) is a FronNode which will help
// matching input tokens.
//
// The FronNode of Concatenate node has two specialities.
// (1) Once a child is decided as the starting FronNode, all remaining nodes will
//     be counted as in the same FronNode because Concatenate node requires all nodes
//     involved.
// (2) There could be some leading children nodes before the node on the circle path,
//     eg. 'Add' in this case. Those leading children nodes are MaybeZero, and they
//     could matching nothing.
// So for such a concatenate FronNode, it needs both the parent node and the index
// of the starting child. And it could contains more than one nodes.

void Recursion::FindFronNodes(unsigned circle_index) {
  unsigned *circle = mCircles[circle_index];
  unsigned len = circle[0];
  RuleTable *prev = mLeadNode;
  SmallVector<FronNode> *fron_nodes = mFronNodes.ValueAtIndex(circle_index);

  for (unsigned j = 1; j <= len; j++) {
    unsigned child_index = circle[j];
    FronNode node = RuleFindChildAtIndex(prev, child_index);
    MASSERT(node.mType == FNT_Rule);
    RuleTable *next = node.mData.mTable;

    // The last one should be the back edge.
    if (j == len)
      MASSERT(next == mLeadNode);

    // prev is mLeadNode, FronNode here is a LeadFronNode and will be
    // traversed in TraverseLeadNode();
    if (j == 1) {
      prev = next;
      continue;
    }

    EntryType prev_type = prev->mType;
    switch(prev_type) {
    case ET_Oneof: {
      // Look into every childof 'prev'. If it's not 'next' and
      // not in 'mRecursionNodes', it's a FronNode.
      //
      // [NOTE] This is a per circle algorithm. So a FronNode found here could
      //        be a recursion node in another circle.
      // Actually if it's Recursion node, we can still include it as FronNode,
      // and we can skip the traversal when we figure out it re-enters the same
      // recursion. We'd like to handle it here instead of there.
      for (unsigned i = 0; i < prev->mNum; i++) {
        TableData *data = prev->mData + i;
        FronNode fnode;
        if (data->mType == DT_Token) {
          fnode.mPos = j;
          fnode.mType = FNT_Token;
          fnode.mData.mToken = &gSystemTokens[data->mData.mTokenId];
          fron_nodes->PushBack(fnode);
          //std::cout << "  Token " << data->mData.mToken->GetName() << std::endl;
        } else if (data->mType = DT_Subtable) {
          RuleTable *ruletable = data->mData.mEntry;
          bool found = IsRecursionNode(ruletable);
          if (!found && (ruletable != next)) {
            fnode.mPos = j;
            fnode.mType = FNT_Rule;
            fnode.mData.mTable = ruletable;
            fron_nodes->PushBack(fnode);
            //std::cout << "  Rule " << GetRuleTableName(ruletable) << std::endl;
          }
        } else {
          MASSERT(0 && "unexpected data type in ruletable.");
        }
      }
      break;
    }

    case ET_Zeroormore:
    case ET_Zeroorone:
    case ET_Data:
      // There is one and only one child. And it must be in circle.
      // In this case, there is no FronNode.
      MASSERT((prev->mNum == 1) && "zeroorxxx node has more than one elements?");
      MASSERT((child_index == 0));
      break;

    // The described in the comments of this function, concatenate node has
    // complicated FronNode.
    // We don't check if the remaining children are RecursionNode or not, they
    // can be handled later if we see re-entering the same recursion or parent
    // recursions.
    case ET_Concatenate: {
      if (child_index < prev->mNum - 1) {
        FronNode fnode;
        fnode.mPos = j;
        fnode.mType = FNT_Concat;
        fnode.mData.mStartIndex = child_index + 1;
        fron_nodes->PushBack(fnode);
      }
      break;
    }

    case ET_Null:
    default:
      MASSERT(0 && "Wrong node type in a circle");
      break;
    }// end of switch

    prev = next;
  }
}

void Recursion::FindFronNodes() {
  const char *name = GetRuleTableName(mLeadNode);
  //std::cout << "FindFronNodes for " << name << std::endl;
  for (unsigned i = 0; i < mNum; i++)
    FindFronNodes(i);
}

void Recursion::DumpFronNode(FronNode fnode) {
  std::cout << "Pos:" << fnode.mPos << " ";
  switch (fnode.mType) {
  case FNT_Token:
    std::cout << "token ";
    std::cout << fnode.mData.mToken->GetName();
    break;
  case FNT_Rule:
    std::cout << "Rule ";
    std::cout << GetRuleTableName(fnode.mData.mTable);
    break;
  case FNT_Concat:
    std::cout << "Concat@";
    std::cout << fnode.mData.mStartIndex;
    break;
  }
}

void Recursion::Dump() {
  MASSERT(mRecursionNodes.GetNum() == mNum);
  std::cout << "LeadNode:" << GetRuleTableName(mLeadNode) << std::endl;
  for (unsigned i = 0; i < mRecursionNodes.GetNum(); i++) {
    // dump recursion nodes
    std::cout << "Circle " << i << " RecNodes: ";
    SmallVector<RuleTable*> *rec_nodes = mRecursionNodes.ValueAtIndex(i);
    for (unsigned j = 0; j < rec_nodes->GetNum(); j++) {
      RuleTable *rt = rec_nodes->ValueAtIndex(j);
      std::cout << GetRuleTableName(rt) << ", ";
    }
    std::cout << std::endl;

    // dump FronNodes
    std::cout << "Circle " << i << " FronNodes: ";
    SmallVector<FronNode> *fron_nodes = mFronNodes.ValueAtIndex(i);
    for (unsigned j = 0; j < fron_nodes->GetNum(); j++) {
      std::cout << "[";
      FronNode fn = fron_nodes->ValueAtIndex(j);
      DumpFronNode(fn);
      std::cout << "], ";
    }
    std::cout << std::endl;
  }
}

/////////////////////////////////////////////////////////////////////////////////////
//               Implementation of class RecursionAll
/////////////////////////////////////////////////////////////////////////////////////

void RecursionAll::Init() {
  for (unsigned i = 0; i < gLeftRecursionsNum; i++) {
    Recursion *rec = new Recursion(gLeftRecursions[i]);
    mRecursions.PushBack(rec);
  }
}

void RecursionAll::Release() {
  for (unsigned i = 0; i < mRecursions.GetNum(); i++) {
    Recursion *rec = mRecursions.ValueAtIndex(i);
    delete rec;
  }
  // Release the SmallVector
  mRecursions.Release();
}

// Find the LeftRecursion with 'rt' the LeadNode.
Recursion* RecursionAll::FindRecursion(RuleTable *rt) {
  for (unsigned i = 0; i < mRecursions.GetNum(); i++) {
    Recursion *rec = mRecursions.ValueAtIndex(i);
    if (rec->GetLeadNode() == rt)
      return rec;
  }
  return NULL;
}

bool RecursionAll::IsLeadNode(RuleTable *rt) {
  if (FindRecursion(rt))
    return true;
  else
    return false;
}

void RecursionAll::Dump() {
  std::cout << "===================== Total ";
  std::cout << mRecursions.GetNum();
  std::cout << " Recursions =====================" << std::endl;
  for (unsigned i = 0; i < mRecursions.GetNum(); i++) {
    std::cout << "No." << i << std::endl;
    Recursion *rec = mRecursions.ValueAtIndex(i);
    rec->Dump();
  }
  std::cout << "===================== End Recursions Dump ===============";
  std::cout << std::endl;
}
}
