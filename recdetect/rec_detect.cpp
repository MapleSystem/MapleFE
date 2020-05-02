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
bool RecDetector::IsProcessed(RuleTable *t) {
  for (unsigned i = 0; i < mProcessed.GetNum(); i++) {
    if (t == mProcessed.ValueAtIndex(i))
      return true;
  }
  return false;
}

void RecDetector::Detect(RuleTable *rule_table) {
  EntryType type = rule_table->mType;
  switch(type) {
  case ET_Oneof:
    DetectOneof(rule_table);
    break;
  case ET_Zeroormore:
    DetectZeroormore(rule_table);
    break;
  case ET_Zeroorone:
    DetectZeroorone(rule_table);
    break;
  case ET_Concatenate:
    DetectConcatenate(rule_table);
    break;
  case ET_Data:
    DetectTableData(rule_table->mData);
    break;
  case ET_Null:
  default:
    break;
  }
}

void RecDetector::DetectOneof(RuleTable *rule_table) {
}

void RecDetector::DetectZeroormore(RuleTable *rule_table) {
}

void RecDetector::DetectZeroorone(RuleTable *rule_table) {
}

void RecDetector::DetectConcatenate(RuleTable *rule_table) {
}

void RecDetector::DetectTableData(TableData *table_data) {
}

// We start from the top tables.
// Tables not accssible from top tables won't be handled.
void RecDetector::Detect() {
}

int main(int argc, char *argv[]) {
  RecDetector dtc;
  dtc.Detect();
  return 0;
}
