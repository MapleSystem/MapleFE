# Copyright (C) [2021] Futurewei Technologies, Inc. All rights reverved.
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

#-------------------------------------------------------------------------------
#                                    Expressions
#-------------------------------------------------------------------------------

##-----------------------------------
##rule IdentifierReference[Yield] :
##  Identifier
##  [~Yield] yield

rule IdentifierReference : ONEOF(
  Identifier,
  "yield")

##-----------------------------------
##rule BindingIdentifier[Yield] :
##  Identifier
##  [~Yield] yield

rule BindingIdentifier : ONEOF(Identifier, "yield")

##-----------------------------------
##rule LabelIdentifier[Yield] :
##  Identifier
##  [~Yield] yield

##-----------------------------------
##rule Identifier :
##  IdentifierName but not ReservedWord

##-----------------------------------
##rule PrimaryExpression[Yield] :
##  this
##  IdentifierReference[?Yield]
##  Literal
##  ArrayLiteral[?Yield]
##  ObjectLiteral[?Yield]
##  FunctionExpression
##  ClassExpression[?Yield]
##  GeneratorExpression
##  RegularExpressionLiteral
##  TemplateLiteral[?Yield]
##  CoverParenthesizedExpressionAndArrowParameterList[?Yield]

rule PrimaryExpression : ONEOF(
  "this",
  IdentifierReference,
  Literal)
#  ArrayLiteral[?Yield]
#  ObjectLiteral[?Yield]
#  FunctionExpression
#  ClassExpression[?Yield]
#  GeneratorExpression
#  RegularExpressionLiteral
#  TemplateLiteral[?Yield]
#  CoverParenthesizedExpressionAndArrowParameterList[?Yield]

##-----------------------------------
##rule CoverParenthesizedExpressionAndArrowParameterList[Yield] :
##  ( Expression[In, ?Yield] )
##  ( )
##  ( ... BindingIdentifier[?Yield] )
##  ( Expression[In, ?Yield] , ... BindingIdentifier[?Yield] )
##  When processing the production
##        PrimaryExpression[Yield] : CoverParenthesizedExpressionAndArrowParameterList[?Yield]
##  the interpretation of CoverParenthesizedExpressionAndArrowParameterList is refined using the following grammar:

##-----------------------------------
##rule ParenthesizedExpression[Yield] :
##  ( Expression[In, ?Yield] )

##-----------------------------------
##rule Literal :
##  NullLiteral
##  BooleanLiteral
##  NumericLiteral
##  StringLiteral

##-----------------------------------
##rule ArrayLiteral[Yield] :
##  [ Elisionopt ]
##  [ ElementList[?Yield] ]
##  [ ElementList[?Yield] , Elisionopt ]

##-----------------------------------
##rule ElementList[Yield] :
##  Elisionopt AssignmentExpression[In, ?Yield]
##  Elisionopt SpreadElement[?Yield]
##  ElementList[?Yield] , Elisionopt AssignmentExpression[In, ?Yield]
##  ElementList[?Yield] , Elisionopt SpreadElement[?Yield]

##-----------------------------------
##rule Elision :
##  ,
##  Elision ,

rule Elision : ONEOF(',',
                     Elision + ',')

##-----------------------------------
##rule SpreadElement[Yield] :
##  ... AssignmentExpression[In, ?Yield]

##-----------------------------------
##rule ObjectLiteral[Yield] :
##  { }
##  { PropertyDefinitionList[?Yield] }
##  { PropertyDefinitionList[?Yield] , }

##-----------------------------------
##rule PropertyDefinitionList[Yield] :
##  PropertyDefinition[?Yield]
##  PropertyDefinitionList[?Yield] , PropertyDefinition[?Yield]

##-----------------------------------
##rule PropertyDefinition[Yield] :
##  IdentifierReference[?Yield]
##  CoverInitializedName[?Yield]
##  PropertyName[?Yield] : AssignmentExpression[In, ?Yield]
##  MethodDefinition[?Yield]

##-----------------------------------
##rule PropertyName[Yield] :
##  LiteralPropertyName
##  ComputedPropertyName[?Yield]

##-----------------------------------
##rule LiteralPropertyName :
##  IdentifierName
##  StringLiteral
##  NumericLiteral

##-----------------------------------
##rule ComputedPropertyName[Yield] :
##  [ AssignmentExpression[In, ?Yield] ]

##-----------------------------------
##rule CoverInitializedName[Yield] :
##  IdentifierReference[?Yield] Initializer[In, ?Yield]

##-----------------------------------
##rule Initializer[In, Yield] :
##  = AssignmentExpression[?In, ?Yield]
rule Initializer : '=' + AssignmentExpression

