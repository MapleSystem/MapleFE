# Copyright (C) [2020] Futurewei Technologies, Inc. All rights reverved.
#
# OpenArkFE is licensed under the Mulan PSL v1.
# You can use this software according to the terms and conditions of the Mulan PSL v1.
# You may obtain a copy of Mulan PSL v1 at:
#
#  http://license.coscl.org.cn/MulanPSL
#
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR
# FIT FOR A PARTICULAR PURPOSE.
# See the Mulan PSL v1 for more details.
#

# This file defines reserved rule elements.
#
# If a rule is without content, then it is like reserved keyword which
# autogen understand.
#

# CHAR refers to the 26 letters in autogen
rule CHAR  : ONEOF('a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z', 'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z')

# DIGIT refers to the 10 digits
rule DIGIT : ONEOF('0', '1', '2', '3', '4', '5', '6', '7', '8', '9')

# The ASCII character exclude ", ', and \
#
# [NOTE] Becareful of ' over here. It's duplicated in ESCAPE as '\' + '''. There is a reason.
#        When the lexer read a string "doesn't", which is wrong since Java request ' be escaped but
#        many code does NOT escape, the string in memory is "doesn't" too. The system Reading function
#        which is in C doesn't escape '. So I duplicate here to catch this case.
#
#        Please see test case java2mpl/literal-string-2.java for example.
#
rule ASCII : ONEOF(' ', '!', '#', '$', '%', ''', '&', '(', ')', '*', '+', ',', '-', '.', '/', ':', ';', '<', '=', '>', '?', '@', '[', ']', '^', '_', '`', '{', '|', '}', '~', CHAR, DIGIT)

# About the handling of escape character in autogen, xx_gen.cpp/h, and stringutil.cpp
# please refer to the comments in StringToValue::StringToString() in stringutil.cpp
rule ESCAPE : ONEOF('\' + 'b', '\' + 't', '\' + 'n', '\' + 'f', '\' + 'r', '\' + '"', '\' + ''', '\' + '\')
