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

#define NOTYETIMPL(K)      { if (mTraceA2c) { MNYI(K);      }}
#define AST2CPPMSG0(K)     { if (mTraceA2c) { MMSG0(K);     }}
#define AST2CPPMSG(K,v)    { if (mTraceA2c) { MMSG(K,v);    }}
#define AST2CPPMSG2(K,v,w) { if (mTraceA2c) { MMSG2(K,v,w); }}

class AstOpt {
private:
  const char *mFileName;
  bool mTraceAstOpt;
  unsigned mUniqNum;

public:
  explicit AstOpt(const char *filename) : mFileName(filename) {}
  ~AstOpt() = default;

  void ProcessAST(bool trace_opt);
};

}
#endif
