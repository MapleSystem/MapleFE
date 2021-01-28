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

bool FEMIRBuilder::TraverseToNamedField(MIRStructType *structType, uint32 &fieldID, FieldData *fieldData) {
  if (!structType) {
    return false;
  }

  for (uint32 fieldidx = 0; fieldidx < structType->fields.size(); fieldidx++) {
    fieldID++;
    if (structType->fields[fieldidx].first == fieldData->GetStrIdx()) {
      fieldData->SetTyIdx(structType->fields[fieldidx].second.first);
      fieldData->SetFieldAttrs(structType->fields[fieldidx].second.second);
      // for pointer type, check their pointed type
      MIRType *type = GlobalTables::GetTypeTable().GetTypeFromTyIdx(fieldData->GetTyIdx());
      MIRType *typeit = GlobalTables::GetTypeTable().GetTypeFromTyIdx(structType->fields[fieldidx].second.first);
      if (IsOfSameType(type, typeit)) {
        return true;
      }
    }

    MIRType *fieldtype = GlobalTables::GetTypeTable().GetTypeFromTyIdx(structType->fields[fieldidx].second.first);
    if (fieldtype->typeKind == kTypeStruct || fieldtype->typeKind == kTypeStructIncomplete) {
      MIRStructType *substructtype = static_cast<MIRStructType *>(fieldtype);
      if (TraverseToNamedField(substructtype, fieldID, fieldData)) {
        return true;
      }
    } else if (fieldtype->typeKind == kTypeClass || fieldtype->typeKind == kTypeClassIncomplete) {
      MIRClassType *subclasstype = static_cast<MIRClassType *>(fieldtype);
      if (TraverseToNamedField(subclasstype, fieldID, fieldData)) {
        return true;
      }
    } else if (fieldtype->typeKind == kTypeInterface || fieldtype->typeKind == kTypeInterfaceIncomplete) {
      MIRInterfaceType *subinterfacetype = static_cast<MIRInterfaceType *>(fieldtype);
      if (TraverseToNamedField(subinterfacetype, fieldID, fieldData)) {
        return true;
      }
    }
  }
  return false;
}

}
