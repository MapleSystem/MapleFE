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

#include "maplefe_mir_builder.h"

namespace maplefe {

bool FEMIRBuilder::TraverseToNamedField(maple::MIRStructType *structType, unsigned &fieldID, FieldData *fieldData) {
  if (!structType) {
    return false;
  }

  for (unsigned fieldidx = 0; fieldidx < structType->GetFieldsSize(); fieldidx++) {
    fieldID++;
    const maple::FieldPair fp = structType->GetFieldsElemt(fieldidx);
    if (structType->GetFieldsElemt(fieldidx).first == fieldData->GetStrIdx()) {
      fieldData->SetTyIdx(fp.second.first);
      fieldData->SetFieldAttrs(fp.second.second);
      // for pointer type, check their pointed type
      maple::MIRType *type = maple::GlobalTables::GetTypeTable().GetTypeFromTyIdx(fieldData->GetTyIdx());
      maple::MIRType *typeit = maple::GlobalTables::GetTypeTable().GetTypeFromTyIdx(fp.second.first);
      if (type->IsOfSameType(*typeit)) {
        return true;
      }
    }

    maple::MIRType *fieldtype = maple::GlobalTables::GetTypeTable().GetTypeFromTyIdx(fp.second.first);
    maple::MIRTypeKind kind = fieldtype->GetKind();
    if (kind == maple::kTypeStruct || kind == maple::kTypeStructIncomplete) {
      maple::MIRStructType *substructtype = static_cast<maple::MIRStructType *>(fieldtype);
      if (TraverseToNamedField(substructtype, fieldID, fieldData)) {
        return true;
      }
    } else if (kind == maple::kTypeClass || kind == maple::kTypeClassIncomplete) {
      maple::MIRClassType *subclasstype = static_cast<maple::MIRClassType *>(fieldtype);
      if (TraverseToNamedField(subclasstype, fieldID, fieldData)) {
        return true;
      }
    } else if (kind == maple::kTypeInterface || kind == maple::kTypeInterfaceIncomplete) {
      maple::MIRInterfaceType *subinterfacetype = static_cast<maple::MIRInterfaceType *>(fieldtype);
      if (TraverseToNamedField(subinterfacetype, fieldID, fieldData)) {
        return true;
      }
    }
  }
  return false;
}

maple::BaseNode *FEMIRBuilder::CreateExprDread(const maple::MIRSymbol *symbol, maple::FieldID fieldID) {
  maple::PrimType prim = symbol->GetType()->GetPrimType();
  maple::BaseNode *nd = new maple::AddrofNode(maple::OP_dread, prim, symbol->GetStIdx(), fieldID);
  return nd;
}

}
