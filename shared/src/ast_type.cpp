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

#include "ast_type.h"
#include "gen_type.h"   // to include the type keyword table, language specific
#include "ast.h"

static const char* FindPrimTypeName(TypeId id) {
  for (unsigned i = 0; i < TY_NA; i++) {
    if (TypeKeywordTable[i].mId == id)
      return TypeKeywordTable[i].mText;
  }
  return NULL;
}

const char* ASTType::GetName() {
  const char *name = NULL;
  if (IsPrim()) {
    // Get name from keyword table
    name = FindPrimTypeName(GetPrimType());
  } else if (IsUser()) {
    // Get name from Identifier stringpool
    IdentifierNode *node = GetIdentifier();
    name = node->GetName();
  }

  MASSERT(name && "Could not find prim type name!");
  return name;
}
