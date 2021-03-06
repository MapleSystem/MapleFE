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

#ifndef __AST2MPL_HEADER__
#define __AST2MPL_HEADER__

#include "ast_module.h"
#include "ast.h"
#include "ast_type.h"

#include "mir_module.h"
#include "maplefe_mir_builder.h"

namespace maplefe {

#define NOTYETIMPL(K)      { if (mTraceA2m) { MNYI(K);      }}
#define AST2MPLMSG0(K)     { if (mTraceA2m) { MMSG0(K);     }}
#define AST2MPLMSG(K,v)    { if (mTraceA2m) { MMSG(K,v);    }}
#define AST2MPLMSG2(K,v,w) { if (mTraceA2m) { MMSG2(K,v,w); }}

enum StmtExprKind {
  SK_Null,
  SK_Stmt,
  SK_Expr
};

class A2M {
private:
  const char *mFileName;
  bool mTraceA2m;
  FEMIRBuilder *mMirBuilder;
  maple::MIRType *mDefaultType;
  FieldData *mFieldData;
public:
  maple::MIRModule *mMirModule;
  // use type's uniq name as key
  std::map<const char *, maple::MIRType*> mNodeTypeMap;
  std::map<const char *, std::vector<maple::MIRFunction*>> mNameFuncMap;
  std::map<BlockNode*, maple::BlockNode*> mBlockNodeMap;
  std::map<BlockNode*, maple::MIRFunction*> mBlockFuncMap;
  std::map<TreeNode*, maple::MIRFunction*> mFuncMap;
  std::map<std::pair<const char *, BlockNode*>, maple::MIRSymbol*> mNameBlockVarMap;

  A2M(const char *filename);
  ~A2M();

  void Init();
  bool IsStmt(TreeNode *tnode);

  void UpdateFuncName(maple::MIRFunction *func);

  virtual const char *Type2Label(const maple::MIRType *type);
  void Type2Name(std::string &str, const maple::MIRType *type);

  BlockNode *GetSuperBlock(BlockNode *block);
  maple::MIRSymbol *GetSymbol(TreeNode *tnode, BlockNode *block);
  maple::MIRSymbol *CreateSymbol(TreeNode *tnode, BlockNode *block);
  maple::MIRSymbol *CreateTempVar(const char *prefix, maple::MIRType *type);
  maple::MIRFunction *GetFunc(BlockNode *block);
  maple::MIRFunction *SearchFunc(const char *name, const maple::MapleVector<maple::BaseNode *> &args);
  maple::MIRClassType *GetClass(BlockNode *block);
  void UpdateUniqName(std::string &str);

  virtual maple::MIRType *MapPrimType(PrimTypeNode *tnode)=0;

  maple::MIRType *MapType(TreeNode *tnode);
  void MapAttr(maple::GenericAttrs &attr, const IdentifierNode *inode);

  maple::Opcode MapUnaOpcode(OprId);
  maple::Opcode MapBinOpcode(OprId);
  maple::Opcode MapBinCmpOpcode(OprId);
  maple::Opcode MapBinComboOpcode(OprId);

  void ProcessAST(bool trace_a2m);
  maple::BaseNode *ProcessNodeDecl(StmtExprKind, TreeNode *tnode, BlockNode *);
  maple::BaseNode *ProcessNode(StmtExprKind, TreeNode *tnode, BlockNode *);

  // Process different TreeNode kind while expecting StmtExprKind as stmt or expr
#undef  NODEKIND
#define NODEKIND(K) maple::BaseNode *Process##K(StmtExprKind, TreeNode*, BlockNode*);
#include "ast_nk.def"

  maple::BaseNode *ProcessClassDecl(StmtExprKind, TreeNode*, BlockNode*);
  maple::BaseNode *ProcessInterfaceDecl(StmtExprKind, TreeNode*, BlockNode*);
  maple::BaseNode *ProcessFieldDecl(StmtExprKind, TreeNode*, BlockNode*);
  maple::BaseNode *ProcessFuncDecl(StmtExprKind, TreeNode*, BlockNode*);
  maple::BaseNode *ProcessBlockDecl(StmtExprKind, TreeNode*, BlockNode*);

  maple::BaseNode *ProcessFuncSetup(StmtExprKind, TreeNode*, BlockNode*);

  maple::BaseNode *ProcessUnaOperatorMpl(StmtExprKind skind,
                                         maple::Opcode op,
                                         maple::BaseNode *bn,
                                         BlockNode *block);
  maple::BaseNode *ProcessBinOperatorMplAssign(StmtExprKind skind,
                                               maple::BaseNode *lhs,
                                               maple::BaseNode *rhs,
                                               BlockNode *block);
  maple::BaseNode *ProcessBinOperatorMplComboAssign(StmtExprKind skind,
                                                    maple::Opcode op,
                                                    maple::BaseNode *lhs,
                                                    maple::BaseNode *rhs,
                                                    BlockNode *block);
  maple::BaseNode *ProcessBinOperatorMplArror(StmtExprKind skind,
                                              maple::BaseNode *lhs,
                                              maple::BaseNode *rhs,
                                              BlockNode *block);

  maple::BaseNode *ProcessLoopCondBody(StmtExprKind skind, TreeNode *cond, TreeNode *body, BlockNode *block);
};

}
#endif
