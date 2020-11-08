#include "common_header_autogen.h"
TableData TblJavaChar_data[3] ={{DT_Subtable, &TblCHAR},{DT_Char, {.mChar='_'}},{DT_Char, {.mChar='$'}}};
RuleTable TblJavaChar ={ET_Oneof, RP_NA, 3, TblJavaChar_data, 0, NULL};
TableData TblCharOrDigit_data[2] ={{DT_Subtable, &TblCHAR},{DT_Subtable, &TblDIGIT}};
RuleTable TblCharOrDigit ={ET_Oneof, RP_NA, 2, TblCharOrDigit_data, 0, NULL};
TableData TblIdentifier_sub1_data[1] ={{DT_Subtable, &TblCharOrDigit}};
RuleTable TblIdentifier_sub1 ={ET_Zeroormore, RP_NA, 1, TblIdentifier_sub1_data, 0, NULL};
TableData TblIdentifier_data[2] ={{DT_Subtable, &TblJavaChar},{DT_Subtable, &TblIdentifier_sub1}};
RuleTable TblIdentifier ={ET_Concatenate, RP_NA, 2, TblIdentifier_data, 0, NULL};
