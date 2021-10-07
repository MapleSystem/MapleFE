# Copyright (C) [2020] Futurewei Technologies, Inc. All rights reverved.
#
# OpenArkFE is licensed under the Mulan PSL v2.
# You can use this software according to the terms and conditions of the Mulan PSL v2.
# You may obtain a copy of Mulan PSL v2 at:
#
#  http://license.coscl.org.cn/MulanPSL2
#
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR
# FIT FOR A PARTICULAR PURPOSE.
# See the Mulan PSL v2 for more details.
#

##############################################################################
#                 This file defines reserved rule elements.                  #
##############################################################################

# CHAR refers to the 26 letters in autogen
rule CHAR  : ONEOF('a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z', 'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z')

# DIGIT refers to the 10 digits
rule DIGIT : ONEOF('0', '1', '2', '3', '4', '5', '6', '7', '8', '9')

# The ASCII character exclude ", ', \, and \n
rule ASCII : ONEOF(' ', '!', '#', '$', '%', '&', '(', ')', '*', '+', ',', '-', '.', '/', ':', ';', '<', '=', '>', '?', '@', '[', ']', '^', '_', '`', '{', '|', '}', '~', CHAR, DIGIT)

# About the handling of escape character in autogen, xx_gen.cpp/h, and stringutil.cpp
# please refer to the comments in StringToValue::StringToString() in stringutil.cpp
rule ESCAPE : ONEOF('\' + 'b', '\' + 't', '\' + 'n', '\' + 'f', '\' + 'r', '\' + '"', '\' + ''', '\' + '\')

rule HEXDIGIT : ONEOF(DIGIT, 'a', 'b', 'c', 'd', 'e', 'f', 'A', 'B', 'C', 'D', 'E', 'F')

# irregular char like  \n,  \, DEL, etc. will be handled in lexer.cpp if some language allows them in string literal.
rule IRREGULAR_CHAR : "this_is_for_fake_rule"

# We will catch any utf-8 char in lexer in a short-cut.
rule UTF8 : "this_is_for_fake_rule"

# Below are special rules handled in lexer source code. Since it'll be in lexer code, it means
# it's a shared rule of all languages. It has to be in reserved.spec.
rule TemplateLiteral : "this_is_for_fake_rule"
rule RegularExpression : "this_is_for_fake_rule"
