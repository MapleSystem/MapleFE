#include <string>

#include "stringutil.h"

int StringToValue::StringToInt(std::string &str) {
  return stoi(str);
}

float StringToValue::StringToFloat(std::string &str) {
  return stof(str);
}

double StringToValue::StringToDouble(std::string &str) {
  return stod(str);
}

bool StringToValue::StringToBool(std::string &str) {
  bool b = false;
  std::string true_text("true");
  std::string false_text("false");
  if (StringEqualNoCase(true_text, str))
    b = true;
  else if (StringEqualNoCase(false_text, str))
    b = false;
  else
    MASSERT(0 && "Wrong boolean string, only accepts true/false");
  return b;
}

// Just take in the first char
char StringToValue::StringToChar(std::string &str) {
  const char *s = str.c_str();
  return *s;
}

// Some languares have 'null' keyword with Null type.
bool StringToValue::StringIsNull(std::string &str) {
  bool b = false;
  if (StringEqualNoCase("null", str))
    b = true;
  return b;
}


