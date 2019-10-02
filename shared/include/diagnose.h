///////////////////////////////////////////////////////////////////////////
// Diagnose records all the language warnings/errors during parsing.     //
//                                                                       //
///////////////////////////////////////////////////////////////////////////

#ifndef __DIAGNOSE_H__
#define __DIAGNOSE_H__

#include <vector>

class DiagMessage {
public:
  char     File[128];
  unsigned Line;
  unsigned Col;
  char     Msg[256];
public:
  DiagMessage(const char*, unsigned, unsigned, const char*); 
}

// We save OBJECT not POINTER of DiagMessage in Diagnose.
class Diagnose {
public:
  std::vector<DiagMessage>   mWarnings;
  std::vector<DiagMessage>   mErrors;
public:
  void AddWarning(const char*, unsigned, unsigned, const char*);
  void AddError(const char*, unsigned, unsigned, const char*);
};

#endif

