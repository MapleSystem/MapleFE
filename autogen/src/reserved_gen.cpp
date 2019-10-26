#include <cstring>

#include "reserved_gen.h"
#include "rule.h"

// Add the reserved Ops and Elems
ReservedGen::ReservedGen(const char *dfile, const char *hf, const char *cppf)
          : BaseGen(dfile, hf, cppf) {
  ReservedOp oneof = {"ONEOF", RO_Oneof};
  ReservedOp zeroplus = {"ZEROORMORE", RO_Zeroormore};
  ReservedOp zeroorone = {"ZEROORONE", RO_Zeroorone};

  mOps.push_back(oneof);
  mOps.push_back(zeroplus);
  mOps.push_back(zeroorone);
}

// Needs to override BaseGen::Run, since we dont need process
// STRUCT data
void ReservedGen::Run(SPECParser *parser) {
  SetParser(parser);
  Parse();
}

/////////////////////////////////////////////////////////////////////
//                   Generate the output files                     //
/////////////////////////////////////////////////////////////////////

void ReservedGen::Generate() {
  GenRuleTables();
  GenHeaderFile();
  GenCppFile();
}

void ReservedGen::GenHeaderFile() {
  mHeaderFile.WriteOneLine("#ifndef __RESERVED_GEN_H__", 26);
  mHeaderFile.WriteOneLine("#define __RESERVED_GEN_H__", 26);
  mHeaderFile.WriteOneLine("#include \"ruletable.h\"", 22);
  mHeaderFile.WriteFormattedBuffer(&mRuleTableHeader);
  mHeaderFile.WriteOneLine("#endif", 6);
}

void ReservedGen::GenCppFile() {
  mCppFile.WriteOneLine("#include \"common_header_autogen.h\"", 34);
  mCppFile.WriteFormattedBuffer(&mRuleTableCpp);
}

