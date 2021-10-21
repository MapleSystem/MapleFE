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

//////////////////////////////////////////////////////////////////////////////////////////////
//                This is the interface to translate AST to C++
//////////////////////////////////////////////////////////////////////////////////////////////

#ifndef __AST_UTIL_HEADER__
#define __AST_UTIL_HEADER__

#include <unordered_set>

namespace maplefe {

class Module_Handler;

class AST_Util {
 private:
  Module_Handler *mHandler;

  std::unordered_set<unsigned> mCppKeywords;

  void BuildCppKeyWordSet();

 public:
  explicit AST_Util(Module_Handler *h) : mHandler(h) {
    BuildCppKeyWordSet();
  }
  ~AST_Util() {}

  bool IsDirectField(TreeNode *node);
  bool IsCppKeyWord(unsigned stridx);
  bool IsCppKeyWord(std::string name);
  bool IsCppName(std::string name);
  bool IsCppField(TreeNode *node);
};

}
#endif
