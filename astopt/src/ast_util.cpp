/*
* Copyright (C) [2021] Futurewei Technologies, Inc. All rights reverved.
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

#include "ast.h"
#include "ast_util.h"
#include "ast_handler.h"
#include "gen_astdump.h"

namespace maplefe {

#define CPPKEYWORD(K) stridx = gStringPool.GetStrIdx(#K); mCppKeywords.insert(stridx);
void AST_Util::BuildCppKeyWordSet() {
  unsigned stridx;
#include "cpp_keywords.def"
}

bool AST_Util::IsDirectField(TreeNode *node) {
  return mHandler->IsDirectField(node);
}

bool AST_Util::IsCppKeyWord(unsigned stridx) {
  return mCppKeywords.find(stridx) != mCppKeywords.end();
}

bool AST_Util::IsCppKeyWord(std::string name) {
  unsigned stridx = gStringPool.GetStrIdx(name);
  return mCppKeywords.find(stridx) != mCppKeywords.end();
}

// 
bool AST_Util::IsCppName(std::string name) {
  // check first char [a-z][A-Z]_
  char c = name[0];
  if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_')) {
    return false;
  }

  // check char [a-z][A-Z][0-9]_
  for (int i = 1; i < name.length(); i++) {
    char c = name[i];
    if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_')) {
      return false;
    }
  }

  return true;
}

bool AST_Util::IsCppField(TreeNode *node) {
  // check if it is a direct field
  if (!IsDirectField(node)) {
    return false;
  }

  // check if it's name is a C++ keyworld
  unsigned stridx = node->GetStrIdx();
  if (IsCppKeyWord(stridx)) {
    return false;
  }

  // check if it's name is a valid C++ name
  std::string name = gStringPool.GetStringFromStrIdx(stridx);
  if (!IsCppName(name)) {
    return false;
  }
  return true;
}

void AST_Util::SetTypeId(TreeNode *node, TypeId tid) {
  if (tid != TY_None && node && node->GetTypeId() != tid) {
    if (mFlags & FLG_trace_3) {
      std::cout << " NodeId : " << node->GetNodeId() << " Set TypeId : "
                << AstDump::GetEnumTypeId(node->GetTypeId()) << " --> "
                << AstDump::GetEnumTypeId(tid) << std::endl;
    }
    node->SetTypeId(tid);
  }
}

void AST_Util::SetTypeIdx(TreeNode *node, unsigned tidx) {
  if (node->GetTypeIdx() != tidx) {
    if (mFlags & FLG_trace_3) {
      std::cout << " NodeId : " << node->GetNodeId() << " Set TypeIdx : "
                << node->GetTypeIdx() << " --> " << tidx << std::endl;
    }
    node->SetTypeIdx(tidx);
  }
}

}
