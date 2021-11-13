/*
* Copyright (C) [2021] Futurewei Technologies, Inc. All rights reverved.
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

#ifndef __HELPER_H__
#define __HELPER_H__

#include <unordered_map>
#include <vector>
#include <string>
#include "massert.h"
#include "ast.h"
#include "typetable.h"

namespace maplefe {

extern std::unordered_map<TypeId, std::string>TypeIdToJSType;
extern std::unordered_map<TypeId, std::string>TypeIdToJSTypeCXX;
extern TypeId hlpGetTypeId(TreeNode* node);
extern std::string hlpClassFldAddProp(std::string, std::string, std::string, std::string, std::string);
extern std::string tab(int n);
}
#endif  // __HELPER_H__

