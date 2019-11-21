#include <string>

#include "stringutil.h"

// Assume the 'l' or 'L' is removed by the caller.
long StringToDecNumeral(std::string s) {
  return std::stol(s);
}

// Assume the '0x' or '0X' is removed by the caller.
long StringToHexNumeral(std::string s){
  long value = 0;
  for (unsigned i = 0; i < s.length(); i++) {
    char c = s[i];
    int c_val;
    if (c <= '9' && c >= '0')
      c_val = c - '0';
    else if (c <= 'f' && c >= 'a')
      c_val = c - 'a' + 10;
    else if (c <= 'F' && c >= 'A')
      c_val = c - 'A' + 10;

    value = value * 16 + c_val;
  }
  return value;
}

// Assume the '0b' or '0B' is removed by the caller.
long StringToBinNumeral(std::string s){
  int value = 0;
  for (unsigned i = 0; i < s.length(); i++) {
    char c = s[i];
    int c_val;
    c_val = c - '0';
    value = value * 2 + c_val;
  }
  return value;
}

// Assume the leading '0' is removed by the caller.
long StringToOctNumeral(std::string s){
  return 0;
}

///////////////////////////////////////////////////////////////////////////////////////
//                  Common Implementation of StringToValue
// Most languages have the same numeral format. We'll reuse this as much as possible.
///////////////////////////////////////////////////////////////////////////////////////

int StringToValue::StringToInt(std::string &str) {
  int value;
  value = (int)StringToLong(str);
  return value;
}

long StringToValue::StringToLong(std::string &s) {
  long value;
  std::string value_string;
  bool is_hex = false;
  bool is_oct = false;
  bool is_bin = false;

  // check if it's hex
  if (s[0] == '0' && s[1] == 'x')
    is_hex = true;
  else if (s[0] == '0' && s[1] == 'X')
    is_hex = true;
  else if (s[0] == '0' && s[1] == 'b')
    is_bin = true;
  else if (s[0] == '0' && s[1] == 'B')
    is_bin = true;
  else if (s[0] == '0' && s.length() > 1)
    // Octal integer starts with '0' and followed by 0 to 7.
    // We dont check the following '0' to '7' since it's done by lexer.
    is_oct = true;

  if (is_hex || is_bin)
    value_string = s.substr(2);
  else if (is_oct)
    value_string = s.substr(1);
  else
    value_string = s;

  // Java allows '_', remove it.
  value_string = StringRemoveChar(value_string, '_');

  if (is_hex)
    value = StringToHexNumeral(value_string);
  else if (is_bin)
    value = StringToBinNumeral(value_string);
  else if (is_oct)
    value = StringToOctNumeral(value_string);
  else
    value = StringToDecNumeral(value_string);

  return value;
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


