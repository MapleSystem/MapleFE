/*
* Copyright (C) [2020] Futurewei Technologies, Inc. All rights reverved.
*
* OpenArkFE is licensed under the Mulan PSL v1.
* You can use this software according to the terms and conditions of the Mulan PSL v1.
* You may obtain a copy of Mulan PSL v1 at:
*
*  http://license.coscl.org.cn/MulanPSL
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR
* FIT FOR A PARTICULAR PURPOSE.
* See the Mulan PSL v1 for more details.
*/
////////////////////////////////////////////////////////////////////////
//                     Literal Generation                             //
////////////////////////////////////////////////////////////////////////

#ifndef __LITERAL_GEN_H__
#define __LITERAL_GEN_H__

#include "base_struct.h"
#include "base_gen.h"
#include "rule.h"

class LiteralGen : public BaseGen {
public:
  LiteralGen(const char *dfile, const char *hfile, const char *cfile)
      : BaseGen(dfile, hfile, cfile) {}
  ~LiteralGen(){}

  void ProcessStructData(){}
  void Generate();
  void GenCppFile();
  void GenHeaderFile();
};

#endif
