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

////////////////////////////////////////////////////////////////////////////////////
//             This file defines the Left Recursion in the rules.
////////////////////////////////////////////////////////////////////////////////////

#ifndef __RECT_DETECT_H__
#define __RECT_DETECT_H__

#include "container.h"
#include "ruletable.h"

// There are some observations that help build the recursion data structure.
// 1. A path is composed of a list of positions. Each position tells which rule elem
//    it goes into from current rule. So a list of unsigned integer make up the cirle.
// 2. Each elem in this circle is a rule by itself. It cannot be token.

// This defines one left recursion, and it's just one path of circle. A rule could
// have multiple recursions. We define one circle route as a RecPath.
class RecPath {
private:
  SmallVector<unsigned> mPositions;
public:
  RecPath(){}
  ~RecPath(){ Release(); }

  void AddPos(unsigned i) { mPositions.PushBack(i); }
  void Release() {mPositions.Clear(); mPositions.Release();}
  void Dump();
};

// All recursions of a rule.
class Recursion {
private:
  SmallVector<RecPath*> mPaths;
  RuleTable            *mRuleTable;

public:
  Recursion(){}
  ~Recursion() {Release();}

  void SetRuleTable(RuleTable *t) {mRuleTable = t;}
  RuleTable* GetRuleTable() {return mRuleTable;}

  void AddPath(RecPath *p) {mPaths.PushBack(p);}
  void Release();
};

// Left Recursion Detector.
class RecDetector {
private:
  SmallVector<Recursion*> mRecursions;
  SmallVector<RuleTable*> mTopTables;     // top tables we start detection from.

  SmallVector<RuleTable*> mInProcess;     // tables currently in process.
  SmallVector<RuleTable*> mDone;          // tables done.
  ContTree<RuleTable*>    mTree;          // the traversing tree.

  bool IsInProcess(RuleTable*);
  bool IsDone(RuleTable*);

  void AddRecursion(RuleTable*, ContTreeNode<RuleTable*>*);
  Recursion* FindOrCreateRecursion(RuleTable*);

  void SetupTopTables();

  void DetectRuleTable(RuleTable*, ContTreeNode<RuleTable*>*);
  void DetectOneof(RuleTable*, ContTreeNode<RuleTable*>*);
  void DetectZeroormore(RuleTable*, ContTreeNode<RuleTable*>*);
  void DetectZeroorone(RuleTable*, ContTreeNode<RuleTable*>*);
  void DetectConcatenate(RuleTable*, ContTreeNode<RuleTable*>*);
  void DetectTableData(TableData*, ContTreeNode<RuleTable*>*);

public:
  RecDetector(){}
  ~RecDetector(){Release();}

  void Detect();
  void Release();
};

// We are using a single memory pool to allocate everything in the tool.
// The pool will free all memory automatically.
MemPool gMemPool;

#endif
