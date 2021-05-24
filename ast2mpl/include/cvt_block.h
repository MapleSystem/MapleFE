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

#ifndef __AST_CVT_BLOCK_H__
#define __AST_CVT_BLOCK_H__

#include "ast_module.h"
#include "ast.h"
#include "gen_astvisitor.h"

namespace maplefe {

// CvtBlockVisitor is to fix up some tree nodes after the AST is created
class CvtToBlockVisitor : public AstVisitor {
  private:
    ModuleNode *mASTModule;
    bool       mUpdated;

  public:
    CvtToBlockVisitor(ModuleNode *m) : mASTModule(m), mUpdated(false) {}

    bool CvtToBlock();

    CondBranchNode *VisitCondBranchNode(CondBranchNode *node);
    ForLoopNode *VisitForLoopNode(ForLoopNode *node);
};

}
#endif
