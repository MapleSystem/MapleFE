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

#ifndef __A2C_CFG_HEADER__
#define __A2C_CFG_HEADER__

#include "ast_module.h"
#include "ast.h"
#include "ast_type.h"
#include "gen_astvisitor.h"

namespace maplefe {

  class CfgVisitor : public AstVisitor {
    private:
      bool mTrace;
    public:

      explicit CfgVisitor(bool t, bool base = false) : mTrace(t), AstVisitor(t && base) {}
      ~CfgVisitor() = default;

      FunctionNode *VisitFunctionNode(FunctionNode *node);
  };

  class A2C_CFG {
    private:
      ASTModule *mModule;
      bool mTraceCFG;

    public:
      explicit A2C_CFG(ASTModule *module, bool trace) : mModule(module), mTraceCFG(trace) {}
      ~A2C_CFG() = default;

      void BuildCFG();
      ASTModule* GetModule() { return mModule; }
      bool GetTraceCFG() { return mTraceCFG; }
      void Dump();
  };

}
#endif
