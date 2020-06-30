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


bool Parser::InRecStack(RuleTable *rt, unsigned start_token) {
  RecStackEntry e = {rt, start_token};
  return mRecStack.Find(e);
}

void Parser::PushRecStack(RuleTable *rt, unsigned start_token) {
  RecStackEntry e = {rt, start_token};
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

bool Parser::TraverseLeadNode(AppealNode *appeal, AppealNode *parent) {
  bool found = false;
  unsigned saved_mCurToken = mCurToken;

  // Preprocess the recursion stack information.
  RuleTable *rt = appeal->GetTable();
  if (mTraceLeftRec) {
    DumpIndentation();
    std::cout << "<<<Enter LeadNode " << GetRuleTableName(rt)  << std::endl;
  }

  // It's possible that we re-enter a LeadNode. We will skip if re-entering.
  if (InRecStack(rt, mCurToken))
    return false;
  PushRecStack(rt, mCurToken);

  // Main body of recursion traversal.
  RecursionTraversal rec_tra(appeal, parent, this);
  rec_tra.Work();

  // We only update the mCurToken when succeeds. The restore of mCurToken
  // when fail is handled by TraverseRuleTable.
  if (rec_tra.mSucc)
    mCurToken = rec_tra.mLastToken;

  RecStackEntry entry = mRecStack.Back();
  MASSERT((entry.mLeadNode == rt) && (entry.mStartToken == saved_mCurToken));
  mRecStack.PopBack();

  // The gSuccTokens/Num will be updated in the caller in parser.cpp
  // We don't handle over here.

  if (mTraceLeftRec) {
    DumpIndentation();
    std::cout << "<<<Exit LeadNode " << GetRuleTableName(rt)  << std::endl;
  }

  return found;
}

RecursionTraversal::RecursionTraversal(AppealNode *self, AppealNode *parent, Parser *parser) {
  mParser = parser;
  mSelf = self;
  mParent = parent;
  mRuleTable = mSelf->GetTable();
  mRec = mParser->mRecursionAll.FindRecursion(mRuleTable);

  mSucc = false;
  mStartToken = mParser->mCurToken;
  mLastToken = 0;
}

RecursionTraversal::~RecursionTraversal() {
}

void RecursionTraversal::Work() {
  mSucc = FindInstances();
  if (mSucc)
    ConnectInstances();
}

bool RecursionTraversal::FindInstances() {
  bool found = false;
  bool temp_found = FindFirstInstance();
  found |= temp_found;
  while (temp_found) {
    bool temp_found = FindNextInstance();
    found |= temp_found;
  }
  return found;
}

bool RecursionTraversal::FindFirstInstance() {
  bool found = false;
  return found;
}

bool RecursionTraversal::FindNextInstance() {
  bool found = false;
  return found;
}

void RecursionTraversal::ConnectInstances() {
}
