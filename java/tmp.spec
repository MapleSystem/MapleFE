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
# An identifier is an unlimited-length sequence of Java letters and Java digits, the
# first of which must be a Java letter
#
# https://docs.oracle.com/javase/specs/jls/se12/html/jls-3.html#jls-Identifier
#
# TODO: So far we dont support unicode which are not major goal right now.

rule JavaLetter: ONEOF(CHAR, '_' , '$')
rule JavaLetterOrDigit: ONEOF(JavaLetter, DIGIT)
rule IdentifierChars: JavaLetter + ZEROORMORE(JavaLetterOrDigit)
rule Identifier: IdentifierChars #but not a Keyword or BooleanLiteral or NullLiteral
