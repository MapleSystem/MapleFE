#ifndef __DEBUG_GEN_H__
#define __DEBUG_GEN_H__
#include "ruletable.h"
typedef struct {
  const RuleTable *mAddr;
  const char      *mName;
}RuleTableName;
extern RuleTableName gRuleTableNames[];
extern unsigned RuleTableNum;
extern const char* GetRuleTableName(const RuleTable*);
#endif
