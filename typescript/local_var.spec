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
# This file defines the Java Local Variable declaration.
#
###################################################################################

####include "type.spec"

###rule VariableModifier: (one of) Annotation final
rule VariableModifier: "final"

#rule VariableDeclarator : VariableDeclaratorId + ZEROORONE('=' + VariableInitializer)
rule VariableDeclarator : VariableDeclaratorId
rule VariableDeclaratorList : VariableDeclarator + ZEROORMORE(',' + VariableDeclarator)

###rule VariableDeclaratorId : Identifier [Dims]
rule VariableDeclaratorId : Identifier

#rule UnannPrimitiveType : ONEOF(NumericType, Boolean)
rule UnannPrimitiveType : ONEOF("int", "boolean")

###rule UnannType : ONEOF(UnannPrimitiveType, UnannReferenceType)
rule UnannType : ONEOF(UnannPrimitiveType)

###rule LocalVariableDeclaration : ZEROORMORE(VariableModifier) + UnannType + VariableDeclaratorList
rule LocalVariableDeclaration : UnannType + VariableDeclaratorList

rule LocalVariableDeclarationStatement : LocalVariableDeclaration + ';'
