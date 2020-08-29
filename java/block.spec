# Copyright (C) [2020] Futurewei Technologies, Inc. All rights reverved.
#
# OpenArkFE is licensed under the Mulan PSL v1.
# You can use this software according to the terms and conditions of the Mulan PSL v1.
# You may obtain a copy of Mulan PSL v1 at:
#
#  http://license.coscl.org.cn/MulanPSL
#
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR
# FIT FOR A PARTICULAR PURPOSE.
# See the Mulan PSL v1 for more details.
#
###################################################################################
# This file defines the Java Block/Class/Interface statement.
###################################################################################

rule ClassDeclaration : ONEOF(NormalClassDeclaration, EnumDeclaration)
rule NormalClassDeclaration : ZEROORMORE(ClassModifier) + "class" + Identifier +
                              ZEROORONE(TypeParameters) + ZEROORONE(Superclass) +
                              ZEROORONE(Superinterfaces) + ClassBody
attr.action : BuildClass(%3)
attr.action : AddAttribute(%1)
attr.action : AddClassBody(%7)
attr.action : AddSuperClass(%5)
attr.action : AddSuperInterface(%6)

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
  attr.action: BuildBlock(%2)

rule ClassBodyDeclaration   : ONEOF(ClassMemberDeclaration,
                                    InstanceInitializer,
                                    StaticInitializer,
                                    ConstructorDeclaration)
rule InstanceInitializer    : Block
  attr.action: BuildInstInit(%1)
rule StaticInitializer      : "static" + Block
  attr.action: BuildInstInit(%2)
  attr.action: AddAttributeTo(%2, %1)

rule ClassMemberDeclaration : ONEOF(FieldDeclaration,
                                    MethodDeclaration,
                                    ClassDeclaration,
                                    InterfaceDeclaration,
                                    ';')
rule FieldDeclaration  : ZEROORMORE(FieldModifier) + UnannType + VariableDeclaratorList + ';'
  attr.action: BuildDecl(%2, %3)
  attr.action: AddAttribute(%1)

rule MethodDeclaration : ZEROORMORE(MethodModifier) + MethodHeader + MethodBody
  attr.action: AddAttributeTo(%2, %1)
  attr.action: AddFunctionBodyTo(%2, %3)

rule MethodBody        : ONEOF(Block, ';')
rule MethodHeader      : ONEOF(Result + MethodDeclarator + ZEROORONE(Throws),
                               TypeParameters + ZEROORMORE(Annotation) + Result + MethodDeclarator +
                               ZEROORONE(Throws))
  attr.action.%1: AddTypeTo(%2, %1)
  attr.action.%1: AddThrowsTo(%2, %3)
  attr.action.%2: AddThrowsTo(%4, %5)

rule Result            : ONEOF(UnannType, "void")
rule MethodDeclarator  : Identifier + '(' + ZEROORONE(FormalParameterList) + ')' + ZEROORONE(Dims)
  attr.action: BuildFunction(%1)
  attr.action: AddParams(%3)
  attr.action: AddDims(%5)

rule Throws            : "throws" + ExceptionTypeList
  attr.action: BuildThrows(%2)

rule ExceptionTypeList : ExceptionType + ZEROORMORE(',' + ExceptionType)
rule ExceptionType     : ONEOF(ClassType, TypeVariable)

rule MethodModifier    : ONEOF(Annotation, "public", "protected", "private", "abstract", "static",
                               "final", "synchronized", "native", "strictfp")

# The Java Lang Spec v8 about FormalParameterList is wrong. It doesn't have a ZEROORONE op
# enclosing the LastFormalParameter, in which case the parser will match all the tokens
# of parameters before it goes to LastFormalParameter, which in turn cause LastFormalParameter
# left un-matched. So I added ZEROORONE to fix it.
rule FormalParameterList : ONEOF(ReceiverParameter,
                                 FormalParameters + ZEROORONE(',' + LastFormalParameter),
                                 LastFormalParameter)
rule FormalParameters  : ONEOF(FormalParameter + ZEROORMORE(',' + FormalParameter),
                               ReceiverParameter + ZEROORMORE(',' + FormalParameter))
  attr.action.%1: BuildVarList(%1, %2)

rule FormalParameter   : ZEROORMORE(VariableModifier) + UnannType + VariableDeclaratorId
  attr.action: BuildDecl(%2, %3)
  attr.action: AddAttribute(%1)
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
  attr.action : AddFunctionBodyTo(%2, %4)

rule ConstructorModifier    : ONEOF(Annotation, "public", "protected", "private")
rule ConstructorDeclarator  : ZEROORONE(TypeParameters) + SimpleTypeName + '(' +
                              ZEROORONE(FormalParameterList) + ')'
  attr.action : BuildConstructor(%2)
rule SimpleTypeName         : Identifier
rule ConstructorBody        : '{' + ZEROORONE(ExplicitConstructorInvocation) +
                              ZEROORONE(BlockStatements) + '}'
  attr.action : BuildBlock(%3)

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
  attr.action: BuildBlock(%2)


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
  attr.action : BuildAnnotationType(%4)
  attr.action : AddAttribute(%1)
  attr.action : AddAnnotationTypeBody(%5)

rule AnnotationTypeBody : '{' + ZEROORMORE(AnnotationTypeMemberDeclaration) + '}'
  attr.action : BuildBlock(%2)
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

######################################################################
#                        Package                                     #
######################################################################
rule PackageDeclaration: ZEROORMORE(PackageModifier) + "package" + Identifier + ZEROORMORE('.' + Identifier) + ';'
rule PackageModifier: Annotation
rule ImportDeclaration: ONEOF(SingleTypeImportDeclaration,
                              TypeImportOnDemandDeclaration,
                              SingleStaticImportDeclaration,
                              StaticImportOnDemandDeclaration)
rule SingleTypeImportDeclaration: "import" + TypeName + ';'
rule TypeImportOnDemandDeclaration: "import" + PackageOrTypeName + '.' + '*' + ';'
rule SingleStaticImportDeclaration: "import" + "static" + TypeName + '.' + Identifier + ';'
rule StaticImportOnDemandDeclaration: "import" + "static" + TypeName + '.' + '*' + ';'
rule TypeDeclaration: ONEOF(ClassDeclaration,
                            InterfaceDeclaration,
                            ';')
