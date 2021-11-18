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
//   This file defines buffers to be written to files                  //
/////////////////////////////////////////////////////////////////////////

#ifndef __BUFFER_2_WRITE_H__
#define __BUFFER_2_WRITE_H__

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#define MAX_LINE_LIMIT  100
#define LINES_PER_BLOCK 4     // This must be 2^x

namespace maplefe {

class FormattedBuffer;
class BaseGen;

//////////////////////////////////////////////////////////////////////////
//                              Buffer Design                           //
// All buffers are saved line by line, without the ending '\n', which   //
// WriteOneLine() will add it.                                          //
//                                                                      //
// When output the generated files, each line of source code will NOT be//
// cut in the middle. Otherwise, it will be hard to check where to cut  //
// to avoid cut a word into halves. So a LOC must be complete.          //
//                                                                      //
// There is an issue about the length of a LOC. Two cases to consider   //
// LineBuffer and RectBuffer (Rectangle Buffer)                         //
// 1) if a line is over MAX_LINE_LIMIT, it will be put into a LineBuffer//
//    which is using malloc/free                                        //
// 2) A RectBuffer is of LINES_PER_BLOCK lines, each line is less than  //
//    MAX_LINE_LIMIT. It's an char array, with Nth line starting at the //
//    N * MAX_LINE_LIMIT inside the block.                              //
//                                                                      //
// RectBuffer is preferred than LineBuffer. This is followed in some    //
// default construction of buffers.                                     //
//////////////////////////////////////////////////////////////////////////

typedef enum WriteStatus {
  WR_GOOD,
  WR_OVERSIZE
}WriteStatus;

typedef enum SimpleBufferType {
  SB_Line,
  SB_Rect
}SimpleBufferType;

class SimpleBuffer {
public:
  // The indentation will not be saved in any buffer, it's just a
  // information telling the FileWriter how many white spaces it needs
  // when generating the files.
  unsigned mIndentation;

  // a LineBuffer or a RectBuffer
  SimpleBufferType mType;

public:
  SimpleBuffer(unsigned ind) : mIndentation(ind) {}
  virtual ~SimpleBuffer() = default;

  virtual WriteStatus AddChar(const char) = 0;
  virtual WriteStatus AddString(const char*) = 0;
  virtual WriteStatus AddInteger(const int) = 0;

  virtual void AddCommentHeader() = 0;
  virtual void SetLineEmpty() = 0;      // set current line empty

  virtual bool CurrentLineEmpty() = 0;
};

// A single line buffer, over MAX_LINE_LIMIT. Theoritically the LineBuffer
// can extend to unlimited length.
class LineBuffer : public SimpleBuffer {
public:
  char    *mData;
  unsigned mSize;
  unsigned mPos;

private:
  void Extend(unsigned);

public:
  LineBuffer(unsigned ind, unsigned len);
  ~LineBuffer();

  const char* GetData() {return mData;}

  WriteStatus AddChar(const char);
  WriteStatus AddString(const char*);
  WriteStatus AddInteger(const int);

  void AddCommentHeader();
  void SetLineEmpty();      // set current line empty
  bool CurrentLineEmpty() {return mPos == 0;} // current line avialbe and empty
};

// RectBuffer
// 1) Every line in RectBuffer has the same indentation
// 2) An white space line can be represented by (1) either NULL, (2) or a set of
//    white space. But we choose MAX_LINE_LIMIT white space as an empty line,
//    in order to avoid handling NULL line in this way.
// 3) The difficult part for RectBuffer is when it adds a line which is
//    over MAX_LINE_LIMIT. In this case the current RectBuffer will be
//    done, and the new line will be a separate LineBuffer. The drawback
//    of this design is we are wasting the rest empty space in mData.

class RectBuffer : public SimpleBuffer {
public:
  unsigned mUsed;  // a LINES_PER_BLOCK bitmap telling which lines are used.
  char     mData[MAX_LINE_LIMIT * LINES_PER_BLOCK];
  char    *mCurrLine;
  unsigned mCurrLineSize;  // how many char-s in the line

public:
  RectBuffer(unsigned ind);
  ~RectBuffer();

