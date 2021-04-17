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

rule LocalVariableDeclarationStatement : LocalVariableDeclaration + ';'

rule LocalVariableDeclaration : ZEROORMORE(VariableModifier) + UnannType + VariableDeclaratorList
  attr.action: BuildDecl(%2, %3)
  attr.action: AddModifier(%1)
  attr.property.%1 : ZomFast

rule VariableModifier : ONEOF(
  Annotation,
  "final")

rule VariableDeclaratorList : VariableDeclarator + ZEROORMORE(',' + VariableDeclarator)
  attr.action: BuildVarList(%1, %2)

rule VariableDeclarator : VariableDeclaratorId + ZEROORONE('=' + VariableInitializer)
  attr.action: AddInitTo(%1, %2)

rule VariableDeclaratorId : Identifier + ZEROORONE(Dims)
  attr.action: AddDimsTo(%1, %2)

rule Dims : Dim + ZEROORMORE(Dim)
 attr.action: BuildDims(%1, %2)

rule Dim  : ZEROORMORE(Annotation) + '[' + ']'
 attr.action: BuildDim(%1)

rule VariableInitializer : ONEOF(
  Expression,
  ArrayInitializer)

rule VariableInitializerList: VariableInitializer + ZEROORMORE(',' + VariableInitializer)

# Statements are actually dis-ambiguous, and it's "top" level.
# There should NOT be any two statements matching at the same start tokens.
rule Statement : ONEOF(LocalVariableDeclarationStatement,
                       StatementWithoutTrailingSubstatement,
                       LabeledStatement,
                       IfThenElseStatement,
                       IfThenStatement,
                       WhileStatement,
                       ForStatement)
  attr.property: Single


# Statements are actually dis-ambiguous, and it's "top" level.
# There should NOT be any two statements matching at the same start tokens.
rule StatementNoShortIf : ONEOF( StatementWithoutTrailingSubstatement,
                                 LabeledStatementNoShortIf,
                                 IfThenElseStatementNoShortIf,
                                 WhileStatementNoShortIf,
                                 ForStatementNoShortIf)
  attr.property: Single

# Statements are actually dis-ambiguous, and it's "top" level.
# There should NOT be any two statements matching at the same start tokens.
rule StatementWithoutTrailingSubstatement : ONEOF(
         Block,
         EmptyStatement,
         ExpressionStatement,
         AssertStatement,
         SwitchStatement,
         DoStatement,
         BreakStatement,
         ContinueStatement,
         ReturnStatement,
         SynchronizedStatement,
         ThrowStatement,
         TryStatement)
  attr.property: Single

rule IfThenStatement : "if" + '(' + Expression + ')' + Statement
  attr.action: BuildCondBranch(%3)
  attr.action: AddCondBranchTrueStatement(%5)

rule IfThenElseStatement : "if" + '(' + Expression + ')' + StatementNoShortIf + "else" + Statement
  attr.action: BuildCondBranch(%3)
  attr.action: AddCondBranchTrueStatement(%5)
  attr.action: AddCondBranchFalseStatement(%7)

rule IfThenElseStatementNoShortIf : "if" + '(' + Expression + ')' + StatementNoShortIf + "else" + StatementNoShortIf
  attr.action: BuildCondBranch(%3)
  attr.action: AddCondBranchTrueStatement(%5)
  attr.action: AddCondBranchFalseStatement(%7)

rule EmptyStatement : ';'

rule LabeledStatement : Identifier + ':' + Statement
  attr.action: AddLabel(%3, %1)

rule LabeledStatementNoShortIf : Identifier + ':' + StatementNoShortIf
  attr.action: AddLabel(%3, %1)

rule ExpressionStatement : StatementExpression + ';'

rule StatementExpression : ONEOF(
  Assignment,
  PreIncrementExpression,
  PreDecrementExpression,
  PostIncrementExpression,
  PostDecrementExpression,
  MethodInvocation,
  ClassInstanceCreationExpression)