##-----------------------------------
##rule TemplateLiteral[Yield] :
##  NoSubstitutionTemplate
##  TemplateHead Expression[In, ?Yield] TemplateSpans[?Yield]

##-----------------------------------
##rule TemplateSpans[Yield] :
##  TemplateTail
##  TemplateMiddleList[?Yield] TemplateTail

##-----------------------------------
##rule TemplateMiddleList[Yield] :
##  TemplateMiddle Expression[In, ?Yield]
##  TemplateMiddleList[?Yield] TemplateMiddle Expression[In, ?Yield]

##-----------------------------------
##rule MemberExpression[Yield] :
##  PrimaryExpression[?Yield]
##  MemberExpression[?Yield] [ Expression[In, ?Yield] ]
##  MemberExpression[?Yield] . IdentifierName
##  MemberExpression[?Yield] TemplateLiteral[?Yield]
##  SuperProperty[?Yield]
##  MetaProperty
##  new MemberExpression[?Yield] Arguments[?Yield]

rule MemberExpression : ONEOF(
  PrimaryExpression,
#  MemberExpression[?Yield] [ Expression[In, ?Yield] ]
  MemberExpression + '.' + Identifier,
#  MemberExpression[?Yield] TemplateLiteral[?Yield]
#  SuperProperty[?Yield]
#  MetaProperty
  "new" + MemberExpression + Arguments)
  attr.action.%2 : BuildField(%1, %3)

##-----------------------------------
##rule SuperProperty[Yield] :
##  super [ Expression[In, ?Yield] ]
##  super . IdentifierName

##-----------------------------------
##rule MetaProperty :
##  NewTarget

##-----------------------------------
##rule NewTarget :
##  new . target

##-----------------------------------
##rule NewExpression[Yield] :
##  MemberExpression[?Yield]
##  new NewExpression[?Yield]
rule NewExpression : ONEOF(MemberExpression, "new" + NewExpression)

##-----------------------------------
##rule CallExpression[Yield] :
##  MemberExpression[?Yield] Arguments[?Yield]
##  SuperCall[?Yield]
##  CallExpression[?Yield] Arguments[?Yield]
##  CallExpression[?Yield] [ Expression[In, ?Yield] ]
##  CallExpression[?Yield] . IdentifierName
##  CallExpression[?Yield] TemplateLiteral[?Yield]

rule CallExpression : ONEOF(
  MemberExpression + Arguments,
#  SuperCall[?Yield]
  CallExpression + Arguments,
#  CallExpression[?Yield] [ Expression[In, ?Yield] ]
  CallExpression + '.' + Identifier)
#  CallExpression[?Yield] TemplateLiteral[?Yield]
  attr.action.%1,%2 : BuildCall(%1)
  attr.action.%1,%2 : AddArguments(%2)

##-----------------------------------
##rule SuperCall[Yield] :
##  super Arguments[?Yield]

##-----------------------------------
##rule Arguments[Yield] :
##  ( )
##  ( ArgumentList[?Yield] )

rule Arguments : ONEOF(
  '(' + ')',
  '(' + ArgumentList + ')')
  attr.action.%2 : PassChild(%2)

##-----------------------------------
##rule ArgumentList[Yield] :
##  AssignmentExpression[In, ?Yield]
##  ... AssignmentExpression[In, ?Yield]
##  ArgumentList[?Yield] , AssignmentExpression[In, ?Yield]
##  ArgumentList[?Yield] , ... AssignmentExpression[In, ?Yield]

rule ArgumentList : ONEOF(AssignmentExpression,
                          "..." + AssignmentExpression,
                          ArgumentList + ',' + AssignmentExpression,
                          ArgumentList + ',' + "..." + AssignmentExpression)

##-----------------------------------
##rule LeftHandSideExpression[Yield] :
##  NewExpression[?Yield]
##  CallExpression[?Yield]

rule LeftHandSideExpression : ONEOF(NewExpression, CallExpression)

##-----------------------------------
##rule PostfixExpression[Yield] :
##  LeftHandSideExpression[?Yield]
##  LeftHandSideExpression[?Yield] [no LineTerminator here] ++
##  LeftHandSideExpression[?Yield] [no LineTerminator here] --

rule PostfixExpression : ONEOF(
  LeftHandSideExpression)
#  LeftHandSideExpression[?Yield] [no LineTerminator here] ++
#  LeftHandSideExpression[?Yield] [no LineTerminator here] --

