#include "all_supported.h"

//////////////   Types supported /////////////////
#undef  TYPE
#define TYPE(T) {#T, AG_TY_##T},
TypeMapping TypesSupported[AG_TY_NA] = {
#include "supported_types.def"
};

AGTypeId FindAGTypeIdLangIndep(const std::string &s) {
  for (unsigned u = 0; u < AG_TY_NA; u++) {
    if (!TypesSupported[u].mName.compare(0, s.length(), s))
      return TypesSupported[u].mType;
  }
  return AG_TY_NA;
}

#undef  TYPE
#define TYPE(T) case AG_TY_##T: return #T;
char *GetTypeString(AGTypeId tid) {
  switch (tid) {
#include "supported_types.def"
  default:
    return "NA";
  }
};

//////////////   literals supported /////////////////

#undef  LITERAL
#define LITERAL(S) {#S, LIT_##S},
LiteralSuppStruct LiteralsSupported[LIT_NA] = {
#include "supported_literals.def"
};

// s : the name 
// return the LiteralId
LiteralId FindLiteralId(const std::string &s) {
  for (unsigned u = 0; u < LIT_NA; u++) {
    if (!LiteralsSupported[u].mName.compare(0, s.length(), s))
      return LiteralsSupported[u].mLiteralId;
  }
  return LIT_NA;
}

std::string FindLiteralName(LiteralId id) {
  std::string s("");
  for (unsigned u = 0; u < LIT_NA; u++) {
    if (LiteralsSupported[u].mLiteralId == id){
      s = LiteralsSupported[u].mName;
      break;
    }
  }
  return s;
}
