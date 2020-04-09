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

/////////////////////////////////////////////////////////////////////////////////
// This is the portal of verifying. We decided to have a standalone verification
// which takes an ASTModule recently generated. At this point we have a complete
// module with AST trees created by ASTBuilder.
//
// 1. The verification is a top-down traversal on the AST trees. 
// 2. It contains two parts. One is verification, the other locating and reporting.
/////////////////////////////////////////////////////////////////////////////////

#ifndef __VFY_HEADER__
#define __VFY_HEADER__

class ASTModule;

class Verifier {
private:

public:
  Verifier();
  ~Verifier();

  void Do();
};

#endif
