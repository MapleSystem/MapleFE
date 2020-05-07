/*
* Copyright (C) [2020] Futurewei Technologies, Inc. All rights reverved.
*
* OpenArkFE is licensed under the Mulan PSL v1.
* You can use this software according to the terms and conditions of the Mulan PSL v1.
* You may obtain a copy of Mulan PSL v1 at:
*
*  http://license.coscl.org.cn/MulanPSL
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR
* FIT FOR A PARTICULAR PURPOSE.
* See the Mulan PSL v1 for more details.
*/

#include "common_header_autogen.h"
#include "ruletable_util.h"
#include "rec_detect.h"

////////////////////////////////////////////////////////////////////////////////////
// The idea of Rursion Dectect is through a Depth First Traversal in the tree.
// There are a few things we need make it clear.
//  1) We are looking for back edge when traversing the tree. Those back edges form
//     the recursions. We differentiate a recursion using the first node, ie, the topmost
//     node in the tree in this recursion.
//  2) Each node (ie rule table) could have multiple recursions.
//  3) Recursions could include children recursions inside. 
////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////
//                                RecPath
////////////////////////////////////////////////////////////////////////////////////

void RecPath::Dump() {
  for (unsigned i = 0; i < mPositions.GetNum(); i++) {
    std::cout << mPositions.ValueAtIndex(i) << ",";
  }
  std::cout << std::endl;
}

////////////////////////////////////////////////////////////////////////////////////
//                                Recursion
////////////////////////////////////////////////////////////////////////////////////

void Recursion::Release() {
  for (unsigned i = 0; i < mPaths.GetNum(); i++) {
    RecPath *path = mPaths.ValueAtIndex(i);
    path->Release();
  }

  mPaths.Release();
}

////////////////////////////////////////////////////////////////////////////////////
//                                RecDetector
////////////////////////////////////////////////////////////////////////////////////

void RecDetector::SetupTopTables() {
  mTopTables.PushBack(&TblStatement);
  mTopTables.PushBack(&TblClassDeclaration);
  mTopTables.PushBack(&TblInterfaceDeclaration);
}

// A talbe is already been processed.
bool RecDetector::IsInProcess(RuleTable *t) {
  for (unsigned i = 0; i < mInProcess.GetNum(); i++) {
    if (t == mInProcess.ValueAtIndex(i))
      return true;
  }
  return false;
}

// A talbe is already done.
bool RecDetector::IsDone(RuleTable *t) {
  for (unsigned i = 0; i < mDone.GetNum(); i++) {
    if (t == mDone.ValueAtIndex(i))
      return true;
  }
  return false;
}

// There is a back edge from 'p' to the first appearance of 'rt'. Actually in our
// traversal tree, in the current path (from 'p' upward to the root) there is one
// and only node representing 'rt', since the second appearance will be treated
// as a back edge. 
void RecDetector::AddRecursion(RuleTable *rt, ContTreeNode<RuleTable*> *p) {
  RecPath *path = (RecPath*)gMemPool.Alloc(sizeof(RecPath));
  new (path) RecPath();

  // Step 1. Traverse upwards to find the target tree node.

  ContTreeNode<RuleTable*> *target = NULL;
  ContTreeNode<RuleTable*> *node = p;
  SmallVector<ContTreeNode<RuleTable*>*> node_list;
  while (node) {
    node_list.PushBack(node);
    // I will let it go until the root, even we already find the target in middle.
    // I do this to verify my code is correct. [TODO] will come back to remove
    // this extra cost.
    if (node->GetData() == rt) {
      MASSERT(!target && "duplicated target node in a path.");
      target = node;
      break;
    }
    node = node->GetParent();
  }

  MASSERT(target);
  MASSERT(target->GetData() == rt);

  // Step 2. Construct the path from target to 'p'. It's already in node_list,
  //         and we simply read it in reverse order. Also find the index of
  //         of each child rule in the parent rule.
  //
  // There are some cases that are instant recursion, like:
  //   rule A : A + B
  // where there is only one position which should 0.

  for (unsigned i = node_list.GetNum() - 2; i >= 0; i--) {
    ContTreeNode<RuleTable*> *child_node = node_list.ValueAtIndex(i);
    RuleTable *child_rule = child_node->GetData();
    unsigned index = 0;
    bool succ = RuleFindChild(rt, child_rule, index);
    MASSERT(succ && "Cannot find child rule in parent rule.");
    path->AddPos(index);
  }

  if (node_list.GetNum() == 1) {
    unsigned index = 0;
    bool succ = RuleFindChild(rt, rt, index);
    MASSERT(succ && (index == 0) && "Cannot find child rule in parent rule.");
    path->AddPos(index);
  }

  // Step 3. Get the right Recursion, Add the path to the Recursioin.
  Recursion *rec = FindOrCreateRecursion(rt);
  rec->AddPath(path);
}

