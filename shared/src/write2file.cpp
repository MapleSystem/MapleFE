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
#include <cstring>
#include <cstdlib>

#include "write2file.h"
#include "massert.h"

namespace maplefe {

///////////////////////////////////////////////////////////////////////
//                            Write2File                             //
///////////////////////////////////////////////////////////////////////

Write2File::Write2File(const std::string &s) {
  mName = s;
  mFile.open(s.c_str(), std::ofstream::out | std::ofstream::trunc);
  if (!mFile.good())
    std::cout << "file " << s << " is bad" << std::endl;

  mCurChar = NULL;
  mPos = 0;

  mIndentation = 0;
  mLineLimit = MAX_LINE_LIMIT;
}

// Write a single line. The caller has guaranteed that it won't exceed
// the max length of a single line.
void Write2File::WriteOneLine(const char *s, int l, bool iscomment) {
  if (iscomment)
    mFile.write("// ", 3);

  for (int i = 0; i < mIndentation; i++) {
    mFile.put(' ');
  }

  mFile.write(s, l);
  mFile.put('\n');
}

// Write a buffer with following features,
// 1) mIndentation effective
// 2) mLineLimit effective, extra data will be write into the next lines,
//    may or may not with extra indentation.
// 3) Extra indentation will cause extra 4 white space in the following lines.
void Write2File::WriteLine(const char *s, int l, bool extra_indent, bool iscomment) {
  unsigned old_indent = mIndentation;
  const char *pos = s;
  int left = l;
  bool first_line = true;

  while (left) {
    if (!first_line)
      mIndentation += 4;

    if (left + mIndentation > mLineLimit) {
      int to_write = mLineLimit - mIndentation;
      left -= to_write;
      WriteOneLine(pos, to_write, iscomment);
      pos += to_write;
      first_line = false;
    } else {
      WriteOneLine(pos, left, iscomment);
    }
  }

  mIndentation = old_indent;
}

// Write a block in the FormattedBuffer.
void Write2File::WriteBlock(const char *block) {
  for (int i = 0; i < LINES_PER_BLOCK; i++) {
    if (*block != 0) {
      int len = strlen(block) >= MAX_LINE_LIMIT ? MAX_LINE_LIMIT
                                                : strlen(block);
      WriteOneLine(block, len);
      block += MAX_LINE_LIMIT;
    } else {
      // A NULL line doesn't means end of buffer.
      block += MAX_LINE_LIMIT;
      continue;
    }
  }
}
}