##-----------------------------------
##rule UnaryExpression[Yield] :
##  PostfixExpression[?Yield]
##  delete UnaryExpression[?Yield]
##  void UnaryExpression[?Yield]
##  typeof UnaryExpression[?Yield]
##  ++ UnaryExpression[?Yield]
##  -- UnaryExpression[?Yield]
##  + UnaryExpression[?Yield]
##  - UnaryExpression[?Yield]
##  ~ UnaryExpression[?Yield]
##  ! UnaryExpression[?Yield]

rule UnaryExpression : ONEOF(
  PostfixExpression)
#  delete UnaryExpression[?Yield]
#  void UnaryExpression[?Yield]
#  typeof UnaryExpression[?Yield]
#  ++ UnaryExpression[?Yield]
#  -- UnaryExpression[?Yield]
#  + UnaryExpression[?Yield]
#  - UnaryExpression[?Yield]
#  ~ UnaryExpression[?Yield]
#  ! UnaryExpression[?Yield]

##-----------------------------------
##rule MultiplicativeExpression[Yield] :
##  UnaryExpression[?Yield]
##  MultiplicativeExpression[?Yield] MultiplicativeOperator UnaryExpression[?Yield]

rule MultiplicativeExpression : ONEOF(
  UnaryExpression)
#  MultiplicativeExpression[?Yield] MultiplicativeOperator UnaryExpression[?Yield]

##-----------------------------------
##rule MultiplicativeOperator : one of
##  * / %

##-----------------------------------
##rule AdditiveExpression[Yield] :
##  MultiplicativeExpression[?Yield]
##  AdditiveExpression[?Yield] + MultiplicativeExpression[?Yield]
##  AdditiveExpression[?Yield] - MultiplicativeExpression[?Yield]

rule AdditiveExpression : ONEOF(
  MultiplicativeExpression,
  AdditiveExpression + '+' + MultiplicativeExpression,
  AdditiveExpression + '-' + MultiplicativeExpression)
  attr.action.%2,%3 : BuildBinaryOperation(%1, %2, %3)

##-----------------------------------
##rule ShiftExpression[Yield] :
##  AdditiveExpression[?Yield]
##  ShiftExpression[?Yield] << AdditiveExpression[?Yield]
##  ShiftExpression[?Yield] >> AdditiveExpression[?Yield]
##  ShiftExpression[?Yield] >>> AdditiveExpression[?Yield]
rule ShiftExpression : ONEOF(AdditiveExpression,
                             ShiftExpression + "<<" + AdditiveExpression,
                             ShiftExpression + ">>" + AdditiveExpression,
                             ShiftExpression + ">>>" + AdditiveExpression)
  attr.action.%2,%3,%4 : BuildBinaryOperation(%1, %2, %3)

##-----------------------------------
##rule RelationalExpression[In, Yield] :
##  ShiftExpression[?Yield]
##  RelationalExpression[?In, ?Yield] < ShiftExpression[?Yield]
##  RelationalExpression[?In, ?Yield] > ShiftExpression[?Yield]
##  RelationalExpression[?In, ?Yield] <= ShiftExpression[? Yield]
##  RelationalExpression[?In, ?Yield] >= ShiftExpression[?Yield]
##  RelationalExpression[?In, ?Yield] instanceof ShiftExpression[?Yield]
##  [+In] RelationalExpression[In, ?Yield] in ShiftExpression[?Yield]

rule RelationalExpression : ONEOF(ShiftExpression,
                                  RelationalExpression + '<' + ShiftExpression,
                                  RelationalExpression + '>' + ShiftExpression,
                                  RelationalExpression + "<=" + ShiftExpression,
                                  RelationalExpression + ">=" + ShiftExpression,
                                  RelationalExpression + "instanceof" + ShiftExpression)
#  [+In] RelationalExpression[In, ?Yield] in ShiftExpression[?Yield]
  attr.action.%2,%3,%4,%5,%6 : BuildBinaryOperation(%1, %2, %3)

##-----------------------------------
##rule EqualityExpression[In, Yield] :
##  RelationalExpression[?In, ?Yield]
##  EqualityExpression[?In, ?Yield] == RelationalExpression[?In, ?Yield]
##  EqualityExpression[?In, ?Yield] != RelationalExpression[?In, ?Yield]
##  EqualityExpression[?In, ?Yield] === RelationalExpression[?In, ?Yield]
##  EqualityExpression[?In, ?Yield] !== RelationalExpression[?In, ?Yield]

rule EqualityExpression : ONEOF(
  RelationalExpression)
#  EqualityExpression[?In, ?Yield] == RelationalExpression[?In, ?Yield]
#  EqualityExpression[?In, ?Yield] != RelationalExpression[?In, ?Yield]
#  EqualityExpression[?In, ?Yield] === RelationalExpression[?In, ?Yield]
#  EqualityExpression[?In, ?Yield] !== RelationalExpression[?In, ?Yield]

