/////////////////////////////////////////////////////////////////////
// Log file 
//
/////////////////////////////////////////////////////////////////////

#ifndef __LOG_H__
#define __LOG_H__

#include <iostream>
#include <fstream>

#define LogInfo(msg)    { gLog << "Info: " << msg; }
#define LogWarning(msg) { gLog << "Warning: " << msg; }
#define LogFatal(msg)   { gLog << "Fatal: " << msg; }

class Log {
private:
  std::ofstream mFile;

public:
  typedef enum {
    LOG_NONE,
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARNING,
    LOG_ERROR,
    LOG_FATAL
  }Level;
  
  Log() {mFile.open("log");}
  ~Log(){mFile.close();}
  

  Log& operator<<(float f);
  Log& operator<<(int i);
  Log& operator<<(size_t st);
  Log& operator<<(const std::string &msg);
  Log& operator<<(const bool b);
  Log& operator<<(const char *s);
};

extern Log gLog;

#endif // __LOG_H__
