/////////////////////////////////////////////////////////////////////////////////
//            Language Specific Implementations                                //
/////////////////////////////////////////////////////////////////////////////////

#ifndef __LANG_SPEC_H__
#define __LANG_SPEC_H__

#include "stringutil.h"
#include "token.h"

class StringToValueImpl : public StringToValue {
public:
  int    StringToInt(std::string &s) {return 0;}
  float  StringToFloat(std::string &s) {return 0.0;}
  double StringToDouble(std::string &s) {return 0.0;}
  bool   StringToBool(std::string &s) {return false;}
  char   StringToChar(std::string &s) {return 'c';}
  bool   StringIsNull(std::string &s) {return false;}
};

extern LitData ProcessLiteral(LT_Type type, const char *str);
#endif
