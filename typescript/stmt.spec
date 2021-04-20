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
#
#rule IdentifierReference[Yield] :
#  Identifier
#  [~Yield] yield
#
#rule BindingIdentifier[Yield] :
#
#rule Identifier
#  [~Yield] yield
#
#rule LabelIdentifier[Yield] :
#  Identifier
#  [~Yield] yield
#
#rule Identifier :
#  IdentifierName but not ReservedWord
#
#rule PrimaryExpression[Yield] :
#  this
#  IdentifierReference[?Yield]
#  Literal
#  ArrayLiteral[?Yield]
#  ObjectLiteral[?Yield]
#  FunctionExpression
#  ClassExpression[?Yield]
#  GeneratorExpression
#  RegularExpressionLiteral
#  TemplateLiteral[?Yield]
#  CoverParenthesizedExpressionAndArrowParameterList[?Yield]
#
#rule CoverParenthesizedExpressionAndArrowParameterList[Yield] :
#  ( Expression[In, ?Yield] )
#  ( )
#  ( ... BindingIdentifier[?Yield] )
#  ( Expression[In, ?Yield] , ... BindingIdentifier[?Yield] )
#
#  When processing the production
#
#        PrimaryExpression[Yield] : CoverParenthesizedExpressionAndArrowParameterList[?Yield]
#  the interpretation of CoverParenthesizedExpressionAndArrowParameterList is refined using the following grammar:
#
#rule ParenthesizedExpression[Yield] :
#  ( Expression[In, ?Yield] )
#
#rule Literal :
#  NullLiteral
#  BooleanLiteral
#  NumericLiteral
#  StringLiteral
#
#rule ArrayLiteral[Yield] :
#  [ Elisionopt ]
#  [ ElementList[?Yield] ]
#  [ ElementList[?Yield] , Elisionopt ]
#
#rule ElementList[Yield] :
#  Elisionopt AssignmentExpression[In, ?Yield]
#  Elisionopt SpreadElement[?Yield]
#  ElementList[?Yield] , Elisionopt AssignmentExpression[In, ?Yield]
#  ElementList[?Yield] , Elisionopt SpreadElement[?Yield]
#
#rule Elision :
#  ,
#  Elision ,
#
#rule SpreadElement[Yield] :
#  ... AssignmentExpression[In, ?Yield]
#
#rule ObjectLiteral[Yield] :
#  { }
#  { PropertyDefinitionList[?Yield] }
#  { PropertyDefinitionList[?Yield] , }
#
#rule PropertyDefinitionList[Yield] :
#  PropertyDefinition[?Yield]
#  PropertyDefinitionList[?Yield] , PropertyDefinition[?Yield]
#
#rule PropertyDefinition[Yield] :
#  IdentifierReference[?Yield]
#  CoverInitializedName[?Yield]
#  PropertyName[?Yield] : AssignmentExpression[In, ?Yield]
#  MethodDefinition[?Yield]
#
#rule PropertyName[Yield] :
#  LiteralPropertyName
#  ComputedPropertyName[?Yield]
#
#rule LiteralPropertyName :
#  IdentifierName
#  StringLiteral
#  NumericLiteral
#
#rule ComputedPropertyName[Yield] :
#  [ AssignmentExpression[In, ?Yield] ]
#
#rule CoverInitializedName[Yield] :
#  IdentifierReference[?Yield] Initializer[In, ?Yield]
#
#rule Initializer[In, Yield] :
#  = AssignmentExpression[?In, ?Yield]
#
#rule TemplateLiteral[Yield] :
#  NoSubstitutionTemplate
#  TemplateHead Expression[In, ?Yield] TemplateSpans[?Yield]
#
#rule TemplateSpans[Yield] :
#  TemplateTail
#  TemplateMiddleList[?Yield] TemplateTail
#
#rule TemplateMiddleList[Yield] :
#  TemplateMiddle Expression[In, ?Yield]
#  TemplateMiddleList[?Yield] TemplateMiddle Expression[In, ?Yield]
#
#rule MemberExpression[Yield] :
#  PrimaryExpression[?Yield]
#  MemberExpression[?Yield] [ Expression[In, ?Yield] ]
#  MemberExpression[?Yield] . IdentifierName
#  MemberExpression[?Yield] TemplateLiteral[?Yield]
#  SuperProperty[?Yield]
#  MetaProperty
#  new MemberExpression[?Yield] Arguments[?Yield]
#
#rule SuperProperty[Yield] :
#  super [ Expression[In, ?Yield] ]
#  super . IdentifierName
#
#rule MetaProperty :
#  NewTarget
#
#rule NewTarget :
#  new . target
#
#rule NewExpression[Yield] :
#  MemberExpression[?Yield]
#  new NewExpression[?Yield]
#
#rule CallExpression[Yield] :
#  MemberExpression[?Yield] Arguments[?Yield]
#  SuperCall[?Yield]
#  CallExpression[?Yield] Arguments[?Yield]
#  CallExpression[?Yield] [ Expression[In, ?Yield] ]
#  CallExpression[?Yield] . IdentifierName
#  CallExpression[?Yield] TemplateLiteral[?Yield]
#
#rule SuperCall[Yield] :
#  super Arguments[?Yield]
#
#rule Arguments[Yield] :
#  ( )
#  ( ArgumentList[?Yield] )
#
#rule ArgumentList[Yield] :
#  AssignmentExpression[In, ?Yield]
#  ... AssignmentExpression[In, ?Yield]
#  ArgumentList[?Yield] , AssignmentExpression[In, ?Yield]
#  ArgumentList[?Yield] , ... AssignmentExpression[In, ?Yield]
#
#rule LeftHandSideExpression[Yield] :
#  NewExpression[?Yield]
#  CallExpression[?Yield]
#
#rule PostfixExpression[Yield] :
#  LeftHandSideExpression[?Yield]
#  LeftHandSideExpression[?Yield] [no LineTerminator here] ++
#  LeftHandSideExpression[?Yield] [no LineTerminator here] --
#
#rule UnaryExpression[Yield] :
#  PostfixExpression[?Yield]
#  delete UnaryExpression[?Yield]
#  void UnaryExpression[?Yield]
#  typeof UnaryExpression[?Yield]
#  ++ UnaryExpression[?Yield]
#  -- UnaryExpression[?Yield]
#  + UnaryExpression[?Yield]
#  - UnaryExpression[?Yield]
#  ~ UnaryExpression[?Yield]
#  ! UnaryExpression[?Yield]
#
#rule MultiplicativeExpression[Yield] :
#  UnaryExpression[?Yield]
#  MultiplicativeExpression[?Yield] MultiplicativeOperator UnaryExpression[?Yield]
#
#rule MultiplicativeOperator : one of
#  * / %
#
#rule AdditiveExpression[Yield] :
#  MultiplicativeExpression[?Yield]
#  AdditiveExpression[?Yield] + MultiplicativeExpression[?Yield]
#  AdditiveExpression[?Yield] - MultiplicativeExpression[?Yield]
#
#rule ShiftExpression[Yield] :
#  AdditiveExpression[?Yield]
#  ShiftExpression[?Yield] << AdditiveExpression[?Yield]
#  ShiftExpression[?Yield] >> AdditiveExpression[?Yield]
#  ShiftExpression[?Yield] >>> AdditiveExpression[?Yield]
#
#rule RelationalExpression[In, Yield] :
#  ShiftExpression[?Yield]
#  RelationalExpression[?In, ?Yield] < ShiftExpression[?Yield]
#  RelationalExpression[?In, ?Yield] > ShiftExpression[?Yield]
#  RelationalExpression[?In, ?Yield] <= ShiftExpression[? Yield]
#  RelationalExpression[?In, ?Yield] >= ShiftExpression[?Yield]
#  RelationalExpression[?In, ?Yield] instanceof ShiftExpression[?Yield]
#  [+In] RelationalExpression[In, ?Yield] in ShiftExpression[?Yield]
#
#rule EqualityExpression[In, Yield] :
#  RelationalExpression[?In, ?Yield]
#  EqualityExpression[?In, ?Yield] == RelationalExpression[?In, ?Yield]
#  EqualityExpression[?In, ?Yield] != RelationalExpression[?In, ?Yield]
#  EqualityExpression[?In, ?Yield] === RelationalExpression[?In, ?Yield]
#  EqualityExpression[?In, ?Yield] !== RelationalExpression[?In, ?Yield]
#
#rule BitwiseANDExpression[In, Yield] :
#  EqualityExpression[?In, ?Yield]
#  BitwiseANDExpression[?In, ?Yield] & EqualityExpression[?In, ?Yield]
#
#rule BitwiseXORExpression[In, Yield] :
#  BitwiseANDExpression[?In, ?Yield]
#  BitwiseXORExpression[?In, ?Yield] ^ BitwiseANDExpression[?In, ?Yield]
#
#rule BitwiseORExpression[In, Yield] :
#  BitwiseXORExpression[?In, ?Yield]
#  BitwiseORExpression[?In, ?Yield] | BitwiseXORExpression[?In, ?Yield]
#
#rule LogicalANDExpression[In, Yield] :
#  BitwiseORExpression[?In, ?Yield]
#  LogicalANDExpression[?In, ?Yield] && BitwiseORExpression[?In, ?Yield]
#
#rule LogicalORExpression[In, Yield] :
#  LogicalANDExpression[?In, ?Yield]
#  LogicalORExpression[?In, ?Yield] || LogicalANDExpression[?In, ?Yield]
#
#rule ConditionalExpression[In, Yield] :
#  LogicalORExpression[?In, ?Yield]
#  LogicalORExpression[?In,?Yield] ? AssignmentExpression[In, ?Yield] : AssignmentExpression[?In, ?Yield]
#
#rule AssignmentExpression[In, Yield] :
#  ConditionalExpression[?In, ?Yield]
#  [+Yield] YieldExpression[?In]
#  ArrowFunction[?In, ?Yield]
#  LeftHandSideExpression[?Yield] = AssignmentExpression[?In, ?Yield]
#  LeftHandSideExpression[?Yield] AssignmentOperator AssignmentExpression[?In, ?Yield]
#
#rule AssignmentOperator : one of
#  *= /= %= += -= <<= >>= >>>= &= ^= |=
#
#rule Expression[In, Yield] :
#  AssignmentExpression[?In, ?Yield]
#  Expression[?In, ?Yield] , AssignmentExpression[?In, ?Yield]
#
#-------------------------------------------------------------------------------
#                                    Statements
#-------------------------------------------------------------------------------
#
#rule Statement[Yield, Return] :
#  BlockStatement[?Yield, ?Return]
#  VariableStatement[?Yield]
#  EmptyStatement
#  ExpressionStatement[?Yield]
#  IfStatement[?Yield, ?Return]
#  BreakableStatement[?Yield, ?Return]
#  ContinueStatement[?Yield]
#  BreakStatement[?Yield]
#  [+Return] ReturnStatement[?Yield]
#  WithStatement[?Yield, ?Return]
#  LabelledStatement[?Yield, ?Return]
#  ThrowStatement[?Yield]
#  TryStatement[?Yield, ?Return]
#  DebuggerStatement
#
#rule Declaration[Yield] :
#  HoistableDeclaration[?Yield]
#  ClassDeclaration[?Yield]
#  LexicalDeclaration[In, ?Yield]
#
#rule HoistableDeclaration[Yield, Default] :
#  FunctionDeclaration[?Yield,?Default]
#  GeneratorDeclaration[?Yield, ?Default]
#
#rule BreakableStatement[Yield, Return] :
#  IterationStatement[?Yield, ?Return]
#  SwitchStatement[?Yield, ?Return]
#
#rule BlockStatement[Yield, Return] :
#  Block[?Yield, ?Return]
#
#rule Block[Yield, Return] :
#  { StatementList[?Yield, ?Return]opt }
#
#rule StatementList[Yield, Return] :
#  StatementListItem[?Yield, ?Return]
#  StatementList[?Yield, ?Return] StatementListItem[?Yield, ?Return]
#
#rule StatementListItem[Yield, Return] :
#  Statement[?Yield, ?Return]
#  Declaration[?Yield]
#
#rule LexicalDeclaration[In, Yield] :
#  LetOrConst BindingList[?In, ?Yield] ;
#
#rule LetOrConst :
#  let
#  const
#
#rule BindingList[In, Yield] :
#  LexicalBinding[?In, ?Yield]
#  BindingList[?In, ?Yield] , LexicalBinding[?In, ?Yield]
#
#rule LexicalBinding[In, Yield] :
#  BindingIdentifier[?Yield] Initializer[?In, ?Yield]opt
#  BindingPattern[?Yield] Initializer[?In, ?Yield]
#
#rule VariableStatement[Yield] :
#  var VariableDeclarationList[In, ?Yield] ;
#
#rule VariableDeclarationList[In, Yield] :
#  VariableDeclaration[?In, ?Yield]
#  VariableDeclarationList[?In, ?Yield] , VariableDeclaration[?In, ?Yield]
#
#rule VariableDeclaration[In, Yield] :
#  BindingIdentifier[?Yield] Initializer[?In, ?Yield]opt
#  BindingPattern[?Yield] Initializer[?In, ?Yield]
#
#rule BindingPattern[Yield] :
#  ObjectBindingPattern[?Yield]
#  ArrayBindingPattern[?Yield]
#
#rule ObjectBindingPattern[Yield] :
#  { }
#  { BindingPropertyList[?Yield] }
#  { BindingPropertyList[?Yield] , }
#
#rule ArrayBindingPattern[Yield] :
#  [ Elisionopt BindingRestElement[?Yield]opt ]
#  [ BindingElementList[?Yield] ]
#  [ BindingElementList[?Yield] , Elisionopt BindingRestElement[?Yield]opt ]
#
#rule BindingPropertyList[Yield] :
#  BindingProperty[?Yield]
#  BindingPropertyList[?Yield] , BindingProperty[?Yield]
#
#rule BindingElementList[Yield] :
#  BindingElisionElement[?Yield]
#  BindingElementList[?Yield] , BindingElisionElement[?Yield]
#
#rule BindingElisionElement[Yield] :
#  Elisionopt BindingElement[?Yield]
#
#rule BindingProperty[Yield] :
#  SingleNameBinding[?Yield]
#  PropertyName[?Yield] : BindingElement[?Yield]
#
#rule BindingElement[Yield] :
#  SingleNameBinding[?Yield]
#  BindingPattern[?Yield] Initializer[In, ?Yield]opt
#
#rule SingleNameBinding[Yield] :
#  BindingIdentifier[?Yield] Initializer[In, ?Yield]opt
#
#rule BindingRestElement[Yield] :
#  ... BindingIdentifier[?Yield]
#
#rule EmptyStatement :
#  ;
#
#rule ExpressionStatement[Yield] :
#  [lookahead NotIn {{, function, class, let [}] Expression[In, ?Yield] ;
#
#rule IfStatement[Yield, Return] :
#  if ( Expression[In, ?Yield] ) Statement[?Yield, ?Return] else Statement[?Yield, ?Return]
#  if ( Expression[In, ?Yield] ) Statement[?Yield, ?Return]
#
#rule IterationStatement[Yield, Return] :
#  do Statement[?Yield, ?Return] while ( Expression[In, ?Yield] ) ;
#  while ( Expression[In, ?Yield] ) Statement[?Yield, ?Return]
#  for ( [lookahead NotIn {let [}] Expression[?Yield]opt ; Expression[In, ?Yield]opt ; Expression[In, ?Yield]opt ) Statement[?Yield, ?Return]
#  for ( var VariableDeclarationList[?Yield] ; Expression[In, ?Yield]opt ; Expression[In, ?Yield]opt ) Statement[?Yield, ?Return]
#  for ( LexicalDeclaration[?Yield] Expression[In, ?Yield]opt ; Expression[In, ?Yield]opt ) Statement[?Yield, ?Return]
#  for ( [lookahead NotIn {let [}] LeftHandSideExpression[?Yield] in Expression[In, ?Yield] ) Statement[?Yield, ?Return]
#  for ( var ForBinding[?Yield] in Expression[In, ?Yield] ) Statement[?Yield, ?Return]
#  for ( ForDeclaration[?Yield] in Expression[In, ?Yield] ) Statement[?Yield, ?Return]
#  for ( [lookahead NotEq let ] LeftHandSideExpression[?Yield] of AssignmentExpression[In, ?Yield] ) Statement[?Yield, ?Return]
#  for ( var ForBinding[?Yield] of AssignmentExpression[In, ?Yield] ) Statement[?Yield, ?Return]
#  for ( ForDeclaration[?Yield] of AssignmentExpression[In, ?Yield] ) Statement[?Yield, ?Return]
#
#rule ForDeclaration[Yield] :
#  LetOrConst ForBinding[?Yield]
#
#rule ForBinding[Yield] :
#  BindingIdentifier[?Yield]
#  BindingPattern[?Yield]
#
#rule ContinueStatement[Yield] :
#  continue ;
#  continue [no LineTerminator here] LabelIdentifier[?Yield] ;
#
#rule BreakStatement[Yield] :
#  break ;
#  break [no LineTerminator here] LabelIdentifier[?Yield] ;
#
#rule ReturnStatement[Yield] :
#  return ;
#  return [no LineTerminator here] Expression[In, ?Yield] ;
#
#rule WithStatement[Yield, Return] :
#  with ( Expression[In, ?Yield] ) Statement[?Yield, ?Return]
#
#rule SwitchStatement[Yield, Return] :
#  switch ( Expression[In, ?Yield] ) CaseBlock[?Yield, ?Return]
#
#rule CaseBlock[Yield, Return] :
#  { CaseClauses[?Yield, ?Return]opt }
#  { CaseClauses[?Yield, ?Return]opt DefaultClause[?Yield, ?Return] CaseClauses[?Yield, ?Return]opt }
#
#rule CaseClauses[Yield, Return] :
#  CaseClause[?Yield, ?Return]
#  CaseClauses[?Yield, ?Return] CaseClause[?Yield, ?Return]
#
#rule CaseClause[Yield, Return] :
#  case Expression[In, ?Yield] : StatementList[?Yield, ?Return]opt
#
#rule DefaultClause[Yield, Return] :
#  default : StatementList[?Yield, ?Return]opt
#
#rule LabelledStatement[Yield, Return] :
#  LabelIdentifier[?Yield] : LabelledItem[?Yield, ?Return]
#
#rule LabelledItem[Yield, Return] :
#  Statement[?Yield, ?Return]
#  FunctionDeclaration[?Yield]
#
#rule ThrowStatement[Yield] :
#  throw [no LineTerminator here] Expression[In, ?Yield] ;
#
#rule TryStatement[Yield, Return] :
#  try Block[?Yield, ?Return] Catch[?Yield, ?Return]
#  try Block[?Yield, ?Return] Finally[?Yield, ?Return]
#  try Block[?Yield, ?Return] Catch[?Yield, ?Return] Finally[?Yield, ?Return]
#
#rule Catch[Yield, Return] :
#  catch ( CatchParameter[?Yield] ) Block[?Yield, ?Return]
#
#rule Finally[Yield, Return] :
#  finally Block[?Yield, ?Return]
#
#rule CatchParameter[Yield] :
#  BindingIdentifier[?Yield]
#  BindingPattern[?Yield]
#
#rule DebuggerStatement :
#  debugger ;
