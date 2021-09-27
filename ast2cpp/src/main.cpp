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

#include <sstream>
#include <fstream>
#include <iterator>
#include "gen_astload.h"
#include "ast_handler.h"
#include "ast2cpp.h"

static void help() {
  std::cout << "ast2cpp a.ast[,b.ast] [options]:" << std::endl;
  std::cout << "   --out=x.cpp      : cpp output file" << std::endl;
  std::cout << "   --help           : print this help" << std::endl;
  std::cout << "   --trace=n        : Emit trace with 4-bit combo levels 1...15" << std::endl;
  std::cout << "           1        : Emit ast tree visits" << std::endl;
  std::cout << "           2        : Emit graph" << std::endl;
  std::cout << "   --emit-ts-only   : Emit ts code only" << std::endl;
  std::cout << "   --emit-ts        : Emit ts code" << std::endl;
  std::cout << "   --format-cpp     : Format cpp" << std::endl;
  std::cout << "   --no-imported    : Do not process the imported modules" << std::endl;
  std::cout << "default out name uses the first input name: a.cpp" << std::endl;
}

int main (int argc, char *argv[]) {
  if (argc == 1 || (!strncmp(argv[1], "--help", 6) && (strlen(argv[1]) == 6))) {
    help();
    exit(-1);
  }

  unsigned flags;
  // one or more input .ast files separated by ','
  const char *inputname = argv[1];
  // output .cpp file
  const char *outputname = nullptr;

  // Parse the argument
  for (unsigned i = 2; i < argc; i++) {
    if (!strncmp(argv[i], "--trace=", 8)) {
      int val = atoi(argv[i] + 8);
      if (val < 1 || val > 15) {
        help();
        exit(-1);
      }
      flags |= val;
    } else if (!strncmp(argv[i], "--emit-ts-only", 14)) {
      flags |= maplefe::FLG_emit_ts_only;
    } else if (!strncmp(argv[i], "--emit-ts", 9)) {
      flags |= maplefe::FLG_emit_ts;
    } else if (!strncmp(argv[i], "--format-cpp", 12)) {
      flags |= maplefe::FLG_format_cpp;
    } else if (!strncmp(argv[i], "--no-imported", 13)) {
      flags |= maplefe::FLG_no_imported;
    } else if (!strncmp(argv[i], "--in=", 5)) {
      inputname = argv[i]+5;
    } else if (!strncmp(argv[i], "--out=", 6)) {
      outputname = argv[i]+6;
    } else {
      std::cerr << "unknown option " << argv[i] << std::endl;
      exit(-1);
    }
  }

  // input ast files
  std::vector<std::string> inputfiles;
  if (inputname) {
    std::stringstream ss;
    ss.str(inputname);
    std::string item;
    while (std::getline(ss, item, ',')) {
      // std::cout << "item " << item << " xxx"<< std::endl;
      inputfiles.push_back(item);
    }
  }

  unsigned trace = (flags & maplefe::FLG_trace);
  maplefe::AST_Handler handler(trace);
  for (auto astfile: inputfiles) {
    std::ifstream input(astfile, std::ifstream::binary);
    input >> std::noskipws;
    std::istream_iterator<uint8_t> s(input), e;
    maplefe::AstBuffer vec(s, e);
    maplefe::AstLoad loadAst;
    maplefe::ModuleNode *mod = loadAst.LoadFromAstBuf(vec);
    // add mod to the vector
    while(mod) {
      handler.AddModule(mod);
      mod = loadAst.Next();
    }
  }

  maplefe::A2C *a2c = new maplefe::A2C(&handler, flags);
  int res = a2c->ProcessAST();

  return res;
}
