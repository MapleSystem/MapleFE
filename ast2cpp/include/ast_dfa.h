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

#ifndef __AST_DFA_HEADER__
#define __AST_DFA_HEADER__

#include <stack>
#include <utility>
#include "ast_module.h"
#include "ast.h"
#include "ast_type.h"
#include "ast_cfg.h"
#include "gen_astvisitor.h"

namespace maplefe {

class AST_DFA {
 private:
  AST_Module   *mModule;
  bool          mTrace;
  SmallVector<BitVector>  mReachDefIn;     // reaching definition bit vector entering bb

 public:
  explicit AST_DFA(AST_Module *m, bool t) : mModule(m), mTrace(t) {}
  ~AST_DFA() {}

  void  Build();

  void  SplitDecl();
  void  BuildReachDefIn();
  // unsigned GetDecl(VarNode *);

  void DumpReachDefIn();
};

class DeclSplitVisitor : public AstVisitor {
 private:
  AST_Module   *mModule;
  bool          mTrace;

  AST_Function *mCurrentFunction;
  AST_BB       *mCurrentBB;

 public:
  explicit DeclSplitVisitor(AST_Module *m, bool t, bool base = false)
    : mModule(m), mTrace(t), AstVisitor(t && base) {}
  ~DeclSplitVisitor() = default;

  void SetCurrentFunction(AST_Function *f) { mCurrentFunction = f; }
  void SetCurrentBB(AST_BB *b) { mCurrentBB = b; }

  DeclNode *VisitDeclNode(DeclNode *node);
};

class ReachDefInVisitor : public AstVisitor {
 private:
  AST_Module   *mModule;
  bool          mTrace;

  AST_Function *mCurrentFunction;
  AST_BB       *mCurrentBB;

 public:
  explicit ReachDefInVisitor(AST_Module *m, bool t, bool base = false)
    : mModule(m), mTrace(t), AstVisitor(t && base) {}
  ~ReachDefInVisitor() = default;

  void SetCurrentFunction(AST_Function *f) { mCurrentFunction = f; }
  void SetCurrentBB(AST_BB *b) { mCurrentBB = b; }

  DeclNode *VisitDeclNode(DeclNode *node);
};
}
#endif
