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

// Find the LeftRecursion with 'rt' the LeadNode.
LeftRecursion* Parser::FindRecursion(RuleTable *rt) {
  for (unsigned i = 0; i < gLeftRecursionsNum; i++) {
    LeftRecursion *rec = gLeftRecursions[i];
    if (rec->mRuleTable == rt)
      return rec;
  }
  return NULL;
}

bool Parser::IsLeadNode(RuleTable *rt) {
  if (FindRecursion(rt))
    return true;
  else
    return false;
}

// Find the LeadFronNode-s of a LeadNode.
// Caller assures 'rt' is a LeadNode.
static void FindLeadFronNodes(RuleTable *rt, LeftRecursion *rec, SmallVector<FronNode> *nodes) {
  SmallVector<unsigned> circle_nodes;
  for (unsigned i = 0; i < rec->mNum; i++) {
    unsigned *circle = rec->mCircles[i];
    unsigned num = circle[0];
    MASSERT((num >= 1) && "Circle has no nodes?");
    unsigned first_node = circle[1];
    circle_nodes.PushBack(first_node);
  }

  for (unsigned i = 0; i < rt->mNum; i++) {
    bool found = false;
    for (unsigned j = 0; j < circle_nodes.GetNum(); j++) {
      if (i == circle_nodes.ValueAtIndex(j)) {
        found = true;
        break;
      }
    }

    if (!found) {
      FronNode = FindChildAtIndex(rt, i);
      nodes->PushBack(FronNode);
    }
  }
}

// Find all the nodes in all circles of the recursion, save them in 'nodes'.
// Each node should be a RuleTable* since it forms a circle.
static void FindRecursionNodes(RuleTable *lead,
                               LeftRecursion *rec,
                               SmallVector<RuleTable*> *nodes) {
  nodes->PushBack(lead);
  for (unsigned i = 0; i < rec->mNum; i++) {
    unsigned *circle = rec->mCircles[i];
    unsigned len = circle[0];
    RuleTable *prev = lead;
    for (j = 1; j <= len; j++) {
      unsigned child_index = circle[j];
      FronNode node = RuleFindChildAtIndex(prev, j);
      MASSERT(node.mIsTable);
      RuleTable *rt = node.mData.mTable;

      if (j == len) {
        // The last one should be the back edge.
        MASSERT(rt == lead);
      } else {
        // a node could be shared among multiple circles, need avoid duplication.
        bool found = false;
        for (unsigned k = 0; k < nodes->GetNum(); k++) {
          if (rt == nodes->ValueAtIndex(k)) {
            found = true;
            break;
          }
        }
        if (!found)
          nodes->PushBack(rt);
      }

      prev = rt;
    }
  }
}

// Find the FronNode along one of the circles.
// The algorithm is simple and includes two steps.
//   1. Find all the nodes belong to the Recursion, that is all nodes on
//      all circles of the recursion. This is said RecursionNodes.
//   2. Nodes directly accessible from 'circle' but not in RecursionNodes.
static void FindFronNodes(RuleTable *rt,
                          LeftRecursion *rec,
                          SmallVector<RuleTable*> *rec_nodes,
                          unsigned *circle,
                          SmallVector<FronNode> *nodes) {
  EntryType type = rt->mType;
  switch(type) {
  case ET_Oneof:
    break;

  case ET_Zeroormore:
  case ET_Zeroorone:
    // There is one and only one child. And it must be in loop.
    // There could be more than one loops for 'rt'. I need check if
    // every loop contains the first only child.
    //
    // In this case, there is no (Lead)FronNode.
    MASSERT((rt->mNum == 1) && "zeroorxxx node has more than one elements?");
    TableData *data = rt->mData + index;
    MASSERT(data->mType == DT_Subtable);
    for (unsigned i = 0; i < rec->mNum; i++) {
      unsigned *path = rec->mCircles[i];
      // second element is the first child table's index
      MASSERT(path[0] >= 1);
      MASSERT(path[1] == 0);
    }
    break;
  case ET_Concatenate:
    break;
  case ET_Data:
    break;
  case ET_Null:
  default:
    break;
  }
}