rule AssertStatement : ONEOF(
  "assert" + Expression + ';',
  "assert" + Expression + ':' + Expression + ';')
  attr.action.%1 : BuildAssert(%2)
  attr.action.%2 : BuildAssert(%2, %4)

rule SwitchStatement : "switch" + '(' + Expression + ')' + SwitchBlock
  attr.action : BuildSwitch(%3, %5)

rule SwitchBlock : '{' + ZEROORMORE(ZEROORMORE(SwitchBlockStatementGroup) + ZEROORMORE(SwitchLabel)) + '}'
  attr.action : BuildAllCases(%2)

rule SwitchBlockStatementGroup : SwitchLabels + BlockStatements
  attr.action : BuildOneCase(%1, %2)

rule SwitchLabels : SwitchLabel + ZEROORMORE(SwitchLabel)

rule SwitchLabel : ONEOF("case" + ConstantExpression + ':',
                         "case" + EnumConstantName + ':',
                         "default" + ':')
  attr.action.%1,%2 : BuildSwitchLabel(%2)
  attr.action.%3    : BuildDefaultSwitchLabel()


rule EnumConstantName : Identifier

rule WhileStatement : "while" + '(' + Expression + ')' + Statement
  attr.action : BuildWhileLoop(%3, %5)

rule WhileStatementNoShortIf : "while" + '(' + Expression + ')' + StatementNoShortIf
  attr.action : BuildWhileLoop(%3, %5)

rule DoStatement : "do" + Statement + "while" + '(' + Expression + ')' + ';'
  attr.action : BuildDoLoop(%5, %2)

rule ForStatement : ONEOF(
  BasicForStatement,
  EnhancedForStatement)

rule ForStatementNoShortIf : ONEOF(
  BasicForStatementNoShortIf,
  EnhancedForStatementNoShortIf)

rule BasicForStatement : "for" + '(' + ZEROORONE(ForInit) + ';' + ZEROORONE(Expression) + ';' + ZEROORONE(ForUpdate) + ')' + Statement
  attr.action: BuildForLoop(%3, %5, %7, %9)

rule BasicForStatementNoShortIf : "for" + '(' + ZEROORONE(ForInit) + ';' + ZEROORONE(Expression) + ';' + ZEROORONE(ForUpdate) + ')' + StatementNoShortIf
  attr.action: BuildForLoop(%3, %5, %7, %9)

rule ForInit : ONEOF(
  StatementExpressionList,
  LocalVariableDeclaration)

rule ForUpdate : StatementExpressionList

rule StatementExpressionList : StatementExpression + ZEROORMORE(',' + StatementExpression)

rule EnhancedForStatement : "for" + '(' + ZEROORMORE(VariableModifier) + UnannType + VariableDeclaratorId + ':' + Expression + ')' + Statement

rule EnhancedForStatementNoShortIf : "for" + '(' + ZEROORMORE(VariableModifier) + UnannType + VariableDeclaratorId + ':' + Expression + ')' + StatementNoShortIf

rule BreakStatement : "break" + ZEROORONE(Identifier) + ';'
  attr.action: BuildBreak(%2)

rule ContinueStatement : "continue" + ZEROORONE(Identifier) + ';'

rule ReturnStatement : "return" + ZEROORONE(Expression) + ';'
  attr.action : BuildReturn(%2)

rule ThrowStatement : "throw" + Expression + ';'

rule SynchronizedStatement : "synchronized" + '(' + Expression + ')' + Block
  attr.action : AddSyncToBlock(%3, %5)

rule TryStatement : ONEOF(
  "try" + Block + Catches,
  "try" + Block + ZEROORONE(Catches) + Finally,
  TryWithResourcesStatement)

rule Catches : CatchClause + ZEROORMORE(CatchClause)

rule CatchClause : "catch" + '(' + CatchFormalParameter + ')' + Block

rule CatchFormalParameter : ZEROORMORE(VariableModifier) + CatchType + VariableDeclaratorId

