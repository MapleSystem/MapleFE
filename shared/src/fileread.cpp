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
///////////////////////////////////////////////////////////////////////
//                            FileReader                             //
///////////////////////////////////////////////////////////////////////

#include <cstring>
#include <cstdlib>

#include "fileread.h"
#include "massert.h"

namespace maplefe {

FileReader::FileReader(const std::string &s) {
  mName = s;
  mDefFile.open(s.c_str(), std::ifstream::in);
  if (!mDefFile.good()) {
    MERROR("unable to read from file %s\n", s.c_str());
  }

  mCurChar = NULL;
  mEndOfFile = false;
  mPos = 0;

  mLineNo = 0;
}

// Skip the white space, starting from mCurChar, until the end of line
// Return N : number of space
int FileReader::SkipWhiteSpace(){
  int i = 0;
  while (*mCurChar == ' '){
    i++;
    MoveCursor(1);
    if (EndOfLine())
      return i;
  }
  return i;
}

static bool StringMatch(const char *target,
                        std::vector<const char *> &styles) {
  std::vector<const char *>::iterator it = styles.begin();
  for (; it != styles.end(); it ++) {
    const char *s = *it;
    if (strncmp(target, s, strlen(s)) == 0)
      return true;
  }
  return false;
}

// If 'target' is a separator
// A separator is a char, one of those in "separators"
static bool IsSeparator(char target, const char *separators) {
  const char *p = separators;
  for (int i = 0; i < strlen(separators); i++) {
    if (target == *p)
      return true;
    p++;
  }
  return false;
}

// Skip the current line and the next lines if they are comments.
// Each comment style will be able to be identified by the leading sub strings.
//
// Assumption: The calling function need make sure it's starting from the
//             beginning of a line.
//
void FileReader::SkipComments(std::vector<const char *> &commTypes){
  SkipWhiteSpace();
  while (StringMatch(mCurChar, commTypes)) {
    if (!ReadLine())
      break;
    SkipWhiteSpace();
  }
}

// skip the popular End Of Line comment: //
bool FileReader::SkipEOLComment(){
  SkipWhiteSpace();
  if (*mCurChar == '/' && *(mCurChar+1) == '/') {
    MoveCursor(2);
    MoveToEndOfLine();
    return true;
  } else {
    return false;
  }
}

// Move until hit the 'target'.
// Return true if hit.
// Cursor won't pass the 'target', remaining at the beginning of 'target'.
bool FileReader::MoveUntil(const char *target){
  while(!EndOfFile()) {
    if (!strncmp(mCurChar, target, strlen(target)))
      return true;
    else
      MoveCursor(1);

    while(EndOfLine()){
      ReadLine();
    }
  }
  return false;
}

// skip the popular Traditional comment: /* ... */
bool FileReader::SkipTRAComment(){
  SkipWhiteSpace();
  if (*mCurChar == '/' && *(mCurChar+1) == '*') {
    MoveCursor(2);
    // Move until the end of traditional comment
    if (MoveUntil("*/")){
      MoveCursor(2);
      return true;
    } else {
      Assert(0, "No ending */ of traditional comment)");
    }
  }
  return false;
}

// Skip the next separator designated by 'c'.
// [Assumption] mCurChar is separtor, or leading white space before a separator
void FileReader::SkipNextSeparator(char sep) {
  SkipWhiteSpace();
  if (*mCurChar == sep) {
    MoveCursor(1);
  } else {
    Assert (EndOfLine(), "No speparator caught by SkipNextSeparator()");
  }
}

// Just read a line from the file. It doesn't do anything more.
// The reason for this wrapper function is to provide a single entry to
// read file. Thus we can easily manipulate the line number, column number,
// cursor, etc.
//
// Returns true : if file is good()
//        false : if file is not good()
bool FileReader::ReadLine() {
  std::getline(mDefFile, mCurLine);
  mLineNo++;
  // No matter the line is empty or not, or end of file,
  // we need set the mPos/mCurChar correctly, otherwise it uses the previous
  // values and cause trouble in EndOfLine().
  mCurChar = mCurLine.c_str();
  mPos = 0;
  if (mDefFile.good())
    return true;
  else
    return false;
}

// Read line until the line is not empty
bool FileReader::ReadLineNonEmpty(){
  while(1) {
    ReadLine();
    if (mCurLine.size() > 0)
      return true;
  }
  return false;
}

// Read line from the file, and return the number of read chars.
// Skip empty line.
// Skip Comment line.
// Skip the leading white space-s.
// The trailing new-line character has been removed.
//
int FileReader::ReadLine(std::vector<const char *> &commentTypes) {
  while (ReadLine()) {
    if (mCurLine.length() == 0)
      continue;

    // mCurChar is referenced by SkipXXX(), so should be set up early.
    mCurChar = mCurLine.c_str();
    mPos = 0;
    SkipComments(commentTypes);
    SkipWhiteSpace();
    if (EndOfLine())
      continue;
    break;
  }

  if (mDefFile.eof()) {
    return 0;
  }

  mCurChar = mCurLine.c_str();
  mPos = 0;
  return mCurLine.length();
}

// Starting from mCurChar, read a word, until hit a separator
// Return : address of the word, and len : the number of chars read.
//        : NULL, and len = 0 If mCurChar is a separator, or End of Line.
//
// Assumption: Each separator should be a character.
const char* FileReader::ReadWord(const char *separators, int &len) {
  const char *word = mCurChar;
  const char *p = mCurChar;
  len = 0;
  while (!EndOfLine() && !IsSeparator(*p, separators)) {
    MoveCursor(1);
    len++;
    p++;
  }

  if (!len)
    return NULL;

  return word;
}

// Return a word.
// total_len : #WS + Length(word)
// len : Length(word)
const char* FileReader::ReadWordSkipLeadingWS(
      const char *separator, int &len, int &total_len) {
  int lenWS = SkipWhiteSpace();
  const char *word = ReadWord(separator, len);
  total_len = len + lenWS;
  return word;
}

// Return a wanted word.
// total_len : #WS + Length(word)
// return      0 : if the wanted is not found, cursor moved to the first non-WS
//   Len of Word : if found
int FileReader::ReadWantedWordSkipLeadingWS(
      const char *wanted,
      const char *separator, int &len, int &total_len) {
  int lenWS = SkipWhiteSpace();
  const char *word = ReadWord(separator, len);
  if (len == strlen(wanted) && !strncmp(word, wanted, len)) {
    return len;
  } else {
    MoveCursor((-1)*len);
  }
  total_len = len + lenWS;
  return 0;
}

// Read a literal char, return the char read.
//   len is 3 : if read successfully
//          0 : if read failed
// Leading WS is skipped no matter succeed or not.
char FileReader::ReadLitChar(int &len) {
  len = ReadCharSkipLeadingWS('\'');
  if (!len)
    return 0;
  int len1 = 0;
  char c = ReadCharSkipLeadingWS(len1);
  int len2 = ReadCharSkipLeadingWS('\'');
  // If it's not a literal char, i.e. 'c', we need move back by 2.
  if (!len2) {
    MoveCursor(-2);
    len = 0;
    return 0;
  }
  return c;
}

// Read a literal string, return the string read.
// We are returning std::string to avoid the error-prone memory issue.
//   len is N : length of string if read successfully
//          0 : if read failed
//
// (1) Leading WS is skipped no matter succeed or not.
// (2) Empty string "" is allowed. It returns empty string, but the cursor is moved
//     past the "".
std::string FileReader::ReadLitString(int &len) {
  std::string s = "";
  len = ReadCharSkipLeadingWS('\"');
  if (!len)
    return s;

  int len1 = 0;
  const char *data = ReadWord("\"", len1);

  int len2 = ReadCharSkipLeadingWS('\"');

  // Empty string
  if (len2 && !len1) {
    len = 0;
    return s;
  } else if (len2 && len1) {
    len = len1;
    s.assign(data, len1);
    return s;
  } else {
    std::cout << "Illegal literal string: " << mCurChar << std::endl;
    exit(1);
  }
}

// Read a random char, c.
// Return char : if successful, moving the mCurPos/mPos
//          0  : if failed, moving mCurChar/mPos to first non-WS, or EOL
//
// len is   1  : if sucessful.
char FileReader::ReadCharSkipLeadingWS(int &len){
  SkipWhiteSpace();
  if (EndOfLine()) {
    len = 0;
    return 0;
  }

  char c = *mCurChar;
  MoveCursor(1);
  len = 1;
  return c;
}

// Read a specified char, c.
// Return 1: if successful, moving the mCurPos/mPos
//        0: if failed, moving mCurChar/mPos to first non-WS, or EOL
int FileReader::ReadCharSkipLeadingWS(const char c){
  SkipWhiteSpace();
  if (EndOfLine())
    return 0;
  if (*mCurChar == c){
    MoveCursor(1);
    return 1;
  } else {
    return 0;
  }
}

// Read the first ",", skipping leading white space
// Return : 0, if didn't get it.
int FileReader::ReadComma() {
  return ReadCharSkipLeadingWS(',');
}

// Read the first ":", skipping leading white space
// Return : 0, if didn't get it.
int FileReader::ReadColon() {
  return ReadCharSkipLeadingWS(':');
}

// Read the first "+", skipping leading white space
// Return : 0, if didn't get it.
int FileReader::ReadPlus() {
  return ReadCharSkipLeadingWS('+');
}

// Read the first "(", skipping leading white space
// Return : 0, if didn't get it.
int FileReader::ReadLeftParenthesis() {
  return ReadCharSkipLeadingWS('(');
}

// Read the first ")", skipping leading white space
// Return : 0, if didn't get it.
int FileReader::ReadRightParenthesis() {
  return ReadCharSkipLeadingWS(')');
}

// Dumps the data at current position of input file
void FileReader::DumpCursor() {
  std::cout << "File: " << mName.c_str() << " ";
  std::cout << "Line: " << mLineNo << " [line]:" << mCurLine;
  std::cout << " [pos]:" << mCurChar << std::endl;
}

void FileReader::Assert(int exp, const char *message) {
  if (!exp) {
    std::cout << message << std::endl;
    DumpCursor();
    exit(1);
  }
}
}
