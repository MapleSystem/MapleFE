/*
* Copyright (CtNode) [2020] Futurewei Technologies, Inc. All rights reverved.
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

#ifndef __MAPLEFE_MIR_BUILDER_H__
#define __MAPLEFE_MIR_BUILDER_H__

#include "mir_module.h"
#include "mir_builder.h"

namespace maplefe {

class FieldData {
 private:
   maple::GStrIdx strIdx;
   maple::TyIdx tyIdx;
   maple::FieldAttrs attr;

 public:
  FieldData() : strIdx(0), tyIdx(0), attr() {}
  FieldData(maple::GStrIdx idx) : strIdx(idx), tyIdx(0), attr() {}
  ~FieldData() {}

  void SetStrIdx(maple::GStrIdx s) {
    strIdx.SetIdx(s.GetIdx());
  }

  void SetTyIdx(maple::TyIdx t) {
    tyIdx.SetIdx(t.GetIdx());
  }

  void SetFieldAttrs(maple::FieldAttrs a) {
    attr = a;
  }

  maple::GStrIdx GetStrIdx() {
    return strIdx;
  }

  maple::TyIdx GetTyIdx() {
    return tyIdx;
  }

  maple::FieldAttrs GetFieldAttrs() {
    return attr;
  }

  void Clear() {
    strIdx.SetIdx(0);
    tyIdx.SetIdx(0);
    attr.Clear();
  }

  void ResetStrIdx(maple::GStrIdx s) {
    strIdx.SetIdx(s.GetIdx());
    tyIdx.SetIdx(0);
    attr.Clear();
  }
};

class FEMIRBuilder : public maple::MIRBuilder {
 private:

 public:
  FEMIRBuilder(maple::MIRModule *mod) : maple::MIRBuilder(mod) {}

  bool TraverseToNamedField(maple::MIRStructType *structType, unsigned &fieldID, FieldData *fieldData);
  maple::BaseNode *CreateExprDread(const maple::MIRSymbol *symbol, maple::FieldID fieldID = maple::FieldID(0));

  // use maple::PTY_ref for pointer types
  maple::MIRType *GetOrCreatePointerType(const maple::MIRType *pointTo) {
    return maple::GlobalTables::GetTypeTable().GetOrCreatePointerType(*pointTo, maple::PTY_ref);
  }
};

}
#endif
