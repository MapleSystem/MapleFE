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
void Parser::FindLeadFronNodes(RuleTable *rt,
                               LeftRecursion *rec,
                               SmallVector<RuleTable*> *nodes) {
  EntryType type = rt->mType;
  switch(type) {
  case ET_Oneof:
    break;

  case ET_Zeroormore:
  case ET_Zeroorone:
    // There is one and only one child. And it must be in loop.
    // There could be more than one loops for 'rt'. I need check if
    // every loop contains the first only child.
    MASSERT((rt->mNum == 1) && "zeroorxxx node has more than one elements?");
    TableData *data = rt->mData;
    MASSERT(data->mType == DT_Subtable);
    for (unsigned i = 0; i < rec->mNum; i++) {
      unsigned *path = rec->mPaths[i];
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

// 'node' is assured to be a real LeadNode by caller.
void Parser::TraverseLeadNode(AppealNode *node, AppealNode *parent) {
  RuleTable *rt = node->GetTable();
  LeftRecursion *rec = FindRecursion(rt);

  // step 1. Find the Loops and LeadFronNodes
  SmallVector<RuleTable*> lead_fron_nodes;
  FindLeadFronNodes(rt, rec, &lead_fron_nodes);
}
