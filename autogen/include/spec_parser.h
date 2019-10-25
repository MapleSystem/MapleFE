/////////////////////////////////////////////////////////////////////
// This is the base functions of auto generation.                  //
// Each individual part is supposed to inherit from this class.    //
/////////////////////////////////////////////////////////////////////

#ifndef __SPEC_PARSER_H__
#define __SPEC_PARSER_H__

#include <iostream>
#include <fstream>
#include <stack>

#include "rule.h"
#include "base_struct.h"
#include "spec_lexer.h"

#include "stringpool.h"

class MemPool;
class RuleElemPool;
class ReservedGen;
class AutoGen;
class BaseGen;

class SPECParser {
public:
  SPECLexer    *mLexer;
  AutoGen      *mAutoGen;
  BaseGen      *mBaseGen;
  Rule         *mCurrrule;

public:
  SPECParser() { mLexer = new SPECLexer(); }
  SPECParser(const std::string &dfile) { mLexer = new SPECLexer(); ResetParser(dfile); }
  ~SPECParser() { delete mLexer; }

  // for all ParseXXX routines
  // Return true  : succeed
  //        false : failed
  bool Parse();
  void ResetParser(const std::string &dfile);

  bool ParseRule();
  RuleAction *GetAction();
  bool ParseActionFunc(RuleElem *&elem);
  bool ParseElement(RuleElem *&elem, bool allowConcat);
  bool ParseElementSet(RuleElem *parent);
  bool ParseConcatenate(RuleElem *parent);

  bool ParseStruct();
  bool ParseStructElements();
  bool ParseElemData(StructElem *elem);
  
  bool ParseType();

  bool ParseAttr();
  bool ParseAttrType();
  bool ParseAttrValidity();
  bool ParseAttrAction();

  void SetAutoGen(AutoGen *ag) { mAutoGen = ag; }

  void SetVerbose(int i) { mLexer->SetVerbose(i); }
  int GetVerbose() { return mLexer->GetVerbose(); }

  void DumpRules();
  void DumpStruct();
  void Dump() { DumpStruct(); DumpRules(); }
};

#endif
