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

#ifndef __AST_AST_HEADER__
#define __AST_AST_HEADER__

#include <stack>
#include <utility>
#include "ast_module.h"
#include "ast.h"
#include "ast_type.h"
#include "gen_astvisitor.h"

namespace maplefe {

class AST_AST {
 private:
  AST_Handler  *mHandler;
  bool          mTrace;

 public:
  explicit AST_AST(AST_Handler *h, bool t) : mHandler(h), mTrace(t) {}
  ~AST_AST() {}

  void  Build();
  void  AdjustAST();
};

class AdjustASTVisitor : public AstVisitor {
 private:
  AST_Handler  *mHandler;
  bool          mTrace;

  AST_Function *mCurrentFunction;
  AST_BB       *mCurrentBB;

 public:
  explicit AdjustASTVisitor(AST_Handler *h, bool t, bool base = false)
    : mHandler(h), mTrace(t), AstVisitor(t && base) {}
  ~AdjustASTVisitor() = default;

  void SetCurrentFunction(AST_Function *f) { mCurrentFunction = f; }
  void SetCurrentBB(AST_BB *b) { mCurrentBB = b; }

  DeclNode *VisitDeclNode(DeclNode *node);
};

}
#endif
