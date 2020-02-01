/*
* Copyright (C) [2020] Futurewei Technologies, Inc. All rights reverved.
*
* OpenArkFE is licensed under the Mulan PSL v1.
* You can use this software according to the terms and conditions of the Mulan PSL v1.
* You may obtain a copy of Mulan PSL v1 at:
*
*  http://license.coscl.org.cn/MulanPSL
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR
* FIT FOR A PARTICULAR PURPOSE.
* See the Mulan PSL v1 for more details.
*/
#include <cstring>

#include "literal_gen.h"
#include "massert.h"

//// Fill Map
//void LiteralGen::ProcessStructData() {
//  for (auto it: mStructs) {
//    for (auto eit: it->mStructElems) {
//      std::string lang(eit->mDataVec[0]->GetString());
//      std::string autogen(eit->mDataVec[1]->GetString());
//      AddEntry(lang, autogen);
//    }
//  }
//}

// The major work of LiteralGen is to generate the function GetLiteral(..)
// which is as follows.
//
//   For each type of literals defined in [LANG:java]/literal.spec
//   which are also defined in autogen/supported_literals.spec, LiteralGen()
//   tells if the next token is such a literal.
//
//   LitData GetLiteral(Lexer *lex, int &len, StringPool *pool)
//   {
//     LitData literal;
//     lex->PosSave();
//
//     literal = GetLiteralInt(lex, len, pool);
//     if (literal.mType != LT_NA)
//       return literal;
//
//     lex->PosReset();
//     literal = GetLiteralChar(lex, len, pool);
//     if (literal.mType != LT_NA)
//       return literal;
//     
//     ...
//     
//     //finally return fail
//     lex->PosReset();
//     literal.mType = LT_NA;
//     return literal;
//   } 
//
// 1) GetLIteral() is the driver of reading literal tokens.
// 2) Functions like GetLiteralInt(..) and etc are corresponding to those
//    known literal token types in share/include/token.h.
// 3) The second huge part is to generate GetDigit(...) and etc from
//    the LANGUAGE/literal.spec. This is handled in rule_gen.h/cpp.
//

//void LiteralGen::GenGetLiteral() {
//  // need the 'extern' in mDecl for header file.
//  mFuncGetLiteral.mDecl.AddString("extern ");
//
//  // Add the following to both mDecl and mHeader
//  mFuncGetLiteral.AddReturnType("LitData");
//  mFuncGetLiteral.AddFunctionName("GetLiteral");
//  mFuncGetLiteral.AddParameter("Lexer*", "lex");
//  mFuncGetLiteral.AddParameter("int&", "len"); // end of param list
//  mFuncGetLiteral.AddParameter("StringPool*", "pool", true); // end of param list
//
//  // Now work on the mBody
//  mFuncGetLiteral.mBody.NewLine();
//  mFuncGetLiteral.mBody.AddString("const char *pos = lex->mCurChar;");
//
//#define LITERAL(NAME) literal_types.push_back(#NAME);
//  std::vector<const char *> literal_types;
//#include "supported_literals.def"
//
//  // Generate:
//  //     LitData literal;
//  //     lex->PosSave();
//  //
//  mFuncGetLiteral.mBody.NewOneBuffer();
//  mFuncGetLiteral.mBody.AddStringWholeLine("LitData literal;");
//  mFuncGetLiteral.mBody.AddStringWholeLine("lex->PosSave();");
//
//  // Generate the following for each type of literal
//  //     lex->PosReset();
//  //     literal = GetLiteralChar(lex, len, pool);
//  //     if (literal.mType != LT_NA)
//  //       return literal;
//  for (int idx = 0; idx < literal_types.size(); idx++) {
//    // The buffers need to be new/free-d
//    if (idx > 0){
//      const char *s1 = "lex->PosReset();";
//      mFuncGetLiteral.mBody.AddStringWholeLine(s1);
//    }
//    std::string s2("literal = GetLiteral");
//    s2 += literal_types[idx];
//    s2 += "(lex, len, pool);";
//    const char *s3 = "if (literal.mType != LT_NA)";
//    const char *s4 = "  return literal;";
//    mFuncGetLiteral.mBody.AddStringWholeLine(s2);
//    mFuncGetLiteral.mBody.AddStringWholeLine(s3);
//    mFuncGetLiteral.mBody.AddStringWholeLine(s4);
//  }
//}

void LiteralGen::Generate() {
  GenRuleTables();
  GenHeaderFile();
  GenCppFile();
}

void LiteralGen::GenHeaderFile() {
  mHeaderFile.WriteOneLine("#ifndef __LITERAL_GEN_H__", 25);
  mHeaderFile.WriteOneLine("#define __LITERAL_GEN_H__", 25);
  mHeaderFile.WriteOneLine("#include \"ruletable.h\"", 22);
  mHeaderFile.WriteFormattedBuffer(&mRuleTableHeader);
  mHeaderFile.WriteOneLine("#endif", 6);
}

void LiteralGen::GenCppFile() {
  mCppFile.WriteOneLine("#include \"common_header_autogen.h\"", 34);
  mCppFile.WriteFormattedBuffer(&mRuleTableCpp);
}
