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
//   This file defines the structure for STRUCT                        //
/////////////////////////////////////////////////////////////////////////

#ifndef __STRUCTURE_H__
#define __STRUCTURE_H__

#include <string.h>
#include <iostream>
#include <string>
#include <vector>
#include <queue>

namespace maplefe {

class StringPool;

///////////////////////////////////////////////////////////////////////////////
// In the .spec file, a STRUCT has the syntax as:
//   STRUCT name : ((xxx,yyy),
//                  (aaa,bbb))
// The (xxx,yyy) is called StructElem. It could be N-tuple, but mostly it's
// 2-tuple or 3 tuple. The data in each StructElem could be number, string,
// string literal (those quoted with ").
//
// Each type of .spec has its own definition of STRUCT and so its data in
// StructElem. So the parsing of a StructElem is left to each .spec parser.
// But we do provide some common functions of parsing certain StructElem.
//
///////////////////////////////////////////////////////////////////////////////


// This class is an abstract class which provides the commonly used interfaces
// each XxxGen share. So that the BaseGen can write a common parsing workflow
// for all XxxGen's.
//
// Each XxxGen will have to
//  (1) define its own data
//  (2) implement ParseStructElem()
//

enum DataKind {
  DK_Char,
  DK_Int,
  DK_Float,
  DK_Double,
  DK_Name,
  DK_String,
  DK_Invalid
};

class StructData {
 public:
  DataKind    mKind;
  union {
    char        mChar;
    int64_t     mInt;
    float       mFloat;
    double      mDouble;
    const char *mName;
    const char *mString;
  } mValue;

  void SetInt(int64_t v)   { mKind = DK_Int; mValue.mInt = v; }
  void SetChar(char v)     { mKind = DK_Char; mValue.mChar = v; }
  void SetFloat(float v)   { mKind = DK_Float; mValue.mFloat = v; }
  void SetDouble(double v) { mKind = DK_Double; mValue.mDouble = v; }
  void SetString(const std::string name, StringPool *pool);
  void SetName(const std::string name, StringPool *pool);

  char GetChar() { return mValue.mChar; }
  int64_t GetInt() { return mValue.mInt; }
  float GetFloat() { return mValue.mFloat; }
  double GetDouble() { return mValue.mDouble; }
  const char *GetName() { return mValue.mName; }
  const char *GetString() { return mValue.mString; }

  void Dump();
};

class StructElem {
public:
  std::vector<StructData *> mDataVec;
  const char* GetString(unsigned i) {return mDataVec[i]->GetString();}
  void Dump();
};

class StructBase {
public:
  // A .spec file could define multiple STRUCT, and each XxxGen uses the
  // 'mName' to locate the appropriate one.
  std::string mName;
  std::vector<StructElem *> mStructElems;

public:
  StructBase(){}
  StructBase(const char *s) : mName(s) {}
  ~StructBase(){}

  void SetName(const char *s) { mName = s; }
  bool Empty() {return mStructElems.size() == 0;}
  void Dump();
  void Sort(unsigned i);  // sort by the length of i-th element which is astring
};

}

#endif
