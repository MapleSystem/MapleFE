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

#include <string>
#include "ast.h"
#include "vfy_log.h"

namespace maplefe {

static const char* GetNodeTypeName(TreeNode *tree) {
#undef  NODEKIND
#define NODEKIND(K) if (tree->Is##K()) return #K;
#include "ast_nk.def"
}

void VfyLog::Duplicate(const char *desc, TreeNode *na, TreeNode *nb) {
  std::string name_a = GetNodeTypeName(na);
  name_a += ":";
  name_a += na->GetName();

  std::string name_b = GetNodeTypeName(nb);
  name_b += ":";
  name_b += nb->GetName();

  std::string result(desc);
  result += name_a;
  result += " is duplicated with ";
  result += name_b;

  const char *addr = mPool.FindString(result);
  mEntries.PushBack(addr);
}

void VfyLog::MissDecl(TreeNode *na) {
  std::string name_a = GetNodeTypeName(na);
  name_a += ":";
  name_a += na->GetName();

  std::string result(name_a);
  result += " has no decl.";

  const char *addr = mPool.FindString(result);
  mEntries.PushBack(addr);
}

void VfyLog::Dump() {
  for (unsigned i = 0; i < mEntries.GetNum(); i++) {
    std::cout << mEntries.ValueAtIndex(i) << std::endl;
  }
}
}
