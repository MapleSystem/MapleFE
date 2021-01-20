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
# An identifier is an unlimited-length sequence of Java letters and Java digits, the
# first of which must be a Java letter
#
# TODO: So far we dont support unicode which are not major goal right now.

rule JavaChar : ONEOF(CHAR, '_' , '$')
rule CharOrDigit : ONEOF(JavaChar, DIGIT)
rule Identifier : JavaChar + ZEROORMORE(CharOrDigit)
  attr.property.%2 : ZomFast
