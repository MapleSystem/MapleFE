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
#include <cstring>
#include <cstdlib>

#include "file_write.h"
#include "massert.h"

///////////////////////////////////////////////////////////////////////
//                            FileWriter                             //
///////////////////////////////////////////////////////////////////////

FileWriter::FileWriter(const std::string &s) {
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
void FileWriter::WriteOneLine(const char *s, int l, bool iscomment) {
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
void FileWriter::WriteLine(const char *s, int l, bool extra_indent, bool iscomment) {
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

// Write a FormattedBuffer, simply dumping it to file.
// NOTE:
//       The mIdentation is used to output everything, and it needs to be
//       restored after finishing dumping a buffer. The save&restore are
//       happened in this function, so other WriteXXXXBuffer don't do it.
void FileWriter::WriteFormattedBuffer(const FormattedBuffer *fb) {
  unsigned old_indent = mIndentation;
  if (const IfBuffer *ifbuf = dynamic_cast<const IfBuffer*>(fb)) {
    WriteIfBuffer(ifbuf);
    return;
  } else if (const ScopedBuffer *scoped = dynamic_cast<const ScopedBuffer*>(fb)) {
    WriteScopedBuffer(scoped);
    return;
  } else {
    WriteSimpleBuffers(fb);
  }
  mIndentation = old_indent;
}

void FileWriter::WriteSimpleBuffers(const FormattedBuffer *fb) {
  std::vector<OneBuffer*>::const_iterator it = fb->mBuffers.begin();
  for (; it != fb->mBuffers.end(); it++) {
    OneBuffer *one = *it;
    if (one->mIsSimple) { 
      RectBuffer *rect = one->GetRectBuffer();
      if (rect) {
        mIndentation = rect->mIndentation;
        WriteBlock(rect->mData);
      } else {
        LineBuffer *line = dynamic_cast<LineBuffer*>(one->mData.mSimple);
        MASSERT(line && "This should be a LineBuffer");
        mIndentation = line->mIndentation;
        WriteOneLine(line->mData, line->mPos);
      }
    } else {
      WriteFormattedBuffer(one->mData.mFormatted);
    }
  }
}

// Write a block in the FormattedBuffer.
void FileWriter::WriteBlock(const char *block) {
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

void FileWriter::WriteScopedBuffer(const ScopedBuffer *scoped) {
  // 1. Write the "   {"
  char line[MAX_LINE_LIMIT];
  mIndentation = scoped->mIndentation - 2;
  line[0] = '{';
  WriteOneLine(line, 1);

  // 2. Increate the indentation of body, write the body
  WriteSimpleBuffers(scoped);

  // 3. Write the "   }"
  mIndentation = scoped->mIndentation - 2;
  line[0] = '}';
  WriteOneLine(line, 1);
}

void FileWriter::WriteIfBuffer(const IfBuffer *ifbuf) {
  WriteFormattedBuffer(ifbuf->GetCondition());
  WriteFormattedBuffer(ifbuf->GetBody());
}


