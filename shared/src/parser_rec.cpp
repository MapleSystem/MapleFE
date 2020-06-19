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

bool Parser::TraverseFronNode(AppealNode *parent, FronNode fnode, Recursion *rec, unsigned cir_idx) {
  bool found = false;
  if (fnode.mType == FNT_Token) {
    Token *token = fnode.mData.mToken;
    found = TraverseToken(token, parent);
    //const char *name = token->GetName();
    //std::cout << "FronNode " << name << " " << temp_found << std::endl;
  } else if (fnode.mType == FNT_Rule) {
    RuleTable *rt = fnode.mData.mTable;
    found = TraverseRuleTable(rt, parent);
    //const char *name = GetRuleTableName(rt);
    //std::cout << "FronNode " << name << " " << temp_found << std::endl;
  } else if (fnode.mType == FNT_Concat) {
    // All rules/tokens from the starting point will be checked.
    RuleTable *ruletable = NULL;
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
}

// This is the main entry for traversing a new recursion.
// 'node' is assured to be a real LeadNode by caller.
//
// TraverseRuleTablePre() has been called before this function, to check
// the already-succ or already-fail scenarios.

bool Parser::TraverseLeadNode(AppealNode *appeal, AppealNode *parent) {
  // Step 1. We are entering a new recursion right now. Need prepare the
  //         the recursion stack information.
  RuleTable *rt = appeal->GetTable();

  //std::cout << "Enter LeadNode " << GetRuleTableName(rt)  << std::endl;

  // It's possible that we re-enter a LeadNode. But we will skip it if it's
  // already in the middle of traversal.
  if (InRecStack(rt, mCurToken))
    return false;
  PushRecStack(rt, mCurToken);

  // Step 2. Find Recursion
  Recursion *rec = mRecursionAll.FindRecursion(rt);

  // Step 3. Traverse LeadFronNodes
  unsigned saved_mCurToken = mCurToken;
  bool found = false;

  for (unsigned i = 0; i < rec->mLeadFronNodes.GetNum(); i++){
    bool temp_found = false;
    gSuccTokensNum = 0;
    mCurToken = saved_mCurToken;
    FronNode fnode = rec->mLeadFronNodes.ValueAtIndex(i);
    temp_found = TraverseFronNode(appeal, fnode);
    found |= temp_found;

    // Add succ to 'appeal' and SuccMatch.
    if (found) {
      appeal->mAfter = Succ;
      UpdateSuccInfo(saved_mCurToken, appeal);
    }
  }

  // Step 4. Traverse Circles.
  //         The 'appeal' succ match info will be saved inside TraverseCircle()
  //         which also construct the path.
  unsigned temp_gSuccTokensNum = 0;
  unsigned temp_gSuccTokens[MAX_SUCC_TOKENS];
  for (unsigned i = 0; i < rec->mNum; i++) {
    mCurToken = saved_mCurToken;
    //std::cout << "TraverseCircle:" << i << std::endl;
    bool temp_found = TraverseCircle(appeal, rec, i);
    found |= temp_found;
  }

  // Step 5. Restore the recursion stack.
  RecStackEntry entry = mRecStack.Back();
  MASSERT((entry.mLeadNode == rt) && (entry.mStartToken == saved_mCurToken));
  mRecStack.PopBack();

  // Step 6. The gSuccTokens/Num will be updated in the caller in parser.cpp

  //std::cout << "Exit LeadNode " << GetRuleTableName(rt)  << std::endl;
}

// There are several things to be done in this function.
// 1. Traverse each FronNode
// 2. No matter succ or fail, build the path in Appeal tree, and add succ match
//    info to all node in the path and the lead node.
bool Parser::TraverseCircle(AppealNode *lead, Recursion *rec, unsigned idx) {
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

    // Create a path.
    unsigned *circle = rec->mCircles[idx];
    ConstructPath(lead, pseudo_parent, circle, fnode.mPos);

    // Apply the start index / succ info to the path.
    mCurToken = saved_mCurToken;
    ApplySuccInfoOnPath(lead, pseudo_parent, found);
  }

  return found;
}

// Construct a path from 'lead' to 'ps_node'. The rule tables in between are
// determined by 'circle'. Totoal number of rule tables involved is num_steps - 1.
// The path should be like,
//  lead -> rule table 1 -> ... -> rule table(num_steps-1) -> ps_node
//
// [NOTE] Since it's a circle, you can actually create endless number of paths.
//        However, in reality we need only the simplest path.
void Parser::ConstructPath(AppealNode *lead, AppealNode *ps_node, unsigned *circle,
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

  // in the end, we need connect to ps_node.
  ps_node->SetParent(prev_anode);
  prev_anode->AddChild(ps_node);
}

// set start index, and succ info in all node from 'lead' to 'pseudo'.
void Parser::ApplySuccInfoOnPath(AppealNode *lead, AppealNode *pseudo, bool succ) {
  AppealNode *node = pseudo;
  RuleTable *rt = NULL;
  const char *name = NULL;
  while(1) {
    if (node->IsPseudo()) {
      name = " pseudo ";
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
      node->mAfter = FailChildrenFailed;
      //std::cout << " fail." << std::endl;
    }

    if (node == lead)
      break;
    else
      node = node->GetParent();
  }
}
