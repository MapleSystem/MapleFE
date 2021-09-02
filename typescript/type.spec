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
###################################################################################
# This file defines the Java types.
#
#  1. Child rules have to be defined before parent rules.
#  2. For type.spec, Autogen allows the programmer to have special keyword section to
#     define the individual <keyword, meaning>. Please keep in mind this
#     keyword section has to appear before the rule section.
#
#     Keyword section is defined through STRUCT().
#
#  3. Autogen will first read in the keyword section by TypeGen functions, then
#     read the rules section by inheriting BaseGen::Parse, etc.
#  4. The keyword duplex is defined as <TYPE, "keyword">.
#     [NOTE] TYPE should be one of those recognized by the "parser". Please refer
#            to ../shared/include/type.h. The data representation of same type
#            doesnt have to be the same in each language, we just need have the
#            same name of TYPE. The physical representation of each type of
#            different language will be handled by HandleType() by each language.
#            e.g. Char in java and in C are different. but Autogen doesnt care.
#            java2mpl and c2mpl will provide their own HandleType() to map this
#            to the correct types in Maple IR.
#
#     So there are 4 type systems involved in frontend.
#     (1) Types in the .spec file, of each language
#     (2) Types in Autogen. Super set of (1) in all languages.
#         types in Autogen have an exact mapping to those in Parser.
#         Autogen will generate type related files used in parser, mapping
#         types to it.
#     (3) Types in Parser. From now on, physical representation is defined.
#         Each language has its own HandleTypes() to map type in Parser to
#         those in MapleIR, considering physical representation.
#     (4) Types in Maple IR. This is the only place where types have physical
#         representation.
#
#  5. The supported STRUCT in type.spec is:
#         Keyword
#     Right now, only one STRUCT is supported.
#
#     It is highly possible that a .spec file needs more than one STRUCT.
###################################################################################

# The types recoganized by Autogen are in shared/supported_types.def
# where Boolean, Byte, .. are defined. That said, "Boolean" and the likes are
# used in type.spec as a keyword.
#
# This STRUCT tells the primitive types

STRUCT Keyword : (("boolean",   Boolean),
                  ("string",    String),
                  ("number",    Number),
                  ("symbol",    Symbol),
                  ("any",       Any),
                  ("void",      Void),
                  ("unknown",   Unknown),
                  ("never",     Never),
                  ("undefined", Undefined))

rule BooleanType : "boolean"
rule NumberType  : "number"
rule SymbolType  : "symbol"
rule AnyType     : "any"
rule StringType  : "string"
rule VoidType    : "void"
rule UnknownType : "unknown"
rule NeverType   : "never"
rule UndefinedType : "undefined"

rule TYPE : ONEOF(BooleanType, NumberType, SymbolType, AnyType, StringType, VoidType, UnknownType, NeverType, UndefinedType)
