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

#ifndef __AST_FIXUP_H__
#define __AST_FIXUP_H__

#include "ast_module.h"
#include "ast.h"
#include "gen_astvisitor.h"

namespace maplefe {

// FixUpVisitor is to fix up some tree nodes after the AST is created
class FixUpVisitor : public AstVisitor {
  private:
    ModuleNode *mASTModule;
    bool       mUpdated;

  public:
    FixUpVisitor(ModuleNode *m) : mASTModule(m), mUpdated(false) {}

    bool FixUp();

    // Fix up OprId of a UnaOperatorNode
    //   OPR_Add              --> OPR_Plus
    //   OPR_Sub              --> OPR_Minus
    //   OPR_Inc && !IsPost() --> OPR_PreInc
    //   OPR_Dec && !IsPost() --> OPR_DecInc
    //
    UnaOperatorNode *VisitUnaOperatorNode(UnaOperatorNode *node);

    // Fix up literal boolean 'true' or 'false' as a type
    UserTypeNode *VisitUserTypeNode(UserTypeNode *node);

    // Fix up the name string of a UserTypeNode
    // Fix up literal boolean 'true' or 'false'
    IdentifierNode *VisitIdentifierNode(IdentifierNode *node);

    // Update mFilename of a ModuleNode with a connonical absolute path
    ModuleNode *VisitModuleNode(ModuleNode *node);
};

}
#endif
