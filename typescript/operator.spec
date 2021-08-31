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
##########################################################################
# The syntax of operator.spec contains two parts.
#  1. The Keyword STRUCT defining the <keyword,opr> of all operations.
#        keyword : the text defined in the language
#        opr     : the standard Autogen defined operator.
#                  See autogen/include/supported_operators.def
#  2. The rule part, defining the language restrictions of each operator.
##########################################################################

# NOTE
# Some languages could have one synatx belonging to both separator and operators.
# eg., ':' in Java 8, it's both a separator colon and operator select.
# We need avoid such duplication in .spec files.

STRUCT Operator : ONEOF(
                    # Arithmetic
                    ("+",    Add),
                    ("-",    Sub),
                    ("*",    Mul),
                    ("/",    Div),
                    ("%",    Mod),
                    ("++",   Inc),
                    ("--",   Dec),
                    ("**",   Exp),
                    ("??",   NullCoalesce),
                    ("??=",  NullAssign),
                    # Relation
                    ("==",   EQ),
                    ("!=",   NE),
                    (">",    GT),
                    ("<",    LT),
                    (">=",   GE),
                    ("<=",   LE),
                    ("===",  StEq),
                    ("!==",  StNe),
                    # Bitwise
                    ("&",    Band),
                    ("|",    Bor),
                    ("^",    Bxor),
                    ("~",    Bcomp),
                    # Shift
                    ("<<",   Shl),
                    (">>",   Shr),
                    (">>>",  Zext),
                    # Logical
                    ("&&",   Land),
                    ("||",   Lor),
                    ("!",    Not),
                    # Assign
                    ("=",    Assign),
                    ("+=",   AddAssign),
                    ("-=",   SubAssign),
                    ("*=",   MulAssign),
                    ("/=",   DivAssign),
                    ("%=",   ModAssign),
                    ("<<=",  ShlAssign),
                    (">>=",  ShrAssign),
                    ("&=",   BandAssign),
                    ("|=",   BorAssign),
                    ("^=",   BxorAssign),
                    (">>>=", ZextAssign),
                    # arrow function
                    ("=>",   ArrowFunction))
