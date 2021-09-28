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

#ifndef __A2C_UTIL_HEADER__
#define __A2C_UTIL_HEADER__

#include <vector>
#include <string>
#include "gen_astvisitor.h"

namespace maplefe {

// A helper function to get the target filename of an ImportNode
std::string GetTargetFilename(ImportNode *node);

//  To collect all filenames for imported modules
class ImportedFiles : public AstVisitor {
  public:
    std::vector<std::string> mFilenames;
  public:
    ImportNode *VisitImportNode(ImportNode *node);
};

}
#endif
