/*
* Copyright (C) [2020] Futurewei Technologies, Inc. All rights reverved.
*
* MapleFE is licensed under the Mulan PSL v2.
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
//                             LookAhead Detection
// Each rule has a limited set of tokens at the beginning. We traverse and find out
// these tokens. During parsing, it can expedite if we check the lookahead and skip
// if it doesn't match.
////////////////////////////////////////////////////////////////////////////////////

#ifndef __LA_DETECT_H__
#define __LA_DETECT_H__

#include "container.h"
#include "ruletable.h"
#include "write2file.h"

namespace maplefe {

// A mapping between Rule and the set of lookahead.
class RuleLookAhead {
public:
  RuleTable *mRule;
  SmallVector<LookAhead> mLookAheads;
public:
  RuleLookAhead() : mRule(NULL) {}
  RuleLookAhead(RuleTable *r) : mRule(r) {}
  ~RuleLookAhead() {Release();}

  bool FindLookAhead(LookAhead);
  void AddLookAhead(LookAhead);

  void Release() {mLookAheads.Release();}
};

// The rules which depend on a pending node. This happens when a succ
// node is the lead node of a recursion.
//
// [NOTE] It's user's duty to assure no duplicated dependents.

class Pending {
public:
  RuleTable  *mRule;   // The pending one
  SmallVector<RuleTable*> mDependents;  // Those depending on it
public:
  Pending() : mRule(NULL) {}
  Pending(RuleTable *r) : mRule(r) {}
  ~Pending() {mDependents.Release();}

  void AddDependent(RuleTable *rt) {mDependents.PushBack(rt);}

  void Release() {mDependents.Release();}
};

// Return result of most detect functions.
enum TResult {
  TRS_MaybeZero,
  TRS_NA
};

class LADetector {
public:
  SmallVector<RuleLookAhead*>  mRuleLookAheads;
  SmallVector<Pending*>        mPendings;

  SmallVector<RuleTable*> mInProcess;     // tables currently in process.
  SmallVector<RuleTable*> mDone;          // tables done.
  SmallList<RuleTable*>   mToDo;          // tables to be traversed.

  // We put rules into two categories: Fail, MaybeZero.
  SmallVector<RuleTable*> mMaybeZero;
  SmallVector<RuleTable*> mFail;

  ContTree<RuleTable*>    mTree;          // the spanning tree during each
                                          // traversal.

  bool IsInProcess(RuleTable*);
  bool IsDone(RuleTable*);
  bool IsToDo(RuleTable*);
  void AddToDo(RuleTable*);
  void AddToDo(RuleTable*, unsigned);

  bool IsMaybeZero(RuleTable*);
  bool IsFail(RuleTable*);

  void SetupTopTables();

  TResult DetectRuleTable(RuleTable*, ContTreeNode<RuleTable*>*);
  TResult DetectOneof(RuleTable*, ContTreeNode<RuleTable*>*);
  TResult DetectData(RuleTable*, ContTreeNode<RuleTable*>*);
  TResult DetectZeroorXXX(RuleTable*, ContTreeNode<RuleTable*>*);
  TResult DetectConcatenate(RuleTable*, ContTreeNode<RuleTable*>*);
  TResult DetectTableData(TableData*, ContTreeNode<RuleTable*>*);
  TResult DetectOneDataEntry(TableData*, RuleTable*, ContTreeNode<RuleTable*>*);

  Pending* GetPending(RuleTable *pending);

  void BackPatch();
  void PatchPending(Pending*);

  void AddPending(RuleTable *pending, RuleTable *dependent);
  void AddRuleLookAhead(RuleTable *rt, LookAhead la);
  void CopyRuleLookAhead(RuleTable *to, RuleTable *from);

  RuleLookAhead* GetRuleLookAhead(RuleTable*);
  RuleLookAhead* CreateRuleLookAhead(RuleTable*);

private:
  Write2File *mCppFile;
  Write2File *mHeaderFile;

  void WriteHeaderFile();
  void WriteCppFile();

public:
  LADetector() : mCppFile(NULL), mHeaderFile(NULL) {}
  ~LADetector(){Release();}

  void Detect();
  void Write();
  void Release();
};

extern MemPool gMemPool;

}
#endif