##-----------------------------------
##rule BitwiseANDExpression[In, Yield] :
##  EqualityExpression[?In, ?Yield]
##  BitwiseANDExpression[?In, ?Yield] & EqualityExpression[?In, ?Yield]

rule BitwiseANDExpression : ONEOF(
  EqualityExpression)
#  BitwiseANDExpression[?In, ?Yield] & EqualityExpression[?In, ?Yield]

##-----------------------------------
##rule BitwiseXORExpression[In, Yield] :
##  BitwiseANDExpression[?In, ?Yield]
##  BitwiseXORExpression[?In, ?Yield] ^ BitwiseANDExpression[?In, ?Yield]

rule BitwiseXORExpression : ONEOF(
  BitwiseANDExpression)
#  BitwiseXORExpression[?In, ?Yield] ^ BitwiseANDExpression[?In, ?Yield]

##-----------------------------------
##rule BitwiseORExpression[In, Yield] :
##  BitwiseXORExpression[?In, ?Yield]
##  BitwiseORExpression[?In, ?Yield] | BitwiseXORExpression[?In, ?Yield]

rule BitwiseORExpression : ONEOF(
  BitwiseXORExpression)
##  BitwiseORExpression[?In, ?Yield] | BitwiseXORExpression[?In, ?Yield]

##-----------------------------------
##rule LogicalANDExpression[In, Yield] :
##  BitwiseORExpression[?In, ?Yield]
##  LogicalANDExpression[?In, ?Yield] && BitwiseORExpression[?In, ?Yield]

rule LogicalANDExpression : ONEOF(
  BitwiseORExpression)
#  LogicalANDExpression[?In, ?Yield] && BitwiseORExpression[?In, ?Yield]

##-----------------------------------
##rule LogicalORExpression[In, Yield] :
##  LogicalANDExpression[?In, ?Yield]
##  LogicalORExpression[?In, ?Yield] || LogicalANDExpression[?In, ?Yield]

rule LogicalORExpression : ONEOF(
  LogicalANDExpression)
#  LogicalORExpression[?In, ?Yield] || LogicalANDExpression[?In, ?Yield]

##-----------------------------------
##rule ConditionalExpression[In, Yield] :
##  LogicalORExpression[?In, ?Yield]
##  LogicalORExpression[?In,?Yield] ? AssignmentExpression[In, ?Yield] : AssignmentExpression[?In, ?Yield]

rule ConditionalExpression : ONEOF(
  LogicalORExpression)
#  LogicalORExpression[?In,?Yield] ? AssignmentExpression[In, ?Yield] : AssignmentExpression[?In, ?Yield]

##-----------------------------------
##rule AssignmentExpression[In, Yield] :
##  ConditionalExpression[?In, ?Yield]
##  [+Yield] YieldExpression[?In]
##  ArrowFunction[?In, ?Yield]
##  LeftHandSideExpression[?Yield] = AssignmentExpression[?In, ?Yield]
##  LeftHandSideExpression[?Yield] AssignmentOperator AssignmentExpression[?In, ?Yield]

rule AssignmentExpression : ONEOF(
  ConditionalExpression,
#  [+Yield] YieldExpression[?In]
#  ArrowFunction[?In, ?Yield]
  LeftHandSideExpression + '=' + AssignmentExpression,
  LeftHandSideExpression + AssignmentOperator + AssignmentExpression)
  attr.action.%2,%3 : BuildAssignment(%1, %2, %3)

rule AssignmentOperator : ONEOF("*=", "/=", "%=", "+=", "-=", "<<=", ">>=", ">>>=", "&=", "^=", "|=")

##-----------------------------------
##rule Expression[In, Yield] :
##  AssignmentExpression[?In, ?Yield]
##  Expression[?In, ?Yield] , AssignmentExpression[?In, ?Yield]

rule Expression : ONEOF(
  AssignmentExpression)
#  Expression[?In, ?Yield] , AssignmentExpression[?In, ?Yield]

#-------------------------------------------------------------------------------
#                                    Statements
#-------------------------------------------------------------------------------

##-----------------------------------
##rule Statement[Yield, Return] :
##  BlockStatement[?Yield, ?Return]
##  VariableStatement[?Yield]
##  EmptyStatement
##  ExpressionStatement[?Yield]
##  IfStatement[?Yield, ?Return]
##  BreakableStatement[?Yield, ?Return]
##  ContinueStatement[?Yield]
##  BreakStatement[?Yield]
##  [+Return] ReturnStatement[?Yield]
##  WithStatement[?Yield, ?Return]
##  LabelledStatement[?Yield, ?Return]
##  ThrowStatement[?Yield]
##  TryStatement[?Yield, ?Return]
##  DebuggerStatement

