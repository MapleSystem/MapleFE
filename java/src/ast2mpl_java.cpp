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

#include "ast2mpl_java.h"

namespace maplefe {

MIRType *A2MJava::MapPrimType(PrimTypeNode *ptnode) {
  PrimType prim;
  switch (ptnode->GetPrimType()) {
    case TY_Boolean: prim = PTY_u1; break;
    case TY_Byte:    prim = PTY_u8; break;
    case TY_Short:   prim = PTY_i16; break;
    case TY_Int:     prim = PTY_i32; break;
    case TY_Long:    prim = PTY_i64; break;
    case TY_Char:    prim = PTY_u16; break;
    case TY_Float:   prim = PTY_f32; break;
    case TY_Double:  prim = PTY_f64; break;
    case TY_Void:    prim = PTY_void; break;
    case TY_Null:    prim = PTY_void; break;
    default: MASSERT("Unsupported PrimType"); break;
  }

  TyIdx tid(prim);
  return GlobalTables::GetTypeTable().GetTypeFromTyIdx(tid);
}
}
