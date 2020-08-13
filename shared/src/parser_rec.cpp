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

////////////////////////////////////////////////////////////////////////////
// This is file is merely for the implemenation of Left Recursion Parsing.
////////////////////////////////////////////////////////////////////////////

#include "container.h"
#include "parser.h"
#include "parser_rec.h"
#include "ruletable_util.h"
#include "gen_debug.h"


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

// This is used in traversal of both LeadFronNode and FronNode.
// When in LeadFronNode, rec is NULL and cir_idx is useless.
// When in FronNode, rec tells the LeftRecursion and cir_idx tells the circle index.
bool Parser::TraverseFronNode(AppealNode *parent, FronNode fnode, Recursion *rec, unsigned cir_idx) {
  bool found = false;
  const char *name = NULL;
  if (fnode.mType == FNT_Token) {
    Token *token = fnode.mData.mToken;
    found = TraverseToken(token, parent);
    if (mTraceLeftRec)
      name = token->GetName();
  } else if (fnode.mType == FNT_Rule) {
    RuleTable *rt = fnode.mData.mTable;
    found = TraverseRuleTable(rt, parent);
    if (mTraceLeftRec)
      name = GetRuleTableName(rt);
  } else if (fnode.mType == FNT_Concat) {
    // All rules/tokens from the starting point will be checked.
    RuleTable *ruletable = NULL;
    if (mTraceLeftRec)
      name = "FronNode Concat ";
    if (!rec) {
      MASSERT(!parent->IsPseudo());
      ruletable = parent->GetTable();
    } else {
      ruletable = rec->FindRuleOnCircle(cir_idx, fnode.mPos);
    }
    found = TraverseConcatenate(ruletable, parent, fnode.mData.mStartIndex);
  } else {
    MASSERT(0 && "Unexpected FronNode type.");
  }

  if (mTraceLeftRec) {
    std::string trace("Done TraverseFronNode ");
    trace += name;
    if(found)
      trace += " succ";
    else
      trace += " fail";
    DumpIndentation();
    std::cout << "<<<" << trace.c_str() << std::endl; 
  }

  return found;
}

// This is the main entry for traversing a new recursion.
// 'node' is assured to be a real LeadNode by caller.
//
// TraverseRuleTablePre() has been called before this function, to check
// the already-succ or already-fail scenarios.
//
// [NOTE] Move mCurToken when succ, and restore it when fails.
//
//bool Parser::TraverseLeadNode(AppealNode *appeal, AppealNode *parent) {
//  // Step 1. We are entering a new recursion right now. Need prepare the
//  //         the recursion stack information.
//  RuleTable *rt = appeal->GetTable();
//
//  if (mTraceLeftRec) {
//    DumpIndentation();
//    std::cout << "<<<Enter LeadNode " << GetRuleTableName(rt)  << std::endl;
//  }
//
//  // It's possible that we re-enter a LeadNode. But we will skip it if it's
//  // already in the middle of traversal.
//  if (InRecStack(rt, mCurToken))
//    return false;
//
//  PushRecStack(rt, mCurToken);
//
//  // Step 2. Find Recursion
//  Recursion *rec = mRecursionAll.FindRecursion(rt);
//
//  // We will set the new mCurToken to the largest one.
//  // LeadFronNode and Circle's FronNodes are like OneOf children of LeadNode.
//  unsigned new_mCurToken = mCurToken;
//
//  // Step 3. Traverse LeadFronNodes
//  unsigned saved_mCurToken = mCurToken;
//  bool found = false;
//
//  for (unsigned i = 0; i < rec->mLeadFronNodes.GetNum(); i++){
//    bool temp_found = false;
//    gSuccTokensNum = 0;
//    mCurToken = saved_mCurToken;
//    FronNode fnode = rec->mLeadFronNodes.ValueAtIndex(i);
//    temp_found = TraverseFronNode(appeal, fnode);
//    found |= temp_found;
//
//    new_mCurToken = mCurToken > new_mCurToken ? mCurToken : new_mCurToken;
//
//    // Add succ to 'appeal' and SuccMatch, using gSuccTokens.
//    if (found) {
//      appeal->mAfter = Succ;
//      UpdateSuccInfo(saved_mCurToken, appeal);
//    }
//  }
//
//  // Step 4. Traverse Circles.
//  //         The 'appeal' succ match info will be saved inside TraverseCircle()
//  //         which also construct the path.
//  unsigned temp_gSuccTokensNum = 0;
//  unsigned temp_gSuccTokens[MAX_SUCC_TOKENS];
//  for (unsigned i = 0; i < rec->mNum; i++) {
//    mCurToken = saved_mCurToken;
//    if (mTraceLeftRec) {
//      DumpIndentation();
//      std::cout << "<<<Enter TraverseCircle:" << i << std::endl;
//    }
//    bool temp_found = TraverseCircle(appeal, rec, i, new_mCurToken);
//    if (mTraceLeftRec) {
//      DumpIndentation();
//      std::cout << "<<<Exit TraverseCircle:" << i << std::endl;
//    }
//    found |= temp_found;
//  }
//
//  MASSERT(new_mCurToken >= saved_mCurToken);
//  mCurToken = new_mCurToken;
//
//  // Step 5. Restore the recursion stack.
//  RecStackEntry entry = mRecStack.Back();
//  MASSERT((entry.mLeadNode == rt) && (entry.mStartToken == saved_mCurToken));
//  mRecStack.PopBack();
//
//  // Step 6. The gSuccTokens/Num will be updated in the caller in parser.cpp
//
//  if (mTraceLeftRec) {
//    DumpIndentation();
//    std::cout << "<<<Exit LeadNode " << GetRuleTableName(rt)  << std::endl;
//  }
//
//  return found;
//}

