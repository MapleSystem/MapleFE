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
#include "log.h"

namespace maplefe {

// The global Log
Log gLog;

Log& Log::operator<< (float f) {
  mFile << f;
  return *this;
}

Log& Log::operator<<(int i) {
  mFile << i;
  return *this;
}

Log& Log::operator<<(size_t st) {
  mFile << st;
  return *this;
}

Log& Log::operator<<(const std::string &msg) {
  mFile << msg.c_str();
  return *this;
}

Log& Log::operator<<(const bool b) {
  mFile << b;
  return *this;
}

Log& Log::operator<<(const char *s) {
  mFile << s;
  return *this;
}
}

