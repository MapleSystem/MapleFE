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

////////////////////////////////////////////////////////////////////////////
// This is file is merely for the implemenation of Left Recursion Parsing.
////////////////////////////////////////////////////////////////////////////

#include "container.h"
#include "parser.h"
#include "parser_rec.h"
#include "ruletable_util.h"
#include "rule_summary.h"

namespace maplefe {

RecursionTraversal* Parser::FindRecStack(unsigned group_id, unsigned start_token) {
  for (unsigned i = 0; i < mRecStack.GetNum(); i++) {
    RecStackEntry e = mRecStack.ValueAtIndex(i);
    if ((e.mGroupId == group_id) && (e.mStartToken == start_token))
      return e.mRecTra;
  }
  return NULL;
}

void Parser::PushRecStack(unsigned group_id, RecursionTraversal *rectra, unsigned start_token) {
  RecStackEntry e = {rectra, group_id, start_token};
  return mRecStack.PushBack(e);
}

///////////////////////////////////////////////////////////////////////////////////////////
//                           RecursionTraversal
///////////////////////////////////////////////////////////////////////////////////////////

// We arrive here only when the first time we hit this RecursionGroup because
// 1. All the 2nd appearance (of all LeadNodes) of all instance is caught by
//    TraverseRuleTable() before entering this function.
// 2. All the 1st appearance (of the mast LeadNode) of all instances is done by
//    FindInstance() and then TraverseRuleTableRegular().
//                       --- OR ---
// we arrive TraverseLeadNode because it's the first time we hit 'other' LeadNodes
// of this group.

bool Parser::TraverseLeadNode(AppealNode *appeal, AppealNode *parent) {
  unsigned saved_mCurToken = mCurToken;
  RuleTable *rt = appeal->GetTable();
  const char *name = GetRuleTableName(rt);
  unsigned group_id;
  bool found = false;
  bool found_group = false;
  found_group = FindRecursionGroup(rt, group_id);
  MASSERT(found_group);

  if (mTraceLeftRec) {
    DumpIndentation();
    std::cout << "<LR>: Enter LeadNode " << name
      << "@" << appeal->GetStartIndex() << " node:" << appeal;
    std::cout << " group id:" << group_id << std::endl;
  }

  RecursionTraversal *rec_tra= FindRecStack(group_id, mCurToken);

  // A recurion group could have multiple recursions or multiple LeadNodes.
  // We could have already rec_tra. In this case, it should be done through
  // regular traversal
  if (rec_tra) {
    MASSERT(rt != rec_tra->GetRuleTable());
    rec_tra->AddVisitedLeadNode(rt);
    rec_tra->AddLeadNode(appeal);
    bool found = TraverseRuleTableRegular(rt, appeal);
    if (mTraceTable) {
      DumpIndentation();
      std::cout << "<LR>: Exit Other LeadNode " << name << std::endl;
      //DumpExitTable(...) is called in the caller. We don't dump here.
    }
    return found;
  }

  rec_tra = new RecursionTraversal(appeal, parent, this);
  rec_tra->SetTrace(mTraceLeftRec);
  rec_tra->SetIndentation(mIndentation);
  PushRecStack(group_id, rec_tra, mCurToken);

  // Main body of recursion traversal.
  rec_tra->Work();

  // We only update the mCurToken when succeeds. The restore of mCurToken
  // when fail is handled by TraverseRuleTable.
  // We pick the longest match rec_tra.
  if (rec_tra->IsSucc()) {
    found = true;
    mCurToken = rec_tra->LongestMatch();
    MoveCurToken();
  }

  if (mTraceLeftRec) {
    DumpIndentation();
    std::cout << "<LR>: Exit LeadNode " << GetRuleTableName(rt);
    if (appeal->IsSucc())
      std::cout << " succ longest match@" << appeal->LongestMatch() << std::endl;
    else
      std::cout << " fail" << std::endl;
  }

  RecStackEntry entry = mRecStack.Back();
  MASSERT((entry.mGroupId == group_id) && (entry.mStartToken == saved_mCurToken));
  mRecStack.PopBack();

  delete rec_tra;

  return found;
}

RecursionTraversal::RecursionTraversal(AppealNode *self, AppealNode *parent, Parser *parser) {
  mParser = parser;
  mSelf = self;
  mParent = parent;
  mRuleTable = mSelf->GetTable();
  mRec = mParser->mRecursionAll.FindRecursion(mRuleTable);

  mInstance = InstanceNA;
  mSucc = false;
  mStartToken = mParser->mCurToken;

  mTrace = false;
  mIndentation = 0;

  bool found = FindRecursionGroup(mRuleTable, mGroupId);
  MASSERT(found);
}

RecursionTraversal::~RecursionTraversal() {
  mLeadNodes.Release();
}

// We only do FinalConnection when succ.
void RecursionTraversal::Work() {
  mSucc = FindInstances();
  if (mSucc)
    FinalConnection();
  else
    mSelf->mResult = FailChildrenFailed;
}

// Connect curr_node to its counterpart in the prev instance.
// [NOTE] There are two appearance of a rule in each instance. The connection
//        happens between the second appearance of current instance and
//        the first appearance of the prev instance.
bool RecursionTraversal::ConnectPrevious(AppealNode *curr_node) {
  MASSERT(mInstance == InstanceRest);
  MASSERT(curr_node->IsTable());
  bool found = false;

  // Connect to the previous one.
  for (unsigned i = 0; i < mPrevLeadNodes.GetNum(); i++) {
    AppealNode *prev_lead = mPrevLeadNodes.ValueAtIndex(i);
    MASSERT(prev_lead->IsTable());
    if (prev_lead->GetTable() == curr_node->GetTable()) {
      // It could be a fail.
      if (prev_lead->IsFail()) {
        curr_node->mResult = FailChildrenFailed;
        break;
      }

      // copy the previous instance result to 'curr_node'.
      curr_node->CopyMatch(prev_lead);

      // connect the previous instance to 'curr_node'.
      // It's obvious that A previous instance could be connected to multiple
      // back edge in multiple circles of a next instance of recursion. AddParent()
      // will handle the multiple parents issue.
      curr_node->AddChild(prev_lead);
      prev_lead->AddParent(curr_node);

      // there should be only one match.
      MASSERT(!found);
      found = true;
    }
  }
  MASSERT(found);

  return curr_node->IsSucc();
}

// I don't need worry about moving mCurToken if succ, because
// it's handled in TraverseLeadNode().
bool RecursionTraversal::FindInstances() {
  bool found = false;
  unsigned saved_mCurToken = mParser->mCurToken;
  bool temp_found = FindFirstInstance();
  found |= temp_found;
  while (temp_found) {
    // Copy the match info to mSelf
    AppealNode *prev_lead = mLeadNodes.ValueAtIndex(0);
    mSelf->CopyMatch(prev_lead);
    mSelf->AddChild(prev_lead);
    prev_lead->AddParent(mSelf);

    // Move current mLeadNodes to mPrevLeadNodes
    mPrevLeadNodes.Clear();
    for (unsigned i = 0; i < mLeadNodes.GetNum(); i++)
      mPrevLeadNodes.PushBack(mLeadNodes.ValueAtIndex(i));

    // Clear LeadNodes and Visited LeadNodes/recursion nodes
    mVisitedLeadNodes.Clear();
    mVisitedRecursionNodes.Clear();
    mLeadNodes.Clear();

    // Find the instance.
    // Remember to reset mEndOfFile since the prev instance could reach the end of file
    // We need start from the beginning.
    mParser->mCurToken = saved_mCurToken;
    if (mParser->mEndOfFile)
      mParser->mEndOfFile = false;
    temp_found = FindRestInstance();
  }

  // Set all ruletable's SuccMatch to IsDone in this recursion group.
  // Even though the whole recursion fails, some nodes can succeed.
  mParser->SetIsDone(mGroupId, mStartToken);

  return found;
}

bool RecursionTraversal::FindFirstInstance() {
  bool found = false;
  mInstance = InstanceFirst;

  // Create a lead node
  AppealNode *lead = new AppealNode();
  lead->SetStartIndex(mStartToken);
  lead->SetTable(mRuleTable);
  mParser->mAppealNodes.push_back(lead);

  if (mTrace) {
    DumpIndentation();
    std::cout << "<LR>: FirstInstance " << lead << std::endl;
  }

  AddLeadNode(lead);
  AddVisitedLeadNode(mRuleTable);

  found = mParser->TraverseRuleTableRegular(mRuleTable, lead);

  // Appealing of the mistaken Fail nodes.
  //
  // This is for appealing those affected by the 2nd appearance
  // of 2nd instance which returns false. 2ndOf1st is not add to WasFail, but
  // those affected will be AddFailed().
  //
  // I still keep an assertion in TraverseRuleTablePre() when it has SuccMatch,
  // asserting !WasFail. But I believe there are still WasFail at the same time.
  // We will see.

  if (found) {
    unsigned num = mAppealPoints.GetNum();
    for (unsigned i = 0; i < num; i++) {
      AppealNode *start = mAppealPoints.ValueAtIndex(i);
      mParser->Appeal(start, lead);
    }
  }

  return found;
}

// Find one of the rest instances. Excludes the first instance.
bool RecursionTraversal::FindRestInstance() {
  bool found = false;
  mInstance = InstanceRest;

  AppealNode *prev_lead = mPrevLeadNodes.ValueAtIndex(0);

  // Create a lead node
  AppealNode *lead = new AppealNode();
  lead->SetStartIndex(mStartToken);
  lead->SetTable(mRuleTable);

  AddLeadNode(lead);
  AddVisitedLeadNode(mRuleTable);

  if (mTrace) {
    DumpIndentation();
    std::cout << "<LR>: RestInstance " << lead << std::endl;
  }

  found = mParser->TraverseRuleTableRegular(mRuleTable, lead);
  MASSERT(found);

  // Need check if 'lead' REALLY matches tokens. Rest instance always
  // returns true since the prev instance is succ.
  if (lead->LongestMatch() > prev_lead->LongestMatch()) {
    return true;
  } else {
    if (mTrace) {
      DumpIndentation();
      std::cout << "<LR>: Fake Succ. Fixed Point." << std::endl;
    }
    // Need remove it from the SuccMatch, so that later parser won't
    // take it as an input.
    mParser->RemoveSuccNode(mStartToken, lead);

    return false;
  }
}

// Connect the generated tree to mSelf, which is the main entry of whole tree.
// We only do FinalConnection when succ. It's caller's duty to assure it's succ.

void RecursionTraversal::FinalConnection() {
  // The latest instance is Fail. The one before last instance is succ.
  MASSERT(mSucc);

  AppealNode *last = mPrevLeadNodes.ValueAtIndex(0);
  MASSERT(last->IsSucc());

  last->SetParent(mSelf);
  mSelf->AddChild(last);
  mSelf->CopyMatch(last);
}

void RecursionTraversal::DumpIndentation() {
  for (unsigned i=0; i < mIndentation; i++)
    std::cout << " ";
}
}
