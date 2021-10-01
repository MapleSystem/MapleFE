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

#include "common_header_autogen.h"
#include "ruletable_util.h"
#include "rule_summary.h"
#include "la_detect.h"
#include "container.h"

namespace maplefe {

MemPool gMemPool;

// dump LookAhead like {LA_Char, 'c'}
std::string GetLookAheadString(LookAhead la) {
  std::string str = "{";
  switch (la.mType) {
  case LA_Char:
    str += "LA_Char, ";
    str += la.mData.mChar;
    break;
  case LA_String:
    str += "LA_String, \"";
    str += la.mData.mString;
    str += "\"";
    break;
  case LA_Token:
    str += "LA_Token, ";
    str += std::to_string(la.mData.mTokenId);
    break;
  case LA_Identifier:
    str += "LA_Identifier, NULL";
    break;
  case LA_Literal:
    str += "LA_Literal, NULL";
    break;
  default:
    MASSERT(0 && "Unsupported lookahead type");
    break;
  }
  str += "}";
  return str;
}

// Need to guarantee there is no duplicated lookahed
void RuleLookAhead::AddLookAhead(LookAhead la) {
  bool found = FindLookAhead(la);
  if (!found) {
    mLookAheads.PushBack(la);
  }
}

bool RuleLookAhead::FindLookAhead(LookAhead la) {
  bool found = false;
  for (unsigned i = 0; i < mLookAheads.GetNum(); i++) {
    LookAhead temp_la = mLookAheads.ValueAtIndex(i);
    if (LookAheadEqual(la, temp_la)) {
      found = true;
      break;
    }
  }
  return found;
}

////////////////////////////////////////////////////////////////////////////////////
// The idea of LookAhead Dectect is through a Depth First Traversal in the tree.
////////////////////////////////////////////////////////////////////////////////////

void LADetector::SetupTopTables() {
  for (unsigned i = 0; i < gTopRulesNum; i++)
    mToDo.PushBack(gTopRules[i]);
}

// A talbe is already been processed.
bool LADetector::IsInProcess(RuleTable *t) {
  for (unsigned i = 0; i < mInProcess.GetNum(); i++) {
    if (t == mInProcess.ValueAtIndex(i))
      return true;
  }
  return false;
}

// A talbe is already done.
bool LADetector::IsDone(RuleTable *t) {
  for (unsigned i = 0; i < mDone.GetNum(); i++) {
    if (t == mDone.ValueAtIndex(i))
      return true;
  }
  return false;
}

// A talbe is in ToDo
bool LADetector::IsToDo(RuleTable *t) {
  for (unsigned i = 0; i < mToDo.GetNum(); i++) {
    if (t == mToDo.ValueAtIndex(i))
      return true;
  }
  return false;
}

// Add a rule to the ToDo list, if it's not in any of the following list,
// mInProcess, mDone, mToDo.
void LADetector::AddToDo(RuleTable *rule) {
  if (!IsInProcess(rule) && !IsDone(rule) && !IsToDo(rule))
    mToDo.PushBack(rule);
}

// Add all child rules into ToDo, starting from index 'start'.
void LADetector::AddToDo(RuleTable *rule_table, unsigned start) {
  for (unsigned i = start; i < rule_table->mNum; i++) {
    TableData *data = rule_table->mData + i;
    if (data->mType == DT_Subtable) {
      RuleTable *child = data->mData.mEntry;
      AddToDo(child);
    }
  }
}

bool LADetector::IsMaybeZero(RuleTable *t) {
  for (unsigned i = 0; i < mMaybeZero.GetNum(); i++) {
    if (t == mMaybeZero.ValueAtIndex(i))
      return true;
  }
  return false;
}

Pending* LADetector::GetPending(RuleTable *rt) {
  for (unsigned i = 0; i < mPendings.GetNum(); i++) {
    Pending *p = mPendings.ValueAtIndex(i);
    if (p->mRule == rt) {
      return p;
    }
  }
  return NULL;
}

// Add the pending info. 'dependent' is dependent on 'pending'.
// 'pending' is still in process.
void LADetector::AddPending(RuleTable *pending_rt, RuleTable *dependent_rt) {
  Pending *p = GetPending(pending_rt);
  if (p) {
    if (!(p->mDependents.Find(dependent_rt)))
      p->AddDependent(dependent_rt);
    return;
  }

  p = (Pending*)gMemPool.Alloc(sizeof(Pending));
  new (p) Pending(pending_rt);
  p->AddDependent(dependent_rt);
  mPendings.PushBack(p);
}

RuleLookAhead* LADetector::GetRuleLookAhead(RuleTable *rule) {
  RuleLookAhead *la = NULL;
  for (unsigned i = 0; i < mRuleLookAheads.GetNum(); i++) {
    RuleLookAhead *temp_la = mRuleLookAheads.ValueAtIndex(i);
    if (temp_la->mRule == rule) {
      la = temp_la;
      break;
    }
  }
  return la;
}

RuleLookAhead* LADetector::CreateRuleLookAhead(RuleTable *rule) {
  RuleLookAhead *rule_la = (RuleLookAhead*)gMemPool.Alloc(sizeof(RuleLookAhead));
  new (rule_la) RuleLookAhead(rule);
  return rule_la;
}

// Add a new lookahead to a rule.
void LADetector::AddRuleLookAhead(RuleTable *rule, LookAhead la) {
  RuleLookAhead *rule_la = GetRuleLookAhead(rule);
  if (rule_la) {
    if (!(rule_la->FindLookAhead(la)))
      rule_la->AddLookAhead(la);
    return;
  }

  rule_la = CreateRuleLookAhead(rule);
  rule_la->AddLookAhead(la);
  mRuleLookAheads.PushBack(rule_la);
}

// Copy all LookAheads from 'from' to 'to'.
void LADetector::CopyRuleLookAhead(RuleTable *to, RuleTable *from) {
  RuleLookAhead *rule_la_from = GetRuleLookAhead(from);

  // Some times 'from' is not done yet, like recursion LeadNode.
  if(!rule_la_from)
    return;

  for (unsigned i = 0; i < rule_la_from->mLookAheads.GetNum(); i++) {
    LookAhead temp_la = rule_la_from->mLookAheads.ValueAtIndex(i);
    AddRuleLookAhead(to, temp_la);
  }
}

// 'rt' is the rule table to be detected.
// 'p' is the tree node of parent.
// We are traversing an edge p->rt.
TResult LADetector::DetectRuleTable(RuleTable *rt, ContTreeNode<RuleTable*> *p) {
  TResult res = TRS_NA;

  if (IsDone(rt)) {
    if (IsMaybeZero(rt))
      return TRS_MaybeZero;
    else
      return TRS_NA;
  }

  if (IsInProcess(rt)) {
    // 1. If 'p' is the same as 'rt', it's the back edge of a one-node recursion.
    //    This is not a pending node. We just do nothing.
    //    We add Pending if they are >1 nodes recursion.
    // 2. All ancestors of 'p' need be added as dependent on 'rt'.
    //    It should always reach 'rt' since it's an ancestor. We stop there.
    ContTreeNode<RuleTable*> *dep_tree_node = p;
    bool met = false;
    while (1) {
      RuleTable *dep_rt = dep_tree_node->GetData();
      if (dep_rt == rt) {
        met = true;
        break;
      }
      AddPending(rt, dep_rt);
      dep_tree_node = dep_tree_node->GetParent();
    }
    MASSERT(met);
    return TRS_NA;
  } else {
    mInProcess.PushBack(rt);
  }

  // For Identifier and literal, we stop going to children.
  if (rt == &TblIdentifier) {
    LookAhead la;
    la.mType = LA_Identifier;
    AddRuleLookAhead(rt, la);
  } else if (rt == &TblLiteral) {
    LookAhead la;
    la.mType = LA_Literal;
    AddRuleLookAhead(rt, la);
  } else {
    // Create new tree node.
    ContTreeNode<RuleTable*> *node = mTree.NewNode(rt, p);
    EntryType type = rt->mType;
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

  if (res == TRS_MaybeZero) {
    MASSERT(!IsMaybeZero(rt));
    mMaybeZero.PushBack(rt);
  }

  return res;
}

// Detect one entry of TableData.
// The result is attached to the LookAhead of 'rule_table'.
//
// 'tree_node' is the node of rule_table.
TResult LADetector::DetectOneDataEntry(TableData *data,
                                       RuleTable *rule_table,
                                       ContTreeNode<RuleTable*> *tree_node) {
  TResult temp_res = TRS_NA;
  RuleTable *child = NULL;
  if (data->mType == DT_Subtable) {
    child = data->mData.mEntry;
    temp_res = DetectRuleTable(child, tree_node);
    CopyRuleLookAhead(rule_table, child);
  } else if (data->mType == DT_String) {
    LookAhead la;
    la.mType = LA_String;
    la.mData.mString = data->mData.mString;
    AddRuleLookAhead(rule_table, la);
  } else if (data->mType == DT_Char) {
    LookAhead la;
    la.mType = LA_Char;
    la.mData.mChar = data->mData.mChar;
    AddRuleLookAhead(rule_table, la);
  } else if (data->mType == DT_Token) {
    LookAhead la;
    la.mType = LA_Token;
    la.mData.mTokenId = data->mData.mTokenId;
    AddRuleLookAhead(rule_table, la);
  } else {
    MASSERT(0 && "Unexpected data type in rule table.");
  }

  return temp_res;
}

// For Oneof rule, we try to detect for all its children if they are a rule table too.
// 1. If there is one or more children are MaybeZero, the parent is MaybeZero
// 2. Or the parent is NA.
//
// LookAhead info is added to rule_table by DetectOneDataEntry().

TResult LADetector::DetectOneof(RuleTable *rule_table, ContTreeNode<RuleTable*> *tree_node) {
  TResult result = TRS_NA;
  for (unsigned i = 0; i < rule_table->mNum; i++) {
    TableData *data = rule_table->mData + i;
    TResult temp_res = DetectOneDataEntry(data, rule_table, tree_node);
    if (temp_res == TRS_MaybeZero)
      result = TRS_MaybeZero;
  }

  return result;
}

// Zeroormore and Zeroorone has the same way to handle.
TResult LADetector::DetectZeroorXXX(RuleTable *rule_table, ContTreeNode<RuleTable*> *tree_node) {
  TResult result = TRS_NA;
  MASSERT((rule_table->mNum == 1) && "zeroormore node has more than one elements?");
  TableData *data = rule_table->mData;
  result = DetectOneDataEntry(data, rule_table, tree_node);
  return TRS_MaybeZero;
}

TResult LADetector::DetectData(RuleTable *rule_table, ContTreeNode<RuleTable*> *tree_node) {
  TResult result = TRS_NA;
  RuleTable *child = NULL;
  MASSERT((rule_table->mNum == 1) && "Data node has more than one elements?");
  TableData *data = rule_table->mData;
  result = DetectOneDataEntry(data, rule_table, tree_node);
  return result;
}

// 1. If any child is NOT MaybeZero, the parent is NOT a MaybeZero.
// 2. If any child is NOT MaybeZero, we stop the traversal. All rest children are put in
//    ToDo list.
TResult LADetector::DetectConcatenate(RuleTable *rule_table, ContTreeNode<RuleTable*> *tree_node) {
  TResult res = TRS_NA;

  TableData *data = rule_table->mData;
  RuleTable *child = NULL;

  unsigned maybezero = true;
  unsigned stop_child = 0;
  for (unsigned i = 0; i < rule_table->mNum; i++) {
    TableData *data = rule_table->mData + i;
    TResult temp_res = DetectOneDataEntry(data, rule_table, tree_node);
    if (temp_res != TRS_MaybeZero) {
      stop_child = i;
      maybezero = false;
      res = TRS_NA;
      break;
    }
  }

  // Add rest children to Todo list, if the child is a rule table.
  if (!maybezero)
    AddToDo(rule_table, stop_child + 1);
  else
    res = TRS_MaybeZero;

  return res;
}

// We start from the top tables.
// Iterate until mToDo is empty.
void LADetector::Detect() {
  mDone.Clear();
  SetupTopTables();

  while(!mToDo.Empty()) {
    // Each iteration is going to build a new tree. The previous
    // iteration has finished all its traversal with the spanning tree
    // already traversed. So we will clean up both mTree and mInProcess.
    mTree.Clear();
    mInProcess.Clear();

    // we also need clear the pending info
    mPendings.Clear();

    RuleTable *front = mToDo.Front();
    mToDo.PopFront();

    // It's possible that a rule is put in ToDo multiple times. So it's possible
    // the first instance IsDone while the second is still in ToDo. So is InProcess.
    if (!IsDone(front) && !IsInProcess(front)) {
      DetectRuleTable(front, NULL);
      BackPatch();
    }
  }
}

// Start from the root since it never depends on others. According to the way we
// traverse the graph during Detect, it's always true that dependent node is a children
// and pending node is a ancestor.
void LADetector::BackPatch() {
  SmallList<ContTreeNode<RuleTable*>*> working_list;
  if (!mTree.Empty())
    working_list.PushBack(mTree.GetRoot());

  while(!working_list.Empty()) {
    ContTreeNode<RuleTable*> *node = working_list.Front();
    working_list.PopFront();

    // patch dependents on this pending
    RuleTable *rule_table = node->GetData();
    Pending *pending = GetPending(rule_table);
    if (pending)
      PatchPending(pending);

    // Add children of 'node' in span tree to working list. It's guaranteed no
    // duplication in working_list.
    for (unsigned i = 0; i < node->GetChildrenNum(); i++) {
      ContTreeNode<RuleTable*> *child_node = node->GetChild(i);
      working_list.PushBack(child_node);
    }
  }
}

// Add lookaheads of pending to dependents.
void LADetector::PatchPending(Pending *pending) {
  RuleTable *rule = pending->mRule;
  RuleLookAhead *la_rule = GetRuleLookAhead(rule);
  MASSERT(la_rule);
  for (unsigned i = 0; i < pending->mDependents.GetNum(); i++) {
    RuleTable *dep = pending->mDependents.ValueAtIndex(i);
    CopyRuleLookAhead(dep, rule);
  }
}

// The reason I have a Release() is to make sure the destructors of RuleLookAheads
// and Pendings are invoked ahead of destructor of gMemPool.
void LADetector::Release() {
  for (unsigned i = 0; i < mRuleLookAheads.GetNum(); i++) {
    RuleLookAhead *la = mRuleLookAheads.ValueAtIndex(i);
    la->Release();
  }

  for (unsigned i = 0; i < mPendings.GetNum(); i++) {
    Pending *pending = mPendings.ValueAtIndex(i);
    pending->Release();
  }

  mToDo.Release();
  mInProcess.Release();
  mDone.Release();
  mMaybeZero.Release();

  mTree.Release();
}

// To write the external decl of all LookAheadTable of each rule.
//
//   extern LookAheadTable TblStatementLookAheadTable;
//   extern LookAhead     *TblStatementLookAhead;
//   ...
//
void LADetector::WriteHeaderFile() {
  mHeaderFile->WriteOneLine("#ifndef __GEN_LOOKAHEAD_H__", 27);
  mHeaderFile->WriteOneLine("#define __GEN_LOOKAHEAD_H__", 27);
  mHeaderFile->WriteOneLine("#include \"ruletable.h\"", 22);
  mHeaderFile->WriteOneLine("#endif", 6);
}

// There is one little problem. The rules LADetector traversed are all
// reachable from the top tables. It's possible that some rules are missing
// in LADetector::mRuleLookAheads.
//
// But we need dump all tables' LookAhead information no matter reachable
// or not. So we need look into gen_summary.h/cpp for all rule tables. If they
// cannot be reached, their lookahead is just set to 0.

void LADetector::WriteCppFile() {
  mCppFile->WriteOneLine("#include \"gen_lookahead.h\"", 26);
  mCppFile->WriteOneLine("#include \"common_header_autogen.h\"", 34);
  mCppFile->WriteOneLine("namespace maplefe {", 19);

  // Step 1. Write all the necessary LookAhead info, like
  //   LookAhead TblStatementLookAhead[] = {{LA_Char, 'c'}, {LA_Char, 'd'}};
  for (unsigned i = 0; i < RuleTableNum; i++) {
    RuleTableSummary rtn = gRuleTableSummarys[i];
    const RuleTable *rule_table = rtn.mAddr;
    const char *rule_table_name = rtn.mName;

    // see if it has data in mRuleLookAheads
    bool found = false;
    RuleLookAhead *lookahead = NULL;
    for (unsigned j = 0; j < mRuleLookAheads.GetNum(); j++) {
      lookahead = mRuleLookAheads.ValueAtIndex(j);
      RuleTable *la_rule = lookahead->mRule;
      if (la_rule == rule_table) {
        found = true;
        break;
      }
    }

    if (found) {
      unsigned num = lookahead->mLookAheads.GetNum();
      // LookAhead TblStatementLookAhead[] = {{LA_Char, 'c'}, {LA_Char, 'd'}};
      std::string s = "LookAhead ";
      s += rule_table_name;
      s += "LookAhead[] = {";
      // Write all lookaheads
      for (unsigned j = 0; j < num; j++) {
        LookAhead la = lookahead->mLookAheads.ValueAtIndex(j);
        std::string la_str = GetLookAheadString(la);
        if (j < num - 1)
          la_str += ",";
        s += la_str;
      }
      s += "};";
      mCppFile->WriteOneLine(s.c_str(), s.size());
    }
  }

  // Step 2. Write the global array of lookahead of all rules.

  // LookAheadTable *loalLookAheadTable;
  std::string global = "LookAheadTable localLookAheadTable[] = {";
  mCppFile->WriteOneLine(global.c_str(), global.size());

  for (unsigned i = 0; i < RuleTableNum; i++) {
    RuleTableSummary rtn = gRuleTableSummarys[i];
    const RuleTable *rule_table = rtn.mAddr;
    const char *rule_table_name = rtn.mName;

    // see if it has data in mRuleLookAheads
    bool found = false;
    RuleLookAhead *lookahead = NULL;
    for (unsigned j = 0; j < mRuleLookAheads.GetNum(); j++) {
      lookahead = mRuleLookAheads.ValueAtIndex(j);
      RuleTable *la_rule = lookahead->mRule;
      if (la_rule == rule_table) {
        found = true;
        break;
      }
    }

    if (found) {
      unsigned num = lookahead->mLookAheads.GetNum();
      std::string s = "  {";
      std::string num_str = std::to_string(num);
      s += num_str;
      s += ", ";
      s += rule_table_name;
      s += "LookAhead},";
      mCppFile->WriteOneLine(s.c_str(), s.size());
    } else {
      // write {0, NULL},
      std::string s = "  {0, NULL},";
      mCppFile->WriteOneLine(s.c_str(), s.size());
    }
  }

  global = "};";
  mCppFile->WriteOneLine(global.c_str(), global.size());
  global = "LookAheadTable *gLookAheadTable = localLookAheadTable;";
  mCppFile->WriteOneLine(global.c_str(), global.size());
  mCppFile->WriteOneLine("}", 1);
}

// Write the recursion to gen/genmore_lookahead.h and gen/genmore_lookahead.cpp
void LADetector::Write() {
  std::string lang_path("../gen/");

  std::string file_name = lang_path + "genmore_lookahead.cpp";
  mCppFile = new Write2File(file_name);
  file_name = lang_path + "gen_lookahead.h";
  mHeaderFile = new Write2File(file_name);

  WriteHeaderFile();
  WriteCppFile();

  delete mCppFile;
  delete mHeaderFile;
}
}

int main(int argc, char *argv[]) {
  maplefe::gMemPool.SetBlockSize(4096);
  maplefe::LADetector dtc;
  dtc.Detect();
  dtc.Write();
  dtc.Release();
  return 0;
}
