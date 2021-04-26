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
/////////////////////////////////////////////////////////////////////
// Log file
//
/////////////////////////////////////////////////////////////////////

#ifndef __LOG_H__
#define __LOG_H__

#include <iostream>
#include <fstream>

namespace maplefe {

#define LogInfo(msg)    { gLog << "Info: " << msg; }
#define LogWarning(msg) { gLog << "Warning: " << msg; }
#define LogFatal(msg)   { gLog << "Fatal: " << msg; }

class Log {
private:
  std::ofstream mFile;

public:
  typedef enum Level {
    LOG_NONE,
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARNING,
    LOG_ERROR,
    LOG_FATAL
  }Level;

  Log() {mFile.open("log");}
  ~Log(){mFile.close();}


  Log& operator<<(float f);
  Log& operator<<(int i);
  Log& operator<<(size_t st);
  Log& operator<<(const std::string &msg);
  Log& operator<<(const bool b);
  Log& operator<<(const char *s);
};

extern Log gLog;

}
#endif // __LOG_H__