rule CatchType : UnannClassType + ZEROORMORE('|' + ClassType)

rule Finally : "finally" + Block

rule TryWithResourcesStatement : "try" + ResourceSpecification + Block + ZEROORONE(Catches) + ZEROORONE(Finally)

rule ResourceSpecification : '(' + ResourceList + ZEROORONE(';') + ')'

rule ResourceList : Resource + ZEROORMORE(';' + Resource)

rule Resource : ONEOF(
  ZEROORMORE(VariableModifier) + UnannType + Identifier + '=' + Expression,
  VariableAccess)

rule VariableAccess : ONEOF(
  ExpressionName,
  FieldAccess)

###################################################################################
# This file defines the Java Block/Class/Interface statement.
###################################################################################

rule ClassDeclaration : ONEOF(NormalClassDeclaration, EnumDeclaration)
  attr.property : Single, Top
rule NormalClassDeclaration : ZEROORMORE(ClassModifier) + "class" + Identifier +
                              ZEROORONE(TypeParameters) + ZEROORONE(Superclass) +
                              ZEROORONE(Superinterfaces) + ClassBody
  attr.action : BuildClass(%3)
  attr.action : AddModifier(%1)
  attr.action : AddClassBody(%7)
  attr.action : AddSuperClass(%5)
  attr.action : AddSuperInterface(%6)
  attr.property.%1,%4,%5,%6 : ZomFast

rule ClassAttr : ONEOF("public", "protected", "private", "abstract", "static", "final", "strictfp")
  attr.property : Single
rule ClassModifier : ONEOF(Annotation, ClassAttr)
  attr.property : Single

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
  attr.action:       BuildBlock(%2)
  attr.property.%2 : ZomFast

rule ClassBodyDeclaration   : ONEOF(ClassMemberDeclaration,
                                    InstanceInitializer,
                                    StaticInitializer,
                                    ConstructorDeclaration)
  attr.property : Single

rule InstanceInitializer    : Block
  attr.action: BuildInstInit(%1)
rule StaticInitializer      : "static" + Block
  attr.action: BuildInstInit(%2)
  attr.action: AddModifierTo(%2, %1)

rule ClassMemberDeclaration : ONEOF(FieldDeclaration,
                                    MethodDeclaration,
                                    ClassDeclaration,
                                    InterfaceDeclaration,
                                    ';')
  attr.property : Single

rule FieldDeclaration  : ZEROORMORE(FieldModifier) + UnannType + VariableDeclaratorList + ';'
  attr.action: BuildDecl(%2, %3)
  attr.action: AddModifier(%1)

rule MethodDeclaration : ZEROORMORE(MethodModifier) + MethodHeader + MethodBody
  attr.action: AddModifierTo(%2, %1)
  attr.action: AddFunctionBodyTo(%2, %3)

rule MethodBody        : ONEOF(Block, ';')
  attr.property : Single
rule MethodHeader      : ONEOF(Result + MethodDeclarator + ZEROORONE(Throws),
                               TypeParameters + ZEROORMORE(Annotation) + Result + MethodDeclarator +
                               ZEROORONE(Throws))
  attr.action.%1: AddTypeTo(%2, %1)
  attr.action.%1: AddThrowsTo(%2, %3)
  attr.action.%2: AddTypeTo(%4, %3)
  attr.action.%2: AddThrowsTo(%4, %5)
  attr.property : Single

rule Result            : ONEOF(UnannType, "void")
  attr.property : Single
rule MethodDeclarator  : Identifier + '(' + ZEROORONE(FormalParameterList) + ')' + ZEROORONE(Dims)
  attr.action: BuildFunction(%1)
  attr.action: AddParams(%3)
  attr.action: AddDims(%5)

rule Throws            : "throws" + ExceptionTypeList
  attr.action: BuildThrows(%2)

rule ExceptionTypeList : ExceptionType + ZEROORMORE(',' + ExceptionType)
rule ExceptionType     : ONEOF(ClassType, TypeVariable)

