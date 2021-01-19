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

#ifndef __FILE_WRITE_H__
#define __FILE_WRITE_H__

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "write2file.h"
#include "buffer2write.h"

namespace maplefe {

class FileWriter : public Write2File {
public:
  FileWriter() {}
  FileWriter(const std::string &s) : Write2File(s) {}
  ~FileWriter(){}

  void WriteSimpleBuffers(const FormattedBuffer *);

  // Write a formatted buffer.
  void WriteFormattedBuffer(const FormattedBuffer *);
  void WriteIfBuffer(const IfBuffer*);
  void WriteScopedBuffer(const ScopedBuffer*);
};

}

#endif
