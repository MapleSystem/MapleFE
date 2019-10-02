#include "log.h"

// The global Log
Log gLog;

Log& Log::operator<< (float f) {
  mFile << f;
  return *this;
}

Log& Log::operator<<(int i) {
  mFile << i;
  return *this;
}

Log& Log::operator<<(size_t st) {
  mFile << st;
  return *this;
}

Log& Log::operator<<(const std::string &msg) {
  mFile << msg.c_str();
  return *this;
}

Log& Log::operator<<(const bool b) {
  mFile << b;
  return *this;
}

Log& Log::operator<<(const char *s) {
  mFile << s;
  return *this;
}