  bool IsFull(); // if there is still available lines.
  bool CurrentLineEmpty() {return !IsFull() && (mCurrLineSize == 0);}

  // Add a new line. Returns false if over MAX_LINE_LIMIT, or returns true
  bool AddLine(const char *str);

  WriteStatus AddChar(const char);
  WriteStatus AddString(const char*);
  WriteStatus AddInteger(const int);

  void  AddCommentHeader();
  char* GetNextAvailLine();
  void  SetLineEmpty();      // set current line empty
};

//////////////////////////////////////////////////////////////////////////
//                     OneBuffer
// OneBuffer is the control element inside a FormattedBuffer. It could be
// a SimpleBuffer or a nested formatted buffer.
//
// A nest formatted buffer in OneBuffer is NOT editable.

class OneBuffer {
public:
  union {
    SimpleBuffer    *mSimple;
    FormattedBuffer *mFormatted;
  }mData;
  bool               mIsSimple;

public:
  OneBuffer(unsigned ind);
  OneBuffer(unsigned ind, unsigned len, bool isLineBuffer = false);
  OneBuffer(FormattedBuffer *fb) {mIsSimple = false; mData.mFormatted = fb;}
  ~OneBuffer();

private:
  void  NewBlock();

public:
  WriteStatus AddChar(const char);
  WriteStatus AddString(const char*);
  WriteStatus AddInteger(const int);

  void  AddCommentHeader();
  char* NewCommentLine(const char*);

public:
  bool  IsFull();
  bool  IsSimple() {return mIsSimple;}
  bool  IsLineBuffer() {return mData.mSimple->mType == SB_Line;}
  bool  IsRectBuffer() {return mData.mSimple->mType == SB_Rect;}
  bool  CurrentLineEmpty();  // current line availabe and empty.
  char* GetNextAvailLine();
  RectBuffer* GetRectBuffer(); // return the rect buffer if it's a RectBuffer
};

/////////////////////////////////////////////////////////////////////////////
// [FormattedBuffer]
//
// A formatted buffer is composed of a set of SimpleBuffer or nested
// formatted buffer. Its goal is to provide interface for buffer users.
// The inside details of the OneBuffer is protected.
//
// The nested FormattedBuffer is never modified by the parent FormattedBuffer.
// So the children should be fixed before added into the parent.

class FormattedBuffer {
public:
  bool                      mIsComment;
  std::vector<OneBuffer*>   mBuffers;
  OneBuffer                *mCurrOneBuffer;
  unsigned                  mIndentation;

public:
  OneBuffer* NewOneBuffer();
  OneBuffer* NewOneBuffer(unsigned len, bool isLineBuffer = false);

public:
  FormattedBuffer(unsigned ind = 0, bool iscomment = false);
  ~FormattedBuffer();

public:
  void  AddNestedBuffer(FormattedBuffer *);

  char* NewLine();
  char* NewCommentLine(const char*);
  char* NewEmptyLine();  // empty line is MAX_LINE_LIMIT whitespace

  virtual void  AddInteger(const int);
  virtual void  AddChar(const char);
  virtual void  AddString(const char *);
  virtual void  AddString(const std::string s) {AddString(s.c_str());}
  virtual void  AddStringWholeLine(const char *);
  virtual void  AddStringWholeLine(const std::string s) {AddStringWholeLine(s.c_str());}

  unsigned GetIndent() {return mIndentation;}
  void  IncIndent() {mIndentation += 2;}
  void  DecIndent() {mIndentation -= 2;}

