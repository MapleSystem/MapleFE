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

#include <fstream>
#include "gen_astload.h"
#include "ast2cpp.h"

static void help() {
  std::cout << "ts2cpp sourcefile [options]:\n" << std::endl;
  std::cout << "   --help            : print this help" << std::endl;
  std::cout << "   --trace-a2c       : Trace MPL Builder" << std::endl;
}

int main (int argc, char *argv[]) {
  if (argc == 1 || (!strncmp(argv[1], "--help", 6) && (strlen(argv[1]) == 6))) {
    help();
    exit(-1);
  }

  bool trace_a2c = false;

  // Parse the argument
  for (unsigned i = 2; i < argc; i++) {
    if (!strncmp(argv[i], "--trace-a2c", 11) && (strlen(argv[i]) == 11)) {
      trace_a2c = true;
    } else {
      std::cerr << "unknown option " << argv[i] << std::endl;
      exit(-1);
    }
  }

  std::ifstream input(argv[1], std::ifstream::binary);
  // get length of file:
  input.seekg(0, input.end);
  int length = input.tellg();
  input.seekg(0, input.beg);
  char *buf = (char*)calloc(length, 1);
  input.read(buf, length);

  maplefe::AstLoad loadAst;
  maplefe::AstBuffer vec(buf, buf + length);
  maplefe::gModule = loadAst.LoadFromAstBuf(vec);

  maplefe::A2C *a2c = new maplefe::A2C(maplefe::gModule->GetFileName());
  a2c->ProcessAST(trace_a2c);

  delete buf;
  return 0;
}
