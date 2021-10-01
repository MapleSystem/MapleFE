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
//  The data strcutures used during parsing left recursions are defined
//  in this file.
////////////////////////////////////////////////////////////////////////////

#ifndef __PARSER_REC_H__
#define __PARSER_REC_H__

#include "ruletable.h"

namespace maplefe {

// After we traverse successfully on a LeadFronNode or a Circle, we need record
// the path which reach from LeadNode to the successful LeadFronNode or FronNode
// of a circle.
class AppealNode;
class RecPath {
public:
  AppealNode *mLeadNode;
  bool        mInCircle;  // true : from FronNode of a circle.
                          // false: from a LeadFronNode
  unsigned    mCircleIdx; // Index of the Circle in the circle vector of LeadNode.

  SmallVector<AppealNode*> mPath;  // This is the final path we concluded.
                                   // first element is the LeadNode.
                                   // last element is the successful matching one.
public:
  RecPath(){};
  ~RecPath() {mPath.Release();}

};

enum RecTraInstance {
  InstanceFirst,    // we are handling the first instance
  InstanceRest,     // we are handling the rest instances.
  InstanceNA
};

// The parsing is done as a Wavefront traversal. It takes a recursion group
// as a unit for traversal. Other rule tables not belonging to any recursion
// groups are treated as a standalone unit without iteration.

// Wavefront traversal on the recursion group is done by iterations. Each
// iteration it walks through all recursion nodes of the group, and also the
// FronNodes. If it hit a LeadNode for the second time in one iteration, it
// takes the result of the previous iteration, and build a edge between
// this iteration and the preivous iteration.

class RecursionTraversal {
private:
  Parser     *mParser;
  Recursion  *mRec;
  RuleTable  *mRuleTable;
  AppealNode *mSelf;
  AppealNode *mParent;
  unsigned    mGroupId;   // Id of recursion group
  RecTraInstance mInstance;

  // These are the 1st appearance of current instance.
  SmallVector<AppealNode*> mLeadNodes;

  // These are the 1st appearance of previous instance. Used when
  // connect the 2nd appearance of current instance to the previous.
  SmallVector<AppealNode*> mPrevLeadNodes;

  // Visited LeadNodes. This is a per-iteration data.
  //
  // In each iteration, the first time a LeadNode is visited, it will be saved
  // in this vector. The second time it's visited, it should go to connect
  // with the node in the previous instance or set as Failed2ndOf1st.
  SmallVector<RuleTable*> mVisitedLeadNodes;

  // Visited Recursion Node. This is a per-iteration data too.
  // This doesn't include lead node.
  // Visiting an un-visited recursion node will do the regular traversal.
  // Visiting a visited recursion node will take the result of TraversalRuleTablePre().
  SmallVector<RuleTable*> mVisitedRecursionNodes;

  SmallVector<AppealNode*> mAppealPoints; // places to start appealing

  bool     mTrace;
  unsigned mIndentation;

private:
  bool        mSucc;
  unsigned    mStartToken;

  bool FindFirstInstance();
  bool FindRestInstance();
  bool FindInstances();
  void FinalConnection();

public:
  bool     IsSucc()        {return mSucc;}
  unsigned GetStartToken() {return mStartToken;}
  RecTraInstance GetInstance() {return mInstance;}
  unsigned LongestMatch()  {return mSelf->LongestMatch();}
  void     AddAppealPoint(AppealNode *n) {mAppealPoints.PushBack(n);}

  RuleTable* GetRuleTable() {return mRuleTable;}

  void     SetTrace(bool b){mTrace = b;}
  void     SetIndentation(unsigned i) {mIndentation = i;}
  void     DumpIndentation();

  void AddVisitedLeadNode(RuleTable *rt) {mVisitedLeadNodes.PushBack(rt);}
  bool LeadNodeVisited(RuleTable *rt) {return mVisitedLeadNodes.Find(rt);}

  void AddVisitedRecursionNode(RuleTable *rt) {mVisitedRecursionNodes.PushBack(rt);}
  bool RecursionNodeVisited(RuleTable *rt) {return mVisitedRecursionNodes.Find(rt);}

  void AddLeadNode(AppealNode *n) {mLeadNodes.PushBack(n);}

public:
  RecursionTraversal(AppealNode *sel, AppealNode *parent, Parser *parser);
  ~RecursionTraversal();

  void Work();
  bool ConnectPrevious(AppealNode*);
};

}
#endif