rule MethodAttr : ONEOF("public", "protected", "private", "abstract", "static",
                        "final", "synchronized", "native", "strictfp")
  attr.property : Single

rule MethodModifier    : ONEOF(Annotation, MethodAttr)
  attr.property : Single

rule FormalParameterListNoReceiver : ONEOF(FormalParameters + ',' + LastFormalParameter,
                                           LastFormalParameter)
  attr.action.%1: BuildVarList(%1, %3)

# ReceiverParameter and FormalParameterListNoReceiver could match at the same
# but with different num of tokens. Here is an example
#   foo(T T.this)
# The NoReceiver could match "T T", while Receiver match "T T.this".
# Although later it figures out NoReceiver is wrong, but at this rule, both rule work.
# If we put NoReceiver as the 1st child and set property 'Single',  we will miss
# Receiver which is the correct one.
#
# So I move Receiver to be the 1st child, since NoReceiver is not correct matching
# if Receiver works.
rule FormalParameterList : ONEOF(ReceiverParameter, FormalParameterListNoReceiver)
  attr.property : Single

rule FormalParameters  : ONEOF(FormalParameter + ZEROORMORE(',' + FormalParameter),
                               ReceiverParameter + ZEROORMORE(',' + FormalParameter))
  attr.action.%1: BuildVarList(%1, %2)
  attr.property : Single

rule FormalParameter   : ZEROORMORE(VariableModifier) + UnannType + VariableDeclaratorId
  attr.action: BuildDecl(%2, %3)
  attr.action: AddModifier(%1)
rule ReceiverParameter : ZEROORMORE(Annotation) + UnannType + ZEROORONE(Identifier + '.') + "this"

rule LastFormalParameter : ONEOF(ZEROORMORE(VariableModifier) + UnannType + ZEROORMORE(Annotation) +
                                   "..." + VariableDeclaratorId,
                                 FormalParameter)
  attr.property : Single


rule FieldAttr : ONEOF("public", "protected", "private", "static", "final", "transient", "volatile")
  attr.property : Single
rule FieldModifier : ONEOF(Annotation, FieldAttr)
  attr.property : Single

################################################################
#                        Constructor                           #
################################################################
rule ConstructorDeclaration : ZEROORMORE(ConstructorModifier) + ConstructorDeclarator +
                              ZEROORONE(Throws) + ConstructorBody
  attr.action : AddFunctionBodyTo(%2, %4)

rule ConstructorAttr : ONEOF("public", "protected", "private")
  attr.property : Single
rule ConstructorModifier    : ONEOF(Annotation, ConstructorAttr)
  attr.property : Single
rule ConstructorDeclarator  : ZEROORONE(TypeParameters) + SimpleTypeName + '(' +
                              ZEROORONE(FormalParameterList) + ')'
  attr.action : BuildConstructor(%2)
  attr.action: AddParams(%4)
rule SimpleTypeName         : Identifier
rule ConstructorBody        : '{' + ZEROORONE(ExplicitConstructorInvocation) +
                              ZEROORONE(BlockStatements) + '}'
  attr.action : BuildBlock(%3)

# Although ExpressionName and Primary are not excluding each other, given the rest
# of the concatenate elements this is a 'Single' rule.
rule ExplicitConstructorInvocation : ONEOF(
         ZEROORONE(TypeArguments) + "this" + '(' + ZEROORONE(ArgumentList) + ')' +  ';',
         ZEROORONE(TypeArguments) + "super" + '(' + ZEROORONE(ArgumentList) + ')' +  ';',
         ExpressionName + '.' + ZEROORONE(TypeArguments) + "super" + '(' + ZEROORONE(ArgumentList) + ')' +  ';',
         Primary + '.' + ZEROORONE(TypeArguments) + "super" + '(' + ZEROORONE(ArgumentList) + ')' +  ';')
  attr.property : Single

