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

#Dims:
#{Annotation} [ ] {{Annotation} [ ]}
#VariableInitializer:
#Expression
#ArrayInitializer

#rule UnannPrimitiveType : ONEOF(NumericType, Boolean)
rule UnannPrimitiveType : ONEOF("int", "boolean")

###rule UnannType : ONEOF(UnannPrimitiveType, UnannReferenceType)
rule UnannType : ONEOF(UnannPrimitiveType)

###rule LocalVariableDeclaration : ZEROORMORE(VariableModifier) + UnannType + VariableDeclaratorList
rule LocalVariableDeclaration : UnannType + VariableDeclaratorList

rule LocalVariableDeclarationStatement : LocalVariableDeclaration + ';'
