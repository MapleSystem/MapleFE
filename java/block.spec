###################################################################################
# This file defines the Java Block statement. This will be the entry point of
# parsing a Java file
###################################################################################

rule ClassDeclaration : ONEOF(NormalClassDeclaration, EnumDeclaration)
rule NormalClassDeclaration : ZEROORMORE(ClassModifier) + "class" + Identifier +
                              ZEROORONE(TypeParameters) + ZEROORONE(Superclass) +
                              ZEROORONE(Superinterfaces) + ClassBody

rule ClassModifier : ONEOF(Annotation, "public", "protected", "private", "abstract",
                           "static", "final", "strictfp")

# 1. Generic class
# 2. TypeParameter will be defined in type.spec
#    I just put a fake one here. Will come back to finish it
rule TypeParameters    : '<' + TypeParameterList + '>'
rule TypeParameterList : TypeParameter + ZEROORMORE(',' + TypeParameter)
rule TypeParameter     : "typeparameter"

# ClassType and InterfaceType are defined in type.spec
rule Superclass        : "extends" + ClassType
rule Superinterfaces   : "implements" + InterfaceTypeList
rule InterfaceTypeList : InterfaceType + ZEROORMORE(',' + InterfaceType)

# class body
rule ClassBody              : "{" + ZEROORMORE(ClassBodyDeclaration) + "}"
rule ClassBodyDeclaration   : ONEOF(ClassMemberDeclaration)
#                                    InstanceInitializer,
#                                    StaticInitializer,
#                                    ConstructorDeclaration)
rule ClassMemberDeclaration : ONEOF(FieldDeclaration,
                                    MethodDeclaration)
#                                    ClassDeclaration,
#                                    InterfaceDeclaration,
#                                    ';')
rule FieldDeclaration  : ZEROORMORE(FieldModifier) + UnannType + VariableDeclaratorList + ';'

rule MethodDeclaration : ZEROORMORE(MethodModifier) + MethodHeader + MethodBody
rule MethodBody        : ONEOF(Block, ';')
rule MethodHeader      : ONEOF(Result + MethodDeclarator + ZEROORONE(Throws))
#                               TypeParameters + ZEROORMORE(Annotation) + Result + MethodDeclarator +
#                               ZEROORONE(Throws))
rule Result            : ONEOF(UnannType, "void")
rule MethodDeclarator  : Identifier + '(' + ZEROORONE(FormalParameterList) + ')' + ZEROORONE(Dims)
rule Throws            : "fakethrows"
rule MethodModifier    : "fakeones"
rule FormalParameterList : ONEOF(ReceiverParameter,
                                 FormalParameters + ',' + LastFormalParameter,
                                 LastFormalParameter)
rule FormalParameters  : ONEOF(FormalParameter + ZEROORMORE(',' + FormalParameter),
                               ReceiverParameter + ZEROORMORE(',' + FormalParameter))
rule FormalParameter   : ZEROORMORE(VariableModifier) + UnannType + VariableDeclaratorId
rule ReceiverParameter : ZEROORMORE(Annotation) + UnannType + ZEROORONE(Identifier + '.') + "this"
rule LastFormalParameter : ONEOF(ZEROORMORE(VariableModifier) + UnannType + ZEROORMORE(Annotation) +
                                   "..." + VariableDeclaratorId,
                                 FormalParameter)


# A fake ones
rule FieldModifier          : "fakefieldmodifier"
rule EnumDeclaration        : "fakeenumdeclaration"

rule BlockStatement  : ONEOF(LocalVariableDeclarationStatement,
                             ClassDeclaration,
                             Statement)
rule BlockStatements : BlockStatement + ZEROORMORE(BlockStatement)
rule Block           : '{' + ZEROORONE(BlockStatements) + '}'
