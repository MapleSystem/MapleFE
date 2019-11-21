/////////////////////////////////////////////////////////////////////////////////
//            Language Specific Implementations                                //
/////////////////////////////////////////////////////////////////////////////////

#ifndef __LANG_SPEC_H__
#define __LANG_SPEC_H__

#include "stringutil.h"
#include "token.h"

class StringToValueImpl : public StringToValue {
public:
  float  StringToFloat(std::string &s);
  double StringToDouble(std::string &s);
  bool   StringToBool(std::string &s);
  char   StringToChar(std::string &s);
  bool   StringIsNull(std::string &s);
};

extern LitData ProcessLiteral(LT_Type type, const char *str);
#endif
