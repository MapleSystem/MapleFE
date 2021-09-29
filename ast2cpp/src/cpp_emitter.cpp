/*
 * Copyright (C) [2021] Futurewei Technologies, Inc. All rights reverved.
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

#include <fstream>
#include <cstdlib>
#include "cpp_definition.h"
#include "cpp_declaration.h"
#include "cpp_emitter.h"

namespace maplefe {

bool CppEmitter::EmitCxxFiles() {
  unsigned size = mASTHandler->mModuleHandlers.GetNum();
  for (int i = 0; i < size; i++) {
    Module_Handler *handler = mASTHandler->mModuleHandlers.ValueAtIndex(i);
    CppDecl decl(handler);
    { // Emit C++ header file
      std::string decl_code = decl.Emit();
      std::string fn = decl.GetBaseFilename() + ".h"s;
      std::ofstream out(fn.c_str(), std::ofstream::out);
      out << decl_code;
      out.close();
      if (mFlags & FLG_format_cpp) {
        std::string cmd = "clang-format-10 -i --sort-includes=0 "s + fn;
        std::system(cmd.c_str());
      }
    }
    { // Emit C++ implementation file
      CppDef def(handler, decl);
      std::string def_code = def.Emit();
      std::string fn = def.GetBaseFilename() + ".cpp"s;
      std::ofstream out(fn.c_str(), std::ofstream::out);
      out << def_code;
      out.close();
      if (mFlags & FLG_format_cpp) {
        std::string cmd = "clang-format-10 -i --sort-includes=0 "s + fn;
        std::system(cmd.c_str());
      }
    }
  }
  return true;
}

} // namespace maplefe
