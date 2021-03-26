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

maple::PrimType A2MJava::MapPrim(TypeId id) {
  maple::PrimType prim;
  switch (id) {
    case TY_Boolean: prim = maple::PTY_u1; break;
    case TY_Byte:    prim = maple::PTY_u8; break;
    case TY_Short:   prim = maple::PTY_i16; break;
    case TY_Int:     prim = maple::PTY_i32; break;
    case TY_Long:    prim = maple::PTY_i64; break;
    case TY_Char:    prim = maple::PTY_u16; break;
    case TY_Float:   prim = maple::PTY_f32; break;
    case TY_Double:  prim = maple::PTY_f64; break;
    case TY_Void:    prim = maple::PTY_void; break;
    case TY_Null:    prim = maple::PTY_void; break;
    default: MASSERT("Unsupported PrimType"); break;
  }
  return prim;
}

maple::MIRType *A2MJava::MapPrimType(TypeId id) {
  maple::PrimType prim = MapPrim(id);
  maple::TyIdx tid(prim);
  return maple::GlobalTables::GetTypeTable().GetTypeFromTyIdx(tid);
}

maple::MIRType *A2MJava::MapPrimType(PrimTypeNode *ptnode) {
  return MapPrimType(ptnode->GetPrimType());
}

const char *A2MJava::Type2Label(const maple::MIRType *type) {
  maple::PrimType pty = type->GetPrimType();
  switch (pty) {
    case maple::PTY_u1:   return "Z";
    case maple::PTY_u8:   return "B";
    case maple::PTY_i16:  return "S";
    case maple::PTY_u16:  return "C";
    case maple::PTY_i32:  return "I";
    case maple::PTY_i64:  return "J";
    case maple::PTY_f32:  return "F";
    case maple::PTY_f64:  return "D";
    case maple::PTY_void: return "V";
    default:       return "L";
  }
}

}