bool Parser::InRecStack(RuleTable *rt, unsigned start_token) {
  RecStackEntry e = {rt, start_token};
  return RecStack.Find(e);
}

bool Parser::PushRecStack(RuleTable *rt, unsigned start_token) {
  RecStackEntry e = {rt, start_token};
  return RecStack.PushBack(e);
}

// This is the main entry for traversing a new recursion.
// 'node' is assured to be a real LeadNode by caller.
void Parser::TraverseLeadNode(RuleTable *rt, AppealNode *parent) {
  // Preparation
  AppealNode *appeal = TraverseRuleTablePre(rt, parent);
  if (!appeal)
    return;

  // Step 1. We are entering a new recursion right now. Need prepare the
  //         the recursion stack information.
  unsigned saved_curtoken = mCurToken;
  if (InRecStack(rt, mCurToken))
    return;
  PushRecStack(rt, mCurToken);

  // Step 2. Find RecursionNode, LeadFronNodes
  SmallVector<RuleTable*> rec_nodes;     // all recursion nodes, should be RuleTable
  SmallVector<FronNode> lead_fron_nodes; // all LeadFronNodes.
  LeftRecursion *rec = FindRecursion(rt);
  FindLeadFronNodes(rt, rec, &lead_fron_nodes);
  FindRecursionNodes(rt, rec, &rec_nodes);

  // Step 3. Exhausting Algorithm
  //         To find the best matching of this recursion, we decided to do an
  //         exhausting algorithm, which tries all possible matchings.
  unsigned saved_mCurToken = mCurToken;
  while(1) {
    bool found = false;
    for (unsigned i = 0; i < lead_fron_nodes.GetNum(); i++){
      bool temp_found = false;
      gSuccTokensNum = 0;
      mCurToken = saved_mCurToken;
      FronNode fnode = lead_fron_nodes.ValueAtIndex(i);
      if (!fnode.IsTable) {
        temp_found = TraverseToken(fnode.mData.mToken, appeal);
      } else {
        temp_found = TraverseRuleTable(fnode.mData.mTable, appeal);
      }
      
      found |= temp_found;

      // Add succ to 'appeal'.
      for (unsigned j = 0; j < gSuccTokensNum; j++)
        appeal->AddMatch(gSuccTokens[j]);
    }

    // Step 3.2 Traverse Circles.
    //          The 'appeal' succ match info will be saved inside TraverseCircle()
    //          which also construct the path.
    unsigned temp_gSuccTokensNum = 0;
    unsigned temp_gSuccTokens[MAX_SUCC_TOKENS];
    for (unsigned i = 0; i < rec->mNum; i++) {
      unsigned *circle = rec->mCircles[i];
      mCurToken = saved_mCurToken;
      bool temp_found = TraverseCircle(appeal, rec, circle, &rec_nodes);
      found |= temp_found;
    }

    if (!found)
      break;
  }

  // Step 4. Restore the recursion stack.
  RecStackEntry entry = RecStack.Back();
  MASSERT((entry.mLeadNode == rt) && (entry.mStartToken == saved_curtoken));
}

// There are several things to be done in this function.
// 1. Traverse each FronNode
// 2. if succ, build the path in Appeal tree, and add succ match info to
//    all node in the path and the lead node.
bool Parser::TraverseCircle(AppealNode *lead,
                            LeftRecursion *rec,
                            unsigned *circle,
                            SmallVector<RuleTable*> *rec_nodes) {
  SmallVector<FronNode> fron_nodes;
  RuleTable *rt = lead->GetTable();

  FindFronNodes(rt, rec, &rec_nodes, circle, &fron_nodes);
}
