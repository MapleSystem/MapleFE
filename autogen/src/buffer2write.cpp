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
#include <cassert>
#include <cstdlib>
#include <cmath>

#include "buffer2write.h"
#include "separator_gen.h"
#include "type_gen.h"
#include "massert.h"
#include "stringutil.h"

namespace maplefe {

// Won't take '-' into account
static int NumDigits(int number)
{
  int digits = 0;
  if (!number)
    return 1;

  if (number < 0)
    number = (-1) * number;

  while (number) {
    number /= 10;
    digits++;
  }

  return digits;
}

///////////////////////////////////////////////////////////////////////
//                        LineBuffer                                 //
///////////////////////////////////////////////////////////////////////

// The 'len' could be changed if real lenght is getting bigger.
LineBuffer::LineBuffer(unsigned ind, unsigned len) : SimpleBuffer(ind) {
  mData = (char *)malloc(len+1);
  mSize = len;
  mPos = 0;
  memset((void*)mData, 0, mSize);
  mType = SB_Line;
}

LineBuffer::~LineBuffer(){
  MASSERT(mData && "LineBuffer is not allocated");
  free(mData);
  mData = NULL;
  mPos = 0;
}

// Extend the size of mData
// newSize is the minimum request of new size.
void LineBuffer::Extend(unsigned reqSize) {
  unsigned oldSize = mSize;
  unsigned newSize = (reqSize/mSize + 1) * mSize;
  mSize = newSize;
  mData = (char*)realloc(mData, mSize);
  memset((void*)(mData + oldSize), 0, newSize - oldSize);
}

WriteStatus LineBuffer::AddString(const char *str){
  unsigned len = strlen(str);
  unsigned newPos = mPos + len;
  if (newPos > mSize) {
    Extend(newPos);
  }
  strncpy(mData + mPos, str, len);
  mPos = newPos;
  return WR_GOOD;
}

WriteStatus LineBuffer::AddChar(const char c) {
  unsigned newPos = mPos + 1;
  if (newPos > mSize) {
    Extend(newPos);
  }
  *(mData + mPos) = c;
  mPos++;
  return WR_GOOD;
}

// To add an integer in the buffer
// Need to differentiate positive and negative integers.
WriteStatus LineBuffer::AddInteger(int i) {
  // The '-' will inserted by snprintf. NumDigits() won't
  // take '-' into account.
  unsigned digits = 0;
  if (i < 0) {
    digits = 1;
  }

  digits += NumDigits(i);

  unsigned newPos = mPos + digits;
  if (newPos > mSize) {
    Extend(newPos);
  }

  snprintf(mData + mPos, digits, "%i", i);
  mPos = newPos;

  return WR_GOOD;
}

// It adds the "  // " to the head of the line.
void LineBuffer::AddCommentHeader() {
  for (unsigned u = 0; u < mIndentation; u++) {
    AddChar(' ');
  }
  AddChar('/');
  AddChar('/');
  AddChar(' ');
}

void LineBuffer::SetLineEmpty() {
  // reset mPos to the beginning of line
  mPos = 0;
  memset((void*)mData, 0, mSize);
}

///////////////////////////////////////////////////////////////////////
//                        RectBuffer                                //
///////////////////////////////////////////////////////////////////////
RectBuffer::RectBuffer(unsigned ind) : SimpleBuffer(ind) {
  mUsed = 1;  // the first line is used when constructed
  mCurrLineSize = 0;
  mCurrLine = mData;
  mType = SB_Rect;

  //clear the block
  memset((void*)mData, 0, MAX_LINE_LIMIT * LINES_PER_BLOCK);
}

RectBuffer::~RectBuffer(){}

bool RectBuffer::IsFull() {
  unsigned mask = (1 << LINES_PER_BLOCK) - 1;
  if ((mUsed & mask) == mask)
    return true;
  else
    return false;
}

// Get the next available line from RectBuffer.
// Mark the line as used.  Set the line as current.
//
// We don't handle the buffer-is-full case.
char* RectBuffer::GetNextAvailLine() {
  if (IsFull())
    return NULL;

  if (CurrentLineEmpty())
    return mCurrLine;

  unsigned mask = (1 << LINES_PER_BLOCK) - 1;
  unsigned line = (mUsed & mask) + 1;
  unsigned log2line = (unsigned)log2(line);
  char *addr = mData + MAX_LINE_LIMIT * log2line;
  mUsed = mUsed | line;

  mCurrLine = addr;
  mCurrLineSize = 0;

  return addr;
}

// To add a char in the buffer
//
// The handling of NewLine() when oversize is done by FormattedBuffer
// which is the original caller.  So it simple returns WR_OVERSIZE here.
WriteStatus RectBuffer::AddChar(const char c) {
  MASSERT (mCurrLine && "mCurrLine must NOT be NULL in AddChar");

  char *pos = mCurrLine + mCurrLineSize;
  unsigned left = MAX_LINE_LIMIT - mCurrLineSize;

  if (left == 0) {
    return WR_OVERSIZE;
  } else {
    *pos = c;
    mCurrLineSize += 1;
    return WR_GOOD;
  }
}

// To add an integer in the buffer
// Need to differentiate positive and negative integers.
//
// The handling of NewLine() when oversize is done by FormattedBuffer
// which is the original caller.  So it simple returns WR_OVERSIZE here.
WriteStatus RectBuffer::AddInteger(int i) {
  char *pos = mCurrLine + mCurrLineSize;
  unsigned left = MAX_LINE_LIMIT - mCurrLineSize;
  unsigned digits = 0;

  // The '-' will inserted by snprintf. But NumDigits() won't
  // take '-' into account.
  if (i < 0) {
    digits = 1;
  }

  digits += NumDigits(i);
  left -= digits;
  if (left < 0) {
    return WR_OVERSIZE;
  } else {
    snprintf(pos, digits, "%i", i);
    mCurrLineSize += digits;
    return WR_GOOD;
  }
}

// Add a string in the buffer.
//
// The handling of NewLine() when oversize is done by FormattedBuffer
// which is the original caller.  So it simple returns WR_OVERSIZE here.
// The oversized string will finally be written into a LineBuffer.
WriteStatus RectBuffer::AddString(const char *str){
  // Prepend the Indentation
  // std::string space(MAX_LINE_LIMIT, ' ');
  // char indentation[MAX_LINE_LIMIT];
  // snprintf(indentation, mIndentation+1, "%s", space.c_str());
  // std::string data(indentation);
  // data = data + str;
  // str = data.c_str();

  char *pos = mCurrLine + mCurrLineSize;
  unsigned left = MAX_LINE_LIMIT - mCurrLineSize;
  unsigned len = strlen(str);

  left -= len;
  if (left < 0) {
    return WR_OVERSIZE;
  } else {
    snprintf(pos, len+1, "%s", str);
    mCurrLineSize += len;
    return WR_GOOD;
  }
}


// It adds the "  // " to the head of the line.
void RectBuffer::AddCommentHeader() {
  for (unsigned u = 0; u < mIndentation; u++) {
    AddChar(' ');
  }
  AddChar('/');
  AddChar('/');
  AddChar(' ');
}

// Set current line empty
void RectBuffer::SetLineEmpty() {
  memset((void*)mCurrLine, 0, MAX_LINE_LIMIT);
}

///////////////////////////////////////////////////////////////////////
//                        OneBuffer                                  //
///////////////////////////////////////////////////////////////////////

// default construtor creates RectBuffer.
OneBuffer::OneBuffer(unsigned ind) {
  RectBuffer *rb = new RectBuffer(ind);
  mData.mSimple = (SimpleBuffer*)rb;
  mIsSimple = true;
}

OneBuffer::OneBuffer(unsigned ind, unsigned len, bool isLineBuffer) {
  if (isLineBuffer || (len > MAX_LINE_LIMIT)) {
    LineBuffer *line = new LineBuffer(ind, len);
    mData.mSimple = (SimpleBuffer*)line;
    mIsSimple = true;
  } else {
    RectBuffer *rb = new RectBuffer(ind);
    mData.mSimple = (SimpleBuffer*)rb;
    mIsSimple = true;
  }
}

OneBuffer::~OneBuffer() {
  if (mIsSimple)
    delete mData.mSimple;
}

// Returns the RectBuffer if it's one.
// Returns NULL, or else.
RectBuffer* OneBuffer::GetRectBuffer() {
  if (mIsSimple) {
    RectBuffer* rb = dynamic_cast<RectBuffer*>(mData.mSimple);
    return rb;
  }

  return NULL;
}

// Get the next available line from RectBuffer.
// Mark the line as used.
char* OneBuffer::GetNextAvailLine() {
  MASSERT(mIsSimple && "Only simple buffer can call GetNextAvailLine()");
  RectBuffer* rb = dynamic_cast<RectBuffer*>(mData.mSimple);
  MASSERT(rb);
  return rb->GetNextAvailLine();
}

// OneBuffer is NOT full only when it's a RectBuffer with available lines.
// Any case else is treated as full.
bool OneBuffer::IsFull() {
  if (mIsSimple) {
    RectBuffer *rb = dynamic_cast<RectBuffer*>(mData.mSimple);
    if (rb)
      return rb->IsFull();
  }
  return true;
}

// true if current line is available and empty
bool OneBuffer::CurrentLineEmpty() {
  MASSERT(mIsSimple && "CurrentLineEmpty() only works on simple buffer");
  return mData.mSimple->CurrentLineEmpty();
}

WriteStatus OneBuffer::AddChar(const char c) {
  MASSERT(mIsSimple && "AddChar(): Only simplebuffer is editable");
  return mData.mSimple->AddChar(c);
}

WriteStatus OneBuffer::AddInteger(const int i) {
  MASSERT(mIsSimple && "AddInteger(): Only simplebuffer is editable");
  return mData.mSimple->AddInteger(i);
}

WriteStatus OneBuffer::AddString(const char *s) {
  MASSERT(mIsSimple && "AddString(): Only simplebuffer is editable");
  return mData.mSimple->AddString(s);
}

void OneBuffer::AddCommentHeader() {
  MASSERT(mIsSimple && "AddCommentHeader(): Only SimpleBuffer is editable");
  mData.mSimple->AddCommentHeader();
}

///////////////////////////////////////////////////////////////////////
//                        FormattedBuffer                            //
///////////////////////////////////////////////////////////////////////

// The construction of the first OneBuffer is left in each children
// of FormattedBuffer because some of them want a LineBuffer at first,
// some RectBuffer at first.
FormattedBuffer::FormattedBuffer(unsigned indent, bool iscomment) {
  mIndentation = indent;
  mIsComment = iscomment;
  mCurrOneBuffer = NULL;
}

FormattedBuffer::~FormattedBuffer() {
  std::vector<OneBuffer*>::iterator it = mBuffers.begin();
  for (; it != mBuffers.end(); it++) {
    OneBuffer *ob = *it;
    delete ob;
  }
}

// Allocate a new OneBuffer.
// This calls default constructor of OneBuffer.
OneBuffer* FormattedBuffer::NewOneBuffer() {
  OneBuffer *ob = new OneBuffer(mIndentation);
  mBuffers.push_back(ob);
  mCurrOneBuffer = ob;
  return ob;
}

// Allocate a new OneBuffer with lenght
OneBuffer* FormattedBuffer::NewOneBuffer(unsigned len, bool isLineBuffer) {
  OneBuffer *ob = new OneBuffer(mIndentation, len, isLineBuffer);
  mBuffers.push_back(ob);
  mCurrOneBuffer = ob;
  return ob;
}

// Get a new line, return the address.
//
// Only if there are available lines in the RectBuffer of OneBuffer, we use
// the available line. For everything else, we allocate a new RectBuffer.
char* FormattedBuffer::NewLine() {
  OneBuffer *ob = mCurrOneBuffer;
  if (ob->GetRectBuffer() && !ob->IsFull()) {
    // Do nothing if RectBuffer is available
  } else {
    // default constructor of OneBuffer will allocate a new RectBuffer
    ob = NewOneBuffer();
  }
  return ob->GetNextAvailLine();
}

// Open a new line for comment, return the addr of the line.
// the content of 'str' will be put in the new comment line.
char* FormattedBuffer::NewCommentLine(const char *str) {
  char *addr = NewLine();
  mCurrOneBuffer->AddCommentHeader();
  return addr;
}

// Empty line is a set of white space.
char* FormattedBuffer::NewEmptyLine() {
  char *addr = NewLine();
  MASSERT(mCurrOneBuffer->mIsSimple && "NewEmptyLine(): ONly simple buffer is editable");
  mCurrOneBuffer->mData.mSimple->SetLineEmpty();
  return addr;
}

// It's for sure that 'c' won't exceed the lenght of MAX_LINE_LIMIT,
// so a default OneBuffer constructor is enough to hold it once oversized.
void FormattedBuffer::AddChar(const char c) {
  if (mCurrOneBuffer->AddChar(c) == WR_OVERSIZE) {
    NewOneBuffer();
    MASSERT(mCurrOneBuffer->AddChar(c) == WR_GOOD && "Second try of AddChar must be good");
  } else
    return;
}

// It's for sure that 'i' won't exceed the lenght of MAX_LINE_LIMIT,
// so a default OneBuffer constructor is enough to hold it once oversized.
void FormattedBuffer::AddInteger(const int i) {
  if (mCurrOneBuffer->AddInteger(i) == WR_OVERSIZE) {
    NewOneBuffer();
    AddInteger(i);
  } else
    return;
}

void FormattedBuffer::AddString(const char *str){
  if (mCurrOneBuffer->AddString(str) == WR_OVERSIZE) {
    NewOneBuffer(strlen(str));
    MASSERT(mCurrOneBuffer->AddString(str) == WR_GOOD && "Must be a Good write");
  } else
    return;
}

// Add string and it occupies the whole line
// So if the current line is not empty, we need a new line.
void FormattedBuffer::AddStringWholeLine(const char *str){
  if (mCurrOneBuffer->CurrentLineEmpty()
      && mCurrOneBuffer->AddString(str) == WR_GOOD){
    if (mCurrOneBuffer->IsSimple() && mCurrOneBuffer->IsLineBuffer())
      return;
    // Need mark the current line as used, for RectBuffer
    mCurrOneBuffer->GetNextAvailLine();
  } else {
    NewOneBuffer(strlen(str));
    MASSERT(mCurrOneBuffer->AddString(str) == WR_GOOD && "Must be a Good write");
    mCurrOneBuffer->GetNextAvailLine();
  }
  return;
}

// To add a new nested FormattedBuffer.
// The indentation of nested is 2 less than nesting buffer.
void FormattedBuffer::AddNestedBuffer(FormattedBuffer *nested) {
  OneBuffer *ob = new OneBuffer(nested);
  mBuffers.push_back(ob);
  return;
}

// wipe existing data, fill with whitespace.
// NOTE: The caller needs to make sure that 'line' is that beginning
//       of a line.
void FormattedBuffer::ClearLine(char *line) {
  char ws = ' ';
  memset((void*)line, (int)ws, MAX_LINE_LIMIT);
}

///////////////////////////////////////////////////////////////////////
//                           EnumBuffer                              //
///////////////////////////////////////////////////////////////////////

// the enum structure will have the following pattern. The following is
// for Separator.
//      typedef enum SepId {
//        SEP_Xxx,
//        ...
//        SEP_Null
//      }SepId;
//
void EnumBuffer::Generate(BaseGen *bg, const char *last_item){
  std::string firstline = "typedef enum{";
  NewOneBuffer(firstline.size(), true);   // a separate LineBuffer
  AddString(firstline.c_str());

  //start the enum body.
  IncIndent();
  NewOneBuffer();
  bg->EnumBegin();
  while(!bg->EnumEnd()) {
    NewLine();
    std::string elem = bg->EnumNextElem();
    if (!bg->EnumEnd() || last_item)
      if (!elem.size())
        elem = ",";
      else
        elem = elem + ",";
    AddString(elem);
  }
  NewLine();
  AddString(last_item);

  DecIndent();
  const std::string name = bg->EnumName();
  std::string s = "}" + name + ";";
  NewOneBuffer(s.size(), true);  // a separate LineBuffer
  AddString(s);

  return;
}

///////////////////////////////////////////////////////////////////////
//                           FunctionBuffer                          //
///////////////////////////////////////////////////////////////////////

// mDecl is a LineBuffer
// mHeader is a LineBuffer
// mBody needs a RectBuffer
FunctionBuffer::FunctionBuffer() : mBody(2) {
  mDecl.NewOneBuffer(MAX_LINE_LIMIT, true);
  mHeader.NewOneBuffer(MAX_LINE_LIMIT, true);
  mBody.NewOneBuffer();
}

void FunctionBuffer::AddReturnType(const char *rt) {
  mDecl.AddString(rt);
  mDecl.AddChar(' ');
  mHeader.AddString(rt);
  mHeader.AddChar(' ');
}

void FunctionBuffer::AddFunctionName(const char *fn) {
  mDecl.AddString(fn);
  mDecl.AddChar('(');
  mHeader.AddString(fn);
  mHeader.AddChar('(');
}

void FunctionBuffer::AddParameter(const char *type, const char *name, bool end) {
  mDecl.AddString(type);
  mDecl.AddChar(' ');
  mDecl.AddString(name);
  mHeader.AddString(type);
  mHeader.AddChar(' ');
  mHeader.AddString(name);
  if (end) {
    mDecl.AddString(");");
    mHeader.AddChar(')');
    mHeader.AddChar(' ');
  } else {
    mDecl.AddChar(',');
    mDecl.AddChar(' ');
    mHeader.AddChar(',');
    mHeader.AddChar(' ');
  }
}

///////////////////////////////////////////////////////////////////////
//                           TableBuffer                             //
//
// the table structure will have the following pattern. The following is
// an example for Separator.
//      SepTableEntry SepTable[SEP_Null] = {
//        {"xxx", SEP_Xxx},
//        ...
//      };
// This function just generates the data part in the middle. The declaration
// and the ending "};" will be left to caller

void TableBuffer::Generate(BaseGen *bg, const std::string decl){
  NewOneBuffer(decl.size(), true);   // a separate LineBuffer
  AddString(decl.c_str());

  //start the enum body.
  IncIndent();
  NewOneBuffer();
  bg->EnumBegin();
  while(!bg->EnumEnd()) {
    NewLine();
    std::string elem = bg->EnumNextElem();
    if (!bg->EnumEnd())
      if (!elem.size())
        elem = ",";
      else
        elem = elem + ",";
    AddString(elem);
  }

  DecIndent();
  return;
}
}


