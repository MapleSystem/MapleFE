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
//////////////////////////////////////////////////////////////////////////////////////
//  This file defines all the supported information in Autogen.                     //
//  All information in this file is language independent.                           //
//////////////////////////////////////////////////////////////////////////////////////

#ifndef __ALL_SUPPORTED_H__
#define __ALL_SUPPORTED_H__

#include <string>
#include "supported.h"

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
//                                 Literal Info                                  //
// This is languange independent. This info is just about the LiteralId and its  //
// names.                                                                        //
// This is the SUPER SET of all languages' literals.                             //
///////////////////////////////////////////////////////////////////////////////////

// The separators supported in autogen, this is language independent.
#undef  LITERAL
#define LITERAL(T) LIT_##T,
enum LiteralId {
#include "supported_literals.def"
LIT_NA   // LIT_Null is legal literal, so I put LIT_NA for illegal
};

// The supported structure of literals and their name.
// The 'name' is used as known word in java/literal.spec
struct LiteralSuppStruct {
  std::string mName;
  LiteralId   mLiteralId;
};

extern LiteralId   FindLiteralId(const std::string &s);
extern std::string FindLiteralName(LiteralId id);

#endif
