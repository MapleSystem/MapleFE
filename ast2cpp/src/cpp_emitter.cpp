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

#include "cpp_emitter.h"

namespace maplefe {

using namespace std::string_literals;

std::string CppEmitter::EmitModuleNode(ModuleNode *node) {
  if (node == nullptr)
    return std::string();
  std::string str("// Filename: "s);
  str += node->GetFileName() + "\n"s;
  //str += AstDump::GetEnumSrcLang(node->GetSrcLang());
  /*
  if (auto n = node->GetPackage()) {
    str += " "s + CppEmitPackageNode(n);
  }

  for (unsigned i = 0; i < node->GetImportsNum(); ++i) {
    if (i)
      str += ", "s;
    if (auto n = node->GetImport(i)) {
      str += " "s + CppEmitImportNode(n);
    }
  }
  */
  for (unsigned i = 0; i < node->GetTreesNum(); ++i) {
    if (auto n = node->GetTree(i)) {
      str += EmitTreeNode(n);
    }
  }
  return str;
}

} // namespace maplefe
