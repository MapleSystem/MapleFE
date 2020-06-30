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
//  The data strcutures used during parsing left recursions are defined
//  in this file.
////////////////////////////////////////////////////////////////////////////

#ifndef __PARSER_REC_H__
#define __PARSER_REC_H__

#include "ruletable.h"

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

// The traverse of recursion will be handled in its own approach. It will be achieved
// through two phases. One is FindInstance, which tries to find on instance of a
// recursion that matches tokens. The other is ConnectInstance, which connects all
// successful instances to become an AppealTree.

class RecursionTraversal {
public:
  Parser     *mParser;
  Recursion  *mRec;
  RuleTable  *mRuleTable;
  AppealNode *mSelf;
  AppealNode *mParent;

  bool        mSucc;
  unsigned    mStartToken;
  unsigned    mLastToken;  // the last token this recursion matches.

  bool FindFirstInstance();
  bool FindNextInstance();

  bool FindInstances();
  void ConnectInstances();

public:
  RecursionTraversal(AppealNode *sel, AppealNode *parent, Parser *parser);
  ~RecursionTraversal();

  void Work();
};

#endif
