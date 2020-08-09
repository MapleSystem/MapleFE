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
//
// [NOTE] The key point in recursion detector is to make sure for each loop, there
//        should be one and only one recursion counted, even if there are multiple
//        nodes in the loop.
//        To achieve this, we build the tree, and those back edges of recursion won't
//        be counted as tree edge. So in DFS traversal, children are done before parents.
//        If a child is involved in a loop, only the topmost ancestor will be the
//        leader of this loop.
//
//        This DFS traversal also assures once a node is traversed, all the loops
//        containing it will be found at this single time. We don't need traverse
//        this node any more. -- This is the base of our algorithm.
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

std::string RecPath::DumpToString() {
  std::string s;
  for (unsigned i = 0; i < mPositions.GetNum(); i++) {
    std::string num = std::to_string(mPositions.ValueAtIndex(i));
    s += num;
    if (i < mPositions.GetNum() - 1)
      s += ",";
  }
  return s;
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
  mRuleTables.Release();
}

////////////////////////////////////////////////////////////////////////////////////
//                                Rule2Recursion
////////////////////////////////////////////////////////////////////////////////////

// A rule table could have multiple instance in a recursion if the recursion has
// multiple circle and the rule appears in multiple circles.
void Rule2Recursion::AddRecursion(Recursion *rec) {
  for (unsigned i = 0; i < mRecursions.GetNum(); i++) {
    if (rec == mRecursions.ValueAtIndex(i))
      return;
  }
  mRecursions.PushBack(rec);
}

////////////////////////////////////////////////////////////////////////////////////
//                                RecDetector
////////////////////////////////////////////////////////////////////////////////////