rule Statement : ONEOF(
  BlockStatement,
  VariableStatement,
#  EmptyStatement
  ExpressionStatement,
  IfStatement,
#  BreakableStatement[?Yield, ?Return]
#  ContinueStatement[?Yield]
#  BreakStatement[?Yield]
  ReturnStatement)
#  WithStatement[?Yield, ?Return]
#  LabelledStatement[?Yield, ?Return]
#  ThrowStatement[?Yield]
#  TryStatement[?Yield, ?Return]
#  DebuggerStatement
  attr.property : Top

##-----------------------------------
##rule Declaration[Yield] :
##  HoistableDeclaration[?Yield]
##  ClassDeclaration[?Yield]
##  LexicalDeclaration[In, ?Yield]
rule Declaration : ONEOF(HoistableDeclaration,
##  ClassDeclaration[?Yield]
                         LexicalDeclaration)
  attr.property : Top

##-----------------------------------
##rule HoistableDeclaration[Yield, Default] :
##  FunctionDeclaration[?Yield,?Default]
##  GeneratorDeclaration[?Yield, ?Default]
rule HoistableDeclaration : ONEOF(FunctionDeclaration)
##  GeneratorDeclaration[?Yield, ?Default]

##-----------------------------------
##rule BreakableStatement[Yield, Return] :
##  IterationStatement[?Yield, ?Return]
##  SwitchStatement[?Yield, ?Return]

##-----------------------------------
##rule BlockStatement[Yield, Return] :
##  Block[?Yield, ?Return]
rule BlockStatement : Block

##-----------------------------------
##rule Block[Yield, Return] :
##  { StatementList[?Yield, ?Return]opt }
rule Block : '{' + ZEROORONE(StatementList) + '}'

##-----------------------------------
##rule StatementList[Yield, Return] :
##  StatementListItem[?Yield, ?Return]
##  StatementList[?Yield, ?Return] StatementListItem[?Yield, ?Return]
rule StatementList : ONEOF(StatementListItem,
                           StatementList + StatementListItem)

##-----------------------------------
##rule StatementListItem[Yield, Return] :
##  Statement[?Yield, ?Return]
##  Declaration[?Yield]
rule StatementListItem : ONEOF(Statement, Declaration)

##-----------------------------------
##rule LexicalDeclaration[In, Yield] :
##  LetOrConst BindingList[?In, ?Yield] ;
rule LexicalDeclaration : ONEOF("let" + BindingList + ';',
                                "const" + BindingList + ';')
  attr.action.%1,%2 : BuildDecl(%2)

##-----------------------------------
##rule LetOrConst :
##  let
##  const
rule LetOrConst : ONEOF("let", "const")

##-----------------------------------
##rule BindingList[In, Yield] :
##  LexicalBinding[?In, ?Yield]
##  BindingList[?In, ?Yield] , LexicalBinding[?In, ?Yield]
rule BindingList : ONEOF(LexicalBinding,
                         BindingList + ',' + LexicalBinding)

##-----------------------------------
##rule LexicalBinding[In, Yield] :
##  BindingIdentifier[?Yield] Initializer[?In, ?Yield]opt
##  BindingPattern[?Yield] Initializer[?In, ?Yield]
rule LexicalBinding : ONEOF(BindingIdentifier + ZEROORONE(Initializer),
                            BindingIdentifier + ":" + TYPE + ZEROORONE(Initializer),
                            BindingPattern    + ZEROORONE(Initializer),
                            BindingPattern    + ":" + TYPE + ZEROORONE(Initializer))
  attr.action.%1,%3 : AddInitTo(%1, %2)
  attr.action.%2,%4 : AddInitTo(%1, %4)
  attr.action.%2,%4 : AddTypeTo(%1, %3)

##-----------------------------------
##rule VariableStatement[Yield] :
##  var VariableDeclarationList[In, ?Yield] ;
rule VariableStatement : "var" + VariableDeclarationList + ';'
  attr.action : PassChild(%2)

##-----------------------------------
##rule VariableDeclarationList[In, Yield] :
##  VariableDeclaration[?In, ?Yield]
##  VariableDeclarationList[?In, ?Yield] , VariableDeclaration[?In, ?Yield]
rule VariableDeclarationList : ONEOF(
  VariableDeclaration,
  VariableDeclarationList + ',' + VariableDeclaration)

