#ifndef __RULE_SUMMARY_H__
#define __RULE_SUMMARY_H__
#include "ruletable.h"
#include "succ_match.h"
namespace maplefe {
typedef struct {
  const RuleTable *mAddr;
  const char      *mName;
  unsigned         mIndex;
}RuleTableSummary;
extern RuleTableSummary gRuleTableSummarys[];
extern unsigned RuleTableNum;
extern const char* GetRuleTableName(const RuleTable*);
class BitVector;
extern BitVector gFailed[];
class SuccMatch;
extern SuccMatch gSucc[];
extern unsigned gTopRulesNum;
extern RuleTable* gTopRules[];
}
#endif
