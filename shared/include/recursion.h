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

//////////////////////////////////////////////////////////////////////
//                    Left Recursion   Table                        //
//////////////////////////////////////////////////////////////////////

#ifndef __LEFT_REC_H__
#define __LEFT_REC_H__

#include "ruletable.h"
#include "container.h"

// For each rule it could have multiple left recursions. Each recursion is represented
// as a 'circle'. A 'circle' is simply an integer array, with the first element being the
// length of the array.

struct LeftRecursion {
  RuleTable *mRuleTable;
  unsigned   mNum;  // How many recursions
  unsigned **mCircles; // 
};

extern LeftRecursion **gLeftRecursions; //
extern unsigned gLeftRecursionsNum;  // total number of rule tables having recursion.

// Mapping from a rule table node to all the recursions it's part of. This is useful
// when parser traverse into a node, and wants to know if it's in a recursion.
// I'm using an array for temporary use. Will improve the searching if necessary later.

struct Rule2Recursion {
  RuleTable      *mRuleTable;
  unsigned        mNum;          // num of recursions it's involved.
  LeftRecursion **mRecursions;
};

extern Rule2Recursion **gRule2Recursion;
extern unsigned gRule2RecursionNum;

// This struct is named 'FronNode' since its mainly usage is the FronNode
// in LeftRecursion handling.
enum FronNodeType {
  FNT_Rule,    // a simple rule table as FronNode
  FNT_Token,   // a simple token as FronNode
  FNT_Concat   // this is coming from Concatenate node, please see comments in
               // FindFronNode() in parser_rec.cpp.
};

// If a FronNode is the ending parts of a concatenate, it actually contains multiple
// children nodes. we need know the starting index of the first child node.
//
// We also need know the information of corresponding Recursion node in the circle.
// We use mPos to save it. Later on we need this info to build a path in the Appeal Tree.
struct FronNode {
  unsigned     mPos;
  FronNodeType mType;
  union {
    RuleTable *mTable;
    Token     *mToken;
    unsigned   mStartIndex; // for concatenate FronNode
  }mData;
};

// This function will always get one single child element, either token
// or rule table. It won't take care of FNT_Concat.
extern FronNode RuleFindChildAtIndex(RuleTable *r, unsigned index);

// A single LeftRecursion
class Recursion {
public:
  RuleTable *mLeadNode;
  unsigned   mNum;       // num of circles
  unsigned **mCircles;   // circles

  SmallVector<RuleTable*>             mRecursionNodes;// nodes of all circles
  SmallVector<FronNode>               mLeadFronNodes;
  SmallVector<SmallVector<FronNode>*> mFronNodes;  // a set of vectors of mFronNodes
                                                   // in correspondant to each circle.
public:
  Recursion() {mNum = 0; mLeadNode = NULL; mCircles = NULL;}
  Recursion(LeftRecursion*);
  ~Recursion() {Release();}

  RuleTable* GetLeadNode() {return mLeadNode;}

  void Init(LeftRecursion*);
  void Release();

public:
  bool IsRecursionNode(RuleTable*);
  void FindRecursionNodes();
  void FindLeadFronNodes();
  void FindFronNodes();
  void FindFronNodes(unsigned circle_index);
};

// All LeftRecursions of the current language.
class RecursionAll {
private:
  SmallVector<Recursion*> mRecursions;
public:
  RecursionAll() {}
  ~RecursionAll() {Release();}

  void Init();
  void Release();

  Recursion* FindRecursion(RuleTable *lead);
  bool IsLeadNode(RuleTable*);
};

#endif
