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
///////////////////////////////////////////////////////////////////////////
// Diagnose records all the language warnings/errors during parsing.     //
//                                                                       //
///////////////////////////////////////////////////////////////////////////

#ifndef __DIAGNOSE_H__
#define __DIAGNOSE_H__

#include <vector>

namespace maplefe {

class DiagMessage {
public:
  char     File[128];
  unsigned Line;
  unsigned Col;
  char     Msg[256];
public:
  DiagMessage(const char*, unsigned, unsigned, const char*);
}

// We save OBJECT not POINTER of DiagMessage in Diagnose.
class Diagnose {
public:
  std::vector<DiagMessage>   mWarnings;
  std::vector<DiagMessage>   mErrors;
public:
  void AddWarning(const char*, unsigned, unsigned, const char*);
  void AddError(const char*, unsigned, unsigned, const char*);
};

}
#endif
