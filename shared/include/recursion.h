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

// For each rule it could have multiple left recursions. Each recursion is represented
// as a 'path'. A 'path' is simply an integer array, with the first element being the
// length of the array.
struct LeftRecursion {
  RuleTable *mRuleTable;
  unsigned   mNum;  // How many recursions
  unsigned **mPaths; // 
};

extern LeftRecursion **gLeftRecursions; //
extern unsigned gLeftRecursionsNum;  // total number of rule tables having recursion.

#endif
