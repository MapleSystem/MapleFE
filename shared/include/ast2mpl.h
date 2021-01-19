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
#include "mir_builder.h"

namespace maplefe {

class A2M {
private:
  const char *mFileName;
  bool mTraceA2m;
public:
  maple::MIRModule *mMirModule;
  std::map<TreeNode*, MIRType *> mNodeTypeMap;

  A2M(const char *filename);
  ~A2M();

  void ProcessAST(bool trace_a2m);

#undef  NODEKIND
#define NODEKIND(K) void Process##K(TreeNode *);
#include "ast_nk.def"

   virtual MIRType *MapPrimType(PrimTypeNode *tnode)=0;

   MIRType *MapType(TreeNode *tnode);
   void MapAttr(GenericAttrs &attr, const IdentifierNode *inode);
};

}
#endif
