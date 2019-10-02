#include "parser.h"
#include "driver.h"

// Constructors and Destructors
//
Driver::~Driver() {
}

void Driver::Run() {
  Parser *parser = mAutomata->GetParser();
  parser->Parse();
}