// There are several things to be done in this function.
// 1. Traverse each FronNode
// 2. No matter succ or fail, build the path in Appeal tree, and add succ match
//    info to all node in the path and the lead node.
bool Parser::TraverseCircle(AppealNode *lead,
                            Recursion *rec,
                            unsigned idx,
                            unsigned &new_mCurToken) {
  SmallVector<FronNode> *fron_nodes = rec->mFronNodes.ValueAtIndex(idx);
  unsigned saved_mCurToken = mCurToken;
  bool found = false;

  for (unsigned i = 0; i < fron_nodes->GetNum(); i++) {
    // To access the Appeal Tree created of each FronNode, we use pseudo nodes
    // which play the role of parent of the created nodes.
    AppealNode *pseudo_parent = new AppealNode();
    pseudo_parent->SetIsPseudo();
    pseudo_parent->SetStartIndex(mCurToken);
    mAppealNodes.push_back(pseudo_parent);

    FronNode fnode = fron_nodes->ValueAtIndex(i);
    bool temp_found = TraverseFronNode(pseudo_parent, fnode, rec, idx);
    found |= temp_found;

    if (mCurToken > new_mCurToken)
      new_mCurToken = mCurToken;

    // Create a path.
    unsigned *circle = rec->mCircles[idx];
    AppealNode *last = ConstructPath(lead, pseudo_parent, circle, fnode.mPos);

    // Apply the start index / succ info to the path.
    mCurToken = saved_mCurToken;
    ApplySuccInfoOnPath(lead, last, found);
  }

  return found;
}

// Construct a path from 'lead' to 'ps_node'. The rule tables in between are
// determined by 'circle'. Totoal number of rule tables involved is num_steps - 1.
// The path should be like,
//  lead -> rule table 1 -> ... -> rule table(num_steps-1) -> children in ps_node
// We return rule-table (num_steps-1) as the ending one.
//
// [NOTE] Since it's a circle, you can actually create endless number of paths.
//        However, in reality we need only the simplest path.
AppealNode* Parser::ConstructPath(AppealNode *lead,
                                  AppealNode *ps_node,
                                  unsigned *circle,
                                  unsigned num_steps) {
  RuleTable *prev = lead->GetTable();
  AppealNode *prev_anode = lead;
  RuleTable *next = NULL;
  AppealNode *next_anode = NULL;

  unsigned len = circle[0];
  for (unsigned j = 1; j < num_steps; j++) {
    unsigned child_index = circle[j];
    FronNode node = RuleFindChildAtIndex(prev, child_index);
    MASSERT(node.mType == FNT_Rule);

    next = node.mData.mTable;
    AppealNode *next_anode = new AppealNode();
    next_anode->SetTable(next);
    mAppealNodes.push_back(next_anode);
    next_anode->SetParent(prev_anode);
    prev_anode->AddChild(next_anode);

    prev = next;
    prev_anode = next_anode;
  }

  // in the end, we need remove ps_node, and connect real children node to
  // prev_anode.
  std::vector<AppealNode*>::iterator cit = ps_node->mChildren.begin();
  for (; cit != ps_node->mChildren.end(); cit++) {
    AppealNode *child = *cit;
    child->SetParent(prev_anode);
    prev_anode->AddChild(child);
  }

  return prev_anode;
}

