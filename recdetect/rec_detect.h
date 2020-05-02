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
// 1. A circle is composed of a list of positions. Each position tells which rule elem
//    it goes into from current rule. So a list of unsigned integer make up the cirle.
// 2. Each elem in this circle is a rule by itself. It cannot be token.

// This defines one left recursion, and it's just one path of circle. A rule could
// have multiple recursions (cirle routes). We define one circle route as a Recursion.
class Recursion {
private:
  SmallVector<unsigned> mPositions;
public:
  Recursion(){}
  ~Recursion(){}
};

// All recursions of a rule.
class RuleRecursion {
private:
  SmallVector<Recursion*> mRecursions;
  RuleTable *mRuleTable;

public:
  RuleRecursion(){}
  ~RuleRecursion(){}
};

class RecDetector {
private:
  SmallVector<RuleRecursion*> mRuleRecursions;
  SmallVector<RuleTable*> mTopTables;     // top tables we start detection from.
  SmallVector<RuleTable*> mProcessed;     // tables we already processed.

  bool IsProcessed(RuleTable*);
  void SetupTopTables();

  void Detect(RuleTable*);
  void DetectOneof(RuleTable*);
  void DetectZeroormore(RuleTable*);
  void DetectZeroorone(RuleTable*);
  void DetectConcatenate(RuleTable*);
  void DetectTableData(TableData*);

public:
  RecDetector(){}
  ~RecDetector(){}

  void Detect();
};

#endif