##-----------------------------------
##rule VariableDeclaration[In, Yield] :
##  BindingIdentifier[?Yield] Initializer[?In, ?Yield]opt
##  BindingPattern[?Yield] Initializer[?In, ?Yield]

# Typescript ask for explicit type. But it also allows implicit type if referrable.
rule VariableDeclaration : ONEOF(BindingIdentifier + ':' + TYPE + ZEROORONE(Initializer),
                                 BindingIdentifier + ZEROORONE(Initializer))
  attr.action.%1 : AddInitTo(%1, %4)
  attr.action.%1 : BuildDecl(%3, %1)
  attr.action.%2 : AddInitTo(%1, %2)
  attr.action.%2 : BuildDecl(%1)

##-----------------------------------
##rule BindingPattern[Yield] :
##  ObjectBindingPattern[?Yield]
##  ArrayBindingPattern[?Yield]
rule BindingPattern : ONEOF(ObjectBindingPattern)

##-----------------------------------
##rule ObjectBindingPattern[Yield] :
##  { }
##  { BindingPropertyList[?Yield] }
##  { BindingPropertyList[?Yield] , }
rule ObjectBindingPattern : ONEOF('{' + '}')
#                                   '{' + BindingPropertyList + '}',
#                                   '{' + BindingPropertyList + ',' + '}')

##-----------------------------------
##rule ArrayBindingPattern[Yield] :
##  [ Elisionopt BindingRestElement[?Yield]opt ]
##  [ BindingElementList[?Yield] ]
##  [ BindingElementList[?Yield] , Elisionopt BindingRestElement[?Yield]opt ]

##-----------------------------------
##rule BindingPropertyList[Yield] :
##  BindingProperty[?Yield]
##  BindingPropertyList[?Yield] , BindingProperty[?Yield]

##-----------------------------------
##rule BindingElementList[Yield] :
##  BindingElisionElement[?Yield]
##  BindingElementList[?Yield] , BindingElisionElement[?Yield]

##-----------------------------------
##rule BindingElisionElement[Yield] :
##  Elisionopt BindingElement[?Yield]

##-----------------------------------
##rule BindingProperty[Yield] :
##  SingleNameBinding[?Yield]
##  PropertyName[?Yield] : BindingElement[?Yield]

##-----------------------------------
##rule BindingElement[Yield] :
##  SingleNameBinding[?Yield]
##  BindingPattern[?Yield] Initializer[In, ?Yield]opt
rule BindingElement : ONEOF(SingleNameBinding,
                            BindingPattern + ZEROORONE(Initializer))

##-----------------------------------
##rule SingleNameBinding[Yield] :
##  BindingIdentifier[?Yield] Initializer[In, ?Yield]opt
rule SingleNameBinding : BindingIdentifier + ZEROORONE(Initializer)

##-----------------------------------
##rule BindingRestElement[Yield] :
##  ... BindingIdentifier[?Yield]
rule BindingRestElement : "..." + BindingIdentifier

##-----------------------------------
##rule EmptyStatement :
##  ;

##-----------------------------------
##rule ExpressionStatement[Yield] :
##  [lookahead NotIn {{, function, class, let [}] Expression[In, ?Yield] ;

rule ExpressionStatement : Expression + ';'

##-----------------------------------
##rule IfStatement[Yield, Return] :
##  if ( Expression[In, ?Yield] ) Statement[?Yield, ?Return] else Statement[?Yield, ?Return]
##  if ( Expression[In, ?Yield] ) Statement[?Yield, ?Return]
rule TrueBranch : Statement
rule IfStatement : ONEOF(
  "if" + '(' + Expression + ')' + TrueBranch + "else" + Statement,
  "if" + '(' + Expression + ')' + Statement)
  attr.action.%1,%2: BuildCondBranch(%3)
  attr.action.%1,%2: AddCondBranchTrueStatement(%5)
  attr.action.%1:    AddCondBranchFalseStatement(%7)

## " // This line is to make my vim in right color

