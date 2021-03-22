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
# This file defines the Attribute, Modifier, or any other terms which are used
# to describe some features of certian syntax component.
#
# The keyword duplex is defined as <"keyword", AttributeId>. The AttributeId is defined
# in shared/supported_attributes.def and included in shared/supported.h. The 'keyword'
# is the keyword in a language defining that specific attribute.
###################################################################################


STRUCT Attribute : (("abstract", abstract),
                    ("const", const),
                    ("volatile", volatile),
                    ("final", final),
                    ("native", native),
                    ("private", private),
                    ("protected", protected),
                    ("public", public),
                    ("static", static),
                    ("strictfp", strictfp),
                    ("default", default),
                    ("synchronized", synchronized))

