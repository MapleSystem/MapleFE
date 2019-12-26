###################################################################################
# This file defines the Java Block/Class/Interface statement.
###################################################################################

rule ClassDeclaration : ONEOF(NormalClassDeclaration, EnumDeclaration)
rule NormalClassDeclaration : ZEROORMORE(ClassModifier) + "class" + Identifier +
                              ZEROORONE(TypeParameters) + ZEROORONE(Superclass) +
                              ZEROORONE(Superinterfaces) + ClassBody

rule ClassModifier : ONEOF(Annotation, "public", "protected", "private", "abstract",
                           "static", "final", "strictfp")

# 1. Generic class
# 2. TypeParameter will be defined in type.spec
rule TypeParameters    : '<' + TypeParameterList + '>'
rule TypeParameterList : TypeParameter + ZEROORMORE(',' + TypeParameter)
rule TypeParameter     : ZEROORMORE(TypeParameterModifier) + Identifier + ZEROORONE(TypeBound)
rule TypeParameterModifier : Annotation
rule TypeBound : ONEOF("extends" + TypeVariable,
                       "extends" + ClassOrInterfaceType + ZEROORMORE(AdditionalBound))
rule AdditionalBound : '&' + InterfaceType

# ClassType and InterfaceType are defined in type.spec
rule Superclass        : "extends" + ClassType
rule Superinterfaces   : "implements" + InterfaceTypeList
rule InterfaceTypeList : InterfaceType + ZEROORMORE(',' + InterfaceType)

# class body
rule ClassBody              : "{" + ZEROORMORE(ClassBodyDeclaration) + "}"
rule ClassBodyDeclaration   : ONEOF(ClassMemberDeclaration,
                                    InstanceInitializer,
                                    StaticInitializer,
                                    ConstructorDeclaration)
rule InstanceInitializer    : Block
rule StaticInitializer      : "static" + Block
rule ClassMemberDeclaration : ONEOF(FieldDeclaration,
                                    MethodDeclaration,
                                    ClassDeclaration,
                                    InterfaceDeclaration,
                                    ';')
rule FieldDeclaration  : ZEROORMORE(FieldModifier) + UnannType + VariableDeclaratorList + ';'

rule MethodDeclaration : ZEROORMORE(MethodModifier) + MethodHeader + MethodBody
rule MethodBody        : ONEOF(Block, ';')
rule MethodHeader      : ONEOF(Result + MethodDeclarator + ZEROORONE(Throws),
                               TypeParameters + ZEROORMORE(Annotation) + Result + MethodDeclarator +
                               ZEROORONE(Throws))
rule Result            : ONEOF(UnannType, "void")
rule MethodDeclarator  : Identifier + '(' + ZEROORONE(FormalParameterList) + ')' + ZEROORONE(Dims)
rule Throws            : "fakethrows"
rule MethodModifier    : "fakeones"

# The Java Lang Spec v8 about FormalParameterList is wrong. It doesn't have a ZEROORONE op
# enclosing the LastFormalParameter, in which case the parser will match all the tokens
# of parameters before it goes to LastFormalParameter, which in turn cause LastFormalParameter
# left un-matched. So I added ZEROORONE to fix it.
rule FormalParameterList : ONEOF(ReceiverParameter,
                                 FormalParameters + ZEROORONE(',' + LastFormalParameter),
                                 LastFormalParameter)
rule FormalParameters  : ONEOF(FormalParameter + ZEROORMORE(',' + FormalParameter),
                               ReceiverParameter + ZEROORMORE(',' + FormalParameter))
rule FormalParameter   : ZEROORMORE(VariableModifier) + UnannType + VariableDeclaratorId
rule ReceiverParameter : ZEROORMORE(Annotation) + UnannType + ZEROORONE(Identifier + '.') + "this"
rule LastFormalParameter : ONEOF(ZEROORMORE(VariableModifier) + UnannType + ZEROORMORE(Annotation) +
                                   "..." + VariableDeclaratorId,
                                 FormalParameter)


rule FieldModifier   : ONEOF(Annotation, "public", "protected", "private",
                             "static", "final", "transient", "volatile")

################################################################
#                        Constructor                           #
################################################################
rule ConstructorDeclaration : ZEROORMORE(ConstructorModifier) + ConstructorDeclarator +
                              ZEROORONE(Throws) + ConstructorBody
rule ConstructorModifier    : ONEOF(Annotation, "public", "protected", "private")
rule ConstructorDeclarator  : ZEROORONE(TypeParameters) + SimpleTypeName + '(' +
                              ZEROORONE(FormalParameterList) + ')'
