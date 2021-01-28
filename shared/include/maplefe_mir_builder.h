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
  GStrIdx strIdx;
  TyIdx tyIdx;
  FieldAttrs attr;

 public:
  FieldData() : strIdx(0), tyIdx(0), attr() {}
  FieldData(GStrIdx idx) : strIdx(idx), tyIdx(0), attr() {}
  ~FieldData() {}

  void SetStrIdx(GStrIdx s) {
    strIdx.SetIdx(s.GetIdx());
  }

  void SetTyIdx(TyIdx t) {
    tyIdx.SetIdx(t.GetIdx());
  }

  void SetFieldAttrs(FieldAttrs a) {
    attr = a;
  }

  GStrIdx GetStrIdx() {
    return strIdx;
  }

  TyIdx GetTyIdx() {
    return tyIdx;
  }

  FieldAttrs GetFieldAttrs() {
    return attr;
  }

  void Clear() {
    strIdx.SetIdx(0);
    tyIdx.SetIdx(0);
    attr.Clear();
  }

  void ResetStrIdx(GStrIdx s) {
    strIdx.SetIdx(s.GetIdx());
    tyIdx.SetIdx(0);
    attr.Clear();
  }
};

class FEMIRBuilder : public MIRBuilder {
 private:

 public:
  FEMIRBuilder(MIRModule *mod) : MIRBuilder(mod) {}

  bool TraverseToNamedField(MIRStructType *structType, uint32 &fieldID, FieldData *fieldData);
};

}
#endif
