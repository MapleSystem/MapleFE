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

#include "file_write.h"
#include "massert.h"

namespace maplefe {

///////////////////////////////////////////////////////////////////////
//                            FileWriter                             //
///////////////////////////////////////////////////////////////////////

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

}


