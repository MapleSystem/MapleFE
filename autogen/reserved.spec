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
# TODO: if the range expression is implemented, then I can use a-b, A-B
#

# CHAR refers to the 26 letters in autogen
rule CHAR  : ONEOF('a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z', 'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z')

# DIGIT refers to the 10 digits
rule DIGIT : ONEOF('0', '1', '2', '3', '4', '5', '6', '7', '8', '9')

# The ASCII character exclude " and \
rule ASCII : ONEOF(' ', '!', '#', '$', '%', '&', ''', '(', ')', '*', '+', ',', '-', '.', '/', ':', ';', '<', '=', '>', '?', '@', '[', ']', '^', '_', '`', '{', '|', '}', '~', CHAR, DIGIT)