void RecDetector::SetupTopTables() {
  mToDo.PushBack(&TblStatement);
  mToDo.PushBack(&TblClassDeclaration);
  mToDo.PushBack(&TblInterfaceDeclaration);
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

// A talbe is in ToDo
bool RecDetector::IsToDo(RuleTable *t) {
  for (unsigned i = 0; i < mToDo.GetNum(); i++) {
    if (t == mToDo.ValueAtIndex(i))
      return true;
  }
  return false;
}

// Add a rule to the ToDo list, if it's not in any of the following list,
// mInProcess, mDone, mToDo.
void RecDetector::AddToDo(RuleTable *rule) {
  if (!IsInProcess(rule) && !IsDone(rule) && !IsToDo(rule))
    mToDo.PushBack(rule);
}

// Add all child rules into ToDo, starting from index 'start'.
void RecDetector::AddToDo(RuleTable *rule_table, unsigned start) {
  for (unsigned i = start; i < rule_table->mNum; i++) {
    TableData *data = rule_table->mData + i;
    if (data->mType == DT_Subtable) {
      RuleTable *child = data->mData.mEntry;
      AddToDo(child);
    }
  }
}

bool RecDetector::IsNonZero(RuleTable *t) {
  for (unsigned i = 0; i < mNonZero.GetNum(); i++) {
    if (t == mNonZero.ValueAtIndex(i))
      return true;
  }
  return false;
}

bool RecDetector::IsMaybeZero(RuleTable *t) {
  for (unsigned i = 0; i < mMaybeZero.GetNum(); i++) {
    if (t == mMaybeZero.ValueAtIndex(i))
      return true;
  }
  return false;
}

bool RecDetector::IsFail(RuleTable *t) {
  for (unsigned i = 0; i < mFail.GetNum(); i++) {
    if (t == mFail.ValueAtIndex(i))
      return true;
  }
  return false;
}

Rule2Recursion* RecDetector::FindRule2Recursion(RuleTable *rule) {
  Rule2Recursion *map = NULL;
  for (unsigned i = 0; i < mRule2Recursions.GetNum(); i++) {
    Rule2Recursion *r2r = mRule2Recursions.ValueAtIndex(i);
    if (rule == r2r->mRule) {
      map = r2r;
      break;
    }
  }
  return map;
}

// Add 'rec' to the Rule2Recursion of 'rule' if not existing.
void RecDetector::AddRule2Recursion(RuleTable *rule, Recursion *rec) {
  Rule2Recursion *map = FindRule2Recursion(rule);
  if (!map) {
    map = (Rule2Recursion*)gMemPool.Alloc(sizeof(Rule2Recursion));
    new (map) Rule2Recursion();
    map->mRule = rule;
    mRule2Recursions.PushBack(map);
  }

  // if the mapping already exists?
  bool found = false;
  for (unsigned i = 0; i < map->mRecursions.GetNum(); i++) {
    Recursion *r = map->mRecursions.ValueAtIndex(i);
    if (rec == r) {
      found = true;
      break;
    }
  }

  if (!found) {
    map->mRecursions.PushBack(rec);
  }
}

// There is a back edge from 'p' to the first appearance of 'rt'. Actually in our
// traversal tree, in the current path (from 'p' upward to the root) there is one
// and only node representing 'rt', which is the root. The successor of the back
// edge is ignored.
void RecDetector::AddRecursion(RuleTable *rt, ContTreeNode<RuleTable*> *p) {
  //std::cout << "Recursion in " << GetRuleTableName(rt) << std::endl;

  RecPath *path = (RecPath*)gMemPool.Alloc(sizeof(RecPath));
  new (path) RecPath();

  // Step 1. Traverse upwards to find the target tree node, and add the node
  //         to the node_list. But the first to add is the back edge.

  ContTreeNode<RuleTable*> *target = NULL;
  ContTreeNode<RuleTable*> *node = p;
  SmallVector<RuleTable*> node_list;

  node_list.PushBack(rt);

  while (node) {
    if (node->GetData() == rt) {
      MASSERT(!target && "duplicated target node in a path.");
      target = node;
      break;
    } else {
      node_list.PushBack(node->GetData());
    }
    node = node->GetParent();
  }

  MASSERT(node_list.GetNum() > 0);
  MASSERT(target);
  MASSERT(target->GetData() == rt);

  // Step 2. Construct the path from target to 'p'. It's already in node_list,
  //         and we simply read it in reverse order. Also find the index of
  //         of each child rule in the parent rule.

  RuleTable *parent_rule = rt;
  for (int i = node_list.GetNum() - 1; i >= 0; i--) {
    RuleTable *child_rule = node_list.ValueAtIndex(i);
    unsigned index = 0;
    bool succ = RuleFindChildIndex(parent_rule, child_rule, index);
    MASSERT(succ && "Cannot find child rule in parent rule.");
    path->AddPos(index);
    parent_rule = child_rule;

    //std::cout << " child: " << GetRuleTableName(child_rule) << "@" << index << std::endl;
  }

  // Step 3. Get the right Recursion, Add the path to the Recursioin.
  Recursion *rec = FindOrCreateRecursion(rt);
  rec->AddPath(path);

  // Step 4. Add the Rule2Recursion mapping
  for (int i = node_list.GetNum() - 1; i >= 0; i--) {
    RuleTable *rule = node_list.ValueAtIndex(i);
    AddRule2Recursion(rule, rec);
    rec->AddRuleTable(rule);
  }
}

// Find the Recursion of 'rule'.
// If Not found, create one.
Recursion* RecDetector::FindOrCreateRecursion(RuleTable *rule) {
  for (unsigned i = 0; i < mRecursions.GetNum(); i++) {
    Recursion *rec = mRecursions.ValueAtIndex(i);
    if (rec->GetLead() == rule)
      return rec;
  }

  Recursion *rec = (Recursion*)gMemPool.Alloc(sizeof(Recursion));
  new (rec) Recursion();
  rec->SetLead(rule);
  mRecursions.PushBack(rec);

  return rec;
}

void RecDetector::HandleIsDoneRuleTable(RuleTable *rt, ContTreeNode<RuleTable*> *p) {
#if 0
  Rule2Recursion *map = FindRule2Recursion(rt);
  if (map) {
    for (unsigned i = 0; i < map->mRecursions.GetNum(); i++) {
      Recursion *rec = map->mRecursions.ValueAtIndex(i);
      for (unsigned j = 0; j < rec->mPaths.GetNum(); j++) {
        RecPath *path = rec->GetPath(j);
        SmallVector<RuleTable*> whole_list;  //
        SmallVector<RuleTable*> remain_list; // from 'rt' to the one before the
                                             // lead, which is the succ of back edge.

        // step 1. Check if this path contains 'rt'.
        RuleTable *curr_rt = rec->GetLead();
        bool found = false;
        for (unsigned k = 0; k < path->GetPositionsNum(); k++) {
          whole_list.PushBack(curr_rt);
          if (found)
            remain_list.PushBack(curr_rt);

          unsigned pos = path->GetPosition(k);
          RuleTable *child_rt = RuleFindChild(curr_rt, pos);
          if (child_rt == rt)
            found = true;

          curr_rt = child_rt;
        }

        if (!found)
          continue;

        // step 2. Check if this path can reach 'p'.
        for (unsigned k = 0; k < whole_list.GetNum(); k++) {
          RuleTable *
        }

        // step 3. Create new path from lead to 'p', then 'rt', then rest of path
      }
    }
  }
#endif
}

TResult RecDetector::DetectRuleTable(RuleTable *rt, ContTreeNode<RuleTable*> *p) {
  // The children is done with traversal before. This means we need
  // stop here too, because all the further traversal from this point
  // have been done already. People may ask, what if I miss a recursion
  // from this rule table? The answer is you will never miss a single
  // one. Because any recursion including this rule will be found
  // the first time it's traversed. (We also addressed this at the
  // beginning of this file).
  // This guarantees there is one and only one recursion recorded for a loop
  // even if the loop has multiple nodes.

  // However, there is one complication scenario that we need take care. Below
  // is an example.
  // rule UnannClassType: ONEOF(Identifier + ZEROORONE(TypeArguments),
  //                            UnannClassOrInterfaceType + '.' + ...)
  // rule UnannClassOrInterfaceType: ONEOF(UnannClassType, UnannInterfaceType)
  // rule UnannInterfaceType : UnannClassType
  //
  // These rules form a recursion where there are 2 paths. During recdetect traversal
  // UnannClassOrInterfaceType is the first to hit and becomes the LeadNode.
  // The problem is when handling the edge UnannClassOrInterfaceType --> UnannClassType,
  // we find the first path, and UnannClassType becomes IsDone. Now, we need deal
  // with UnannClassOrInterfaceType --> UnannInterfaceType, but UnannClassType which
  // is the child of UnannInterfaceType is IsDone.
  //
  // In this case, we cannot return directly. We will have to check if the finished child
  // is in a recursion. If it's in a recursion, we need further check if there is a path
  // which can reach the focal node 'p'.

  if (IsDone(rt)) {
    //HandleIsDoneRuleTable(rt, p);
    return TRS_Done;
  }

  // If find a new Recursion, create the recursion. It is the successor of thei
  // back edge. As mentioned in the comments in the beginning of this file, back
  // edge won't be furhter traversed as it is NOT part of the traversal tree.
  if (IsInProcess(rt)) {
    AddRecursion(rt, p);
    return;
  } else {
    mInProcess.PushBack(rt);
  }

  // Create new tree node.
  ContTreeNode<RuleTable*> *node = mTree.NewNode(rt, p);
  EntryType type = rt->mType;
  TResult res = TRS_MaybeZero;
  switch(type) {
  case ET_Oneof:
    res = DetectOneof(rt, node);
    break;
  case ET_Data:
    res = DetectData(rt, node);
    break;
  case ET_Zeroorone:
  case ET_Zeroormore:
    res = DetectZeroorXXX(rt, node);
    break;
  case ET_Concatenate:
    res = DetectConcatenate(rt, node);
    break;
  case ET_Null:
  default:
    MASSERT(0 && "Unexpected EntryType of rule.");
    break;
  }

  // Remove it from InProcess.
  MASSERT(IsInProcess(rt));
  RuleTable *back = mInProcess.Back();
  MASSERT(back == rt);
  mInProcess.PopBack();

  // Add it to IsDone
  // It's clear that a node is push&pop in/off the stack just once, and then
  // it's set as IsDone. It's traversed only once.
  MASSERT(!IsDone(rt));
  mDone.PushBack(rt);

  if (res == TRS_Fail) {
    MASSERT(!IsFail(rt));
    mFail.PushBack(rt);
  } else if (res == TRS_MaybeZero) {
    MASSERT(!IsMaybeZero(rt));
    mMaybeZero.PushBack(rt);
  } else if (res == TRS_NonZero) {
    MASSERT(!IsNonZero(rt));
    mNonZero.PushBack(rt);
  } else {
    // It couldn't be TRS_Done or TRS_NA, since we already
    // handled TRS_Done in those DetectXXX().
    MASSERT(0 && "Unexpected return result.");
  }

  return res;
}

// For Oneof rule, we try to detect for all its children if they are a rule table too.
// 1. If there is one or more children are MaybeZero, the parent is MaybeZero
// 2. If and only if all chidlren are Fail, the parent is Fail.
// 3. All other cases, the parent is NonZero.
//
// No children will be put in ToDo, since all of them should be traversed.
TResult RecDetector::DetectOneof(RuleTable *rule_table, ContTreeNode<RuleTable*> *p) {
  TResult result = TRS_NA;
  for (unsigned i = 0; i < rule_table->mNum; i++) {
    TableData *data = rule_table->mData + i;
    TResult temp_res = TRS_MaybeZero;
    RuleTable *child = NULL;
    if (data->mType == DT_Subtable) {
      child = data->mData.mEntry;
      temp_res = DetectRuleTable(child, p);
    } else {
      temp_res = TRS_Fail;
    }

    if (temp_res == TRS_MaybeZero) {
      // MaybeZero is the strongest claim.
      result = TRS_MaybeZero;
    } else if (temp_res == TRS_NonZero) {
      // Nonzero is just weaker than MaybeZero
      if (result != TRS_MaybeZero)
        result = TRS_NonZero;
    } else if (temp_res == TRS_Fail) {
      // Fail is the weakest,
      if (result == TRS_NA)
        result = TRS_Fail;
    } else if (temp_res == TRS_Done) {
      if (IsMaybeZero(child)) {
        result = TRS_MaybeZero;
      } else if (IsNonZero(child)) {
        if (result != TRS_MaybeZero)
          result = TRS_NonZero;
      } else if (IsFail(child)) {
        if (result == TRS_NA)
          result = TRS_Fail;
      } else
        MASSERT(0 && "Unexpected result of done child.");
    } else
      MASSERT(0 && "Unexpected temp_res of child");
  }

  return result;
}

// Zeroormore and Zeroorone has the same way to handle.
TResult RecDetector::DetectZeroorXXX(RuleTable *rule_table, ContTreeNode<RuleTable*> *p) {
  TResult result = TRS_NA;
  MASSERT((rule_table->mNum == 1) && "zeroormore node has more than one elements?");
  TableData *data = rule_table->mData;
  if (data->mType == DT_Subtable) {
    RuleTable *child = data->mData.mEntry;
    result = DetectRuleTable(child, p);
  } else {
    // For the non-table data, we just do nothing
  }

  return TRS_MaybeZero;
}

TResult RecDetector::DetectData(RuleTable *rule_table, ContTreeNode<RuleTable*> *p) {
  TResult result = TRS_NA;
  RuleTable *child = NULL;
  MASSERT((rule_table->mNum == 1) && "Data node has more than one elements?");
  TableData *data = rule_table->mData;
  if (data->mType == DT_Subtable) {
    child = data->mData.mEntry;
    result = DetectRuleTable(child, p);
  } else {
    result = TRS_Fail;
  }

  if (result == TRS_Done) {
    MASSERT(child);
    if (IsMaybeZero(child)) {
      MASSERT(!IsNonZero(child) && !IsFail(child));
      result = TRS_MaybeZero;
    } else if (IsNonZero(child)) {
      MASSERT(!IsMaybeZero(child) && !IsFail(child));
      result = TRS_NonZero;
    } else if (IsFail(child)) {
      MASSERT(!IsMaybeZero(child) && !IsNonZero(child));
      result = TRS_Fail;
    } else {
      MASSERT(0 && "Child is not in any categories.");
    }
  }

  return result;
}

// The real challenge in left recursion detection is coming from Concatenate node. Before
// we explain the details, we introduce three sets to keep rule table status.
//   ToDo     : The main driver will walk on this list. Any rules which is encountered
//              in the traversal but we won't keep on traversing on it, will be put in
//              this todo list for later traversal.
//   InProcess: The rules are right now in the middle of traversal. This actually tells
//              the current working path of the DFS traversal.
//   Done     : The rules that are finished.
// The sum of the three sets is the total of rule tables.
// And there is no overlapping among the three sets.
//
// We also elaborate a few rules which build the foundation of the algorithm.
//
// [Rule 1]
//
//  E ---> '{' + E + '}',
//     |-> other rules
//
// It's obvious that E on RHS won't be considered as recursion child, and we can stop
// at this point. The and parent rule will give up on it.
//
// [Rule 2]
//
//  A ---> '{' + E + '}',
//     |-> other rules
//
// E on the RHS is not the first element. It won't be traversed at this DFS path.
// The traversal stops at this point, and parent node will give up on this path.
// But later we will work on it to see if there is recursion. So, E will be put into
// ToDo list if it's not in Done or InProcess.
//
// Question is, what if the first element is not a token?
//
// [Rule 3]
//
//  A ---> E + '}',
//     |-> other rules
//
// E is the first one. It always will go through the following traversal.
//
// [Rule 4]
//
// E ---> F + G,
//     |-> other rules
// F ---> ZEROORMORE()
// G ---> E + ...
//
// In this case F is an explicit ZeroorXXX() rule. E and G forms a left recursion.
// Let's look at an even more complicated case.
//
// E ---> F + G,
//     |-> other rules
// F ---> H
// H ---> ZEROORMORE()
// G ---> E + ...
//
// In this case F is an IMPLICIT ZeroorXXX() rule table. E and G form a left
// recursion too. We cannot put G in ToDo list and give up the path.
// Go further there are more complicated case like
//
// E ---> F + G,
//     |-> other rules
// F ---> H
// H ---> ZEROORMORE() + K
// K ---> ZERORORMORE()
// G ---> E + ...
//
// H is a ZeroorXXX() or not depends on all grand children's result.
//
// We introduce a set of TResult to indicate the result of a rule table. This is
// corresponding to the three catagories of rule.
//   TRS_Fail:      Definitely a fail for left recursion. Just stop here.
//   TRS_MaybeZero: The rule table could be empty. For a rule table, at the first
//                  time it's traversed, we will save it to mMaybeZero if it's.
//   TRS_NonZero:   The rule is not empty. The detection for parent stop here. But
//                  the rest children could form some new recursions.
//   TRS_Done:      This one is extra return result which doesn't have corresponding
//                  rule catogeries of it. But the rule can be figured out by
//                  checking IsFail(), IsMaybeZero() and IsNonZero(), because this
//                  rule is done before.

// Keep in mind, all TResult of leading elements are only used for determining
// if the following element should be traversed, or just stop and put into ToDo list.

// At any point when we want to return, we need take the remaining elements
// to ToDo list, if they are not in any of InProcess, Done and ToDo.

// The return result tells how a rule does in loop detection, and we define
// the algorithm of computing parent node based on children as below.
//   a. The first time we get a NonZero or Fail child, the parent stops,
//      and puts rest of children to ToDo list. So this means all the previous
//      children are MaybeZero.
//   b. If TRS_Fail child is the first one, the parent is TRS_Fail. Otherwise, 
//      the parent is NonZero.
//   c. If we get Nonzero, we Stop going furhter, and put the rest children to ToDo list.
//      And the parent is NonZero.
//   d. If all children are TRS_MaybeZero, the parent is MaybeZero.
//   e. If get a TRS_Done child, check if its NonZero, Fail or MaybeZero, and handle it
//      accordingly.
//      
TResult RecDetector::DetectConcatenate(RuleTable *rule_table, ContTreeNode<RuleTable*> *p) {
  // We use 'res' to record the status of the leading children. TRS_MaybeZero is good
  // for the beginning as it means it's empty right now.
  TResult res = TRS_MaybeZero;

  TableData *data = rule_table->mData;
  RuleTable *child = NULL;

  // We accumulate and calculate the status from the beginning of the first child, and
  // make decision of the rest children accordingly.
  for (unsigned i = 0; i < rule_table->mNum; i++) {
    TableData *data = rule_table->mData + i;
    TResult temp_res = TRS_MaybeZero;
    child = NULL;
    if (data->mType == DT_Subtable) {
      child = data->mData.mEntry;
      temp_res = DetectRuleTable(child, p);
    } else {
      // Whenever we got a non-table child, eg. Token, the following children
      // won't be counted, and we return TRS_Fail at the end.
      temp_res = TRS_Fail;
    }
  
    MASSERT((res == TRS_MaybeZero));

    if (temp_res == TRS_NonZero) {
      // No matter how many leading children of MaybeZero, the first time
      // we get a NonZero, the whole result is NonZero. The rest children
      // cannot be traversed for these existing ancestors since they cannot
      // help form a LEFT recursion any more due to this NonZero child.
      // However, we do need add them into ToDo list.
      res = TRS_NonZero;
      AddToDo(rule_table, i+1);
      return res;
    } else if (temp_res == TRS_Fail) {
      res = (i == 0) ? TRS_Fail : TRS_NonZero;
      AddToDo(rule_table, i+1);
      return res;
    } else if (temp_res == TRS_Done) {
      MASSERT(child);
      if (IsFail(child)) {
        res = (i == 0) ? TRS_Fail : TRS_NonZero;
        AddToDo(rule_table, i+1);
        return res;
      } else if (IsNonZero(child)) {
        res = TRS_NonZero;
        AddToDo(rule_table, i+1);
        return res;
      }
    }
  }

  MASSERT(res == TRS_MaybeZero);
  return res;
}

// We start from the top tables.
// Iterate until mToDo is empty.
void RecDetector::Detect() {
  mDone.Clear();
  SetupTopTables();
  while(!mToDo.Empty()) {
    mInProcess.Clear();
    mTree.Clear();
    RuleTable *front = mToDo.Front();
    mToDo.PopFront();
    // It's possible that a ToDo rule was later finished or in the middle of process
    // after it was put into the ToDo list.
    if (!IsDone(front) && !IsInProcess(front))
      DetectRuleTable(front, NULL);
  }
}

// If there is a path from 'from' to 'to', and it's LeftRecursive reachable which
// means 'from' to 'to' could be a left recursion valid path.
//
// Left Recursive Reachable is the fundamental idea of building recursion group.
// We look for recursion group and parse on the groups like waves moving forward.
// A non-left-recursive reachable group breaks this moving since it goes the different
// tokens.

bool RecDetector::LRReachable(RuleTable *from, RuleTable *to) {
  bool found = false;
  SmallList<RuleTable*> working_list;
  SmallVector<RuleTable*> done_list;

  working_list.PushBack(from);
  while (!working_list.Empty()) {
    RuleTable *rt = working_list.Front();
    working_list.PopFront();

    if (rt == to) {
      found = true;
      break;
    }

    bool is_done = false;
    for (unsigned i = 0; i < done_list.GetNum(); i++) {
      if (rt == done_list.ValueAtIndex(i)) {
        is_done = true;
        break;
      }
    }
    if (is_done)
      continue;

    EntryType type = rt->mType;
    switch(type) {
    case ET_Oneof: {
      // Add all table children to working list.
      for (unsigned i = 0; i < rt->mNum; i++) {
        TableData *data = rt->mData + i;
        if (data->mType == DT_Subtable) {
          RuleTable *child = data->mData.mEntry;
          working_list.PushBack(child);
        }
      }
      break;
    }
    case ET_Zeroorone:
    case ET_Zeroormore:
    case ET_Data: {
      MASSERT(rt->mNum == 1);
      TableData *data = rt->mData;
      if (data->mType == DT_Subtable) {
        RuleTable *child = data->mData.mEntry;
        working_list.PushBack(child);
      }
      break;
    }
    case ET_Concatenate: {
      for (unsigned i = 0; i < rt->mNum; i++) {
        TableData *data = rt->mData + i;
        // If the child is not a rule table, all the rest children
        // including this one are not left recursive legitimate.
        if (data->mType != DT_Subtable)
          break;

        RuleTable *child = data->mData.mEntry;
        working_list.PushBack(child);

        // If 'child' is not MaybeZero, the rest children are
        // not left recursive legitimate.
        if (!IsMaybeZero(child))
          break;
      }
      break;
    }
    case ET_Null:
    default:
      MASSERT(0 && "Unexpected EntryType of rule.");
      break;
    }

    done_list.PushBack(rt);
  }

  return found;
}

// Return the RecursionGroup if there is one containing 'rec'.
RecursionGroup* RecDetector::FindRecursionGroup(Recursion *rec) {
  RecursionGroup *group = NULL;
  for (unsigned i = 0; i < mRecursionGroups.GetNum(); i++) {
    RecursionGroup *rg = mRecursionGroups.ValueAtIndex(i);
    for (unsigned j = 0; j < rg->mRecursions.GetNum(); j++) {
      Recursion *r = rg->mRecursions.ValueAtIndex(j);
      if (rec == r) {
        group = rg;
        break;
      }
    }
    if (group)
      break;
  }
  return group;
}

// We are iterate the recursions in order. It has two level of iterations.
// In this algorithm,
// 1. The recursion which doesn't belong to any existing group is always
//    the one to create its group.
// 2. All recursions of a group can be identified in just one single
//    inner iteration.
void RecDetector::DetectGroups() {
  for (unsigned i = 0; i < mRecursions.GetNum(); i++) {
    Recursion *rec_i = mRecursions.ValueAtIndex(i);
    RecursionGroup *group_i = FindRecursionGroup(rec_i);
    // It's already done.
    if (group_i)
      continue;

    group_i = new RecursionGroup();
    group_i->AddRecursion(rec_i);
    mRecursionGroups.PushBack(group_i);

    for (unsigned j = i + 1; j < mRecursions.GetNum(); j++) {
      Recursion *rec_j = mRecursions.ValueAtIndex(j);
      if (LRReachable(rec_i->GetLead(), rec_j->GetLead()) &&
          LRReachable(rec_j->GetLead(), rec_i->GetLead())) {
        RecursionGroup *group_j = FindRecursionGroup(rec_j);
        MASSERT(!group_j);
        group_i->AddRecursion(rec_j);
      }
    }
  }

  // Build the rule2group
  for (unsigned i = 0; i < mRecursionGroups.GetNum(); i++) {
    RecursionGroup *group = mRecursionGroups.ValueAtIndex(i);
    for (unsigned j = 0; j < group->mRecursions.GetNum(); j++) {
      Recursion *rec = group->mRecursions.ValueAtIndex(j);
      for (unsigned k = 0; k < rec->mRuleTables.GetNum(); k++) {
        RuleTable *rt = rec->mRuleTables.ValueAtIndex(k);
        // Duplication is checked in AddRule2Group().
        AddRule2Group(rt, group);
      }
    }
  }
}

void RecDetector::AddRule2Group(RuleTable *rt, RecursionGroup *group) {
  bool found = false;
  for (unsigned i = 0; i < mRule2Group.GetNum(); i++) {
    Rule2Group r2g = mRule2Group.ValueAtIndex(i);
    if ((r2g.mRuleTable == rt) && (r2g.mGroup == group)) {
      found = true;
      break;
    }
  }

  if (found)
    return;

  Rule2Group r2g = {rt, group};
  mRule2Group.PushBack(r2g);
}

// The reason I have a Release() is to make sure the destructors of Recursion and Paths
// are invoked ahead of destructor of gMemPool.
void RecDetector::Release() {
  for (unsigned i = 0; i < mRecursions.GetNum(); i++) {
    Recursion *rec = mRecursions.ValueAtIndex(i);
    rec->Release();
  }

  for (unsigned i = 0; i < mRule2Recursions.GetNum(); i++) {
    Rule2Recursion *map = mRule2Recursions.ValueAtIndex(i);
    map->Release();
  }

  for (unsigned i = 0; i < mRecursionGroups.GetNum(); i++) {
    RecursionGroup *rg = mRecursionGroups.ValueAtIndex(i);
    rg->Release();
  }

  mRule2Recursions.Release();
  mRecursions.Release();
  mRecursionGroups.Release();

  mToDo.Release();
  mInProcess.Release();
  mDone.Release();

  mMaybeZero.Release();
  mNonZero.Release();
  mFail.Release();

  mTree.Release();
}

// The header file would be java/include/gen_recursion.h.
void RecDetector::WriteHeaderFile() {
  mHeaderFile->WriteOneLine("#ifndef __GEN_RECUR_H__", 23);
  mHeaderFile->WriteOneLine("#define __GEN_RECUR_H__", 23);
  mHeaderFile->WriteOneLine("#include \"recursion.h\"", 22);
  mHeaderFile->WriteOneLine("#endif", 6);
}

void RecDetector::WriteCppFile() {
  mCppFile->WriteOneLine("#include \"gen_recursion.h\"", 26);
  mCppFile->WriteOneLine("#include \"common_header_autogen.h\"", 34);

  //Step 1. Dump paths of a rule table's recursions.
  //  unsigned tablename_path_1[N]={1, 2, ...};
  //  unsigned tablename_path_2[M]={1, 2, ...};
  //  unsigned *tablename_path_list[2] = {tablename_path_1, tablename_path_2};
  //  LeftRecursion tablename_rec = {&Tbltablename, 2, tablename_path_list};
  for (unsigned i = 0; i < mRecursions.GetNum(); i++) {
    Recursion *rec = mRecursions.ValueAtIndex(i);
    const char *tablename = GetRuleTableName(rec->GetLead());

    // dump comment of tablename
    std::string comment("// ");
    comment += tablename;
    mCppFile->WriteOneLine(comment.c_str(), comment.size());

    // dump : unsigned tablename_path_1[N]={1, 2, ...};
    for (unsigned j = 0; j < rec->PathsNum(); j++) {
      RecPath *path = rec->GetPath(j);
      std::string path_str = "unsigned ";
      path_str += tablename;
      path_str += "_path_";
      std::string index_str = std::to_string(j);
      path_str += index_str;
      path_str += "[";
      // As mentioned in recursion.h, we put the #elem in the first element
      // tell the users of the array length.
      std::string num_str = std::to_string(path->PositionsNum()+1);
      path_str += num_str;
      path_str += "]= {";
      num_str = std::to_string(path->PositionsNum());
      path_str += num_str;
      path_str += ",";
      std::string path_dump = path->DumpToString();
      path_str += path_dump;
      path_str += "};";
      mCppFile->WriteOneLine(path_str.c_str(), path_str.size());
    }

    //  to dump
    //  unsigned *tablename_path_list[2] = {tablename_path_1, tablename_path_2};
    std::string path_list = "unsigned *";
    path_list += tablename;
    path_list += "_path_list[";
    std::string num_str = std::to_string(rec->PathsNum());
    path_list += num_str;
    path_list += "]={";
    for (unsigned j = 0; j < rec->PathsNum(); j++) {
      std::string path_str;
      path_str += tablename;
      path_str += "_path_";
      std::string index_str = std::to_string(j);
      path_str += index_str;
      if (j < rec->PathsNum() - 1)
        path_str += ",";
      path_list += path_str;
    }
    path_list += "};";
    mCppFile->WriteOneLine(path_list.c_str(), path_list.size());

    // dump
    //  LeftRecursion tablename_rec = {&Tbltablename, 2, tablename_path_list};
    std::string rec_str = "LeftRecursion ";
    rec_str += tablename;
    rec_str += "_rec = {&";
    rec_str += tablename;
    rec_str += ", ";
    rec_str += num_str,
    rec_str += ", ";
    rec_str += tablename;
    rec_str += "_path_list};";
    mCppFile->WriteOneLine(rec_str.c_str(), rec_str.size());
  }

  // Step 2. Dump num of Recursions

  mCppFile->WriteOneLine("// Total recursions", 19);

  std::string s = "unsigned gLeftRecursionsNum=";
  std::string num = std::to_string(mRecursions.GetNum());
  s += num;
  s += ';';
  mCppFile->WriteOneLine(s.c_str(), s.size());

  // Step 3. dump all the recursions array, as below
  //   Recusion* TotalRecursions[N] = {&tablename_rec, &tablename_rec, ...};
  //   Recursion **gLeftRecursions = TotalRecursions;
  std::string total_str = "LeftRecursion* TotalRecursions[";
  std::string num_rec = std::to_string(mRecursions.GetNum());
  total_str += num_rec;
  total_str += "] = {";
  for (unsigned i = 0; i < mRecursions.GetNum(); i++) {
    Recursion *rec = mRecursions.ValueAtIndex(i);
    const char *tablename = GetRuleTableName(rec->GetLead());
    total_str += "&";
    total_str += tablename;
    total_str += "_rec";
    if (i < mRecursions.GetNum() - 1)
      total_str += ", ";
  }
  total_str += "};";
  mCppFile->WriteOneLine(total_str.c_str(), total_str.size());
  std::string last_str = "LeftRecursion **gLeftRecursions = TotalRecursions;";
  mCppFile->WriteOneLine(last_str.c_str(), last_str.size());

  // Step 4. Write RecursionGroups
  last_str = "///////////////////////////////////////////////////////////////";
  mCppFile->WriteOneLine(last_str.c_str(), last_str.size());
  last_str = "//                       RecursionGroup                      //";
  mCppFile->WriteOneLine(last_str.c_str(), last_str.size());
  last_str = "///////////////////////////////////////////////////////////////";
  mCppFile->WriteOneLine(last_str.c_str(), last_str.size());

  WriteRecursionGroups();

  // step 5. Write Rule2Recursion mapping.

  last_str = "///////////////////////////////////////////////////////////////";
  mCppFile->WriteOneLine(last_str.c_str(), last_str.size());
  last_str = "//                       Ruel2Recursion                      //";
  mCppFile->WriteOneLine(last_str.c_str(), last_str.size());
  last_str = "///////////////////////////////////////////////////////////////";
  mCppFile->WriteOneLine(last_str.c_str(), last_str.size());

  WriteRule2Recursion();

  // step 6. Write Rule2Group mapping.

  last_str = "///////////////////////////////////////////////////////////////";
  mCppFile->WriteOneLine(last_str.c_str(), last_str.size());
  last_str = "//                       Ruel2Group                      //";
  mCppFile->WriteOneLine(last_str.c_str(), last_str.size());
  last_str = "///////////////////////////////////////////////////////////////";
  mCppFile->WriteOneLine(last_str.c_str(), last_str.size());

  WriteRule2Group();
}

// unsigned gRecursionGroupsNum = N;
// unsigned group_sizes[N] = {1,2,...};
// unsigned *gRecursionGroupSizes = group_sizes;
// LeftRecursion *RecursionGroup_1[1] = {&TblPrimary_rec, ...};
// LeftRecursion *RecursionGroup_2[1] = {&TblPrimary_rec, ...};
// LeftRecursion **recursion_groups[N] = {RecursionGroup_1, RecursionGroup_2...};
// LeftRecursion ***gRecursionGroups = recursion_groups;
void RecDetector::WriteRecursionGroups() {
  // groups num
  std::string groups_num = "unsigned gRecursionGroupsNum = ";
  std::string num_str = std::to_string(mRecursionGroups.GetNum());
  groups_num += num_str;
  groups_num += ";";
  mCppFile->WriteOneLine(groups_num.c_str(), groups_num.size());

  // group sizes
  std::string group_sizes = "unsigned group_sizes[";
  group_sizes += num_str;
  group_sizes += "] = {";
  for (unsigned i = 0; i < mRecursionGroups.GetNum(); i++) {
    RecursionGroup *rg = mRecursionGroups.ValueAtIndex(i);
    unsigned size = rg->mRecursions.GetNum();
    std::string size_str = std::to_string(size);
    group_sizes += size_str;
    if (i != (mRecursionGroups.GetNum() - 1))
      group_sizes += ",";
  }
  group_sizes += "};";
  mCppFile->WriteOneLine(group_sizes.c_str(), group_sizes.size());

  group_sizes = "unsigned *gRecursionGroupSizes = group_sizes;";
  mCppFile->WriteOneLine(group_sizes.c_str(), group_sizes.size());

  // LeftRecursion *RecursionGroup_1[1] = {&TblPrimary_rec, ...};
  // LeftRecursion *RecursionGroup_2[1] = {&TblPrimary_rec, ...};
  for (unsigned i = 0; i < mRecursionGroups.GetNum(); i++) {
    std::string group = "LeftRecursion *RecursionGroup_";
    std::string i_str = std::to_string(i);
    group += i_str;
    group += "[";
    RecursionGroup *rg = mRecursionGroups.ValueAtIndex(i);
    unsigned size = rg->mRecursions.GetNum();
    i_str = std::to_string(size);
    group += i_str;
    group += "] = {";
    for (unsigned j = 0; j < rg->mRecursions.GetNum(); j++) {
      Recursion *rec = rg->mRecursions.ValueAtIndex(j);
      RuleTable *rule_table = rec->GetLead();
      const char *name = GetRuleTableName(rule_table);
      group += "&";
      group += name;
      group += "_rec";
      if (j < (rg->mRecursions.GetNum() - 1))
        group += ",";
    }
    group += "};";
    mCppFile->WriteOneLine(group.c_str(), group.size());
  } 

  // LeftRecursion **recursion_groups[N] = {RecursionGroup_1, RecursionGroup_2...};
  // LeftRecursion ***gRecursionGroups = recursion_groups;
  std::string final_groups = "LeftRecursion **recursion_groups[";
  final_groups += num_str;
  final_groups += "] = {";
  for (unsigned i = 0; i < mRecursionGroups.GetNum(); i++) {
    final_groups += "RecursionGroup_";
    std::string i_str = std::to_string(i);
    final_groups += i_str;
    if (i < (mRecursionGroups.GetNum() - 1))
      final_groups += ",";
  }
  final_groups += "};";
  mCppFile->WriteOneLine(final_groups.c_str(), final_groups.size());

  final_groups = "LeftRecursion ***gRecursionGroups = recursion_groups;";
  mCppFile->WriteOneLine(final_groups.c_str(), final_groups.size());
}

// We want to dump like below.
//   unsigned gRule2GroupNum = X;
//   Rule2Group gRule2Group[X] = {{&TblPrimary, 1}, {...}, ...};

void RecDetector::WriteRule2Group() {
  std::string comment = "// Rule2Group mapping";
  mCppFile->WriteOneLine(comment.c_str(), comment.size());

  std::string groups_num = "unsigned gRule2GroupNum = ";
  std::string num_str = std::to_string(mRule2Group.GetNum());
  groups_num += num_str;
  groups_num += ";";
  mCppFile->WriteOneLine(groups_num.c_str(), groups_num.size());

  std::string groups = "Rule2Group rule2group[";
  groups += num_str;
  groups += "] = {";
  mCppFile->WriteOneLine(groups.c_str(), groups.size());

  for (unsigned i = 0; i < mRule2Group.GetNum(); i++) {
    Rule2Group r2g = mRule2Group.ValueAtIndex(i);
    RuleTable *rt = r2g.mRuleTable;
    RecursionGroup *group = r2g.mGroup;
    // finde the index of group.
    int index = -1;
    for (unsigned j = 0; j < mRecursionGroups.GetNum(); j++) {
      if (group == mRecursionGroups.ValueAtIndex(j)) {
        index = j;
        break;
      }
    }
    MASSERT(index >= 0);

    std::string map = "  {&";
    const char *tablename = GetRuleTableName(rt);
    map += tablename;
    map += ", ";
    std::string id_str = std::to_string(index);
    map += id_str;
    map += "}";
    if (i < mRule2Group.GetNum() - 1)
      map += ",";

    mCppFile->WriteOneLine(map.c_str(), map.size());
  }

  // The ending
  groups = "};";
  mCppFile->WriteOneLine(groups.c_str(), groups.size());

  groups = "Rule2Group *gRule2Group = rule2group;";
  mCppFile->WriteOneLine(groups.c_str(), groups.size());
}

// Dump rule2recursion mapping, as below
//   LeftRecursion *TblPrimary_r2r_data[2] = {&TblPrimary_rec, &TblBinary_rec};
//   Rule2Recursion TblPrimary_r2r = {&TblPrimary, 2, TblPrimary_r2r_data};
//     ....
//   Rule2Recursion *arrayRule2Recursion[yyy] = {&TblPrimary_r2r, &TblTypeOrPackNam_r2r,...}
//   gRule2RecursionNum = xxx;
//   Rule2Recursion **gRule2Recursion = arrayRule2Recursion;
void RecDetector::WriteRule2Recursion() {
  std::string comment = "// Rule2Recursion mapping";
  mCppFile->WriteOneLine(comment.c_str(), comment.size());

  // Step 1. Write each r2r data.
  for (unsigned i = 0; i < mRule2Recursions.GetNum(); i++) {
    // Dump:
    //   LeftRecursion *TblPrimary_r2r_data[2] = {&TblPrimary_rec, &TblBinary_rec};
    Rule2Recursion *r2r = mRule2Recursions.ValueAtIndex(i);
    const char *tablename = GetRuleTableName(r2r->mRule);
    std::string dump = "LeftRecursion *";
    dump += tablename;
    dump += "_r2r_data[";
    std::string num = std::to_string(r2r->mRecursions.GetNum());
    dump += num;
    dump += "] = {";
    // get the list of recursions.
    std::string lr_str;
    for (unsigned j = 0; j < r2r->mRecursions.GetNum(); j++) {
      Recursion *lr = r2r->mRecursions.ValueAtIndex(j);
      lr_str += "&";
      const char *n = GetRuleTableName(lr->GetLead());
      lr_str += n;
      lr_str += "_rec";
      if (j < (r2r->mRecursions.GetNum() - 1))
        lr_str += ",";
    }
    dump += lr_str;
    dump += "};";
    mCppFile->WriteOneLine(dump.c_str(), dump.size());

    // Dump:
    //   Rule2Recursion TblPrimary_r2r = {&TblPrimary, 2, TblPrimary_r2r_data};
    dump = "Rule2Recursion ";
    dump += tablename;
    dump += "_r2r = {&";
    dump += tablename;
    dump += ", ";
    dump += num;
    dump += ", ";
    dump += tablename;
    dump += "_r2r_data};";
    mCppFile->WriteOneLine(dump.c_str(), dump.size());
  }

  // Step 2. Write arryRule2Recursion as below
  //   Rule2Recursion *arrayRule2Recursion[yyy] = {&TblPrimary_r2r, &TblTypeOrPackNam_r2r,...}
  std::string dump = "Rule2Recursion *arrayRule2Recursion[";
  std::string num = std::to_string(mRule2Recursions.GetNum());
  dump += num;
  dump += "] = {";
  std::string r2r_list;
  for (unsigned i = 0; i < mRule2Recursions.GetNum(); i++) {
    //   &TblPrimary_rec, &TblBinary_rec, ..
    Rule2Recursion *r2r = mRule2Recursions.ValueAtIndex(i);
    const char *tablename = GetRuleTableName(r2r->mRule);
    r2r_list += "&";
    r2r_list += tablename;
    r2r_list += "_r2r";
    if (i < (mRule2Recursions.GetNum() - 1))
      r2r_list += ", ";
  }
  dump += r2r_list;
  dump += "};";
  mCppFile->WriteOneLine(dump.c_str(), dump.size());

  // Step 3. Write gRule2RecursionNum and gRule2Recursion
  //   gRule2RecursionNum = xxx;
  //   Rule2Recursion **gRule2Recursion = arrayRule2Recursion;
  dump = "unsigned gRule2RecursionNum = ";
  dump += num;
  dump += ";";
  mCppFile->WriteOneLine(dump.c_str(), dump.size());

  dump = "Rule2Recursion **gRule2Recursion = arrayRule2Recursion;";
  mCppFile->WriteOneLine(dump.c_str(), dump.size());
}

// Write the recursion to java/gen_recursion.h and java/gen_recursion.cpp
void RecDetector::Write() {
  std::string lang_path_header("../../java/include/");
  std::string lang_path_cpp("../../java/src/");

  std::string file_name = lang_path_cpp + "gen_recursion.cpp";
  mCppFile = new Write2File(file_name);
  file_name = lang_path_header + "gen_recursion.h";
  mHeaderFile = new Write2File(file_name);

  WriteHeaderFile();
  WriteCppFile();

  delete mCppFile;
  delete mHeaderFile;
}

int main(int argc, char *argv[]) {
  gMemPool.SetBlockSize(4096);
  RecDetector dtc;
  dtc.Detect();
  dtc.DetectGroups();
  dtc.Write();
  dtc.Release();
  return 0;
}
