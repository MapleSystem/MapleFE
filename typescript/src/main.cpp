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

#include <fstream>      // std::ofstream
#include "parser.h"
#include "token.h"
#include "common_header_autogen.h"
#include "ruletable_util.h"
#include "gen_summary.h"
#include "gen_aststore.h"
#include "gen_astdump.h"
#include "gen_astgraph.h"
#include "lang_spec.h"

static void help() {
  std::cout << "ts2ast sourcefile [options]:\n" << std::endl;
  std::cout << "   --help            : print this help" << std::endl;
  std::cout << "   --trace-lexer     : Trace lexing" << std::endl;
  std::cout << "   --trace-table     : Trace rule table when entering and exiting" << std::endl;
  std::cout << "   --trace-left-rec  : Trace left recursion parsing" << std::endl;
  std::cout << "   --trace-appeal    : Trace appeal process" << std::endl;
  std::cout << "   --trace-failed    : Trace failed tokens of table" << std::endl;
  std::cout << "   --trace-timing    : Trace parsing time" << std::endl;
  std::cout << "   --trace-stack     : Trace visited-token stack of table" << std::endl;
  std::cout << "   --trace-sortout   : Trace SortOut" << std::endl;
  std::cout << "   --trace-ast-build : Trace AST Builder" << std::endl;
  std::cout << "   --trace-patch-was-succ : Trace Patching of WasSucc nodes" << std::endl;
  std::cout << "   --trace-warning   : Print Warning" << std::endl;
  std::cout << "   --dump-ast        : Dump AST in text format" << std::endl;
  std::cout << "   --dump-dot        : Dump AST in dot format" << std::endl;
}

int main (int argc, char *argv[]) {
  if (argc == 1 || (!strncmp(argv[1], "--help", 6) && (strlen(argv[1]) == 6))) {
    help();
    exit(-1);
  }

  maplefe::Parser *parser = new maplefe::TypescriptParser(argv[1]);

  bool dump_ast = false;
  bool dump_dot = false;
  bool succ;

  // Parse the argument
  for (unsigned i = 2; i < argc; i++) {
    if (!strncmp(argv[i], "--trace-lexer", 13) && (strlen(argv[i]) == 13)) {
      parser->SetLexerTrace();
    } else if (!strncmp(argv[i], "--trace-table", 13) && (strlen(argv[i]) == 13)) {
      parser->mTraceTable = true;
    } else if (!strncmp(argv[i], "--trace-left-rec", 16) && (strlen(argv[i]) == 16)) {
      parser->mTraceLeftRec = true;
    } else if (!strncmp(argv[i], "--trace-appeal", 14) && (strlen(argv[i]) == 14)) {
      parser->mTraceAppeal = true;
    } else if (!strncmp(argv[i], "--trace-stack", 13) && (strlen(argv[i]) == 13)) {
      parser->mTraceVisited = true;
    } else if (!strncmp(argv[i], "--trace-failed", 14) && (strlen(argv[i]) == 14)) {
      parser->mTraceFailed = true;
    } else if (!strncmp(argv[i], "--trace-timing", 14) && (strlen(argv[i]) == 14)) {
      parser->mTraceTiming = true;
    } else if (!strncmp(argv[i], "--trace-sortout", 15) && (strlen(argv[i]) == 15)) {
      parser->mTraceSortOut = true;
    } else if (!strncmp(argv[i], "--trace-ast-build", 17) && (strlen(argv[i]) == 17)) {
      parser->mTraceAstBuild = true;
    } else if (!strncmp(argv[i], "--trace-patch-was-succ", 22) && (strlen(argv[i]) == 22)) {
      parser->mTracePatchWasSucc = true;
    } else if (!strncmp(argv[i], "--trace-warning", 15) && (strlen(argv[i]) == 15)) {
      parser->mTraceWarning = true;
    } else if (!strncmp(argv[i], "--dump-ast", 10) && (strlen(argv[i]) == 10)) {
      dump_ast = true;
    } else if (!strncmp(argv[i], "--dump-dot", 10) && (strlen(argv[i]) == 10)) {
      dump_dot = true;
    } else {
      std::cerr << "unknown option " << argv[i] << std::endl;
      exit(-1);
    }
  }

  parser->InitRecursion();
  succ = parser->Parse();
  if (!succ) {
    delete parser;
    return 1;
  }

  // the module from parser
  maplefe::ModuleNode *module = parser->GetModule();

  if(dump_ast) {
    maplefe::AstDump astdump(module);
    astdump.Dump("ts2ast: Initial AST", &std::cout);
  }

  if(dump_dot) {
    maplefe::AstGraph graph(module);
    graph.DumpGraph("ts2ast: Initial AST", &std::cout);
  }

  maplefe::AstStore saveAst(module);
  saveAst.StoreInAstBuf();
  maplefe::AstBuffer &ast_buf = saveAst.GetAstBuf();

  std::ofstream ofs;
  std::string fname(module->GetFilename());
  fname += ".ast";
  ofs.open(fname, std::ofstream::out);
  const char *addr = (const char *)(&(ast_buf[0]));
  ofs.write(addr, ast_buf.size());
  ofs.close();

  delete parser;
  return 0;
}
