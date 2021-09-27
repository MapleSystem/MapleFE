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
# NOTE - 1
#
# This file defines the separator. The separators are defined using a STRUCT.
# Each separator in this STRUCT is a set of 2 elements.
# STRUCT Separator : ( ("(", LeftParenthesis),
#                      (")", RightParenthesis),
# The first element is the literal name of separator, it needs to be a string
# The second is the ID of the separator. Please check shared/include/supported_separators.def
# to see the supported separator ID.

# NOTE - 2
# Some languages could have one synatx belonging to both separator and operators.
# eg., ':' in Java 8, it's both a separator colon and operator select.
# We need avoid such duplication in .spec files.

STRUCT Separator : ((" ", Whitespace),
                    ("(", Lparen),
                    (")", Rparen),
                    ("{", Lbrace),
                    ("}", Rbrace),
                    ("[", Lbrack),
                    ("]", Rbrack),
                    (";", Semicolon),
                    (",", Comma),
                    (".", Dot),
                    ("...", Dotdotdot),
                    (":", Colon),
                    ("?", Select),
                    ("?.",Optional),
                    ("@", At),
                    ("#", Pound),
                    ("\t", Tab))
