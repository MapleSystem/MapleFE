//////////////////////////////////////////////////////////////////////////////
//                       Assertion Macros                                   //
//////////////////////////////////////////////////////////////////////////////

#ifndef __MACROS_H__
#define __MACROS_H__

#include <cstdio>
#include <cstdlib>

#define NEWLINE do { \
  std::cout << std::endl;\
} while (0)

#define MLOC do { \
  std::cout << "(" << __FILE__ << ":" << __LINE__ << ") ";\
} while (0)

#define MLOCENDL do { \
  std::cout << "(" << __FILE__ << ":" << __LINE__ << ") " << std::endl;\
} while (0)

#endif /* __MACROS_H__ */
