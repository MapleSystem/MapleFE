////////////////////////////////////////////////////////////////////////
// This is the driver of the frontend. It just defines the common
// workflow. The language specific handling should be done in the 
// specific children classes.
////////////////////////////////////////////////////////////////////////

#ifndef __DRIVER_H__
#define __DRIVER_H__

#include "automata.h"
class Parser;

class Driver {
private:
  std::string FileName;
  Automata   *mAutomata;

public:
  Driver(Automata *a) : mAutomata(a) {}
  ~Driver();

  void Run();
};

#endif
