###################################################################################
#
# This file defines the Java Block statement. This will be the entry point of
# parsing a Java file
#
###################################################################################

rule Block : '{' + ZEROORONE(BlockStatements) + '}'
rule BlockStatements : BlockStatement + ZEROORMORE(BlockStatement)
rule BlockStatement  : ONEOF(LocalVariableDeclarationStatement,
                             ClassDeclaration,
                             Statement)
## fake ones
rule ClassDeclaration : "class"
rule Statement : "statement"