##-----------------------------------
##rule IterationStatement[Yield, Return] :
##  do Statement[?Yield, ?Return] while ( Expression[In, ?Yield] ) ;
##  while ( Expression[In, ?Yield] ) Statement[?Yield, ?Return]
##  for ( [lookahead NotIn {let [}] Expression[?Yield]opt ; Expression[In, ?Yield]opt ; Expression[In, ?Yield]opt ) Statement[?Yield, ?Return]
##  for ( var VariableDeclarationList[?Yield] ; Expression[In, ?Yield]opt ; Expression[In, ?Yield]opt ) Statement[?Yield, ?Return]
##  for ( LexicalDeclaration[?Yield] Expression[In, ?Yield]opt ; Expression[In, ?Yield]opt ) Statement[?Yield, ?Return]
##  for ( [lookahead NotIn {let [}] LeftHandSideExpression[?Yield] in Expression[In, ?Yield] ) Statement[?Yield, ?Return]
##  for ( var ForBinding[?Yield] in Expression[In, ?Yield] ) Statement[?Yield, ?Return]
##  for ( ForDeclaration[?Yield] in Expression[In, ?Yield] ) Statement[?Yield, ?Return]
##  for ( [lookahead NotEq let ] LeftHandSideExpression[?Yield] of AssignmentExpression[In, ?Yield] ) Statement[?Yield, ?Return]
##  for ( var ForBinding[?Yield] of AssignmentExpression[In, ?Yield] ) Statement[?Yield, ?Return]
##  for ( ForDeclaration[?Yield] of AssignmentExpression[In, ?Yield] ) Statement[?Yield, ?Return]

##-----------------------------------
##rule ForDeclaration[Yield] :
##  LetOrConst ForBinding[?Yield]

##-----------------------------------
##rule ForBinding[Yield] :
##  BindingIdentifier[?Yield]
##  BindingPattern[?Yield]

##-----------------------------------
##rule ContinueStatement[Yield] :
##  continue ;
##  continue [no LineTerminator here] LabelIdentifier[?Yield] ;

##-----------------------------------
##rule BreakStatement[Yield] :
##  break ;
##  break [no LineTerminator here] LabelIdentifier[?Yield] ;

##-----------------------------------
##rule ReturnStatement[Yield] :
##  return ;
##  return [no LineTerminator here] Expression[In, ?Yield] ;
rule ReturnStatement :ONEOF("return" + ';',
                            "return" + Expression + ';')

##-----------------------------------
##rule WithStatement[Yield, Return] :
##  with ( Expression[In, ?Yield] ) Statement[?Yield, ?Return]

##-----------------------------------
##rule SwitchStatement[Yield, Return] :
##  switch ( Expression[In, ?Yield] ) CaseBlock[?Yield, ?Return]

##-----------------------------------
##rule CaseBlock[Yield, Return] :
##  { CaseClauses[?Yield, ?Return]opt }
##  { CaseClauses[?Yield, ?Return]opt DefaultClause[?Yield, ?Return] CaseClauses[?Yield, ?Return]opt }

##-----------------------------------
##rule CaseClauses[Yield, Return] :
##  CaseClause[?Yield, ?Return]
##  CaseClauses[?Yield, ?Return] CaseClause[?Yield, ?Return]

##-----------------------------------
##rule CaseClause[Yield, Return] :
##  case Expression[In, ?Yield] : StatementList[?Yield, ?Return]opt

##-----------------------------------
##rule DefaultClause[Yield, Return] :
##  default : StatementList[?Yield, ?Return]opt

##-----------------------------------
##rule LabelledStatement[Yield, Return] :
##  LabelIdentifier[?Yield] : LabelledItem[?Yield, ?Return]

##-----------------------------------
##rule LabelledItem[Yield, Return] :
##  Statement[?Yield, ?Return]
##  FunctionDeclaration[?Yield]

##-----------------------------------
##rule ThrowStatement[Yield] :
##  throw [no LineTerminator here] Expression[In, ?Yield] ;

##-----------------------------------
##rule TryStatement[Yield, Return] :
##  try Block[?Yield, ?Return] Catch[?Yield, ?Return]
##  try Block[?Yield, ?Return] Finally[?Yield, ?Return]
##  try Block[?Yield, ?Return] Catch[?Yield, ?Return] Finally[?Yield, ?Return]

##-----------------------------------
##rule Catch[Yield, Return] :
##  catch ( CatchParameter[?Yield] ) Block[?Yield, ?Return]

##-----------------------------------
##rule Finally[Yield, Return] :
##  finally Block[?Yield, ?Return]

##-----------------------------------
##rule CatchParameter[Yield] :
##  BindingIdentifier[?Yield]
##  BindingPattern[?Yield]
rule CatchParameter : ONEOF(BindingIdentifier, BindingPattern)

##-----------------------------------
rule DebuggerStatement : "debugger" + ';'


######################################################################
##                      Function and Class
######################################################################

## FunctionDeclaration[Yield, Default] :
## function BindingIdentifier[?Yield] ( FormalParameters ) { FunctionBody }
## [+Default] function ( FormalParameters ) { FunctionBody }
rule FunctionDeclaration : ONEOF(
  "function" + BindingIdentifier + '(' + FormalParameters + ')' + '{' + FunctionBody + '}',
  "function" + '(' + FormalParameters + ')' + '{' + FunctionBody + '}')
  attr.action.%1 : BuildFunction(%2)
  attr.action.%1 : AddParams(%4)
  attr.action.%1 : AddFunctionBody(%7)

