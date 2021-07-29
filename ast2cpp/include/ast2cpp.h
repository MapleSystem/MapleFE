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

#ifndef __AST2CPP_HEADER__
#define __AST2CPP_HEADER__

#include "astopt.h"
#include "ast_handler.h"

namespace maplefe {

enum A2C_Flags {
  FLG_trace_a2c = 0x01,
  FLG_emit_ts_only = 0x10
};

class A2C : public AstOpt {
private:
  AST_Handler *mASTHandler;
  bool         mTraceA2C;
  bool         mEmitTSOnly;

public:
  explicit A2C(AST_Handler *h, unsigned flag) : AstOpt(h, flag & FLG_trace_a2c), mASTHandler(h) {
    mTraceA2C = (flag & FLG_trace_a2c);
    mEmitTSOnly = (flag & FLG_emit_ts_only) != 0;
  }
  ~A2C() = default;

  void SetTraceA2C(bool t) { mTraceA2C = t; }
  void EmitTS();
  void ProcessAST();
};

}
#endif
