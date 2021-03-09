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
//////////////////////////////////////////////////////////////////////////////
//                       Assertion Macros                                   //
//////////////////////////////////////////////////////////////////////////////

#ifndef __MACROS_H__
#define __MACROS_H__

#include <iostream>

namespace maplefe {

#define NEWLINE do { \
  std::cout << std::endl;\
} while (0)

#define MLOC do { \
  std::cout << "(" << __FILE__ << ":" << __LINE__ << ") ";\
} while (0)

#define MLOCENDL do { \
  std::cout << "(" << __FILE__ << ":" << __LINE__ << ") " << std::endl;\
} while (0)

// SET_INFO_PAIR(module, "filename", strIdx, true);
#define SET_INFO_PAIR(A, B, C, D)                                   \
  A->PushFileInfoPair(maple::MIRInfoPair(maple::GlobalTables::GetStrTable().GetOrCreateStrIdxFromName(B), C)); \
  A->PushFileInfoIsString(D)

}
#endif /* __MACROS_H__ */
