#include "spec_parser.h"
#include "auto_gen.h"
#include "driver.h"
#include "parser.h"
#include "automata.h"
#include "module.h"

// note: this sequence of operations is for the purpose of proof concept
//
int main (int argc, char *argv[]) {
  // create a shared spec parser
  SPECParser *specParser = new SPECParser();
  int verbose = 0;
  int fileIndex = 1;

  if (argc >=2) {
    for (int i = 1; i < argc; i++) {
      int len = strlen(argv[i]);
      if (!strncmp(argv[i], "-verbose=", 9)) {
        verbose = atoi(argv[i]+9);
      } else {
        fileIndex = i;
      }
    }
  }

  // set verbose level
  specParser->SetVerbose(verbose);

  // process AutoGen
  AutoGen *ag = new AutoGen(specParser);
  ag->Gen();

  // process additional spec files
  BaseGen *bg = new BaseGen("tmp.spec");
  bg->SetReserved(ag->GetReservedGen());
  bg->SetParser(specParser);
  bg->Parse();
  // expr.spec
  bg->SetParser(specParser, "expr.spec");
  bg->Parse();
  // stmt.spec
  bg->SetParser(specParser, "stmt.spec");
  bg->Parse();

  bg->BackPatch(ag->GetGenArray());

  Module *module = new Module();

  Parser *parser = new Parser(argv[fileIndex], module);
  // set verbose level
  parser->SetVerbose(verbose);

  // build automata
  Automata *automata = new Automata(ag, bg, parser);

  // set up mToken fields in the rules
  // for string literals like "new", "<", ...
  // build database for matching rules
  automata->ProcessRules();

  // driver to process source code
  Driver *driver = new Driver(automata);

  driver->Run();

  delete driver;
  delete automata;
  delete ag;
  delete bg;
  delete parser;
  delete specParser;
  return 0;
}