######################################################################
#                           Enum                                     #
######################################################################
rule EnumDeclaration: ZEROORMORE(ClassModifier) + "enum" + Identifier +
                      ZEROORONE(Superinterfaces) + EnumBody
  attr.action : BuildClass(%3)
  attr.action : SetClassIsJavaEnum()
  attr.action : AddModifier(%1)
  attr.action : AddSuperInterface(%4)
  attr.action : AddClassBody(%5)

# This returns a PassNode with some ConstantNode and BlockNode. The
# different between ConstantNode and LiteralNode can be found in their definition.
rule EnumBody: '{' + ZEROORONE(EnumConstantList) + ZEROORONE(',') +
               ZEROORONE(EnumBodyDeclarations) + '}'
  attr.action: BuildBlock(%2)
  attr.action: AddToBlock(%4)

# This returns a PassNode
rule EnumConstantList: EnumConstant + ZEROORMORE(',' + EnumConstant)

# AddInitTo() will handle this complicated JavaEnum style initial value of identifier
rule EnumConstant: ZEROORMORE(EnumConstantModifier) + Identifier +
                   ZEROORONE('(' + ZEROORONE(ArgumentList) + ')') + ZEROORONE(ClassBody)
  attr.action : AddInitTo(%2, %3)
  attr.action : AddInitTo(%2, %4)

rule EnumConstantModifier: Annotation

# This returns a PassNode with a set of BlockNode
rule EnumBodyDeclarations: ';' + ZEROORMORE(ClassBodyDeclaration)

######################################################################
#                        Block                                       #
######################################################################

# 1st and 3rd children don't exclude each other. However, if both of them
# match, they will match the same sequence of tokens, being a
# LocalVariableDeclarationStatement. So don't need traverse 3rd any more.
rule BlockStatement  : ONEOF(LocalVariableDeclarationStatement,
                             ClassDeclaration,
                             Statement)
  attr.property : Single

rule BlockStatements : BlockStatement + ZEROORMORE(BlockStatement)
  attr.property.%2 : ZomFast
rule Block           : '{' + ZEROORONE(BlockStatements) + '}'
  attr.action: BuildBlock(%2)


######################################################################
#                        Interface                                   #
######################################################################
rule InterfaceDeclaration : ONEOF(NormalInterfaceDeclaration, AnnotationTypeDeclaration)
  attr.property : Single, Top

rule NormalInterfaceDeclaration : ZEROORMORE(InterfaceModifier) + "interface" + Identifier +
                                  ZEROORONE(TypeParameters) + ZEROORONE(ExtendsInterfaces) + InterfaceBody
  attr.action : BuildInterface(%3)
  attr.action : AddInterfaceBody(%6)

rule InterfaceAttr : ONEOF("public", "protected", "private", "abstract", "static", "strictfp")
  attr.property : Single
rule InterfaceModifier : ONEOF(Annotation, InterfaceAttr)
  attr.property : Single
rule ExtendsInterfaces : "extends" + InterfaceTypeList
rule InterfaceBody     : '{' + ZEROORMORE(InterfaceMemberDeclaration) + '}'
  attr.action : BuildBlock(%2)
  attr.property.%2 : ZomFast

rule InterfaceMemberDeclaration : ONEOF(ConstantDeclaration,
                                        InterfaceMethodDeclaration,
                                        ClassDeclaration,
                                        InterfaceDeclaration,
                                        ';')
  attr.property : Single

# constant decl is also called field decl. In interface, field must have a variable initializer
# However, the rules below don't tell this limitation.
rule ConstantDeclaration : ZEROORMORE(ConstantModifier) + UnannType + VariableDeclaratorList + ';'
rule ConstantAttr : ONEOF("public", "static", "final")
  attr.property : Single
rule ConstantModifier : ONEOF(Annotation, ConstantAttr)
  attr.property : Single

rule InterfaceMethodDeclaration : ZEROORMORE(InterfaceMethodModifier) + MethodHeader + MethodBody
  attr.action: AddModifierTo(%2, %1)
  attr.action: AddFunctionBodyTo(%2, %3)
