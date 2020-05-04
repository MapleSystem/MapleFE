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


void RecDetector::Detect(RuleTable *rule_table, ContTreeNode<RuleTable*> *parent) {
  if (IsDone(rule_table))
    return;

  if (IsInProcess(rule_table)) {
    // Find a new circle.
  } else {
    mInProcess.PushBack(rule_table);
  }

  // Create new tree node.
  ContTreeNode<RuleTable*> *node = mTree.NewNode(rule_table, parent);

  EntryType type = rule_table->mType;
  switch(type) {
  case ET_Oneof:
    DetectOneof(rule_table, node);
    break;
  case ET_Zeroormore:
    DetectZeroormore(rule_table, node);
    break;
  case ET_Zeroorone:
    DetectZeroorone(rule_table, node);
    break;
  case ET_Concatenate:
    DetectConcatenate(rule_table, node);
    break;
  case ET_Data:
    DetectTableData(rule_table->mData, node);
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
