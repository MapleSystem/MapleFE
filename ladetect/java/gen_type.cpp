#include "common_header_autogen.h"
TypeKeyword TypeKeywordTable[TY_NA] = {
  {"boolean", TY_Boolean},
  {"byte", TY_Byte},
  {"short", TY_Short},
  {"int", TY_Int},
  {"long", TY_Long},
  {"char", TY_Char},
  {"float", TY_Float},
  {"double", TY_Double},
  {"void", TY_Void},
  {"null", TY_Null}
};
TableData TblBoolType_data[1] ={{DT_Token, {.mTokenId=68}}};
RuleTable TblBoolType ={ET_Data, RP_NA, 1, TblBoolType_data, 0, NULL};
TableData TblIntType_data[5] ={{DT_Token, {.mTokenId=96}},{DT_Token, {.mTokenId=85}},{DT_Token, {.mTokenId=101}},{DT_Token, {.mTokenId=93}},{DT_Token, {.mTokenId=95}}};
RuleTable TblIntType ={ET_Oneof, RP_NA, 5, TblIntType_data, 0, NULL};
TableData TblFPType_data[2] ={{DT_Token, {.mTokenId=86}},{DT_Token, {.mTokenId=77}}};
RuleTable TblFPType ={ET_Oneof, RP_NA, 2, TblFPType_data, 0, NULL};
TableData TblNumericType_data[2] ={{DT_Subtable, &TblIntType},{DT_Subtable, &TblFPType}};
RuleTable TblNumericType ={ET_Oneof, RP_NA, 2, TblNumericType_data, 0, NULL};
TableData TblPrimitiveType_sub2_data[1] ={{DT_Subtable, &TblAnnotation}};
RuleTable TblPrimitiveType_sub2 ={ET_Zeroormore, RP_NA, 1, TblPrimitiveType_sub2_data, 0, NULL};
TableData TblPrimitiveType_sub1_data[2] ={{DT_Subtable, &TblPrimitiveType_sub2},{DT_Subtable, &TblNumericType}};
RuleTable TblPrimitiveType_sub1 ={ET_Concatenate, RP_NA, 2, TblPrimitiveType_sub1_data, 0, NULL};
TableData TblPrimitiveType_sub4_data[1] ={{DT_Subtable, &TblAnnotation}};
RuleTable TblPrimitiveType_sub4 ={ET_Zeroormore, RP_NA, 1, TblPrimitiveType_sub4_data, 0, NULL};
TableData TblPrimitiveType_sub3_data[2] ={{DT_Subtable, &TblPrimitiveType_sub4},{DT_Subtable, &TblBoolType}};
RuleTable TblPrimitiveType_sub3 ={ET_Concatenate, RP_NA, 2, TblPrimitiveType_sub3_data, 0, NULL};
TableData TblPrimitiveType_data[2] ={{DT_Subtable, &TblPrimitiveType_sub1},{DT_Subtable, &TblPrimitiveType_sub3}};
RuleTable TblPrimitiveType ={ET_Oneof, RP_NA, 2, TblPrimitiveType_data, 0, NULL};
TableData TblNullType_data[1] ={{DT_Type, {.mTypeId=TY_Null}}};
RuleTable TblNullType ={ET_Data, RP_NA, 1, TblNullType_data, 0, NULL};
TableData TblWildcardBounds_sub1_data[2] ={{DT_Token, {.mTokenId=64}},{DT_Subtable, &TblReferenceType}};
RuleTable TblWildcardBounds_sub1 ={ET_Concatenate, RP_NA, 2, TblWildcardBounds_sub1_data, 0, NULL};
TableData TblWildcardBounds_sub2_data[2] ={{DT_Token, {.mTokenId=80}},{DT_Subtable, &TblReferenceType}};
RuleTable TblWildcardBounds_sub2 ={ET_Concatenate, RP_NA, 2, TblWildcardBounds_sub2_data, 0, NULL};
TableData TblWildcardBounds_data[2] ={{DT_Subtable, &TblWildcardBounds_sub1},{DT_Subtable, &TblWildcardBounds_sub2}};
RuleTable TblWildcardBounds ={ET_Oneof, RP_NA, 2, TblWildcardBounds_data, 0, NULL};
TableData TblWildcard_sub1_data[1] ={{DT_Subtable, &TblAnnotation}};
RuleTable TblWildcard_sub1 ={ET_Zeroormore, RP_NA, 1, TblWildcard_sub1_data, 0, NULL};
TableData TblWildcard_sub2_data[1] ={{DT_Subtable, &TblWildcardBounds}};
RuleTable TblWildcard_sub2 ={ET_Zeroorone, RP_NA, 1, TblWildcard_sub2_data, 0, NULL};
TableData TblWildcard_data[3] ={{DT_Subtable, &TblWildcard_sub1},{DT_Token, {.mTokenId=23}},{DT_Subtable, &TblWildcard_sub2}};
RuleTable TblWildcard ={ET_Concatenate, RP_NA, 3, TblWildcard_data, 0, NULL};
TableData TblTypeArgument_data[2] ={{DT_Subtable, &TblReferenceType},{DT_Subtable, &TblWildcard}};
RuleTable TblTypeArgument ={ET_Oneof, RP_NA, 2, TblTypeArgument_data, 0, NULL};
TableData TblTypeArgumentList_sub2_data[2] ={{DT_Token, {.mTokenId=44}},{DT_Subtable, &TblTypeArgument}};
RuleTable TblTypeArgumentList_sub2 ={ET_Concatenate, RP_NA, 2, TblTypeArgumentList_sub2_data, 0, NULL};
TableData TblTypeArgumentList_sub1_data[1] ={{DT_Subtable, &TblTypeArgumentList_sub2}};
RuleTable TblTypeArgumentList_sub1 ={ET_Zeroormore, RP_NA, 1, TblTypeArgumentList_sub1_data, 0, NULL};
TableData TblTypeArgumentList_data[2] ={{DT_Subtable, &TblTypeArgument},{DT_Subtable, &TblTypeArgumentList_sub1}};
RuleTable TblTypeArgumentList ={ET_Concatenate, RP_NA, 2, TblTypeArgumentList_data, 0, NULL};
TableData TblTypeArguments_data[3] ={{DT_Token, {.mTokenId=31}},{DT_Subtable, &TblTypeArgumentList},{DT_Token, {.mTokenId=32}}};
RuleTable TblTypeArguments ={ET_Concatenate, RP_NA, 3, TblTypeArguments_data, 0, NULL};
TableData TblClassType_sub2_data[1] ={{DT_Subtable, &TblAnnotation}};
RuleTable TblClassType_sub2 ={ET_Zeroormore, RP_NA, 1, TblClassType_sub2_data, 0, NULL};
TableData TblClassType_sub3_data[1] ={{DT_Subtable, &TblTypeArguments}};
RuleTable TblClassType_sub3 ={ET_Zeroorone, RP_NA, 1, TblClassType_sub3_data, 0, NULL};
TableData TblClassType_sub1_data[3] ={{DT_Subtable, &TblClassType_sub2},{DT_Subtable, &TblIdentifier},{DT_Subtable, &TblClassType_sub3}};
RuleTable TblClassType_sub1 ={ET_Concatenate, RP_NA, 3, TblClassType_sub1_data, 0, NULL};
TableData TblClassType_sub5_data[1] ={{DT_Subtable, &TblAnnotation}};
RuleTable TblClassType_sub5 ={ET_Zeroormore, RP_NA, 1, TblClassType_sub5_data, 0, NULL};
TableData TblClassType_sub6_data[1] ={{DT_Subtable, &TblTypeArguments}};
RuleTable TblClassType_sub6 ={ET_Zeroorone, RP_NA, 1, TblClassType_sub6_data, 0, NULL};
TableData TblClassType_sub4_data[5] ={{DT_Subtable, &TblClassOrInterfaceType},{DT_Token, {.mTokenId=43}},{DT_Subtable, &TblClassType_sub5},{DT_Subtable, &TblIdentifier},{DT_Subtable, &TblClassType_sub6}};
RuleTable TblClassType_sub4 ={ET_Concatenate, RP_NA, 5, TblClassType_sub4_data, 0, NULL};
TableData TblClassType_data[2] ={{DT_Subtable, &TblClassType_sub1},{DT_Subtable, &TblClassType_sub4}};
RuleTable TblClassType ={ET_Oneof, RP_NA, 2, TblClassType_data, 0, NULL};
TableData TblInterfaceType_data[1] ={{DT_Subtable, &TblClassType}};
RuleTable TblInterfaceType ={ET_Data, RP_NA, 1, TblInterfaceType_data, 0, NULL};
TableData TblTypeVariable_sub1_data[1] ={{DT_Subtable, &TblAnnotation}};
RuleTable TblTypeVariable_sub1 ={ET_Zeroormore, RP_NA, 1, TblTypeVariable_sub1_data, 0, NULL};
TableData TblTypeVariable_data[2] ={{DT_Subtable, &TblTypeVariable_sub1},{DT_Subtable, &TblIdentifier}};
RuleTable TblTypeVariable ={ET_Concatenate, RP_NA, 2, TblTypeVariable_data, 0, NULL};
TableData TblArrayType_sub1_data[2] ={{DT_Subtable, &TblPrimitiveType},{DT_Subtable, &TblDims}};
RuleTable TblArrayType_sub1 ={ET_Concatenate, RP_NA, 2, TblArrayType_sub1_data, 0, NULL};
TableData TblArrayType_sub2_data[2] ={{DT_Subtable, &TblClassOrInterfaceType},{DT_Subtable, &TblDims}};
RuleTable TblArrayType_sub2 ={ET_Concatenate, RP_NA, 2, TblArrayType_sub2_data, 0, NULL};
TableData TblArrayType_sub3_data[2] ={{DT_Subtable, &TblTypeVariable},{DT_Subtable, &TblDims}};
RuleTable TblArrayType_sub3 ={ET_Concatenate, RP_NA, 2, TblArrayType_sub3_data, 0, NULL};
TableData TblArrayType_data[3] ={{DT_Subtable, &TblArrayType_sub1},{DT_Subtable, &TblArrayType_sub2},{DT_Subtable, &TblArrayType_sub3}};
RuleTable TblArrayType ={ET_Oneof, RP_NA, 3, TblArrayType_data, 0, NULL};
TableData TblClassOrInterfaceType_data[2] ={{DT_Subtable, &TblClassType},{DT_Subtable, &TblInterfaceType}};
RuleTable TblClassOrInterfaceType ={ET_Oneof, RP_NA, 2, TblClassOrInterfaceType_data, 0, NULL};
TableData TblReferenceType_data[3] ={{DT_Subtable, &TblClassOrInterfaceType},{DT_Subtable, &TblTypeVariable},{DT_Subtable, &TblArrayType}};
RuleTable TblReferenceType ={ET_Oneof, RP_NA, 3, TblReferenceType_data, 0, NULL};
TableData TblTYPE_data[3] ={{DT_Subtable, &TblPrimitiveType},{DT_Subtable, &TblReferenceType},{DT_Subtable, &TblNullType}};
RuleTable TblTYPE ={ET_Oneof, RP_NA, 3, TblTYPE_data, 0, NULL};
TableData TblUnannClassType_sub2_data[1] ={{DT_Subtable, &TblTypeArguments}};
RuleTable TblUnannClassType_sub2 ={ET_Zeroorone, RP_NA, 1, TblUnannClassType_sub2_data, 0, NULL};
TableData TblUnannClassType_sub1_data[2] ={{DT_Subtable, &TblIdentifier},{DT_Subtable, &TblUnannClassType_sub2}};
RuleTable TblUnannClassType_sub1 ={ET_Concatenate, RP_NA, 2, TblUnannClassType_sub1_data, 0, NULL};
TableData TblUnannClassType_sub4_data[1] ={{DT_Subtable, &TblAnnotation}};
RuleTable TblUnannClassType_sub4 ={ET_Zeroormore, RP_NA, 1, TblUnannClassType_sub4_data, 0, NULL};
TableData TblUnannClassType_sub5_data[1] ={{DT_Subtable, &TblTypeArguments}};
RuleTable TblUnannClassType_sub5 ={ET_Zeroorone, RP_NA, 1, TblUnannClassType_sub5_data, 0, NULL};
TableData TblUnannClassType_sub3_data[5] ={{DT_Subtable, &TblUnannClassOrInterfaceType},{DT_Token, {.mTokenId=43}},{DT_Subtable, &TblUnannClassType_sub4},{DT_Subtable, &TblIdentifier},{DT_Subtable, &TblUnannClassType_sub5}};
RuleTable TblUnannClassType_sub3 ={ET_Concatenate, RP_NA, 5, TblUnannClassType_sub3_data, 0, NULL};
TableData TblUnannClassType_data[2] ={{DT_Subtable, &TblUnannClassType_sub1},{DT_Subtable, &TblUnannClassType_sub3}};
RuleTable TblUnannClassType ={ET_Oneof, RP_NA, 2, TblUnannClassType_data, 0, NULL};
TableData TblUnannClassOrInterfaceType_data[2] ={{DT_Subtable, &TblUnannClassType},{DT_Subtable, &TblUnannInterfaceType}};
RuleTable TblUnannClassOrInterfaceType ={ET_Oneof, RP_NA, 2, TblUnannClassOrInterfaceType_data, 0, NULL};
TableData TblUnannInterfaceType_data[1] ={{DT_Subtable, &TblUnannClassType}};
RuleTable TblUnannInterfaceType ={ET_Data, RP_NA, 1, TblUnannInterfaceType_data, 0, NULL};
TableData TblUnannTypeVariable_data[1] ={{DT_Subtable, &TblIdentifier}};
RuleTable TblUnannTypeVariable ={ET_Data, RP_NA, 1, TblUnannTypeVariable_data, 0, NULL};
TableData TblUnannArrayType_sub1_data[2] ={{DT_Subtable, &TblUnannPrimitiveType},{DT_Subtable, &TblDims}};
RuleTable TblUnannArrayType_sub1 ={ET_Concatenate, RP_NA, 2, TblUnannArrayType_sub1_data, 0, NULL};
TableData TblUnannArrayType_sub2_data[2] ={{DT_Subtable, &TblUnannClassOrInterfaceType},{DT_Subtable, &TblDims}};
RuleTable TblUnannArrayType_sub2 ={ET_Concatenate, RP_NA, 2, TblUnannArrayType_sub2_data, 0, NULL};
TableData TblUnannArrayType_sub3_data[2] ={{DT_Subtable, &TblUnannTypeVariable},{DT_Subtable, &TblDims}};
RuleTable TblUnannArrayType_sub3 ={ET_Concatenate, RP_NA, 2, TblUnannArrayType_sub3_data, 0, NULL};
TableData TblUnannArrayType_data[3] ={{DT_Subtable, &TblUnannArrayType_sub1},{DT_Subtable, &TblUnannArrayType_sub2},{DT_Subtable, &TblUnannArrayType_sub3}};
RuleTable TblUnannArrayType ={ET_Oneof, RP_NA, 3, TblUnannArrayType_data, 0, NULL};
TableData TblUnannReferenceType_data[3] ={{DT_Subtable, &TblUnannClassOrInterfaceType},{DT_Subtable, &TblUnannTypeVariable},{DT_Subtable, &TblUnannArrayType}};
RuleTable TblUnannReferenceType ={ET_Oneof, RP_NA, 3, TblUnannReferenceType_data, 0, NULL};
TableData TblUnannPrimitiveType_data[2] ={{DT_Subtable, &TblNumericType},{DT_Subtable, &TblBoolType}};
RuleTable TblUnannPrimitiveType ={ET_Oneof, RP_NA, 2, TblUnannPrimitiveType_data, 0, NULL};
TableData TblUnannType_data[2] ={{DT_Subtable, &TblUnannPrimitiveType},{DT_Subtable, &TblUnannReferenceType}};
RuleTable TblUnannType ={ET_Oneof, RP_NA, 2, TblUnannType_data, 0, NULL};
