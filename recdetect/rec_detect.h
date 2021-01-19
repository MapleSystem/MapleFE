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

////////////////////////////////////////////////////////////////////////////////////
//             This file defines the Left Recursion in the rules.
////////////////////////////////////////////////////////////////////////////////////

#ifndef __RECT_DETECT_H__
#define __RECT_DETECT_H__

#include "container.h"
#include "ruletable.h"
#include "write2file.h"

namespace maplefe {

// There are some observations that help build the recursion data structure.
// 1. A path is composed of a list of positions. Each position tells which rule elem
//    it goes into from current rule. So a list of unsigned integer make up the cirle.
// 2. Each elem in this circle is a rule by itself. It cannot be token.

// This defines one left recursion, and it's just one path. A rule could
// have multiple recursions. We define one recursion route as a RecPath.
//
// Take a look at an example to understand the meaning of position vector.
// rule PackageOrTypeName : Oneof ( xxxx
//                                  PackageOrTypeName + xxx + xxx)
// The recusion will look like below.
//   PackageOrTypeName <----------
//          |                     |
//          |                     |
//       PackageOrTypeName_sub1 ---
//
// So here is the sequence of position, ie. index of each child in its parent.
// The Path of the above diagram will be <1, 1>. The first '1' means the index of
// PackageOrTypeName_sub1 in PackageOrTypeName, remember index starts from 0. The
// second '1' means the index of PackageOrTypeName in PackageOrTypeName_sub1.
//
// [NOTE] The last index in path is always telling the back edge.

class RecPath {
private:
  SmallVector<unsigned> mPositions;
public:
  RecPath(){}
  ~RecPath(){ Release(); }

  unsigned PositionsNum() {return mPositions.GetNum();}
  unsigned GetPosition(unsigned i) {return mPositions.ValueAtIndex(i);}

  void AddPos(unsigned i) { mPositions.PushBack(i); }

  void Release() {mPositions.Clear(); mPositions.Release();}

  void Dump();
  std::string DumpToString();
};

// All recursions of a rule.
class Recursion {
public:
  SmallVector<RecPath*>   mPaths;
  RuleTable              *mLead;
  SmallVector<RuleTable*> mRuleTables;

public:
  Recursion(){}
  ~Recursion() {Release();}

  void SetLead(RuleTable *t) {mLead = t;}
  RuleTable* GetLead() {return mLead;}

  void AddRuleTable(RuleTable *rt);
  bool HaveRuleTable(RuleTable *rt){return mRuleTables.Find(rt);}

  unsigned PathsNum() {return mPaths.GetNum();}
  RecPath* GetPath(unsigned i) {return mPaths.ValueAtIndex(i);}

  void AddPath(RecPath *p) {mPaths.PushBack(p);}
  void Release();
};

// RuleTable to Recursion Mapping.
// A rule table could be involved in multiple recursions. This information is
// needed in the parser. The LeadNode of a recursion is also counted a mapping
// to its own recursion.
class Rule2Recursion {
public:
  RuleTable *mRule;
  SmallVector<Recursion*> mRecursions;
public:
  Rule2Recursion() {mRule = NULL;}
  ~Rule2Recursion() {Release();}

  void AddRecursion(Recursion *rec);
  void Release() {mRecursions.Release();}
};

// The traversal result of a rule table. The details can be found
// in the comment of DetectConcatenate().
enum TResult {
  TRS_MaybeZero,
  TRS_Fail,       // Failed for Left Recursive.
  TRS_Done,       // This is special one since the rule is done before, we don't
                  // know the real status, but we can find it out by check IsFail()
                  // and IsMaybeZero().
  TRS_NA          // Simply used as the initial status.
};

// Recursion Group: Every recursion can reach to any other recursion in the same group.
// A group could contains only one recursion.
class RecursionGroup {
public:
  RecursionGroup() {}
  ~RecursionGroup(){}
  SmallVector<Recursion*> mRecursions;
  void AddRecursion(Recursion *r) {mRecursions.PushBack(r);}
  void Release() {mRecursions.Release();}
};

// RuleTable 2 RecursionGroup mapping.
// A rule can only map to one single group. But a rule can maps to multiple recursions.
struct Rule2Group{
  RuleTable      *mRuleTable;
  RecursionGroup *mGroup;
};

// Left Recursion Detector.
class RecDetector {
private:
  SmallVector<Recursion*> mRecursions;

  SmallVector<RuleTable*> mInProcess;     // tables currently in process.
  SmallVector<RuleTable*> mDone;          // tables done.
  SmallList<RuleTable*>   mToDo;          // tables to be traversed.

  // We put rules into two categories: Fail, MaybeZero.
  SmallVector<RuleTable*> mMaybeZero;
  SmallVector<RuleTable*> mFail;

  ContTree<RuleTable*>    mTree;          // the traversing tree.

  bool                    mChanged;       // Used in the backpatch process

  bool IsInProcess(RuleTable*);
  bool IsDone(RuleTable*);
  bool IsToDo(RuleTable*);
  void AddToDo(RuleTable*);
  void AddToDo(RuleTable*, unsigned);

  bool IsMaybeZero(RuleTable*);
  bool IsFail(RuleTable*);

  void AddRecursion(RuleTable*, ContTreeNode<RuleTable*>*);
  Recursion* FindOrCreateRecursion(RuleTable*);

  void SetupTopTables();

  TResult DetectRuleTable(RuleTable*, ContTreeNode<RuleTable*>*);
  TResult DetectOneof(RuleTable*, ContTreeNode<RuleTable*>*);
  TResult DetectData(RuleTable*, ContTreeNode<RuleTable*>*);
  TResult DetectZeroorXXX(RuleTable*, ContTreeNode<RuleTable*>*);
  TResult DetectConcatenate(RuleTable*, ContTreeNode<RuleTable*>*);
  TResult DetectTableData(TableData*, ContTreeNode<RuleTable*>*);
  void BackPatch(RuleTable*);

  // rule to recursion mapping.
  SmallVector<Rule2Recursion*> mRule2Recursions;
  Rule2Recursion* FindRule2Recursion(RuleTable *);
  void AddRule2Recursion(RuleTable*, Recursion*);
  void WriteRule2Recursion();
  void HandleIsDoneRuleTable(RuleTable *rt, ContTreeNode<RuleTable*> *p);

  // recursion group
  SmallVector<RecursionGroup*> mRecursionGroups;
  bool LRReachable(RuleTable *from, RuleTable *to);
  RecursionGroup* FindRecursionGroup(Recursion*);
  void WriteRecursionGroups();

  // rule to group mapping
  SmallVector<Rule2Group> mRule2Group;
  void AddRule2Group(RuleTable*, RecursionGroup*);
  void WriteRule2Group();
  void WriteGroup2Rule();

private:
  Write2File *mCppFile;
  Write2File *mHeaderFile;

  void WriteHeaderFile();
  void WriteCppFile();

public:
  RecDetector() : mCppFile(NULL), mHeaderFile(NULL) {}
  ~RecDetector(){Release();}

  void Detect();
  void DetectGroups();
  void Write();
  void Release();
};

// We are using a single memory pool to allocate everything in the tool.
// The pool will free all memory automatically.
MemPool gMemPool;

}
#endif