rule InterfaceMethodAttr : ONEOF("public", "abstract", "default", "static", "strictfp")
  attr.property : Single
rule InterfaceMethodModifier : ONEOF(Annotation, InterfaceMethodAttr)
  attr.property : Single

######################################################################
#                        Annotation Type                             #
######################################################################
rule AnnotationTypeDeclaration : ZEROORMORE(InterfaceModifier) + '@' + "interface" +
                                 Identifier + AnnotationTypeBody
  attr.action : BuildAnnotationType(%4)
  attr.action : AddModifier(%1)
  attr.action : AddAnnotationTypeBody(%5)

rule AnnotationTypeBody : '{' + ZEROORMORE(AnnotationTypeMemberDeclaration) + '}'
  attr.action : BuildBlock(%2)
rule AnnotationTypeMemberDeclaration : ONEOF(AnnotationTypeElementDeclaration,
                                             ConstantDeclaration,
                                             ClassDeclaration,
                                             InterfaceDeclaration,
                                             ';')
  attr.property : Single
rule AnnotationTypeElementDeclaration : ZEROORMORE(AnnotationTypeElementModifier) + UnannType +
                                          Identifier + '(' +  ')' + ZEROORONE(Dims) +
                                          ZEROORONE(DefaultValue) + ';'
rule AnnotationTypeElementAttr : ONEOF("public", "abstract")
  attr.property : Single
rule AnnotationTypeElementModifier : ONEOF(Annotation, AnnotationTypeElementAttr)
  attr.property : Single
rule DefaultValue : "default" + ElementValue

######################################################################
#                        Annotation                                  #
######################################################################
rule Annotation : ONEOF(NormalAnnotation,
                        MarkerAnnotation,
                        SingleElementAnnotation)

rule NormalAnnotation : '@' + TypeName + '(' + ZEROORONE(ElementValuePairList) + ')'
  attr.action : BuildAnnotation(%2)

rule MarkerAnnotation : '@' + TypeName
  attr.action : BuildAnnotation(%2)

rule SingleElementAnnotation : '@' + TypeName + '(' + ElementValue + ')'
  attr.action : BuildAnnotation(%2)

rule ElementValuePairList : ElementValuePair + ZEROORMORE(',' + ElementValuePair)
rule ElementValuePair : Identifier + '=' + ElementValue
rule ElementValue : ONEOF(ConditionalExpression,
                          ElementValueArrayInitializer,
                          Annotation)
rule ElementValueArrayInitializer : '{' + ZEROORONE(ElementValueList) + ZEROORONE(',') + '}'
rule ElementValueList : ElementValue + ZEROORMORE(',' + ElementValue)

######################################################################
#                        Package                                     #
######################################################################
rule PackageModifier: Annotation
rule PackageDeclaration: ZEROORMORE(PackageModifier) + "package" + Identifier + ZEROORMORE('.' + Identifier) + ';'
  attr.action : BuildField(%3, %4)
  attr.action : BuildPackageName()
  attr.property : Top
  attr.property.%1,%4 : ZomFast

rule ImportDeclaration: ONEOF(SingleTypeImportDeclaration,
                              TypeImportOnDemandDeclaration,
                              SingleStaticImportDeclaration,
                              StaticImportOnDemandDeclaration)
  attr.property : Top

rule SingleTypeImportDeclaration: "import" + TypeName + ';'
  attr.action : BuildSingleTypeImport(%2)
rule TypeImportOnDemandDeclaration: "import" + PackageOrTypeName + '.' + '*' + ';'
  attr.action : BuildAllTypeImport(%2)
rule SingleStaticImportDeclaration: "import" + "static" + TypeName + '.' + Identifier + ';'
  attr.action : BuildField(%3, %5)
  attr.action : BuildSingleStaticImport()
rule StaticImportOnDemandDeclaration: "import" + "static" + TypeName + '.' + '*' + ';'
  attr.action : BuildAllStaticImport(%3)
