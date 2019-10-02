/////////////////////////////////////////////////////////////////////////
//   This file defines interfaces of file operations                   //
/////////////////////////////////////////////////////////////////////////

#ifndef __FILE_WRITE_H__
#define __FILE_WRITE_H__

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "buffer2write.h"

class FileWriter {
public:

private:
  std::string   mName;
  std::ofstream mFile;

  std::string   mCurLine;
  const char   *mCurChar;
  unsigned      mPos;      //The index of mCurChar, or #chars processed

  unsigned      mIndentation; // indentation
  unsigned      mLineLimit;   // max length of a line

public:
  FileWriter() {}
  FileWriter(const std::string &s);
  ~FileWriter(){ mFile.close(); }

  // Only one line
  void WriteOneLine(const char *s, int l, bool iscomment = false);

  // Could be split into multi lines
  void WriteLine(const char *s, int l,
                 bool extraindent = false, bool iscomment=false);

  // Write a block in the FormattedBuffer
  void WriteBlock(const char *);
  void WriteSimpleBuffers(const FormattedBuffer *);

  // Write a formatted buffer.
  void WriteFormattedBuffer(const FormattedBuffer *);
  void WriteIfBuffer(const IfBuffer*);
  void WriteScopedBuffer(const ScopedBuffer*);
};

#endif
