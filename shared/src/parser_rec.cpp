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
static void FindLeadFronNodes(RuleTable *rt,
                       LeftRecursion *rec,
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
    TableData *data = rt->mData;
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

  // Step 2. Find the Loops and LeadFronNodes
  SmallVector<FronNode> lead_fron_nodes;
  LeftRecursion *rec = FindRecursion(rt);
  FindLeadFronNodes(rt, rec, &lead_fron_nodes);

  while(1) {
    // Traverse LeadFronNodes
    for (unsigned i = 0; i < lead_fron_nodes.GetNum(); i++){
      bool found = false;
      gSuccTokensNum = 0;
      FronNode fnode = lead_fron_nodes.ValueAtIndex(i);
      if (!fnode.IsTable) {
        found = TraverseToken(fnode.mData.mToken, parent);
      } else {
        found = TraverseRuleTable(fnode.mData.mTable, parent);
      }
    }

    // Traverse Circles.
  }

  // Step 2. Traverse the Lead Fron Nodes to see if they reduce tokens.
  //         Need take care of 

  // Step x. Restore the recursion stack.
  RecStackEntry entry = RecStack.Back();
  MASSERT((entry.mLeadNode == rt) && (entry.mStartToken == saved_curtoken));
}
