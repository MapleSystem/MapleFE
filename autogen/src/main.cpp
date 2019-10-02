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
  SPECParser *parser = new SPECParser();

  bool checkParserOnly = false;
  int verbose = 0;
  int fileIndex = 2;

  if (argc >=2) {
    for (int i = 1; i < argc; i++) {
      int len = strlen(argv[i]);
      if (!strncmp(argv[i], "-verbose=", 9)) {
        verbose = atoi(argv[i]+9);
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
    BaseGen *bg = new BaseGen(argv[fileIndex]);
    bg->SetParser(parser);
    bg->Parse();
    delete bg;
    return 0;
  }

  AutoGen *ag = new AutoGen(parser);
  ag->Gen();

  delete ag;
  return 0;
}

