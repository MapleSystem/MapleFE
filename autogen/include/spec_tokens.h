/*
* Copyright (C) [2020] Futurewei Technologies, Inc. All rights reverved.
*
* OpenArkFE is licensed under the Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*
*  http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR
* FIT FOR A PARTICULAR PURPOSE.
* See the Mulan PSL v2 for more details.
*/
#ifndef SPERC_TOKENS_H
#define SPERC_TOKENS_H
typedef enum SPECTokenKind {
  SPECTK_Invalid,
  // keywords
  #define KEYWORD(S,T) SPECTK_##T,
  #include "spec_keywords.h"
  // non-keywords
  SPECTK_Intconst,
  SPECTK_Floatconst,
  SPECTK_Doubleconst,
  SPECTK_Name,
  SPECTK_Fname,
  SPECTK_Newline,
  SPECTK_Lparen,     // (
  SPECTK_Rparen,     // )
  SPECTK_Lbrace,     // {
  SPECTK_Rbrace,     // }
  SPECTK_Lbrack,     // [
  SPECTK_Rbrack,     // ]
  SPECTK_Langle,     // <
  SPECTK_Rangle,     // >
  SPECTK_Eqsign,     // =
  SPECTK_Coma,       // ,
  SPECTK_Dot,        // .
  SPECTK_Dotdotdot,  // ...
  SPECTK_Colon,      // :
  SPECTK_Semicolon,  // ;
  SPECTK_Asterisk,   // *
  SPECTK_Percent,    // %
  SPECTK_Char,       // a char enclosed between '
  SPECTK_String,     // a literal string enclosed between "
  SPECTK_Concat,     // +
  SPECTK_Actionfunc, // ==>
  SPECTK_Eof
} SPECTokenKind;
#endif
