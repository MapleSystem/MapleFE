//////////////////////////////////////////////////////////////////////////
// This file contains all the functions that are used when we traverse the
// rule tables. Most of the functions are designed to do the following
// 1. Validity check
//    e.g. if an identifier is a type name, variable name, ...
//////////////////////////////////////////////////////////////////////////

#ifndef __RULE_TABLE_UTIL_JAVA_H__
#define __RULE_TABLE_UTIL_JAVA_H__

#include "ruletable_util.h"

class JavaValidityCheck {
public:
  bool IsPackageName(){return true;}
  bool IsTypeName(){return true;}
  bool IsVariable(){return true;}

// Java specific
public:
  bool TypeArgWildcardContain(){return true;}
};

#endif
