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
#include <vector>
#include <cstring>

#include "spec_parser.h"
#include "base_gen.h"
#include "auto_gen.h"
#include "massert.h"

int main(int argc, char *argv[]) {
  // testing parse a def file
  // autogen -p test.spec

  // create a shared spec parser
  maplefe::SPECParser *parser = new maplefe::SPECParser();

  bool checkParserOnly = false;
  int verbose = 0;
  int fileIndex = 2;

  std::string lang;

  if (argc >=2) {
    for (int i = 1; i < argc; i++) {
      int len = strlen(argv[i]);
      if (!strncmp(argv[i], "-verbose=", 9)) {
        verbose = atoi(argv[i]+9);
      } else if (!strncmp(argv[i], "java", 4)) {
        lang = "java";
      } else if (!strncmp(argv[i], "typescript", 10)) {
        lang = "typescript";
      } else if (strcmp(argv[i], "-p") == 0) {
        checkParserOnly = true;
      } else {
        fileIndex = i;
      }
    }
  }

  // set verbose level
  parser->SetVerbose(verbose);

  if (checkParserOnly) {
    // make the parser to process the given spec file
    maplefe::BaseGen *bg = new maplefe::BaseGen(argv[fileIndex]);
    bg->SetParser(parser);
    bg->Parse();
    delete bg;
    return 0;
  }

  maplefe::AutoGen ag(parser);
  ag.SetLang(lang);
  ag.Gen();

  return 0;
}
