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

#ifndef __VFY_LOG_HEADER__
#define __VFY_LOG_HEADER__

#include "stringpool.h"
#include "container.h"

namespace maplefe {

class TreeNode;

// We allow the verification to be done through the whole module. So there could be
// multiple error/warning messages. We use VfyLog to record them and dump them once
// the whole module is done verification.

class VfyLog {
private:
  StringPool         mPool;
  SmallVector<const char*> mEntries;
public:
  VfyLog() {}
  ~VfyLog(){mEntries.Release();}

  void Duplicate(const char *desc, TreeNode *n1, TreeNode *n2);
  void MissDecl(TreeNode *n);

  void Dump();
};

}
#endif
