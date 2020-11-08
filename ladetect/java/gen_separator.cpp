#include "ruletable.h"
SepTableEntry SepTable[SEP_NA] = {
  {"...", SEP_Dotdotdot},
  {"::", SEP_Of},
  {"#", SEP_Pound},
  {"@", SEP_At},
  {":", SEP_Colon},
  {".", SEP_Dot},
  {",", SEP_Comma},
  {";", SEP_Semicolon},
  {"]", SEP_Rbrack},
  {"[", SEP_Lbrack},
  {"}", SEP_Rbrace},
  {"{", SEP_Lbrace},
  {")", SEP_Rparen},
  {"(", SEP_Lparen},
  {" ", SEP_Whitespace}
};
