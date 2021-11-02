#ifndef __RULE_SUMMARY_H__
#define __RULE_SUMMARY_H__
#include "ruletable.h"
#include "succ_match.h"
#include "token.h"
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

// The rule tables of autogen reserved rules.
extern RuleTable TblCHAR;
extern RuleTable TblDIGIT;
extern RuleTable TblASCII;
extern RuleTable TblESCAPE;
extern RuleTable TblHEXDIGIT;
extern RuleTable TblUTF8;
extern RuleTable TblIRREGULAR_CHAR;

extern RuleTable TblTemplateLiteral;
extern RuleTable TblRegularExpression;
extern RuleTable TblExpression;
extern RuleTable TblType;

//
extern RuleTable TblIdentifier;
extern RuleTable TblLiteral;
extern RuleTable TblIntegerLiteral;
extern RuleTable TblFPLiteral;
extern RuleTable TblBooleanLiteral;
extern RuleTable TblCharacterLiteral;
extern RuleTable TblStringLiteral;
extern RuleTable TblNullLiteral;

// The tokens defined by system
extern unsigned gSystemTokensNum;
extern unsigned gOperatorTokensNum;
extern unsigned gSeparatorTokensNum;
extern unsigned gKeywordTokensNum;
extern Token gSystemTokens[];
extern unsigned gAltTokensNum;
extern AltToken gAltTokens[];

}
#endif
