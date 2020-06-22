/*
* Copyright (C) [2020] Futurewei Technologies, Inc. All rights reverved.
*
* OpenArkFE is licensed under the Mulan PSL v1.
* You can use this software according to the terms and conditions of the Mulan PSL v1.
* You may obtain a copy of Mulan PSL v1 at:
*
*  http://license.coscl.org.cn/MulanPSL
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR
* FIT FOR A PARTICULAR PURPOSE.
* See the Mulan PSL v1 for more details.
*/
#include "parser.h"
#include "token.h"
#include "common_header_autogen.h"
#include "ruletable_util.h"
#include "gen_debug.h"
#include "vfy_java.h"

static void help() {
  std::cout << "java2mpl <arguments>:\n" << std::endl;
  std::cout << "   --help            : print this help" << std::endl;
  std::cout << "   --trace-lexer     : Trace lexing" << std::endl;
  std::cout << "   --trace-table     : Trace rule table when entering and exiting" << std::endl;
  std::cout << "   --trace-left-rec  : Trace left recursion parsing" << std::endl;
  std::cout << "   --trace-appeal    : Trace appeal process" << std::endl;
  std::cout << "   --trace-second-try: Trace parser second try." << std::endl;
  std::cout << "   --trace-failed    : Trace failed tokens of table" << std::endl;
  std::cout << "   --trace-stack     : Trace visited-token stack of table" << std::endl;
  std::cout << "   --trace-sortout   : Trace SortOut" << std::endl;
  std::cout << "   --trace-ast-build : Trace AST Builder" << std::endl;
  std::cout << "   --trace-patch-was-succ : Trace Patching of WasSucc nodes" << std::endl;
  std::cout << "   --trace-warning   : Print Warning" << std::endl;
}

int main (int argc, char *argv[]) {
  Parser *parser = new Parser(argv[1]);

  // Parse the argument
  for (unsigned i = 1; i < argc; i++) {
    if (!strncmp(argv[i], "--help", 6) && (strlen(argv[i]) == 6)) {
      help();
      exit(-1);
    } else if (!strncmp(argv[i], "--trace-lexer", 13) && (strlen(argv[i]) == 13)) {
      parser->SetLexerTrace();
    } else if (!strncmp(argv[i], "--trace-table", 13) && (strlen(argv[i]) == 13)) {
      parser->mTraceTable = true;
    } else if (!strncmp(argv[i], "--trace-left-rec", 16) && (strlen(argv[i]) == 16)) {
      parser->mTraceLeftRec = true;
    } else if (!strncmp(argv[i], "--trace-appeal", 14) && (strlen(argv[i]) == 14)) {
      parser->mTraceAppeal = true;
    } else if (!strncmp(argv[i], "--trace-second-try", 18) && (strlen(argv[i]) == 18)) {
      parser->mTraceSecondTry = true;
    } else if (!strncmp(argv[i], "--trace-stack", 13) && (strlen(argv[i]) == 13)) {
      parser->mTraceVisited = true;
    } else if (!strncmp(argv[i], "--trace-failed", 14) && (strlen(argv[i]) == 14)) {
      parser->mTraceFailed = true;
    } else if (!strncmp(argv[i], "--trace-sortout", 15) && (strlen(argv[i]) == 15)) {
      parser->mTraceSortOut = true;
    } else if (!strncmp(argv[i], "--trace-ast-build", 17) && (strlen(argv[i]) == 17)) {
      parser->mTraceAstBuild = true;
    } else if (!strncmp(argv[i], "--trace-patch-was-succ", 22) && (strlen(argv[i]) == 22)) {
      parser->mTracePatchWasSucc = true;
    } else if (!strncmp(argv[i], "--trace-warning", 15) && (strlen(argv[i]) == 15)) {
      parser->mTraceWarning = true;
    }
  }

  parser->InitPredefinedTokens();
  parser->mLexer->PlantTokens();
  parser->SetupTopTables();
  parser->InitRecursion();
  parser->Parse();

  VerifierJava vfy_java;
  vfy_java.Do();

  delete parser;

  return 0;
}
