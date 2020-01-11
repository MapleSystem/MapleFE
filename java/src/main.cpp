#include "driver.h"
#include "parser.h"
#include "token.h"
#include "common_header_autogen.h"
#include "ruletable_util.h"
#include "gen_debug.h"

static void help() {
  std::cout << "java2mpl <arguments>:\n" << std::endl;
  std::cout << "   --help            : print this help" << std::endl;
  std::cout << "   --trace-table     : Trace rule table when entering and exiting" << std::endl;
  std::cout << "   --trace-appeal    : Trace appeal process" << std::endl;
  std::cout << "   --trace-second-try: Trace parser second try." << std::endl;
  std::cout << "   --trace-failed    : Trace failed tokens of table" << std::endl;
  std::cout << "   --trace-stack     : Trace visited-token stack of table" << std::endl;
  std::cout << "   --trace-sortout   : Trace SortOut" << std::endl;
}

int main (int argc, char *argv[]) {
  Parser *parser = new Parser(argv[1]);

  // Parse the argument
  for (unsigned i = 1; i < argc; i++) {
    if (!strncmp(argv[i], "--help", 6) && (strlen(argv[i]) == 6)) {
      help();
      exit(-1);
    } else if (!strncmp(argv[i], "--trace-table", 13) && (strlen(argv[i]) == 13)) {
      parser->mTraceTable = true;
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
    }
  }

  parser->InitPredefinedTokens();
  PlantTokens(parser->mLexer);
  parser->SetupTopTables();
  parser->Parse();
  delete parser;
  return 0;
}