// set start index, and succ info in all node from 'lead' to 'last'.
void Parser::ApplySuccInfoOnPath(AppealNode *lead, AppealNode *last, bool succ) {
  AppealNode *node = last;
  RuleTable *rt = NULL;
  const char *name = NULL;
  while(1) {
    if (node->IsPseudo()) {
      name = " last ";
    } else {
      name = GetRuleTableName(node->GetTable());
    }

    //std::cout << "Update rule:" << name << "@" << mCurToken;
    if (succ) {
      node->SetStartIndex(mCurToken);
      UpdateSuccInfo(mCurToken, node);
      node->mAfter = Succ;
      //std::cout << " succ." << std::endl;
    } else {
      // 'lead' could be a Oneof node containing multiple children, and it could
      // already be Succ due to its other children. We won't update it to Fail.
      if (node->mAfter != AppealStatus_NA) {
        MASSERT(node == lead);
        //std::cout << " not changed." << std::endl;
      } else {
        node->mAfter = FailChildrenFailed;
        //std::cout << " fail." << std::endl;
      }
    }

    if (node == lead)
      break;
    else
      node = node->GetParent();
  }
}


///////////////////////////////////////////////////////////////////////////////////////////
//                           RecursionTraversal
///////////////////////////////////////////////////////////////////////////////////////////

// We arrive here only when the first time we hit this RecursionGroup because
// 1. All the 2nd appearance (of all LeadNodes) of all instance is caught by
//    TraverseRuleTable() before entering this function.
// 2. All the 1st appearance (of the first LeadNode) of all instances is done by
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

  // The gSuccTokens/Num will be updated in the caller in parser.cpp
  // We don't handle over here.

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
    mSelf->mAfter = FailChildrenFailed;
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
        curr_node->mAfter = FailChildrenFailed;
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

// Returns true if first->...->second is one of the circles of mRec
bool RecursionTraversal::IsOnCircle(AppealNode *second, AppealNode *first) {
  unsigned on_times = 0;
  for (unsigned i = 0; i < mRec->mRecursionNodes.GetNum(); i++) {
    SmallVector<RuleTable*> *circle = mRec->mRecursionNodes.ValueAtIndex(i);
    unsigned index = circle->GetNum() - 1;

    // we check backwards.
    // 'second' is not in the circle, 'first' is in circle.
    bool found = true;
    AppealNode *node = second->GetParent();
    while(node) {
      RuleTable *rt = node->GetTable();
      RuleTable *c_rt = circle->ValueAtIndex(index);
      if (rt != c_rt) {
        found = false;
        break;
      }
      node = node->GetParent();
      if (node == first)
        break;
      index--;
    }

    if (found) {
      MASSERT(index == 0);
      on_times++;
    }
  }

  MASSERT(on_times <= 1);
  if (on_times == 1)
    return true;
  else
    return false;
}

// I don't need worry about moving mCurToken if succ, because
// it's handled in TraverseLeadNode().
bool RecursionTraversal::FindInstances() {
  bool found = false;
  unsigned saved_mCurToken = mParser->mCurToken;
  bool temp_found = FindFirstInstance();
  found |= temp_found;
  while (temp_found) {
    // Move current mLeadNodes to mPrevLeadNodes
    mPrevLeadNodes.Clear();
    for (unsigned i = 0; i < mLeadNodes.GetNum(); i++)
      mPrevLeadNodes.PushBack(mLeadNodes.ValueAtIndex(i));

    // Clear LeadNodes and Visited LeadNodes/recursion nodes
    mVisitedLeadNodes.Clear();
    mVisitedRecursionNodes.Clear();
    mLeadNodes.Clear();

    // Find the instance
    mParser->mCurToken = saved_mCurToken;
    temp_found = FindRestInstance();
  }

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
  if (found) {
    for (unsigned i = 0; i < mAppealPoints.GetNum(); i++) {
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
  mParser->mAppealNodes.push_back(lead);

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

    // Set all ruletable's SuccMatch to IsDone in this recursion group.
    mParser->SetIsDone(mGroupId, mStartToken);

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
