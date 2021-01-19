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

///////////////////////////////////////////////////////////////////////////////////////
// TokenGen generates the system token table, including operators, separators and
// keywords.
///////////////////////////////////////////////////////////////////////////////////////

#ifndef __TOKEN_GEN_H__
#define __TOKEN_GEN_H__

#include "base_struct.h"
#include "base_gen.h"
#include "supported.h"

namespace maplefe {

class TokenGen : public BaseGen {
public:
  TokenGen(const char *hfile, const char *cfile)
      : BaseGen("", hfile, cfile) {}
  ~TokenGen(){}

  void Generate();
  void GenCppFile();
  void GenHeaderFile();

  void Run(SPECParser *parser){return;}
};

}

#endif
