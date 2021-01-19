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

//////////////////////////////////////////////////////////////////////////////////////
//  This file defines all the supported information in Autogen.                     //
//  All information in this file is language independent.                           //
//////////////////////////////////////////////////////////////////////////////////////

#ifndef __ALL_SUPPORTED_H__
#define __ALL_SUPPORTED_H__

#include <string>
#include "supported.h"

namespace maplefe {

///////////////////////////////////////////////////////////////////////////////////
//                                  Type Info                                    //
///////////////////////////////////////////////////////////////////////////////////

// The types supported in autogen, shared with parser. this is language independent.
// The supported types and their name.
// The 'name' is used as known word in type.spec
struct TypeMapping {
  std::string mName;
  TypeId      mType;
};

extern TypeId FindTypeIdLangIndep(const std::string &s);
extern char *GetTypeString(TypeId tid);

///////////////////////////////////////////////////////////////////////////////////
//                             Attribute Info                                    //
///////////////////////////////////////////////////////////////////////////////////

// The attribute supported in autogen, shared with parser. this is language independent.
// The supported attributes and their name.
struct AttrMapping {
  std::string mName;
  AttrId      mId;
};

extern AttrId FindAttrId(const std::string &s); // from name to id
extern char *GetAttrString(AttrId tid);         // from id to name

///////////////////////////////////////////////////////////////////////////////////
//                                 Literal Info                                  //
// This is languange independent. This info is just about the LitId and     its  //
// names.                                                                        //
// This is the SUPER SET of all languages' literals.                             //
///////////////////////////////////////////////////////////////////////////////////

// The supported structure of literals and their name.
// The 'name' is used as known word in java/literal.spec
struct LiteralSuppStruct {
  std::string mName;
  LitId   mLiteralId;
};

extern LitId       FindLiteralId(const std::string &s);
extern std::string FindLiteralName(LitId id);
}
#endif
