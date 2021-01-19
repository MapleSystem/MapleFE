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
// This file contains all the functions that are used when we traverse the
// rule tables. Most of the functions are designed to do the following
// 1. Validity check
//    e.g. if an identifier is a type name, variable name, ...
//////////////////////////////////////////////////////////////////////////

#ifndef __RULE_TABLE_UTIL_JAVA_H__
#define __RULE_TABLE_UTIL_JAVA_H__

#include "ruletable_util.h"

namespace maplefe {

class JavaValidityCheck {
public:
  bool IsPackageName(){return true;}
  bool IsTypeName(){return true;}
  bool IsVariable(){return true;}

// Java specific
public:
  bool TypeArgWildcardContain(){return true;}
};

}
#endif
