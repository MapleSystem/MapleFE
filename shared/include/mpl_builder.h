/*
* Copyright (C) [2020] Futurewei Technologies, Inc. All rights reverved.
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
//                This is the interface to translate AST to MapleIR.
//////////////////////////////////////////////////////////////////////////////////////////////

#ifndef __MPL_BUILDER_H__
#define __MPL_BUILDER_H__

#include "ast2mpl.h"

#include "mir_module.h"
#include "mir_builder.h"

namespace maplefe {

#define NOTYETIMPL(K) { if (mTraceA2m) { MNYI(K); }}

enum StmtExprKind {
  SK_Null,
  SK_Stmt,
  SK_Expr
};

class MplBuilder {
private:
  const char *mFileName;
  bool mTraceA2m;
  maple::MIRBuilder *mMirBuilder;
  maple::MIRType *mDefaultType;
public:
  maple::MIRModule *mMirModule;
  maplefe::A2M *mA2M;

  MplBuilder(const char *filename);
  ~MplBuilder();

  void ProcessAST(bool trace_a2m);
  maple::BaseNode *ProcessNode(StmtExprKind, TreeNode *tnode, maple::BlockNode *);

  // Process different TreeNode kind while expecting StmtExprKind as stmt or expr
#undef  NODEKIND
#define NODEKIND(K) maple::BaseNode *Process##K(StmtExprKind, TreeNode *, maple::BlockNode *);
#include "ast_nk.def"

  maple::BaseNode *ProcessBinOperatorMpl(StmtExprKind skind,
                                         maple::Opcode op,
                                         maple::BaseNode *lhs,
                                         maple::BaseNode *rhs,
                                         maple::BlockNode *block);
  maple::BaseNode *ProcessBinOperatorMplAssign(StmtExprKind skind,
                                               maple::BaseNode *lhs,
                                               maple::BaseNode *rhs,
                                               maple::BlockNode *block);
  maple::BaseNode *ProcessBinOperatorMplComboAssign(StmtExprKind skind,
                                                    maple::Opcode op,
                                                    maple::BaseNode *lhs,
                                                    maple::BaseNode *rhs,
                                                    maple::BlockNode *block);
  maple::BaseNode *ProcessBinOperatorMplArror(StmtExprKind skind,
                                              maple::BaseNode *lhs,
                                              maple::BaseNode *rhs,
                                              maple::BlockNode *block);

};

}
#endif