rule SimpleTypeName         : Identifier
rule ConstructorBody        : '{' + ZEROORONE(ExplicitConstructorInvocation) +
                              ZEROORONE(BlockStatements) + '}'

rule ExplicitConstructorInvocation : ONEOF(
         ZEROORONE(TypeArguments) + "this" + '(' + ZEROORONE(ArgumentList) + ')' +  ';',
         ZEROORONE(TypeArguments) + "super" + '(' + ZEROORONE(ArgumentList) + ')' +  ';',
         ExpressionName + '.' + ZEROORONE(TypeArguments) + "super" + '(' + ZEROORONE(ArgumentList) + ')' +  ';',
         Primary + '.' + ZEROORONE(TypeArguments) + "super" + '(' + ZEROORONE(ArgumentList) + ')' +  ';')

################
# A fake ones
rule EnumDeclaration        : "fakeenumdeclaration"

######################################################################
#                        Block                                       #
######################################################################
rule BlockStatement  : ONEOF(LocalVariableDeclarationStatement,
                             ClassDeclaration,
                             Statement)
rule BlockStatements : BlockStatement + ZEROORMORE(BlockStatement)
rule Block           : '{' + ZEROORONE(BlockStatements) + '}'


######################################################################
#                        Interface                                   #
######################################################################
rule InterfaceDeclaration : ONEOF(NormalInterfaceDeclaration, AnnotationTypeDeclaration)
rule NormalInterfaceDeclaration : ZEROORMORE(InterfaceModifier) + "interface" + Identifier +
                                  ZEROORONE(TypeParameters) + ZEROORONE(ExtendsInterfaces) + InterfaceBody
rule InterfaceModifier : ONEOF(Annotation, "public", "protected", "private", "abstract",
                               "static", "strictfp")
rule ExtendsInterfaces : "extends" + InterfaceTypeList
rule InterfaceBody     : '{' + ZEROORMORE(InterfaceMemberDeclaration) + '}'
rule InterfaceMemberDeclaration : ONEOF(ConstantDeclaration,
                                        InterfaceMethodDeclaration,
                                        ClassDeclaration,
                                        InterfaceDeclaration,
                                        ';')
# constant decl is also called field decl. In interface, field must have a variable initializer
# However, the rules below don't tell this limitation.
rule ConstantDeclaration : ZEROORMORE(ConstantModifier) + UnannType + VariableDeclaratorList + ';'
rule ConstantModifier : ONEOF(Annotation, "public", "static", "final")

rule InterfaceMethodDeclaration : ZEROORMORE(InterfaceMethodModifier) + MethodHeader + MethodBody
rule InterfaceMethodModifier : ONEOF(Annotation, "public", "abstract", "default", "static", "strictfp")

######################################################################
#                        Annotation Type                             #
######################################################################
rule AnnotationTypeDeclaration : ZEROORMORE(InterfaceModifier) + '@' + "interface" +
                                 Identifier + AnnotationTypeBody
rule AnnotationTypeBody : '{' + ZEROORMORE(AnnotationTypeMemberDeclaration) + '}'
rule AnnotationTypeMemberDeclaration : ONEOF(AnnotationTypeElementDeclaration,
                                             ConstantDeclaration,
                                             ClassDeclaration,
                                             InterfaceDeclaration,
                                             ';')
rule AnnotationTypeElementDeclaration : ZEROORMORE(AnnotationTypeElementModifier) + UnannType +
                                          Identifier + '(' +  ')' + ZEROORONE(Dims) +
                                          ZEROORONE(DefaultValue) + ';'
rule AnnotationTypeElementModifier : ONEOF(Annotation, "public", "abstract")
rule DefaultValue : "default" + ElementValue

######################################################################
#                        Annotation                                  #
######################################################################
rule Annotation : ONEOF(NormalAnnotation,
                        MarkerAnnotation,
                        SingleElementAnnotation)

rule NormalAnnotation : '@' + TypeName + '(' + ZEROORONE(ElementValuePairList) + ')'
rule ElementValuePairList : ElementValuePair + ZEROORMORE(',' + ElementValuePair)
rule ElementValuePair : Identifier + '=' + ElementValue
rule ElementValue : ONEOF(ConditionalExpression,
                          ElementValueArrayInitializer,
                          Annotation)
rule ElementValueArrayInitializer : '{' + ZEROORONE(ElementValueList) + ZEROORONE(',') + '}'
rule ElementValueList : ElementValue + ZEROORMORE(',' + ElementValue)

rule MarkerAnnotation : '@' + TypeName
rule SingleElementAnnotation : '@' + TypeName + '(' + ElementValue + ')'
