#ifndef __RULE_GEN_H__
#define __RULE_GEN_H__

#include "rule.h"
#include "buffer2write.h"

/////////////////////////////////////////////////////////////////////////
//                     Rule Generation                                 //
// A rule is dumped as an TableEntry. See shared/include/ruletable.h  //
// All tables are defined as global in .cpp file. Their extern declaration
// is in .h file.                                                      //
/////////////////////////////////////////////////////////////////////////

class RuleGen {
private:
  const Rule      *mRule;
  FormattedBuffer *mCppBuffer;
  FormattedBuffer *mHeaderBuffer;

private:
  unsigned mSubTblNum;

  std::string GetTblName(const Rule*);
  std::string GetSubTblName();
  std::string GetEntryTypeName(ElemType, RuleOp);

  std::string Gen4RuleElem(const RuleElem*);
  std::string Gen4TableData(const RuleElem*);

  // These are the two major interfaces used by Generate().
  // There are two scenarios they are called:
  //   1. Generate for the current Rule from the .spec
  //   2. Generate for the RuleElem in the Rule, aka. Sub Table.
  // This is why there are two parameters, but only one of them will be used.
  void Gen4Table(const Rule *, const RuleElem*);       // table def in .cpp
  void Gen4TableHeader(const Rule *, const RuleElem*); // table decl in .h

public:
  RuleGen(const Rule *r, FormattedBuffer *hbuf, FormattedBuffer *cbuf)
    : mRule(r), mHeaderBuffer(hbuf), mCppBuffer(cbuf), mSubTblNum(0) {}
  ~RuleGen() {}

  void Generate(); 
};

#endif
