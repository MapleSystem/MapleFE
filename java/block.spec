###################################################################################
# This file defines the Java Block statement. This will be the entry point of
# parsing a Java file
###################################################################################

## fake ones
rule ClassDeclaration : "class"

rule BlockStatement  : ONEOF(LocalVariableDeclarationStatement,
                             ClassDeclaration,
                             Statement)
rule BlockStatements : BlockStatement + ZEROORMORE(BlockStatement)
rule Block           : '{' + ZEROORONE(BlockStatements) + '}'
