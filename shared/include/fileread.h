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

#ifndef __FILE_READ_H__
#define __FILE_READ_H__

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

namespace maplefe {

class FileReader {
public:
  std::string   mName;
  std::ifstream mDefFile;
  std::string   mCurLine;
  const char   *mCurChar;
  unsigned      mPos;      //The index of mCurChar, or #chars processed
  bool          mEndOfFile;
  unsigned      mLineNo;
public:
  FileReader(const std::string &s);
  ~FileReader(){ mDefFile.close(); }

public:
  int    SkipWhiteSpace();
  void   SkipNextSeparator(char c);

  void   SkipComments(std::vector<const char *> &commTypes);
  bool   SkipEOLComment(); // true: read EOL, and skip it
  bool   SkipTRAComment(); // true: read Traditional, and skip it

  void   MoveCursor(int i) {mCurChar += i; mPos += i;} // i can be <0.
  void   MoveToEndOfLine() {mPos = mCurLine.size();}
  bool   MoveUntil(const char*);

  bool   Good() { return mDefFile.good(); }
  bool   EndOfLine() {return mPos == mCurLine.size();}
  bool   EndOfFile() {return mDefFile.eof();}

  int    ReadLine(std::vector<const char *>& /*comment pattern*/);
  bool   ReadLine();  // the single entry to directly read from file
  bool   ReadLineNonEmpty();  // read a non-empty line

  const char* ReadWord(const char * /*separator pattern*/, int &);
  const char* ReadWordSkipLeadingWS(const char *separator, int &, int &);
  int         ReadWantedWordSkipLeadingWS(const char * wanted,
                                          const char *separator, int &, int &);

  // Read char enclosed by ' and '
  char        ReadLitChar(int &len);
  // Read string enclosed by " and "
  std::string ReadLitString(int &len);

  char ReadCharSkipLeadingWS(int &len);      // read any char
  int  ReadCharSkipLeadingWS(const char c);  // read a specific char
  int  ReadComma();
  int  ReadColon();
  int  ReadPlus();
  int  ReadLeftParenthesis();
  int  ReadRightParenthesis();

  void Assert(int, const char*);  //assertion with file info
  void DumpCursor();
};

}
#endif