  void ClearLine(char *);  // wipe existing data, fill with whitespace
};

///////////////////////////////////////////////////////////////////////////
//                       ScopedBuffer                                    //
// A ScopedBuffer is a block of statements enclosed by { and }.          //
// Extra work of ScopedBuffer is to dump { and } when output the data.   //
//                                                                       //
// Indentation of ScopedBuffer is the one of the real code, not for '{'  //
// and '}'                                                               //
///////////////////////////////////////////////////////////////////////////

class ScopedBuffer : public FormattedBuffer {
public:
  ScopedBuffer(unsigned ind) : FormattedBuffer(ind) {}
  ~ScopedBuffer(){}
};

//////////////////////////////////////////////////////////////////////////
//                         IfBuffer                                     //
// This buffer contains an If statement.                                //
//////////////////////////////////////////////////////////////////////////

class IfBuffer : public FormattedBuffer {
private:
  FormattedBuffer mCondition;
  ScopedBuffer    mBody;
public:
  // Indentation of mBody is two more spaces
  IfBuffer(unsigned ind) : mCondition(ind), mBody(ind+2) {}
  const FormattedBuffer* GetCondition() const {return &mCondition;}
  const ScopedBuffer*    GetBody() const {return &mBody;}
};

//////////////////////////////////////////////////////////////////////////
//                         ForBuffer                                     //
// This buffer contains an For statement.                                //
//////////////////////////////////////////////////////////////////////////

class ForBuffer : public FormattedBuffer {
private:
  FormattedBuffer mCondition;
  ScopedBuffer     mBody;
public:
  FormattedBuffer* GetCondition() {return &mCondition;}
  ScopedBuffer*     GetBody() {return &mBody;}
};

//////////////////////////////////////////////////////////////////////////
//                         EnumBuffer                                   //
// This buffer contains an Enum statement. For most cases, the Enum     //
// is created from a STRUCT.                                            //
//////////////////////////////////////////////////////////////////////////

class StructBase;
class StructSeparator;
class StructType;

class EnumBuffer : public FormattedBuffer {
private:
  FormattedBuffer mCondition;
  ScopedBuffer     mBody;
public:
  // by default, set indentation 0.
  EnumBuffer() : FormattedBuffer(0, false), mCondition(0), mBody(0) {}
  EnumBuffer(unsigned ind) :
        FormattedBuffer(ind), mCondition(ind), mBody(ind) {}

  // last_item is for some enum struct which needs to output xxx_Null.
  // xxx_Null will not be included in the language spec, it's all for parser.
  // If last_item is NULL, xxx_Null won't be added.
  void Generate(BaseGen *, const char *last_item = NULL);
};

//////////////////////////////////////////////////////////////////////////
//                         FunctionBuffer                               //
// A function has three buffers.                                        //
//   1) Declaration in .h file                                          //
//   2) Header in .cpp file                                             //
//   3) Body in .cpp file                                               //
// Here is an example:                                                  //
//     void foo (int a)             <-- Header                          //
//     {                            <-- start of body                   //
//       ....                                                           //
//     }                            <-- end of body                     //
//////////////////////////////////////////////////////////////////////////

class FunctionBuffer {
public:
  FormattedBuffer mDecl;   // function declaration in .h file
  FormattedBuffer mHeader; // function header in cpp file
  ScopedBuffer    mBody;
public:
  FunctionBuffer();
  ~FunctionBuffer() {}

  void AddReturnType(const char*);
  void AddFunctionName(const char*);
  void AddParameter(const char* type, const char *name, bool end = false);
  ScopedBuffer* GetBody() { return &mBody; }
};

//////////////////////////////////////////////////////////////////////////
//                         TableBuffer                                  //
// This buffer contains a Table statement. For most cases, the table    //
// is created from a STRUCT.                                            //
//////////////////////////////////////////////////////////////////////////

class TableBuffer : public FormattedBuffer {
private:
  ScopedBuffer     mBody;
public:
  // by default, set indentation 0.
  TableBuffer() : FormattedBuffer(0, false), mBody(0) {}
  TableBuffer(unsigned ind) : FormattedBuffer(ind), mBody(ind) {}

  // 1. decl is for the declaration of a table.
  void Generate(BaseGen *, const std::string decl);
};

}

#endif
