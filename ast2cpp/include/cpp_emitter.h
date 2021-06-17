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

#ifndef __CPPEMITTER_HEADER__
#define __CPPEMITTER_HEADER__

#include "ast_module.h"

namespace maplefe {

class CppEmitter {
private:
  AST_Handler *mASTHandler;

public:
  CppEmitter(AST_Handler *h) : mASTHandler(h) {}
  bool EmitCxxFiles();
};

} // namespace maplefe
#endif