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

#ifndef __VFY_LOG_HEADER__
#define __VFY_LOG_HEADER__

#include "stringpool.h"
#include "container.h"

class TreeNode;

class VfyLog {
private:
  StringPool         mPool;
  SmallVector<char*> mEntries;
public:
  VfyLog() {}
  ~VfyLog(){mEntries.Release();}

  void Duplicate(const char *desc1, TreeNode *n1,
                 const char *desc2, TreeNode *n2);

  void Dump();
};

#endif
