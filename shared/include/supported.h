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
//////////////////////////////////////////////////////////////////////////
// This file contains shared information system supported.
//////////////////////////////////////////////////////////////////////////

#ifndef __SUPPORTED_H__
#define __SUPPORTED_H__

namespace maplefe {

// The list of all supported types. This covers all the languages.
// NOTE: autogen also relies on this set of supported separators
#undef  TYPE
#undef  PRIMTYPE
#define TYPE(T) TY_##T,
#define PRIMTYPE(T) TY_##T,
typedef enum TypeId {
#include "supported_types.def"
TY_NA
}TypeId;

// The list of all supported separators. This covers all the languages.
// NOTE: autogen also relies on this set of supported separators
#undef  SEPARATOR
#define SEPARATOR(T) SEP_##T,
typedef enum SepId {
#include "supported_separators.def"
SEP_NA
}SepId;

// The list of all supported operators. This covers all the languages.
// NOTE: autogen also relies on this set of supported operators.
#undef  OPERATOR
#define OPERATOR(T, D) OPR_##T,
typedef enum OprId {
#include "supported_operators.def"
OPR_NA
}OprId;

#define LITERAL(T) LT_##T,
typedef enum LitId {
#include "supported_literals.def"
  LT_NA     // N/A, in java, Null is legal type with only one value 'null'
            // reference, a literal. So LT_Null is actually legal.
            // So I put LT_NA for the illegal literal
}LitId;


#undef ATTRIBUTE
#define ATTRIBUTE(X) ATTR_##X,
enum AttrId {
#include "supported_attributes.def"
ATTR_NA
};

// TODO: The action id will come from both the shared part and language specific part.
//       Some language may have its own special action to build AST.
//       For now I just put everything together in order to expediate the overall
//       progress. Will come back.

#undef  ACTION
#define ACTION(T) ACT_##T,
enum ActionId{
#include "supported_actions.def"
ACT_NA
};

}
#endif
