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
      FronNode fnode = RuleFindChildAtIndex(rt, i);
      nodes->PushBack(fnode);
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
    for (unsigned j = 1; j <= len; j++) {
      unsigned child_index = circle[j];
      FronNode node = RuleFindChildAtIndex(prev, child_index);
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
// We also record the position of each FronNode at the circle. 1 means the first
// node after LeadNode.
// FronNode: Nodes directly accessible from 'circle' but not in RecursionNodes.
//           NOTE. If 'prev' node is concatenate, 'next' must be its 1st child. And
//                 the rest children are not counted as FronNode since they cannot
//                 be matched alone without 'next'.
static void FindFronNodes(RuleTable *lead,
                          LeftRecursion *rec,
                          SmallVector<RuleTable*> *rec_nodes,
                          unsigned *circle,
                          SmallVector<FronNode> *nodes,
                          SmallVector<unsigned> *pos) {
  unsigned len = circle[0];
  RuleTable *prev = lead;
  for (unsigned j = 1; j <= len; j++) {
    unsigned child_index = circle[j];
    FronNode node = RuleFindChildAtIndex(prev, child_index);
    MASSERT(node.mIsTable);
    RuleTable *next = node.mData.mTable;

    // The last one should be the back edge.
    if (j == len)
      MASSERT(next == lead);

    // prev is lead, FronNode here is a LeadFronNode and will be
    // traversed in TraverseLeadNode();
    if (j == 1) {
      prev = next;
      continue;
    }

    // Only Oneof node is qualified to have FronNode.
    EntryType prev_type = prev->mType;
    switch(prev_type) {
    case ET_Oneof: {
      // Look into every childof 'prev'. If it's not 'next' and
      // not in 'rec_nodes', it's a FronNode.
      for (unsigned i = 0; i < prev->mNum; i++) {
        TableData *data = prev->mData + i;
        FronNode fnode;
        if (data->mType == DT_Token) {
          fnode.mIsTable = false;
          fnode.mData.mToken = data->mData.mToken;
          nodes->PushBack(fnode);
          pos->PushBack(j);
        } else if (data->mType = DT_Subtable) {
          RuleTable *ruletable = data->mData.mEntry;
          bool found = false;
          for (unsigned k = 0; k < rec_nodes->GetNum(); k++) {
            if (ruletable == rec_nodes->ValueAtIndex(k)) {
              found = true;
              break;
            }
          }
          if (!found && (ruletable != next)) {
            fnode.mIsTable = true;
            fnode.mData.mTable = ruletable;
            nodes->PushBack(fnode);
            pos->PushBack(j);
          }
        } else {
          MASSERT(0 && "unexpected data type in ruletable.");
        }
      }
      break;
    }

    case ET_Zeroormore:
    case ET_Zeroorone:
      // There is one and only one child. And it must be in circle.
      // In this case, there is no FronNode.
      MASSERT((prev->mNum == 1) && "zeroorxxx node has more than one elements?");
      MASSERT((child_index == 0));
      break;

    case ET_Concatenate:
      break;

    case ET_Data:
    case ET_Null:
    default:
      MASSERT(0 && "Wrong node type in a circle");
      break;
    }// end of switch

    prev = next;
  }
}

bool Parser::InRecStack(RuleTable *rt, unsigned start_token) {
  RecStackEntry e = {rt, start_token};
  return RecStack.Find(e);
}

void Parser::PushRecStack(RuleTable *rt, unsigned start_token) {
  RecStackEntry e = {rt, start_token};
  return RecStack.PushBack(e);
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
  unsigned saved_curtoken = mCurToken;
  if (InRecStack(rt, mCurToken)) {
    MERROR("Unexpected nested lead node traversal");
    return false;
  }

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
      if (!fnode.mIsTable) {
        temp_found = TraverseToken(fnode.mData.mToken, parent);
      } else {
        temp_found = TraverseRuleTable(fnode.mData.mTable, parent);
      }
      
      found |= temp_found;

      // Add succ to 'appeal'.
      for (unsigned j = 0; j < gSuccTokensNum; j++)
        parent->AddMatch(gSuccTokens[j]);
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
// 2. No matter succ or fail, build the path in Appeal tree, and add succ match
//    info to all node in the path and the lead node.
bool Parser::TraverseCircle(AppealNode *lead,
                            LeftRecursion *rec,
                            unsigned *circle,
                            SmallVector<RuleTable*> *rec_nodes) {
  SmallVector<FronNode> fron_nodes;
  SmallVector<unsigned> fron_pos;   // position in circle of each fron node.
                                    // 1 means the first node after lead.
  RuleTable *rt = lead->GetTable();

  // A FronNode in a circle be also a FronNode in another circle. They
  // are duplicated and traversed twice. But only the first time we do real
  // traversal, the other times will just check the succ or failure info.
  FindFronNodes(rt, rec, rec_nodes, circle, &fron_nodes, &fron_pos);

  // Try each FronNode.
  // To access the Appeal Tree created of each FronNode, we use pseudo nodes
  // which play the role of parent of the created nodes.
  unsigned saved_mCurToken = mCurToken;
  SmallVector<AppealNode*> PseudoParents;
  for (unsigned i = 0; i < fron_nodes.GetNum(); i++) {
    AppealNode *pseudo_parent = new AppealNode();
    pseudo_parent->SetIsPseudo();
    pseudo_parent->SetStartIndex(mCurToken);
    mAppealNodes.push_back(pseudo_parent);

    FronNode fnode = fron_nodes.ValueAtIndex(i);
    bool found = false;
    if (!fnode.mIsTable) {
      Token *token = fnode.mData.mToken;
      found = TraverseToken(token, pseudo_parent);
    } else {
      RuleTable *rt = fnode.mData.mTable;
      found = TraverseRuleTable(rt, pseudo_parent);
    }

    // Create a path.
    ConstructPath(lead, pseudo_parent, circle, fron_pos.ValueAtIndex(i));

    // Apply the start index / succ info to the path.
    mCurToken = saved_mCurToken;
    ApplySuccInfoOnPath(lead, pseudo_parent);
  }
}

// Construct a path from 'lead' to 'ps_node'. The rule tables in between are
// determined by 'circle'. Totoal number of rule tables involved is num_steps - 1.
// The path should be like,
//  lead -> rule table 1 -> ... -> rule table(num_steps-1) -> ps_node
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
    MASSERT(node.mIsTable);

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
void Parser::ApplySuccInfoOnPath(AppealNode *lead, AppealNode *pseudo) {
  AppealNode *node = pseudo;
  while(1) {
    node->SetStartIndex(mCurToken);
    for (unsigned i = 0; i < gSuccTokensNum; i++)
      node->AddMatch(gSuccTokens[i]);
    if (node == lead)
      break;
    else
      node = node->GetParent();
  }
}
