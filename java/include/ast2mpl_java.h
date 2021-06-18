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

/////////////////////////////////////////////////////////////////////////////////
//                   Java Specific AST2MPL                                     //
/////////////////////////////////////////////////////////////////////////////////

#ifndef __AST2MPL_JAVA_H__
#define __AST2MPL_JAVA_H__

#include "ast2mpl.h"

namespace maplefe {

class A2MJava : public A2M {
private:
public:
  A2MJava(ModuleNode *m) : A2M(m) { }

  const char *Type2Label(const maple::MIRType *type);

  maple::PrimType MapPrim(TypeId id);
  maple::MIRType *MapPrimType(TypeId id);
  maple::MIRType *MapPrimType(PrimTypeNode *ptnode);
};

}
#endif
