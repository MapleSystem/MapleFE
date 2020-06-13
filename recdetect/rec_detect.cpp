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

// A rule could have multiple appearances in a rec, but it's OK.
// We just need add one instance of it.
// LeadNode and its recursion also are added as a mapping.
void RecDetector::AddRule2Recursion(RuleTable *rule, Recursion *rec) {
  Rule2Recursion *map = NULL;
  bool found = false;

  // If the rule table already mapping to some recursion?
  for (unsigned i = 0; i < mRule2Recursions.GetNum(); i++) {
    Rule2Recursion *r2r = mRule2Recursions.ValueAtIndex(i);
    if (rule == r2r->mRule) {
      map = r2r;
      break;
    }
  }

  if (!map) {
    map = (Rule2Recursion*)gMemPool.Alloc(sizeof(Rule2Recursion));
    new (map) Rule2Recursion();
    map->mRule = rule;
    mRule2Recursions.PushBack(map);
  }

  // if the mapping already exists?
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
// and only node representing 'rt', since the second appearance will be treated
// as a back edge. 
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
    bool succ = RuleFindChild(parent_rule, child_rule, index);
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
  }
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
  if (IsDone(rt))
    return TRS_Done;

  // If find a new Recursion, we are done. Don't need go deeper into
  // children nodes because they will be handled in the first traversal.
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

  MASSERT(IsInProcess(rt));
  RuleTable *back = mInProcess.Back();
  MASSERT(back == rt);
  mInProcess.PopBack();

  // It's done.
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
// It's obviously true the sum of the three sets is the total of rule tables.
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
// Question is, what if the first element is not a token? See Rule 4.
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
//   TRS_NonZero:   The rule is not empty. It may or may not help to form a recurion.
//   TRS_Done:      This one is extra return result which doesn't have corresponding
//                  rule catogeries of it. But the rule can be figured out by
//                  checking IsFail(), IsMaybeZero() and IsNonZero(), because this
//                  rule is done before.

// Keep in mind, all TResult of leading elements are only used for determining
// if the following element should be traversed, or just stop and put into ToDo list.

// At any point when we want to return, we need take the remaining elements
// to ToDo list, if they are not in any of InProcess, Done and ToDo.

// The return result tells how good this rule does in loop detection, and we define
// the algorithm of computing parent node based on children as below.
//   a. If the first child is TRS_Fail, the parent is TRS_Fail. The rest children
//      put into ToDo.
//   b. If all children are TRS_MaybeZero, the parent is MaybeZero.
//   c. If more than one leading children are TRS_MaybeZero, followed by a TRS_Fail child,
//      the parent is TRS_NonZero. It's possible the leading children can form
//      a circle, so we cannot say the parent is fail. It should be NonZero.
//   d. If more than one leading children are TRS_MaybeZero, followed by a TRS_Done child,
//      we'll further check that child is MaybeZero. We keep check the children until
//      a child is TRS_Fail, or a child is TRS_Done and NOT MaybeZero.
//      a circle.
//   e. No matter how many leading children of TRS_NonZero or MaybeZero, if the current
//      child is Nonzero, we just keep going. And the parent is NonZero.
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
      temp_res = TRS_Fail;
    }
  
    MASSERT((res == TRS_MaybeZero) || (res == TRS_NonZero));

    if (temp_res == TRS_MaybeZero) {
      // The leading nodes are all MaybeZero at this point.
      // Just let the traversal go on with the rest children rules.
      if (res == TRS_MaybeZero)
        res = TRS_MaybeZero;
    } else if (temp_res == TRS_NonZero) {
      // No matter how many leading children of NonZero or MaybeZero
      // Just let the traversal go on with the rest children rules.
      res = TRS_NonZero;
    } else if (temp_res == TRS_Fail) {
      if (i == 0) {
        res = TRS_Fail;
        AddToDo(rule_table, 1);
        return res;
      } else  {
        res = TRS_NonZero;
        AddToDo(rule_table, 1);
        return res;
      }
    } else if (temp_res == TRS_Done) {
      // further checking the real status of rule.
      MASSERT(child);
      if (IsFail(child)) {
        if (i == 0) {
          res = TRS_Fail;
          AddToDo(rule_table, 1);
          return res;
        } else  {
          res = TRS_NonZero;
          AddToDo(rule_table, 1);
          return res;
        }
      } else if (IsMaybeZero(child)) {
        if (res == TRS_MaybeZero)
          res = TRS_MaybeZero;
      } else if (IsNonZero(child)) {
        res = TRS_NonZero;
      }
    }
  }

  return res;
}

// We start from the top tables.
// Iterate untile mToDo is empty.
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

  mRule2Recursions.Release();
  mRecursions.Release();

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
    const char *tablename = GetRuleTableName(rec->GetRuleTable());

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
    const char *tablename = GetRuleTableName(rec->GetRuleTable());
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

  // step 4. Write Rule2Recursion mapping.
  WriteRule2Recursion();
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
      const char *n = GetRuleTableName(lr->GetRuleTable());
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
  dtc.Write();
  dtc.Release();
  return 0;
}
