//////////////////////////////////////////////////////////////////////////////
//                       Assertion Macros                                   //
//////////////////////////////////////////////////////////////////////////////

#ifndef __MASSERT_H__
#define __MASSERT_H__

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include "macros.h"

#define EXIT_ERROR   1
#define EXIT_SUCCESS 0

#define MASSERT(...) assert(__VA_ARGS__)

#define MERROR(...) do { \
  fprintf(stderr, "ERROR: (%s:%d) ", __FILE__, __LINE__); \
  fprintf(stderr, __VA_ARGS__); \
  fprintf(stderr, "\n"); \
  exit(EXIT_ERROR); \
} while (0)

#define MWARNING(...) do { \
  fprintf(stderr, "WARNING: (%s:%d) ", __FILE__, __LINE__); \
  fprintf(stderr, __VA_ARGS__); \
  fprintf(stderr, "\n"); \
} while (0)

#define MNYI(...) do { \
  fprintf(stderr, "Not Yet Implemented: (%s:%d) ", __FILE__, __LINE__); \
  fprintf(stderr, __VA_ARGS__); \
  fprintf(stderr, "\n"); \
} while (0)

#define MMSG0(val) do { \
  MLOC; \
  std::cout << val << std::endl; \
} while (0)

#define MMSG(msg, val) do { \
  MLOC; \
  std::cout << msg << " " << val << std::endl; \
} while (0)

#define MMSGNOLOC(msg, val) do { \
  std::cout << msg << " " << val << std::endl; \
} while (0)

#define MMSG2(msg, val1, val2) do { \
  MLOC; \
  std::cout << msg << " " << val1 << " " << val2 << std::endl; \
} while (0)

#define MMSG3(msg, val1, val2, val3) do { \
  MLOC; \
  std::cout << msg << " " << val1 << " " << val2 << " " << val3 << std::endl; \
} while (0)

#define MMSGA(msg, val) do { \
  MLOC; \
  std::cout << msg << " " << val << std::endl; \
  assert(0); \
} while (0)

#define MMSGA2(msg, val1, val2) do { \
  MLOC; \
  std::cout << msg << " " << val1 << " " << val2 << std::endl; \
  assert(0); \
} while (0)

#define MMSGA3(msg, val1, val2, val3) do { \
  MLOC; \
  std::cout << msg << " " << val1 << " " << val2 << " " << val3 << std::endl; \
  assert(0); \
} while (0)

////////////////////////////////////////////////////////////////////////
//  Dumping Interfaces without FILE/LINE
////////////////////////////////////////////////////////////////////////

#define DUMP0(msg) do { \
  std::cout << msg << std::endl; \
} while (0)

#define DUMP1(msg, val) do { \
  std::cout << msg << " " << val << std::endl; \
} while (0)

#define DUMP2(msg, val1, val2) do { \
  std::cout << msg << " " << val1 << " " << val2 << std::endl; \
} while (0)

#endif /* __MASSERT_H__ */
