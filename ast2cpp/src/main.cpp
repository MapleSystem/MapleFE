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
#include "gen_astload.h"
#include "ast_handler.h"
#include "cpp_emitter.h"
#include "ast2cpp.h"

static void help() {
  std::cout << "ast2cpp [options]:" << std::endl;
  std::cout << "   --in=a.ast,b.ast : ast input files" << std::endl;
  std::cout << "   --out=x.cpp      : cpp output file" << std::endl;
  std::cout << "   --help           : print this help" << std::endl;
  std::cout << "   --trace-a2c      : Trace MPL Builder" << std::endl;
  std::cout << "default out name is the first input name + .cpp" << std::endl;
}

int main (int argc, char *argv[]) {
  if (argc == 1 || (!strncmp(argv[1], "--help", 6) && (strlen(argv[1]) == 6))) {
    help();
    exit(-1);
  }

  bool trace_a2c = false;
  // one or more input .ast files separated by ','
  const char *inputname = nullptr;
  // output .cpp file
  const char *outputname = nullptr;

  // Parse the argument
  for (unsigned i = 1; i < argc; i++) {
    if (!strncmp(argv[i], "--trace-a2c", 11) && (strlen(argv[i]) == 11)) {
      trace_a2c = true;
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
      //*(result++) = item;
      std::cout << "item " << item << " xxx"<< std::endl;
      inputfiles.push_back(item);
    }
  }

  maplefe::AST_Handler handler(trace_a2c);
  const char *name0 = nullptr;
  for (auto astfile: inputfiles) {
    std::ifstream input(astfile, std::ifstream::binary);
    // get length of file:
    input.seekg(0, input.end);
    int length = input.tellg();
    input.seekg(0, input.beg);
    char *buf = (char*)calloc(length, 1);
    input.read(buf, length);

    maplefe::AstLoad loadAst;
    maplefe::AstBuffer vec(buf, buf + length);
    maplefe::ModuleNode *mod = loadAst.LoadFromAstBuf(vec);
    // add mod to the vector
    handler.AddModule(mod);
    if (!name0) {
      name0 = mod->GetFileName();
    }
  }

  if (!outputname) {
    std::string str(name0);
    str += ".cpp";
    outputname = strdup(str.c_str());
  }
  handler.SetOutputFileName(outputname);

  maplefe::A2C *a2c = new maplefe::A2C(&handler, trace_a2c);
  a2c->ProcessAST();

  if (trace_a2c) {
    std::cout << "============= Emitter ===========" << std::endl;
    maplefe::Emitter emitter(&handler);
    std::string code = emitter.Emit("Convert AST to TypeScript code");
    std::cout << code;
  }

  maplefe::CppEmitter cppemitter(&handler);
  std::string cppcode = cppemitter.Emit("Convert AST to C++ code");
  std::cout << cppcode;

  std::ofstream out(outputname);
  out << cppcode;
  out.close();

  return 0;
}