// Find the Recursion of 'rule'.
// If Not found, create one.
Recursion* RecDetector::FindOrCreateRecursion(RuleTable *rule) {
  for (unsigned i = 0; i < mRecursions.GetNum(); i++) {
    Recursion *rec = mRecursions.ValueAtIndex(i);
    if (rec->GetRuleTable() == rule)
      return rec;
  }

  Recursion *rec = (Recursion*)gMemPool.Alloc(sizeof(Recursion));
  new (rec) Recursion();
  rec->SetRuleTable(rule);
  mRecursions.PushBack(rec);

  return rec;
}

// 1. There is one and only one chance to traverse a rule table. Once it's done
//    it will never be traversed again.
void RecDetector::DetectRuleTable(RuleTable *rt, ContTreeNode<RuleTable*> *p) {
  if (IsDone(rt))
    return;

  // If find a new circle.
  if (IsInProcess(rt)) {
    AddRecursion(rt, p);
  } else {
    mInProcess.PushBack(rt);
  }

  // Create new tree node.
  ContTreeNode<RuleTable*> *node = mTree.NewNode(rt, p);

  EntryType type = rt->mType;
  switch(type) {
  case ET_Oneof:
    DetectOneof(rt, node);
    break;
  case ET_Data:
  case ET_Zeroorone:
  case ET_Zeroormore:
    DetectZeroormore(rt, node);
    break;
  case ET_Concatenate:
    DetectConcatenate(rt, node);
    break;
  case ET_Null:
  default:
    break;
  }

  // It's done.
  MASSERT(!IsDone(rt));
  mDone.PushBack(rt);
}

// For Oneof rule, we try to detect for all its children if they are a rule table too.
void RecDetector::DetectOneof(RuleTable *rule_table, ContTreeNode<RuleTable*> *p) {
  for (unsigned i = 0; i < rule_table->mNum; i++) {
    TableData *data = rule_table->mData + i;
    if (data->mType == DT_Subtable) {
      RuleTable *child = data->mData.mEntry;
      DetectRuleTable(child, p);
    }
  }
}

// Data, Zeroormore and Zeroorone has the same way to handle.
void RecDetector::DetectZeroormore(RuleTable *rule_table, ContTreeNode<RuleTable*> *p) {
  MASSERT((rule_table->mNum == 1) && "zeroormore node has more than one elements?");
  TableData *data = rule_table->mData;
  if (data->mType == DT_Subtable) {
    RuleTable *child = data->mData.mEntry;
    DetectRuleTable(child, p);
  }
}

// For Concatenate node, only the first child is checked. If it's a token, just end.
// If it's a rule table, go deeper.
void RecDetector::DetectConcatenate(RuleTable *rule_table, ContTreeNode<RuleTable*> *p) {
  TableData *data = rule_table->mData;
  if (data->mType == DT_Subtable) {
    RuleTable *child = data->mData.mEntry;
    DetectRuleTable(child, p);
  }
}

// We start from the top tables.
// Tables not accssible from top tables won't be handled.
void RecDetector::Detect() {
  for (unsigned i = 0; i < mTopTables.GetNum(); i++) {
    mInProcess.Clear();
    mDone.Clear();
    mTree.Clear();

    RuleTable *top = mTopTables.ValueAtIndex(i);
    DetectRuleTable(top, NULL);
  }
}

// The reason I have a Release() is to make sure the destructors of Recursion and Paths
// are invoked ahead of destructor of gMemPool.
void RecDetector::Release() {
  for (unsigned i = 0; i < mRecursions.GetNum(); i++) {
    Recursion *rec = mRecursions.ValueAtIndex(i);
    rec->Release();
  }

  mRecursions.Release();
  mTopTables.Release();
  mInProcess.Release();
  mDone.Release();
  mTree.Release();
}

int main(int argc, char *argv[]) {
  gMemPool.SetBlockSize(4096);
  RecDetector dtc;
  dtc.Detect();
  dtc.Release();
  return 0;
}