##
## FunctionExpression :
## function BindingIdentifieropt ( FormalParameters ) { FunctionBody }
rule FunctionExpression : "function" + ZEROORONE(BindingIdentifier) + '(' + FormalParameters + ')' + '{' + FunctionBody + '}'

##
## StrictFormalParameters[Yield] :
## FormalParameters[?Yield]
rule StrictFormalParameters : FormalParameters

##
## FormalParameters[Yield] :
## [empty]
## FormalParameterList[?Yield]
rule FormalParameters : ZEROORONE(FormalParameterList)

##
## FormalParameterList[Yield] :
## FunctionRestParameter[?Yield]
## FormalsList[?Yield]
## FormalsList[?Yield] , FunctionRestParameter[?Yield]
rule FormalParameterList : ONEOF(FunctionRestParameter,
                                 FormalsList,
                                 FormalsList + ',' + FunctionRestParameter)

##
## FormalsList[Yield] :
## FormalParameter[?Yield]
## FormalsList[?Yield] , FormalParameter[?Yield]
rule FormalsList : ONEOF(FormalParameter,
                         FormalsList + ',' + FormalParameter)

##
## FunctionRestParameter[Yield] :
## BindingRestElement[?Yield]
rule FunctionRestParameter : BindingRestElement

##
## FormalParameter[Yield] :
## BindingElement[?Yield]

## Typescript requires type. So this is different than JS spec.
rule FormalParameter : BindingElement + ':' + TYPE
  attr.action : BuildDecl(%3, %1)

##
## FunctionBody[Yield] :
## FunctionStatementList[?Yield]
rule FunctionBody : FunctionStatementList
  attr.action : BuildBlock(%1)

##
## FunctionStatementList[Yield] :
## StatementList[?Yield, Return]opt
rule FunctionStatementList : ZEROORONE(StatementList)

## See 14.2
## ArrowFunction[In, Yield] :
## ArrowParameters[?Yield] [no LineTerminator here] => ConciseBody[?In]
## See 14.2
## ArrowParameters[Yield] :
## BindingIdentifier[?Yield]
## CoverParenthesizedExpressionAndArrowParameterList[?Yield]
## See 14.2
## ConciseBody[In] :
## [lookahead ≠ { ] AssignmentExpression[?In]
## { FunctionBody }
## 
## See 14.3
## MethodDefinition[Yield] :
## PropertyName[?Yield] ( StrictFormalParameters ) { FunctionBody }
## GeneratorMethod[?Yield]
## get PropertyName[?Yield] ( ) { FunctionBody }
## set PropertyName[?Yield] ( PropertySetParameterList ) { FunctionBody }
## See 14.3
## PropertySetParameterList :
## FormalParameter
## See 14.4
## GeneratorMethod[Yield] :
## * PropertyName[?Yield] ( StrictFormalParameters[Yield] ) { GeneratorBody }
## See 14.4
## GeneratorDeclaration[Yield, Default] :
## function * BindingIdentifier[?Yield] ( FormalParameters[Yield] ) { GeneratorBody }
## [+Default] function * ( FormalParameters[Yield] ) { GeneratorBody }
## See 14.4
## GeneratorExpression :
## function * BindingIdentifier[Yield]opt ( FormalParameters[Yield] ) { GeneratorBody }
## See 14.4
## GeneratorBody :
## FunctionBody[Yield]
## See 14.4
## YieldExpression[In] :
## yield
## yield [no LineTerminator here] AssignmentExpression[?In, Yield]
## yield [no LineTerminator here] * AssignmentExpression[?In, Yield]
## See 14.5
## ClassDeclaration[Yield, Default] :
## class BindingIdentifier[?Yield] ClassTail[?Yield]
## [+Default] class ClassTail[?Yield]
## See 14.5
## ClassExpression[Yield] :
## class BindingIdentifier[?Yield]opt ClassTail[?Yield]
## See 14.5
## ClassTail[Yield] :
## ClassHeritage[?Yield]opt { ClassBody[?Yield]opt }
## See 14.5
## ClassHeritage[Yield] :
## extends LeftHandSideExpression[?Yield]
## See 14.5
## ClassBody[Yield] :
## ClassElementList[?Yield]
## See 14.5
## ClassElementList[Yield] :
## ClassElement[?Yield]
## ClassElementList[?Yield] ClassElement[?Yield]
## See 14.5
## ClassElement[Yield] :
## MethodDefinition[?Yield]
## static MethodDefinition[?Yield]
## ;
