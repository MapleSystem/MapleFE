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

#include "a2c_util.h"
#include "gen_astdump.h"

namespace maplefe {

std::string GetTargetFilename(ImportNode *node) {
  std::string filename;
  if (auto n = node->GetTarget()) {
    if (n->IsLiteral()) {
      LiteralNode *lit = static_cast<LiteralNode *>(n);
      LitData data = lit->GetData();
      filename = AstDump::GetEnumLitData(data);
      filename += ".ts.ast"s;
    }
  }
  return filename;
}

ImportNode *ImportedFiles::VisitImportNode(ImportNode *node) {
  std::string name = GetTargetFilename(node);
  if (!name.empty())
      mFilenames.push_back(name);
  return node;
}
}
