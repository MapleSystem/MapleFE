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
/////////////////////////////////////////////////////////////////////////
//   This file defines interfaces of file operations                   //
/////////////////////////////////////////////////////////////////////////

#ifndef __WRITE2FILE_H__
#define __WRITE2FILE_H__

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

namespace maplefe {

#define MAX_LINE_LIMIT  100
#define LINES_PER_BLOCK 4     // This must be 2^x

class Write2File {
public:
  std::string   mName;
  std::ofstream mFile;

  std::string   mCurLine;
  const char   *mCurChar;
  unsigned      mPos;      //The index of mCurChar, or #chars processed

  unsigned      mIndentation; // indentation
  unsigned      mLineLimit;   // max length of a line

public:
  Write2File() {}
  Write2File(const std::string &s);
  ~Write2File(){ mFile.close(); }

  // Only one line
  void WriteOneLine(const char *s, int l, bool iscomment = false);

  // Could be split into multi lines
  void WriteLine(const char *s, int l,
                 bool extraindent = false, bool iscomment=false);

  // Write a block in the FormattedBuffer
  void WriteBlock(const char *);
};

}
#endif
