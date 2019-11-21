#include "lang_spec.h"

// For all the string to value functions below, we assume the syntax of 's' is correct
// for a literal in Java.

float  StringToValueImpl::StringToFloat(std::string &s) {return 0.0;}
double StringToValueImpl::StringToDouble(std::string &s) {return 0.0;}
bool   StringToValueImpl::StringToBool(std::string &s) {return false;}
char   StringToValueImpl::StringToChar(std::string &s) {return 'c';}
bool   StringToValueImpl::StringIsNull(std::string &s) {return false;}


// Each language has its own format of literal. So this function handles Java literals.
// It translate a string into a literal.
//
// 'str' is in the Lexer's string pool.
//
LitData ProcessLiteral(LT_Type type, const char *str) {
  LitData data;
  std::string value_text(str);
  StringToValueImpl s2v;

  switch (type) {
  case LT_IntegerLiteral: {
    int i = s2v.StringToInt(value_text);
    data.mType = LT_IntegerLiteral;
    data.mData.mInt = i;
    break; }
  case LT_FPLiteral: {
    float f = s2v.StringToFloat(value_text);
    data.mType = LT_FPLiteral;
    data.mData.mFloat = f;
    break; }
  case LT_BooleanLiteral: {
    bool b = s2v.StringToBool(value_text);
    data.mType = LT_BooleanLiteral;
    data.mData.mBool = b;
    break; }
  case LT_CharacterLiteral: {
    char c = s2v.StringToChar(value_text);
    data.mType = LT_CharacterLiteral;
    data.mData.mChar = c;
    break; }
  case LT_StringLiteral: {
    data.mType = LT_StringLiteral;
    data.mData.mStr = str;
    break; }
  case LT_NullLiteral: {
    // Just need set the type
    data.mType = LT_NullLiteral;
    break; }
  case LT_NA:    // N/A,
  default:
    data.mType = LT_NA;
    break;
  }

  return data;
}


