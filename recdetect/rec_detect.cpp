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

#include "common_header_autogen.h"
#include "rec_detect.h"

void RecDetector::SetupTopTables() {
  mTopTables.PushBack(&TblStatement);
  mTopTables.PushBack(&TblClassDeclaration);
  mTopTables.PushBack(&TblInterfaceDeclaration);
}

// A talbe is already been processed.
bool RecDetector::IsInProcess(RuleTable *t) {
  for (unsigned i = 0; i < mInProcess.GetNum(); i++) {
    if (t == mInProcess.ValueAtIndex(i))
      return true;
  }
  return false;
}

// A talbe is already done.
bool RecDetector::IsDone(RuleTable *t) {
  for (unsigned i = 0; i < mDone.GetNum(); i++) {
    if (t == mDone.ValueAtIndex(i))
      return true;
  }
  return false;
}

void RecDetector::AddRecursion(RuleTable *rt, ContTreeNode<RuleTable*> *p) {
}

void RecDetector::Detect(RuleTable *rt, ContTreeNode<RuleTable*> *p) {
  if (IsDone(rt))
    return;

  if (IsInProcess(rt)) {
    // Find a new circle.
    AddRecursion(rt, p);
  } else {
    mInProcess.PushBack(rt);
  }

  // Create new tree node.
  ContTreeNode<RuleTable*> *node = mTree.NewNode(rt, p);

  EntryType type = rt->mType;
  switch(type) {
  case ET_Oneof:
    DetectOneof(rt, node);
    break;
  case ET_Zeroormore:
    DetectZeroormore(rt, node);
    break;
  case ET_Zeroorone:
    DetectZeroorone(rt, node);
    break;
  case ET_Concatenate:
    DetectConcatenate(rt, node);
    break;
  case ET_Data:
    DetectTableData(rt->mData, node);
    break;
  case ET_Null:
  default:
    break;
  }
}

void RecDetector::DetectOneof(RuleTable *rule_table, ContTreeNode<RuleTable*> *p) {
}

void RecDetector::DetectZeroormore(RuleTable *rule_table, ContTreeNode<RuleTable*> *p) {
}

void RecDetector::DetectZeroorone(RuleTable *rule_table, ContTreeNode<RuleTable*> *p) {
}

void RecDetector::DetectConcatenate(RuleTable *rule_table, ContTreeNode<RuleTable*> *p) {
}

void RecDetector::DetectTableData(TableData *table_data, ContTreeNode<RuleTable*> *p) {
}

// We start from the top tables.
// Tables not accssible from top tables won't be handled.
void RecDetector::Detect() {
  for (unsigned i = 0; i < mTopTables.GetNum(); i++) {
    mInProcess.Clear();
    mDone.Clear();
    mTree.Clear();

    RuleTable *top = mTopTables.ValueAtIndex(i);
    Detect(top, NULL);
  }
}

int main(int argc, char *argv[]) {
  RecDetector dtc;
  dtc.Detect();
  return 0;
}
