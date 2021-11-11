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

#include <filesystem>
#include "a2c_util.h"
#include "gen_astdump.h"

namespace maplefe {

std::string ImportedFiles::GetTargetFilename(TreeNode *node) {
  std::string filename;
  if (node && node->IsLiteral()) {
    LiteralNode *lit = static_cast<LiteralNode *>(node);
    LitData data = lit->GetData();
    filename = AstDump::GetEnumLitData(data);
    filename += ".ts.ast"s;
    if(filename.front() != '/') {
      std::filesystem::path p = mModule->GetFilename();
      try {
        p = std::filesystem::canonical(p.parent_path() / filename);
        filename = p.string();
      }
      catch(std::filesystem::filesystem_error const& ex) {
        // Ignore std::filesystem::filesystem_error exception
        // keep filename without converting it to a cannonical path
      }
    }
  }
  return filename;
}

ImportNode *ImportedFiles::VisitImportNode(ImportNode *node) {
  std::string name = GetTargetFilename(node->GetTarget());
  if (!name.empty())
      mFilenames.push_back(name);
  for (unsigned i = 0; i < node->GetPairsNum(); ++i)
    if (auto x = node->GetPair(i); x->IsSingle()) {
      std::string s = GetTargetFilename(x->GetBefore());
      if (!s.empty())
        mFilenames.push_back(s);
    }
  return node;
}

ExportNode *ImportedFiles::VisitExportNode(ExportNode *node) {
  std::string name = GetTargetFilename(node->GetTarget());
  if (!name.empty())
      mFilenames.push_back(name);
  return node;
}
}
