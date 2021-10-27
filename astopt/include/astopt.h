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
//                This is the interface to translate AST to C++
//////////////////////////////////////////////////////////////////////////////////////////////

#ifndef __ASTOPT_HEADER__
#define __ASTOPT_HEADER__

#include "ast_module.h"
#include "ast.h"
#include "ast_type.h"

namespace maplefe {

class AST_Handler;

class AstOpt {
private:
  AST_Handler *mASTHandler;
  unsigned     mFlags;
  unsigned     mUniqNum;

public:
  explicit AstOpt(AST_Handler *h, unsigned f) :
    mASTHandler(h), mFlags(f) {}
  ~AstOpt() = default;

  AST_Handler *GetASTHandler() {return mASTHandler;}
  void ProcessAST(unsigned trace);

  void CollectInfo();
  void AdjustAST();
  void ScopeAnalysis();
  void BuildCFG();
  void ControlFlowAnalysis();
  void TypeInference();
  void DataFlowAnalysis();
};

}
#endif
