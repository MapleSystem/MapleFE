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
#                                    A.1 Lexical Grammar
#-------------------------------------------------------------------------------

####   Template and TemplateLiteral are too complicated to be described
####   in rules. We handle them specifically in the source code of lexer.

##Template ::
##NoSubstitutionTemplate
##TemplateHead

##NoSubstitutionTemplate ::
##` TemplateCharactersopt `

##TemplateHead ::
##` TemplateCharactersopt ${

##See 11.8.6
##TemplateSubstitutionTail ::
##TemplateMiddle
##TemplateTail
##See 11.8.6
##TemplateMiddle ::
##} TemplateCharactersopt ${
##See 11.8.6
##TemplateTail ::
##} TemplateCharactersopt `

##TemplateCharacters ::
##TemplateCharacter TemplateCharactersopt

##TemplateCharacter ::
##$ [lookahead ≠ { ]
##\ EscapeSequence
##LineContinuation
##LineTerminatorSequence
##SourceCharacter but not one of ` or \ or $ or LineTerminator

#-------------------------------------------------------------------------------
#                                    A.2 Expressions
#-------------------------------------------------------------------------------

rule KeywordIdentifier : ONEOF("get",
                               "set",
                               "type",
                               "boolean",
                               "string",
                               "catch",
                               "undefined",
                               "never",
                               "number",
                               "symbol",
                               "unique",
                               "any",
                               "constructor",
                               "delete",
                               "abstract",
                               "as",
                               "async",
                               "await",
                               "finally",
                               "from",
                               "is",
                               "in",
                               "of",
                               "declare",
                               "readonly",
                               "debugger",
                               "default",
                               "namespace",
                               "module",
                               "extends",
                               "switch",
                               "infer",
                               "asserts",
                               "require",
                               "import",
                               "for")
## "
  attr.action : BuildIdentifier()

rule JSIdentifier: ONEOF(Identifier,
                         KeywordIdentifier,
                         Identifier + '!',
                         Identifier + '?',
                         KeywordIdentifier + '?')
  attr.action.%3 : SetIsNonNull(%1)
  attr.action.%4 : SetIsOptional(%1)
  attr.action.%5 : SetIsOptional(%1)

rule AsType : "as" + Type
  attr.action : BuildAsType(%2)

##-----------------------------------
##rule IdentifierReference[Yield] :
##  Identifier
##  [~Yield] yield

rule IdentifierReference : ONEOF(
  JSIdentifier,
  "yield")

##-----------------------------------
##rule BindingIdentifier[Yield] :
##  Identifier
##  [~Yield] yield

rule BindingIdentifier : ONEOF(JSIdentifier, "yield")

##-----------------------------------
##rule LabelIdentifier[Yield] :
##  Identifier
##  [~Yield] yield
rule LabelIdentifier : ONEOF(
  JSIdentifier,
  "yield")

##-----------------------------------
##rule Identifier :
##  IdentifierName but not ReservedWord
##
## Identifier and IdentifierName are tricky in Javascript.
## (1) Some 'keywords' like 'get', 'set', should be keyword and reserved,
##     however, they are allowed as identifiers.
## (2) Identifier is a reserved rule in 'autogen', we won't define it
##     in this spec.
##
## I decided to use JSIdentifier instead of Identifier and then I
## can include 'get', 'set' in the JSIdentifier.

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

rule PrimaryExpression : ONEOF("this",
                               "super",
                               IdentifierReference,
                               Literal,
                               ArrayLiteral,
                               ObjectLiteral,
                               FunctionExpression,
#                              ClassExpression[?Yield]
#                              GeneratorExpression
                               RegularExpression,
                               TemplateLiteral,
                               ParenthesizedExpression,
                               Literal + "as" + "const",
                               ArrayLiteral + "as" + "const",
                               ObjectLiteral + "as" + "const")
  attr.action.%11,%12,%13 : SetIsConst(%1)
  attr.action.%11,%12,%13 : PassChild(%1)

##-----------------------------------
##rule CoverParenthesizedExpressionAndArrowParameterList[Yield] :
##  ( Expression[In, ?Yield] )
##  ( )
##  ( ... BindingIdentifier[?Yield] )
##  ( Expression[In, ?Yield] , ... BindingIdentifier[?Yield] )
##  When processing the production
##        PrimaryExpression[Yield] : CoverParenthesizedExpressionAndArrowParameterList[?Yield]
##  the interpretation of CoverParenthesizedExpressionAndArrowParameterList is refined using the following grammar:
##  ParenthesizedExpression[Yield] :
##  ( Expression[In, ?Yield] )
rule ParenthesizedExpression : '(' + Expression + ')'

rule CoverParenthesizedExpressionAndArrowParameterList : ONEOF(
  '(' + Expression + ')',
  '(' + ')',
  '(' + "..." + BindingIdentifier + ')',
  '(' + "..." + BindingPattern + ')',
  '(' + Expression + ',' + "..." + BindingIdentifier + ')',
  '(' + Expression + ',' + "..." + BindingPattern + ')')

##-----------------------------------

##-----------------------------------
##rule Literal :
##  NullLiteral
##  BooleanLiteral
##  NumericLiteral
##  StringLiteral

# Literal is handled in lexer as a token. Also we don't do any Lookahead detect or recursion detect
# inside Literal. So it means parsing stops at Literal. This works for most languages.
#
# NullLiteral, BooleanLiteral, NumericLiteral, StringLiteral can be handled specifically in parser
# to see if it's a string literal.

##-----------------------------------
##rule ArrayLiteral[Yield] :
##  [ Elisionopt ]
##  [ ElementList[?Yield] ]
##  [ ElementList[?Yield] , Elisionopt ]
rule ArrayLiteral : ONEOF(
  '[' + ZEROORONE(Elision) + ']'
  '[' + ElementList + ']'
  '[' + ElementList + ',' + ZEROORONE(Elision) + ']')
  attr.action.%1,%2,%3 : BuildArrayLiteral(%2)

##-----------------------------------
##rule ElementList[Yield] :
##  Elisionopt AssignmentExpression[In, ?Yield]
##  Elisionopt SpreadElement[?Yield]
##  ElementList[?Yield] , Elisionopt AssignmentExpression[In, ?Yield]
##  ElementList[?Yield] , Elisionopt SpreadElement[?Yield]
rule ElementList : ONEOF(
  ZEROORONE(Elision) + AssignmentExpression,
  ZEROORONE(Elision) + SpreadElement,
  ElementList + ',' + ZEROORONE(Elision) + AssignmentExpression,
  ElementList + ',' + ZEROORONE(Elision) + SpreadElement)
  attr.action.%1,%2 : PassChild(%2)
  attr.action.%3,%4 : BuildExprList(%1, %4)

##-----------------------------------
##rule Elision :
##  ,
##  Elision ,

rule Elision : ONEOF(',',
                     Elision + ',')

##-----------------------------------
##rule SpreadElement[Yield] :
##  ... AssignmentExpression[In, ?Yield]
rule SpreadElement : "..." + AssignmentExpression
  attr.action : SetIsRest(%2)

##-----------------------------------
##rule ObjectLiteral[Yield] :
##  { }
##  { PropertyDefinitionList[?Yield] }
##  { PropertyDefinitionList[?Yield] , }
rule ObjectLiteral : ONEOF('{' + '}',
                           '{' + PropertyDefinitionList + '}',
                           '{' + PropertyDefinitionList + ',' + '}')
  attr.action.%1    : BuildStructLiteral()
  attr.action.%2,%3 : BuildStructLiteral(%2)

##-----------------------------------
##rule PropertyDefinitionList[Yield] :
##  PropertyDefinition[?Yield]
##  PropertyDefinitionList[?Yield] , PropertyDefinition[?Yield]
rule PropertyDefinitionList : ONEOF(
  PropertyDefinition,
  PropertyDefinitionList + ',' + PropertyDefinition)

##-----------------------------------
# modified in 2016
##rule PropertyDefinition[Yield] :
##  IdentifierReference[?Yield]
##  CoverInitializedName[?Yield]
##  PropertyName[?Yield] : AssignmentExpression[In, ?Yield]
##  MethodDefinition[?Yield]

##-----------------------------------
##rule PropertyName[Yield] :
##  LiteralPropertyName
##  ComputedPropertyName[?Yield]
rule PropertyName : ONEOF(LiteralPropertyName,
                          ComputedPropertyName)

##-----------------------------------
##rule LiteralPropertyName :
##  IdentifierName
##  StringLiteral
##  NumericLiteral

# I used Identifier instead of IdentifierName because keywords
# are processed before Identifier, and so Identifier is the same
# as IdentifierName here.
#
# I extend StringLiteral/NumericLiteral to Literal. 'tsc' will
# make sure it's legal and I don't need worry about it.

rule LiteralPropertyName : ONEOF(JSIdentifier, Literal)

##-----------------------------------
##rule ComputedPropertyName[Yield] :
##  [ AssignmentExpression[In, ?Yield] ]
rule ComputedPropertyName : ONEOF('[' + AssignmentExpression + ']',
                                  '-' + "readonly" + '[' + AssignmentExpression + ']')
  attr.action.%1 : BuildComputedName(%2)
  attr.action.%2 : BuildComputedName(%4)

##-----------------------------------
##rule CoverInitializedName[Yield] :
##  IdentifierReference[?Yield] Initializer[In, ?Yield]
rule CoverInitializedName : IdentifierReference + Initializer

##-----------------------------------
##rule Initializer[In, Yield] :
##  = AssignmentExpression[?In, ?Yield]
rule Initializer : '=' + AssignmentExpression

##-----------------------------------
##rule TemplateLiteral[Yield] :
##  NoSubstitutionTemplate
##  TemplateHead Expression[In, ?Yield] TemplateSpans[?Yield]
##
## NOTE: TemplateLiteral will be handled specifically in lexer code.
## rule TemplateLiteral : "this_is_for_fake_rule" is defined in reserved.spec

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
  PrimaryExpression + ZEROORMORE(AsType),
  MemberExpression + '[' + Expression + ']' + ZEROORMORE(AsType),
  MemberExpression + '.' + JSIdentifier + ZEROORMORE(AsType),
  MemberExpression + "?." + JSIdentifier + ZEROORMORE(AsType),
  MemberExpression + TemplateLiteral,
#  SuperProperty[?Yield]
#  MetaProperty
  "new" + MemberExpression + ZEROORONE(Arguments),
# NOTE: I created this rule. Typescript extended Type system and allow 'new'
#       on a TypeReference
  "new" + TypeReference + ZEROORONE(Arguments),
  MemberExpression + "?." + '[' + Expression + ']' + ZEROORMORE(AsType),
  IsExpression,
  MemberExpression + '[' + KeyOf + ']',
  MemberExpression + '!',
  MemberExpression + '.' + JSIdentifier + "as" + "const",
  MemberExpression + '.' + "return",
  '<' + Type + '>' + PrimaryExpression)
  attr.action.%1 : AddAsType(%1, %2)
  attr.action.%2 : BuildArrayElement(%1, %3)
  attr.action.%2 : AddAsType(%5)
  attr.action.%3 : BuildField(%1, %3)
  attr.action.%3 : AddAsType(%4)
  attr.action.%4 : SetIsOptional(%1)
  attr.action.%4 : BuildField(%1, %3)
  attr.action.%4 : AddAsType(%4)
  attr.action.%6,%7 : BuildNewOperation(%2, %3)
  attr.action.%8 : SetIsOptional(%1)
  attr.action.%8 : BuildArrayElement(%1, %4)
  attr.action.%8 : AddAsType(%6)
  attr.action.%10: BuildArrayElement(%1, %3)
  attr.action.%11: SetIsNonNull(%1)
  attr.action.%12: BuildField(%1, %3)
  attr.action.%12: SetIsConst()
  attr.action.%13: BuildField(%1, %3)
  attr.action.%14: BuildCast(%2, %4)

rule IsExpression: ONEOF(PrimaryExpression + "is" + Type,
                         ArrowFunction + "is" + Type)
  attr.action.%1,%2 : BuildIs(%1, %3)

rule AssertExpression : "asserts" + MemberExpression
  attr.action : BuildAssert(%2)

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
rule NewExpression : ONEOF(MemberExpression,
                           "new" + NewExpression)
  attr.action.%2 : BuildNewOperation(%2)

##-----------------------------------
##rule CallExpression[Yield] :
##  MemberExpression[?Yield] Arguments[?Yield]
##  SuperCall[?Yield]
##  CallExpression[?Yield] Arguments[?Yield]
##  CallExpression[?Yield] [ Expression[In, ?Yield] ]
##  CallExpression[?Yield] . IdentifierName
##  CallExpression[?Yield] TemplateLiteral[?Yield]

rule CallExpression : ONEOF(
  MemberExpression + ZEROORONE(TypeArguments) + Arguments + ZEROORMORE(AsType),
  SuperCall,
  CallExpression + Arguments + ZEROORMORE(AsType),
  CallExpression + '[' + Expression + ']',
  CallExpression + '.' + JSIdentifier + ZEROORMORE(AsType),
  CallExpression + TemplateLiteral,
  CallExpression + '!' + ZEROORMORE(AsType),
  CallExpression + "?." + JSIdentifier + ZEROORMORE(AsType),
  MemberExpression + "?." + ZEROORONE(TypeArguments) + Arguments + ZEROORMORE(AsType))
  attr.action.%1,%3 : BuildCall(%1)
  attr.action.%1 : AddAsType(%4)
  attr.action.%1 : AddTypeGenerics(%2)
  attr.action.%1 : AddArguments(%3)
  attr.action.%3 : AddArguments(%2)
  attr.action.%3 : AddAsType(%3)
  attr.action.%4 : BuildArrayElement(%1, %3)
  attr.action.%5 : BuildField(%1, %3)
  attr.action.%5 : AddAsType(%4)
  attr.action.%7 : SetIsNonNull(%1)
  attr.action.%7 : AddAsType(%1, %3)
  attr.action.%8 : SetIsOptional(%1)
  attr.action.%8 : BuildField(%1, %3)
  attr.action.%8 : AddAsType(%4)
  attr.action.%9 : SetIsOptional(%1)
  attr.action.%9 : BuildCall(%1)
  attr.action.%9 : AddTypeGenerics(%3)
  attr.action.%9 : AddArguments(%4)
  attr.action.%9 : AddAsType(%5)

##-----------------------------------
##rule SuperCall[Yield] :
##  super Arguments[?Yield]
rule SuperCall : "super" + Arguments

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

## child #3, I added ZEROORONE() since ECMAScript 2017 allows empty argument after ','.
rule ArgumentList : ONEOF(AssignmentExpression,
                          "..." + AssignmentExpression,
                          ArgumentList + ',' + ZEROORONE(AssignmentExpression),
                          ArgumentList + ',' + "..." + AssignmentExpression)
  attr.action.%2 : SetIsRest(%2)
  attr.action.%4 : SetIsRest(%4)

##-----------------------------------
##rule LeftHandSideExpression[Yield] :
##  NewExpression[?Yield]
##  CallExpression[?Yield]

rule LeftHandSideExpression : ONEOF(NewExpression,
                                    CallExpression,
                                    "..." + NewExpression,
                                    "..." + CallExpression)
  attr.action.%3,%4 : SetIsRest(%2)

##-----------------------------------
##rule PostfixExpression[Yield] :
##  LeftHandSideExpression[?Yield]
##  LeftHandSideExpression[?Yield] [no LineTerminator here] ++
##  LeftHandSideExpression[?Yield] [no LineTerminator here] --

rule PostfixExpression : ONEOF(
  LeftHandSideExpression,
  LeftHandSideExpression + "++",
  LeftHandSideExpression + "--")
  attr.action.%2,%3 : BuildPostfixOperation(%2, %1)

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
  PostfixExpression,
  "delete" + UnaryExpression,
  "void" + UnaryExpression,
  "typeof" + UnaryExpression,
  "++" + UnaryExpression,
   "--" + UnaryExpression,
   '+' + UnaryExpression,
   '-' + UnaryExpression,
   '~' + UnaryExpression,
   '!' + UnaryExpression)
  attr.action.%2 : BuildDeleteOperation(%2)
  attr.action.%3 : BuildLiteral(%1)
  attr.action.%4 : BuildTypeOf(%2)
  attr.action.%5,%6,%7,%8,%9,%10 : BuildUnaryOperation(%1, %2)

## UpdateExpression[Yield]:
## LeftHandSideExpression[?Yield]
## LeftHandSideExpression[?Yield][no LineTerminator here]++
## LeftHandSideExpression[?Yield][no LineTerminator here]--
## ++UnaryExpression[?Yield]
## --UnaryExpression[?Yield]
rule UpdateExpression : ONEOF(LeftHandSideExpression,
                              LeftHandSideExpression + "++",
                              LeftHandSideExpression + "--",
                              "++" + UnaryExpression,
                              "--" + UnaryExpression)
  attr.action.%2,%3 : BuildPostfixOperation(%2, %1)
  attr.action.%4,%5: BuildUnaryOperation(%1, %2)


## Added in 2016
## ExponentiationExpression[Yield]:
## UnaryExpression[?Yield]
## UpdateExpression[?Yield]**ExponentiationExpression[?Yield]
rule ExponentiationExpression : ONEOF(UnaryExpression,
                                      UpdateExpression + "**" + ExponentiationExpression)
  attr.action.%2 : BuildBinaryOperation(%1, %2, %3)

##-----------------------------------
##rule MultiplicativeExpression[Yield] :
##  UnaryExpression[?Yield]
##  MultiplicativeExpression[?Yield] MultiplicativeOperator UnaryExpression[?Yield]
## 2016
## MultiplicativeExpression[Yield]:
## ExponentiationExpression[?Yield]
## MultiplicativeExpression[?Yield]MultiplicativeOperatorExponentiationExpression[?Yield]

rule MultiplicativeExpression : ONEOF(
  ExponentiationExpression,
  MultiplicativeExpression + MultiplicativeOperator + ExponentiationExpression)
  attr.action.%2 : BuildBinaryOperation(%1, %2, %3)

##-----------------------------------
##rule MultiplicativeOperator : one of
##  * / %

rule MultiplicativeOperator : ONEOF( '*', '/', '%')

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

rule InExpression : RelationalExpression + "in" + ONEOF(ShiftExpression, Type)
## "
  attr.action : BuildIn(%1, %3)

rule RelationalExpression : ONEOF(ShiftExpression,
                                  RelationalExpression + '<' + ShiftExpression,
                                  RelationalExpression + '>' + ShiftExpression,
                                  RelationalExpression + "<=" + ShiftExpression,
                                  RelationalExpression + ">=" + ShiftExpression,
                                  RelationalExpression + "instanceof" + ShiftExpression,
                                  InExpression)
  attr.action.%2,%3,%4,%5 : BuildBinaryOperation(%1, %2, %3)
  attr.action.%6 : BuildInstanceOf(%1, %3)


##-----------------------------------
##rule EqualityExpression[In, Yield] :
##  RelationalExpression[?In, ?Yield]
##  EqualityExpression[?In, ?Yield] == RelationalExpression[?In, ?Yield]
##  EqualityExpression[?In, ?Yield] != RelationalExpression[?In, ?Yield]
##  EqualityExpression[?In, ?Yield] === RelationalExpression[?In, ?Yield]
##  EqualityExpression[?In, ?Yield] !== RelationalExpression[?In, ?Yield]

rule EqualityExpression : ONEOF(
  RelationalExpression,
  EqualityExpression + "==" + RelationalExpression,
  EqualityExpression + "!=" + RelationalExpression,
  EqualityExpression + "===" + RelationalExpression,
  EqualityExpression + "!==" + RelationalExpression)
  attr.action.%2,%3,%4,%5 : BuildBinaryOperation(%1, %2, %3)

##-----------------------------------
##rule BitwiseANDExpression[In, Yield] :
##  EqualityExpression[?In, ?Yield]
##  BitwiseANDExpression[?In, ?Yield] & EqualityExpression[?In, ?Yield]

rule BitwiseANDExpression : ONEOF(
  EqualityExpression,
  BitwiseANDExpression + '&' + EqualityExpression)
  attr.action.%2 : BuildBinaryOperation(%1, %2, %3)

##-----------------------------------
##rule BitwiseXORExpression[In, Yield] :
##  BitwiseANDExpression[?In, ?Yield]
##  BitwiseXORExpression[?In, ?Yield] ^ BitwiseANDExpression[?In, ?Yield]

rule BitwiseXORExpression : ONEOF(
  BitwiseANDExpression,
  BitwiseXORExpression + '^' + BitwiseANDExpression)
  attr.action.%2 : BuildBinaryOperation(%1, %2, %3)

##-----------------------------------
##rule BitwiseORExpression[In, Yield] :
##  BitwiseXORExpression[?In, ?Yield]
##  BitwiseORExpression[?In, ?Yield] | BitwiseXORExpression[?In, ?Yield]

rule BitwiseORExpression : ONEOF(
  BitwiseXORExpression,
  BitwiseORExpression + '|' + BitwiseXORExpression)
  attr.action.%2 : BuildBinaryOperation(%1, %2, %3)

##-----------------------------------
##rule LogicalANDExpression[In, Yield] :
##  BitwiseORExpression[?In, ?Yield]
##  LogicalANDExpression[?In, ?Yield] && BitwiseORExpression[?In, ?Yield]

rule LogicalANDExpression : ONEOF(
  BitwiseORExpression,
  LogicalANDExpression + "&&" + BitwiseORExpression)
  attr.action.%2 : BuildBinaryOperation(%1, %2, %3)

##-----------------------------------
##rule LogicalORExpression[In, Yield] :
##  LogicalANDExpression[?In, ?Yield]
##  LogicalORExpression[?In, ?Yield] || LogicalANDExpression[?In, ?Yield]

rule LogicalORExpression : ONEOF(
  LogicalANDExpression,
  LogicalORExpression + "||" + LogicalANDExpression)
  attr.action.%2 : BuildBinaryOperation(%1, %2, %3)

##-----------------------------------
##rule ConditionalExpression[In, Yield] :
##  LogicalORExpression[?In, ?Yield]
##  LogicalORExpression[?In,?Yield] ? AssignmentExpression[In, ?Yield] : AssignmentExpression[?In, ?Yield]
rule ConditionalExpression : ONEOF(
  LogicalORExpression,
  LogicalORExpression + '?' + AssignmentExpression + ':' + AssignmentExpression,
  ConditionalExpression + "??" + ConditionalExpression)
  attr.action.%2 : BuildTernaryOperation(%1, %3, %5)
  attr.action.%3 : BuildBinaryOperation(%1, %2, %3)

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
  ArrowFunction,
  LeftHandSideExpression + '=' + AssignmentExpression,
  LeftHandSideExpression + AssignmentOperator + AssignmentExpression)
  attr.action.%3,%4 : BuildAssignment(%1, %2, %3)

rule AssignmentOperator : ONEOF("*=", "/=", "%=", "+=", "-=", "<<=", ">>=", ">>>=", "&=", "^=", "|=", "??=")

##-----------------------------------
##rule Expression[In, Yield] :
##  AssignmentExpression[?In, ?Yield]
##  Expression[?In, ?Yield] , AssignmentExpression[?In, ?Yield]

## NOTE. I added "undefined" to expression because "undefined" is both a type and
##       a value in Typescript. This is a weird rule.
rule Expression : ONEOF(
  AssignmentExpression,
  Expression + ',' + AssignmentExpression,
  "undefined")

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
  EmptyStatement,
  ExpressionStatement,
  IfStatement,
  BreakableStatement,
  ContinueStatement,
  BreakStatement,
  ReturnStatement,
#  WithStatement[?Yield, ?Return]
  LabelledStatement,
  ThrowStatement,
  TryStatement)
#  DebuggerStatement
  attr.property : Top

##-----------------------------------
##rule Declaration[Yield] :
##  HoistableDeclaration[?Yield]
##  ClassDeclaration[?Yield]
##  LexicalDeclaration[In, ?Yield]

## NOTE. Typescript added InterfaceDeclaration, TypeAliasDeclaration, EnumDeclaration
rule Declaration : ONEOF(HoistableDeclaration,
                         ClassDeclaration,
                         LexicalDeclaration + ZEROORONE(';'),
                         InterfaceDeclaration,
                         TypeAliasDeclaration,
                         EnumDeclaration,
                         NamespaceDeclaration,
                         ExternalDeclaration)
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
rule BreakableStatement : ONEOF(IterationStatement,
                                SwitchStatement)

##-----------------------------------
##rule BlockStatement[Yield, Return] :
##  Block[?Yield, ?Return]
rule BlockStatement : Block

##-----------------------------------
##rule Block[Yield, Return] :
##  { StatementList[?Yield, ?Return]opt }
rule Block : '{' + ZEROORONE(StatementList) + '}'
  attr.action : BuildBlock(%2)

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
rule LexicalDeclaration : ONEOF("let" + BindingList,
                                "const" + BindingList)
  attr.action.%1,%2 : BuildDecl(%2)
  attr.action.%1    : SetJSLet()
  attr.action.%2    : SetJSConst()

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
                            BindingIdentifier + ":" + Type + ZEROORONE(Initializer),
                            BindingPattern    + ZEROORONE(Initializer),
                            BindingPattern    + ":" + Type + ZEROORONE(Initializer))
  attr.action.%1,%3 : AddInitTo(%1, %2)
  attr.action.%2,%4 : AddInitTo(%1, %4)
  attr.action.%2,%4 : AddType(%1, %3)

##-----------------------------------
##rule VariableStatement[Yield] :
##  var VariableDeclarationList[In, ?Yield] ;
rule VariableStatement : "var" + VariableDeclarationList + ZEROORONE(';')
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
rule VariableDeclaration : ONEOF(BindingIdentifier + ':' + Type + ZEROORONE(Initializer),
                                 BindingIdentifier + ZEROORONE(Initializer),
                                 BindingPattern + ZEROORONE(TypeAnnotation) + Initializer)
  attr.action.%1 : AddInitTo(%1, %4)
  attr.action.%1 : BuildDecl(%3, %1)
  attr.action.%1 : SetJSVar()
  attr.action.%2 : AddInitTo(%1, %2)
  attr.action.%2 : BuildDecl(%1)
  attr.action.%2 : SetJSVar()
  attr.action.%3 : AddInitTo(%1, %3)
  attr.action.%3 : BuildDecl(%2, %1)
  attr.action.%3 : SetJSVar()

##-----------------------------------
##rule BindingPattern[Yield] :
##  ObjectBindingPattern[?Yield]
##  ArrayBindingPattern[?Yield]
rule BindingPattern : ONEOF(ObjectBindingPattern, ArrayBindingPattern)

##-----------------------------------
##rule ObjectBindingPattern[Yield] :
##  { }
##  { BindingPropertyList[?Yield] }
##  { BindingPropertyList[?Yield] , }
rule ObjectBindingPattern : ONEOF('{' + '}',
                                  '{' + BindingPropertyList + '}',
                                  '{' + BindingPropertyList + ',' + '}')
  attr.action.%1 :    BuildBindingPattern()
  attr.action.%2,%3 : BuildBindingPattern(%2)
  attr.action.%1,%2,%3 : SetObjectBinding()

##-----------------------------------
##rule ArrayBindingPattern[Yield] :
##  [ Elisionopt BindingRestElement[?Yield]opt ]
##  [ BindingElementList[?Yield] ]
##  [ BindingElementList[?Yield] , Elisionopt BindingRestElement[?Yield]opt ]
rule ArrayBindingPattern : ONEOF(
  '[' + ZEROORONE(Elision) + ZEROORONE(BindingRestElement) + ']',
  '[' + BindingElementList + ']',
  '[' + BindingElementList + ',' + ZEROORONE(Elision) + ZEROORONE(BindingRestElement) + ']')
  attr.action.%1 :    BuildBindingPattern(%3)
  attr.action.%2,%3 : BuildBindingPattern(%2)
  attr.action.%1,%2,%3 : SetArrayBinding()

##-----------------------------------
##rule BindingPropertyList[Yield] :
##  BindingProperty[?Yield]
##  BindingPropertyList[?Yield] , BindingProperty[?Yield]
rule BindingPropertyList : ONEOF(BindingProperty,
                                 BindingPropertyList + ',' + BindingProperty)

##-----------------------------------
##rule BindingElementList[Yield] :
##  BindingElisionElement[?Yield]
##  BindingElementList[?Yield] , BindingElisionElement[?Yield]
rule BindingElementList : ONEOF(
  BindingElisionElement,
  BindingElementList + ',' + BindingElisionElement)

##-----------------------------------
##rule BindingElisionElement[Yield] :
##  Elisionopt BindingElement[?Yield]
rule BindingElisionElement : ZEROORONE(Elision) + BindingElement
  attr.action : PassChild(%2)

##-----------------------------------
##rule BindingProperty[Yield] :
##  SingleNameBinding[?Yield]
##  PropertyName[?Yield] : BindingElement[?Yield]
rule BindingProperty : ONEOF(SingleNameBinding,
                             PropertyName + ':' + BindingElement)
  attr.action.%2 : BuildBindingElement(%1, %3)

##-----------------------------------
##rule BindingElement[Yield] :
##  SingleNameBinding[?Yield]
##  BindingPattern[?Yield] Initializer[In, ?Yield]opt
rule BindingElement : ONEOF(SingleNameBinding,
                            BindingPattern + ZEROORONE(Initializer))
  attr.action.%2 : AddInitTo(%1, %2)

##-----------------------------------
##rule SingleNameBinding[Yield] :
##  BindingIdentifier[?Yield] Initializer[In, ?Yield]opt
rule SingleNameBinding : BindingIdentifier + ZEROORONE(Initializer)
  attr.action : AddInitTo(%1, %2)
  attr.action : BuildBindingElement(%1)

##-----------------------------------
##rule BindingRestElement[Yield] :
##  ... BindingIdentifier[?Yield]
rule BindingRestElement : "..." + BindingIdentifier
  attr.action : SetIsRest(%2)
  attr.action : BuildBindingElement(%2)

##-----------------------------------
##rule EmptyStatement :
##  ;
rule EmptyStatement : ';'

##-----------------------------------
##rule ExpressionStatement[Yield] :
##  [lookahead NotIn {{, function, class, let [}] Expression[In, ?Yield] ;

rule ExpressionStatement : ONEOF(
  ConditionalExpression + ';',
#  [+Yield] YieldExpression[?In]
  ArrowFunction + ';',
  LeftHandSideExpression + '=' + AssignmentExpression + ZEROORONE(';'),
  LeftHandSideExpression + AssignmentOperator + AssignmentExpression + ZEROORONE(';'),
  Expression + ',' + AssignmentExpression + ';',
  "undefined" + ';')
  attr.action.%3,%4 : BuildAssignment(%1, %2, %3)

##-----------------------------------
##rule IfStatement[Yield, Return] :
##  if ( Expression[In, ?Yield] ) Statement[?Yield, ?Return] else Statement[?Yield, ?Return]
##  if ( Expression[In, ?Yield] ) Statement[?Yield, ?Return]
rule IfStatement : ONEOF(
  "if" + '(' + Expression + ')' + Statement + "else" + Statement,
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
rule IterationStatement : ONEOF(
  "do" + Statement + "while" + '(' + Expression + ')' + ';',
  "while" + '(' + Expression + ')' + Statement,
  "for" + '(' + ZEROORONE(Expression) + ';' + ZEROORONE(Expression) + ';' + ZEROORONE(Expression) + ')' + Statement,
  "for" + '(' + "var" + VariableDeclarationList + ';' + ZEROORONE(Expression) + ';' + ZEROORONE(Expression) + ')' + Statement,
  "for" + '(' + LexicalDeclaration + ';' + ZEROORONE(Expression) + ';' + ZEROORONE(Expression) + ')' + Statement,
  "for" + '(' + LeftHandSideExpression + "in" + Expression + ')' + Statement,
  "for" + '(' + "var" + ForBinding + "in" + Expression + ')' + Statement,
  "for" + '(' + ForDeclaration + "in" + Expression + ')' + Statement,
  "for" + '(' + LeftHandSideExpression + "of" + AssignmentExpression + ')' + Statement,
  "for" + '(' + "var" + ForBinding + "of" + AssignmentExpression + ')' + Statement,
  "for" + '(' + "let" + ForBinding + "of" + AssignmentExpression + ')' + Statement,
  "for" + '(' + "const" + ForBinding + "of" + AssignmentExpression + ')' + Statement,
  )
  attr.action.%1 : BuildDoLoop(%5, %2)
  attr.action.%2 : BuildWhileLoop(%3, %5)
  attr.action.%3 : BuildForLoop(%3, %5, %7, %9)
  attr.action.%4 : BuildForLoop(%4, %6, %8, %10)
  attr.action.%5 : BuildForLoop(%3, %5, %7, %9)

  attr.action.%7,%10 : BuildDecl(%4)
  attr.action.%7,%10 : SetJSVar()
  attr.action.%7 :    BuildForLoop_In(%6, %8)
  attr.action.%6,%8 : BuildForLoop_In(%3, %5, %7)
  attr.action.%9    : BuildForLoop_Of(%3, %5, %7)
  attr.action.%10 :   BuildForLoop_Of(%6, %8)

  attr.action.%11,%12 : BuildDecl(%4)
  attr.action.%11 : SetJSLet()
  attr.action.%12 : SetJSConst()
  attr.action.%11,%12 : BuildForLoop_Of(%6, %8)

##-----------------------------------
##rule ForDeclaration[Yield] :
##  LetOrConst ForBinding[?Yield]
rule ForDeclaration : ONEOF("let" + ForBinding,
                            "const" + ForBinding)
  attr.action.%1,%2 : BuildDecl(%2)
  attr.action.%1    : SetJSLet()
  attr.action.%2    : SetJSConst()

##-----------------------------------
##rule ForBinding[Yield] :
##  BindingIdentifier[?Yield]
##  BindingPattern[?Yield]
rule ForBinding : ONEOF(BindingIdentifier,
                        BindingPattern)

##-----------------------------------
##rule ContinueStatement[Yield] :
##  continue ;
##  continue [no LineTerminator here] LabelIdentifier[?Yield] ;
rule ContinueStatement : ONEOF(
  "continue" + ZEROORONE(';'),
  "continue" + LabelIdentifier + ZEROORONE(';'))
  attr.action.%1 : BuildContinue()
  attr.action.%2 : BuildContinue(%2)

##-----------------------------------
##rule BreakStatement[Yield] :
##  break ;
##  break [no LineTerminator here] LabelIdentifier[?Yield] ;
rule BreakStatement : ONEOF(
  "break" + ZEROORONE(';'),
  "break" + LabelIdentifier + ZEROORONE(';'))
  attr.action.%1 : BuildBreak()
  attr.action.%2 : BuildBreak(%2)

##-----------------------------------
##rule ReturnStatement[Yield] :
##  return ;
##  return [no LineTerminator here] Expression[In, ?Yield] ;
rule ReturnStatement :ONEOF("return" + ZEROORONE(';'),
                            "return" + Expression + ZEROORONE(';'))
  attr.action.%1 : BuildReturn()
  attr.action.%2 : BuildReturn(%2)

##-----------------------------------
##rule WithStatement[Yield, Return] :
##  with ( Expression[In, ?Yield] ) Statement[?Yield, ?Return]

##-----------------------------------
##rule SwitchStatement[Yield, Return] :
##  switch ( Expression[In, ?Yield] ) CaseBlock[?Yield, ?Return]
rule SwitchStatement :
  "switch" + '(' + Expression + ')' + CaseBlock
  attr.action : BuildSwitch(%3, %5)

##-----------------------------------
##rule CaseBlock[Yield, Return] :
##  { CaseClauses[?Yield, ?Return]opt }
##  { CaseClauses[?Yield, ?Return]opt DefaultClause[?Yield, ?Return] CaseClauses[?Yield, ?Return]opt }
rule CaseBlock : ONEOF(
  '{' + ZEROORONE(CaseClauses) + '}',
  '{' + ZEROORONE(CaseClauses) + DefaultClause + ZEROORONE(CaseClauses) + '}')

##-----------------------------------
##rule CaseClauses[Yield, Return] :
##  CaseClause[?Yield, ?Return]
##  CaseClauses[?Yield, ?Return] CaseClause[?Yield, ?Return]
rule CaseClauses : ONEOF(
  CaseClause,
  CaseClauses + CaseClause)

##-----------------------------------
##rule CaseClause[Yield, Return] :
##  case Expression[In, ?Yield] : StatementList[?Yield, ?Return]opt
rule CaseClause :
  "case" + Expression + ':' + ZEROORONE(StatementList)
  attr.action : BuildSwitchLabel(%2)
  attr.action : BuildOneCase(%4)

##-----------------------------------
##rule DefaultClause[Yield, Return] :
##  default : StatementList[?Yield, ?Return]opt
rule DefaultClause :
  "default" + ':' + ZEROORONE(StatementList)
  attr.action : BuildDefaultSwitchLabel()
  attr.action : BuildOneCase(%3)

##-----------------------------------
##rule LabelledStatement[Yield, Return] :
##  LabelIdentifier[?Yield] : LabelledItem[?Yield, ?Return]
rule LabelledStatement :
  LabelIdentifier + ':' + LabelledItem
  attr.action : AddLabel(%3, %1)

##-----------------------------------
##rule LabelledItem[Yield, Return] :
##  Statement[?Yield, ?Return]
##  FunctionDeclaration[?Yield]
rule LabelledItem : ONEOF(Statement, FunctionDeclaration)

##-----------------------------------
##rule ThrowStatement[Yield] :
##  throw [no LineTerminator here] Expression[In, ?Yield] ;
rule ThrowStatement : "throw" + Expression + ';'
  attr.action : BuildThrows(%2)

##-----------------------------------
##rule TryStatement[Yield, Return] :
##  try Block[?Yield, ?Return] Catch[?Yield, ?Return]
##  try Block[?Yield, ?Return] Finally[?Yield, ?Return]
##  try Block[?Yield, ?Return] Catch[?Yield, ?Return] Finally[?Yield, ?Return]
rule TryStatement : ONEOF(
  "try" + Block + Catch,
  "try" + Block + Finally,
  "try" + Block + Catch + Finally)
  attr.action.%1,%2,%3 : BuildTry(%2)
  attr.action.%1,%3 : AddCatch(%3)
  attr.action.%2 : AddFinally(%3)
  attr.action.%3 : AddFinally(%4)

##-----------------------------------
##rule Catch[Yield, Return] :
##  catch ( CatchParameter[?Yield] ) Block[?Yield, ?Return]
rule Catch : "catch" + '(' + CatchParameter + ')' + Block
  attr.action : BuildCatch(%3, %5)

##-----------------------------------
##rule Finally[Yield, Return] :
##  finally Block[?Yield, ?Return]
rule Finally : "finally" + Block
  attr.action : BuildFinally(%2)

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

## NOTE: Replaced by TS 
## FunctionDeclaration[Yield, Default] :
## function BindingIdentifier[?Yield] ( FormalParameters ) { FunctionBody }
## [+Default] function ( FormalParameters ) { FunctionBody }

##
## FunctionExpression :
## function BindingIdentifieropt ( FormalParameters ) { FunctionBody }

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
rule FormalParameter : BindingElement
  attr.action : BuildDecl(%3, %1)

##
## FunctionBody[Yield] :
## FunctionStatementList[?Yield]
## NOTE. I used ZEROORONE(StatementList) directly in order to avoid
##       an issue where FunctionStatementList fail when looking for
##       its lookahead, if function body is empty.
rule FunctionBody : ZEROORONE(StatementList)
  attr.action : BuildBlock(%1)

##
## FunctionStatementList[Yield] :
## StatementList[?Yield, Return]opt
rule FunctionStatementList : ZEROORONE(StatementList)

## See 14.2
## ArrowFunction[In, Yield] :
## ArrowParameters[?Yield] [no LineTerminator here] => ConciseBody[?In]

# (1) I inline ArrowParameters
# (2) In CoverParent.... is replaced by ArrowFormalParameters which in turn is changed
#     to CallSignature in Typescript. I inline CallSignature here.
rule ArrowFunction : ONEOF(
  BindingIdentifier + "=>" + ConciseBody,
  ZEROORONE(TypeParameters) + '(' + ZEROORONE(ParameterList)  + ')' + ZEROORONE(TypeAnnotation) + "=>" + ConciseBody)
  attr.action.%1 : BuildLambda(%1, %3)
  attr.action.%2 : BuildLambda(%3, %7)
  attr.action.%2 : AddType(%5)
  attr.action.%1,%2 : SetArrowFunction()

## See 14.2
## ArrowParameters[Yield] :
## BindingIdentifier[?Yield]
## CoverParenthesizedExpressionAndArrowParameterList[?Yield]
## When the production ArrowParameters:CoverParenthesizedExpressionAndArrowParameterList is recognized the following
## grammar is used to refine the interpretation of CoverParenthesizedExpressionAndArrowParameterList:
##ArrowFormalParameters[Yield]:
##(StrictFormalParameters[?Yield])
rule ArrowParameters : ONEOF(BindingIdentifier,
                             ArrowFormalParameters)

rule ArrowFormalParameters : '(' + StrictFormalParameters + ')'

## See 14.2
## ConciseBody[In] :
## [lookahead ≠ { ] AssignmentExpression[?In]
## { FunctionBody }
rule ConciseBody : ONEOF(AssignmentExpression,
                         '{' + FunctionBody + '}')

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
rule ClassBody : ClassElementList
  attr.action: BuildBlock(%1)

## See 14.5
## ClassElementList[Yield] :
## ClassElement[?Yield]
## ClassElementList[?Yield] ClassElement[?Yield]
rule ClassElementList : ONEOF(ClassElement,
                              ClassElementList + ClassElement)

## See 14.5
## ClassElement[Yield] :
## MethodDefinition[?Yield]
## static MethodDefinition[?Yield]
## ;

#############################################################################
##                   A.5 Scripts and Modules
#############################################################################

## See 15.1
## Script : ## ScriptBodyopt
rule Script : ZEROORONE(ScriptBody)

## See 15.1
## ScriptBody : ## StatementList
rule ScriptBody : StatementList

## Module : ModuleBodyopt
rule Module : ZEROORONE(ModuleBody)

## ModuleBody : ModuleItemList
rule ModuleBody : ModuleItemList

## ModuleItemList :
## ModuleItem
## ModuleItemList ModuleItem
rule ModuleItemList : ONEOF(ModuleItem,
                            ModuleItemList + ModuleItem)

## ModuleItem :
## ImportDeclaration
## ExportDeclaration
## StatementListItem
rule ModuleItem : ONEOF(ImportDeclaration,
                        ExportDeclaration,
                        StatementListItem)

## ImportDeclaration :
## import ImportClause FromClause ;
## import ModuleSpecifier ;
rule ImportDeclaration : ONEOF("import" + ImportClause + FromClause + ZEROORONE(';'),
                               "import" + ModuleSpecifier + ZEROORONE(';'),
                               "import" + BindingIdentifier + '=' + "require" + '(' + AssignmentExpression + ')' + ZEROORONE(';'))
  attr.property : Top
  attr.action.%1,%2,%3 : BuildImport()
  attr.action.%1 :    SetPairs(%2)
  attr.action.%1 :    SetFromModule(%3)
  attr.action.%2 :    SetFromModule(%2)
  attr.action.%3 :    SetSinglePairs(%6, %2)

## ImportClause :
## ImportedDefaultBinding
## NameSpaceImport
## NamedImports
## ImportedDefaultBinding , NameSpaceImport
## ImportedDefaultBinding , NamedImports
rule ImportClause : ONEOF(ImportedDefaultBinding,
                          NameSpaceImport,
                          NamedImports,
                          ImportedDefaultBinding + ',' + NameSpaceImport,
                          ImportedDefaultBinding + ',' + NamedImports,
                          "type" + NamedImports)

## See 15.2.2
## ImportedDefaultBinding :
## ImportedBinding
rule ImportedDefaultBinding : ImportedBinding
  attr.action : BuildXXportAsPairDefault(%1)

## See 15.2.2
## NameSpaceImport :
## * as ImportedBinding
rule NameSpaceImport : '*' + "as" + ImportedBinding
  attr.action : BuildXXportAsPairEverything(%3)

## See 15.2.2
## NamedImports :
## { }
## { ImportsList }
## { ImportsList , }
rule NamedImports : ONEOF('{' + '}',
                          '{' + ImportsList + '}',
                          '{' + ImportsList + ',' + '}')

## See 15.2.2
## FromClause :
## from ModuleSpecifier
rule FromClause : "from" + ModuleSpecifier

## See 15.2.2
## ImportsList :
## ImportSpecifier
## ImportsList , ImportSpecifier
rule ImportsList : ONEOF(ImportSpecifier,
                         ImportsList + ',' + ImportSpecifier)

## See 15.2.2
## ImportSpecifier :
## ImportedBinding
## IdentifierName as ImportedBinding
rule ImportSpecifier : ONEOF(ImportedBinding,
                             JSIdentifier + "as" + ImportedBinding,
                             "default" + "as" + ImportedBinding)
  attr.action.%2 : BuildXXportAsPair(%1, %3)
  attr.action.%3 : BuildXXportAsPairDefault(%3)

## See 15.2.2
## ModuleSpecifier :
## StringLiteral
## NOTE. I extend StringLiteral to Literal to ease parser. 'tsc' will make sure
##       it's a string literal.
rule ModuleSpecifier : Literal

## See 15.2.2
## ImportedBinding :
## BindingIdentifier
rule ImportedBinding : BindingIdentifier

## See 15.2.3
## ExportDeclaration :
## export * FromClause ;
## export ExportClause FromClause ;
## export ExportClause ;
## export VariableStatement
## export Declaration
## export default HoistableDeclaration[Default]
## export default ClassDeclaration[Default]
## export default [lookahead ∉ {function, class}] AssignmentExpression[In] ;

# export = expr;
# is for export single syntax.
rule ExportDeclaration : ONEOF(ZEROORMORE(Annotation) + "export" + '*' + FromClause + ZEROORONE(';'),
                               ZEROORMORE(Annotation) + "export" + ExportClause + FromClause + ZEROORONE(';'),
                               ZEROORMORE(Annotation) + "export" + ExportClause + ZEROORONE(';'),
                               ZEROORMORE(Annotation) + "export" + VariableStatement,
                               ZEROORMORE(Annotation) + "export" + Declaration + ZEROORONE(';'),
                               ZEROORMORE(Annotation) + "export" + "default" + HoistableDeclaration,
                               ZEROORMORE(Annotation) + "export" + "default" + ClassDeclaration,
                               ZEROORMORE(Annotation) + "export" + "default" + AssignmentExpression + ZEROORONE(';'),
                               ZEROORMORE(Annotation) + "export" + "=" + AssignmentExpression + ZEROORONE(';'),
                               ZEROORMORE(Annotation) + "export" + "type" + ExportClause + FromClause + ZEROORONE(';'),
                               ZEROORMORE(Annotation) + "export" + "type" + ExportClause + ZEROORONE(';'))
  attr.property : Top
  attr.action.%1,%2,%3,%4,%5,%6,%7,%8,%9,%10,%11 : BuildExport()
  attr.action.%1,%2,%3,%4,%5,%6,%7,%8,%9,%10,%11 : AddModifier(%1)
  attr.action.%1       :    SetIsEverything()
  attr.action.%2,%3,%4,%5 : SetPairs(%3)
  attr.action.%6,%7,%8 :    SetDefaultPairs(%4)
  attr.action.%1,%2 :       SetFromModule(%4)
  attr.action.%9          : SetSinglePairs(%4)
  attr.action.%10,%11     : SetPairs(%4)
  attr.action.%10         : SetFromModule(%5)

## See 15.2.3
## ExportClause :
## { }
## { ExportsList }
## { ExportsList , }
rule ExportClause : ONEOF('{' + '}',
                          '{' + ExportsList + '}',
                          '{' + ExportsList + ',' + '}')

## ExportsList :
## ExportSpecifier
## ExportsList , ExportSpecifier
rule ExportsList : ONEOF(ExportSpecifier,
                         ExportsList + ',' + ExportSpecifier)

## See 15.2.3
## ExportSpecifier :
## IdentifierName
## IdentifierName as IdentifierName
rule ExportSpecifier : ONEOF(JSIdentifier,
                             JSIdentifier + "as" + BindingIdentifier,
                             JSIdentifier + "as" + "default",
                             "default" + "as" + JSIdentifier)
  attr.action.%2 : BuildXXportAsPair(%1, %3)
  attr.action.%3 : BuildXXportAsPairDefault(%1)
  attr.action.%4 : BuildXXportAsPairDefault(%3)

#############################################################################
#############################################################################
#############################################################################
##                    Below is Typescript specific
#############################################################################
#############################################################################
#############################################################################

#############################################################################
##                        A.1 Type section
#############################################################################

## rule TypeParameters: < TypeParameterList >
rule TypeParameters: '<' + TypeParameterList + '>'

rule TypeParameterList: ONEOF(TypeParameter,
                              TypeParameterList + ',' + TypeParameter)

## rule TypeParameter: BindingIdentifier Constraintopt
## It supports default type value of type parameter now.
rule TypeParameter: BindingIdentifier + ZEROORONE(Constraint) + ZEROORONE(TypeInitializer)
  attr.action : BuildTypeParameter(%1)
  attr.action : AddInit(%3)
  attr.action : AddTypeParameterExtends(%2)

rule TypeInitializer : '=' + Type

## rule Constraint: extends Type
rule Constraint: "extends" + Type
  attr.action : PassChild(%2)

## rule TypeArguments: < TypeArgumentList >
rule TypeArguments: '<' + TypeArgumentList + '>'

## rule TypeArgumentList: TypeArgument TypeArgumentList , TypeArgument
rule TypeArgumentList: ONEOF(TypeArgument,
                             TypeArgumentList + ',' + TypeArgument)

## rule TypeArgument: Type
rule TypeArgument: Type

rule ConditionalType : MemberExpression + "extends" + Type + '?' + Type + ':' + Type
  attr.action : BuildConditionalType(%1, %3, %5, %7)

rule KeyOf : ONEOF("keyof" + Identifier,
                   "keyof" + '(' + TypeQuery + ')',
                   "keyof" + TypeQuery,
                   "keyof" + MemberExpression)
  attr.action.%1,%3,%4 : BuildKeyOf(%2)
  attr.action.%2 : BuildKeyOf(%3)

rule InferType : "infer" + Identifier
  attr.action : BuildInfer(%2)

rule TypeArray : ONEOF(PrimaryType + '[' + PrimaryExpression + ']',
                       PrimaryType + '[' + TypeReference + ']',
                       TypeArray + '[' + PrimaryExpression + ']')
  attr.action.%1,%2,%3 : BuildArrayElement(%1, %3)

#rule Type : ONEOF(UnionOrIntersectionOrPrimaryType,
#                  FunctionType,
#                  ConstructorType)
rule Type : ONEOF(UnionOrIntersectionOrPrimaryType,
                  FunctionType,
                  ConstructorType,
                  KeyOf,
                  ConditionalType,
                  # Typescript interface[index] can be seen as a type
                  TypeArray,
                  MemberExpression + '[' + KeyOf + ']',
                  PrimaryType + '[' + KeyOf + ']',
                  InferType,
                  IsExpression,
                  PrimaryType + '[' + TypeQuery + ']')
  attr.action.%7,%8,%11 : BuildArrayElement(%1, %3)

#rule UnionOrIntersectionOrPrimaryType: ONEOF(UnionType,
#                                             IntersectionOrPrimaryType)
rule UnionOrIntersectionOrPrimaryType: ONEOF(UnionType,
                                             IntersectionOrPrimaryType)

#rule IntersectionOrPrimaryType : ONEOF(IntersectionType,
#                                       PrimaryType)
rule IntersectionOrPrimaryType : ONEOF(IntersectionType, PrimaryType, TypeArray)

## rule PrimaryType: ParenthesizedType PredefinedType TypeReference ObjectType ArrayType TupleType TypeQuery ThisType
rule PrimaryType: ONEOF(ParenthesizedType,
                        PredefinedType,
                        TypeReference,
                        ObjectType,
                        ArrayType,
                        TupleType,
                        TypeQuery,
                        ThisType,
                        NeverArrayType,
                        Literal)

rule NeverArrayType : '[' + ']'
  attr.action : BuildNeverArrayType()

## rule ParenthesizedType: ( Type )
rule ParenthesizedType: '(' + Type + ')'

## rule PredefinedType: any number boolean string symbol void
rule PredefinedType: ONEOF(TYPE,
                           "unique" + TYPE)
  attr.action.%2 : SetIsUnique(%2)

## rule TypeReference: TypeName [no LineTerminator here] TypeArgumentsopt
rule TypeReference: TypeName + ZEROORONE(TypeArguments) + ZEROORMORE(AsType)
  attr.action : BuildUserType(%1)
  attr.action : AddTypeGenerics(%2)
  attr.action : AddAsType(%3)

## rule TypeName: IdentifierReference NamespaceName . IdentifierReference
rule TypeName: ONEOF(IdentifierReference,
                     NamespaceName + '.' + IdentifierReference,
                     '(' + IdentifierReference + ')',
                     '(' + NamespaceName + '.' + IdentifierReference + ')')
  attr.action.%2 : BuildField(%1, %3)
  attr.action.%4 : BuildField(%2, %4)

## rule NamespaceName: IdentifierReference NamespaceName . IdentifierReference
rule NamespaceName: ONEOF(IdentifierReference,
                          NamespaceName + '.' + IdentifierReference)
  attr.action.%2 : BuildField(%1, %3)

## rule ObjectType: { TypeBodyopt }
rule ObjectType : '{' + ZEROORONE(TypeBody) + '}'
  attr.action : BuildStruct()
  attr.action : AddStructField(%2)

## rule TypeBody: TypeMemberList ;opt TypeMemberList ,opt
rule TypeBody : ONEOF(TypeMemberList + ZEROORONE(';'),
                      TypeMemberList + ZEROORONE(','))

## rule TypeMemberList: TypeMember TypeMemberList ; TypeMember TypeMemberList , TypeMember
rule TypeMemberList : ONEOF(TypeMember,
                            TypeMemberList + ZEROORONE(';') + TypeMember,
                            TypeMemberList + ZEROORONE(',') + TypeMember)

## rule TypeMember: PropertySignature CallSignature ConstructSignature IndexSignature MethodSignature
rule TypeMember : ONEOF(PropertySignature,
                        CallSignature,
                        ConstructSignature,
                        IndexSignature,
                        MethodSignature)

## rule ArrayType: PrimaryType [no LineTerminator here] [ ]
rule ArrayType: ONEOF(ZEROORONE("readonly") + PrimaryType + '[' + ']',
                      SpreadElement + '[' + ']',
                      MemberExpression + '[' + ']')
  attr.action.%1 : BuildArrayType(%2, %2)
  attr.action.%1 : AddModifier(%1)
  attr.action.%2 : BuildArrayType(%1, %1)
  attr.action.%3 : BuildArrayType(%1, %1)

## rule TupleType: [ TupleElementTypes ]
rule TupleType: '[' + TupleElementTypes + ']'
  attr.action : BuildTupleType()
  attr.action : AddStructField(%2)

## rule TupleElementTypes: TupleElementType TupleElementTypes , TupleElementType
rule TupleElementTypes: ONEOF(TupleElementType,
                              TupleElementTypes + ',' + TupleElementType)

## rule TupleElementType: Type
rule TupleElementType: ZEROORONE(JSIdentifier + ':') + Type
  attr.action : BuildNameTypePair(%1, %2)

## rule UnionType: UnionOrIntersectionOrPrimaryType | IntersectionOrPrimaryType
rule UnionType: ZEROORONE('|') + UnionOrIntersectionOrPrimaryType + '|' + IntersectionOrPrimaryType
  attr.action : BuildUnionUserType(%2, %4)

## rule IntersectionType: IntersectionOrPrimaryType & PrimaryType
rule IntersectionType: IntersectionOrPrimaryType + '&' + PrimaryType
  attr.action : BuildInterUserType(%1, %3)

## rule FunctionType: TypeParametersopt ( ParameterListopt ) => Type
rule FunctionType: ZEROORONE(TypeParameters) + '(' + ZEROORONE(ParameterList) + ')' + "=>" + Type
  attr.action : BuildLambda(%3)
  attr.action : AddType(%6)

## rule ConstructorType: new TypeParametersopt ( ParameterListopt ) => Type
## This actually a literal.
rule ConstructorType: "new" + FunctionType
  attr.action : BuildNewOperation(%2)

## rule TypeQuery: typeof TypeQueryExpression
rule TypeQuery: "typeof" + TypeQueryExpression
  attr.action : BuildTypeOf(%2)

## rule TypeQueryExpression: IdentifierReference TypeQueryExpression . IdentifierName
rule TypeQueryExpression: ONEOF(IdentifierReference,
                                TypeQueryExpression + '.' + JSIdentifier,
                                UnaryExpression)
  attr.action.%2 : BuildField(%1, %3)

## rule ThisType: this
rule ThisType: "this"

## rule PropertySignature: PropertyName ?opt TypeAnnotationopt
rule PropertySignature: ONEOF(ZEROORONE(AccessibilityModifier) + PropertyName + ZEROORONE(TypeAnnotation),
                              ZEROORONE(AccessibilityModifier) + PropertyName + '?' + ZEROORONE(TypeAnnotation))
  attr.action.%1 : AddType(%2, %3)
  attr.action.%2 : AddType(%2, %4)
  attr.action.%2 : SetIsOptional(%2)
  attr.action.%1,%2: AddModifierTo(%2, %1)

## JS ECMA has more definition than this Typescript one. I use ECMA one.
## rule PropertyName: IdentifierName StringLiteral NumericLiteral
##rule PropertyName : ONEOF(JSIdentifier,
                          ##StringLiteral,
                          ##NumericLiteral,
##                         )

## rule TypeAnnotation: : Type
rule TypeAnnotation: ':' + Type

## rule CallSignature: TypeParametersopt ( ParameterListopt ) TypeAnnotationopt
rule CallSignature: ZEROORONE(TypeParameters) + '(' + ZEROORONE(ParameterList)  + ')' + ZEROORONE(TypeAnnotation)
  attr.action : BuildFunction()
  attr.action : AddParams(%3)
  attr.action : AddType(%5)
  attr.action : SetCallSignature()

## rule ParameterList: RequiredParameterList OptionalParameterList RestParameter RequiredParameterList , OptionalParameterList RequiredParameterList , RestParameter OptionalParameterList , RestParameter RequiredParameterList , OptionalParameterList , RestParameter
rule ParameterList: ONEOF(RequiredParameterList + ZEROORONE(Elision),
                          OptionalParameterList + ZEROORONE(Elision),
                          RestParameter,
                          RequiredParameterList + ',' + OptionalParameterList + ZEROORONE(Elision),
                          RequiredParameterList + ',' + RestParameter,
                          OptionalParameterList + ',' + RestParameter,
                          RequiredParameterList + ',' + OptionalParameterList + ',' + RestParameter)

## rule RequiredParameterList: RequiredParameter RequiredParameterList , RequiredParameter
rule RequiredParameterList: ONEOF(RequiredParameter,
                                  RequiredParameterList + ',' + RequiredParameter)

## rule RequiredParameter: AccessibilityModifieropt BindingIdentifierOrPattern TypeAnnotationopt BindingIdentifier : StringLiteral
## NOTE: I extend StringLiteral to Literal.
## NOTE: I Added initializer. I guess the spec missed this part.
rule RequiredParameter: ONEOF(
  ZEROORMORE(AccessibilityModifier) + BindingIdentifierOrPattern + ZEROORONE(TypeAnnotation) + ZEROORONE(Initializer),
  "this" + ZEROORONE(TypeAnnotation),
  BindingIdentifier + ':' + Literal,
  ObjectType)
  attr.action.%1 : AddInitTo(%2, %4)
  attr.action.%1 : BuildDecl(%3, %2)
  attr.action.%2 : BuildDecl(%2, %1)

## rule AccessibilityModifier: public private protected
rule AccessibilityModifier: ONEOF("public", "private", "protected", "readonly", "static", "abstract")

## rule BindingIdentifierOrPattern: BindingIdentifier BindingPattern
rule BindingIdentifierOrPattern: ONEOF(BindingIdentifier, BindingPattern)

## rule OptionalParameterList: OptionalParameter OptionalParameterList , OptionalParameter
rule OptionalParameterList: ONEOF(OptionalParameter,
                                  OptionalParameterList + ',' + OptionalParameter)

## rule OptionalParameter: AccessibilityModifieropt BindingIdentifierOrPattern ? TypeAnnotationopt AccessibilityModifieropt BindingIdentifierOrPattern TypeAnnotationopt Initializer BindingIdentifier ? : StringLiteral
rule OptionalParameter: ONEOF(
  ZEROORMORE(AccessibilityModifier) + BindingIdentifierOrPattern + '?' + ZEROORONE(TypeAnnotation),
  ZEROORMORE(AccessibilityModifier) + BindingIdentifierOrPattern + ZEROORONE(TypeAnnotation) + Initializer,
  BindingIdentifier + '?' + ':' + Literal)
  attr.action.%1 : SetOptionalParam(%2)
  attr.action.%1 : BuildDecl(%4, %2)
  attr.action.%2 : AddInitTo(%2, %4)
  attr.action.%2 : BuildDecl(%3, %2)
  attr.action.%3 : SetOptionalParam(%1)

## rule RestParameter: ... BindingIdentifier TypeAnnotationopt
rule RestParameter: "..." + BindingIdentifier + ZEROORONE(TypeAnnotation)
  attr.action : AddType(%2, %3)
  attr.action : SetIsRest(%2)

## rule ConstructSignature: new TypeParametersopt ( ParameterListopt ) TypeAnnotationopt
rule ConstructSignature :
  "new" + ZEROORONE(TypeParameters) + '(' + ZEROORONE(ParameterList) + ')' + ZEROORONE(TypeAnnotation)
  attr.action : BuildFunction()
  attr.action : AddParams(%4)
  attr.action : AddType(%6)
  attr.action : SetConstructSignature()

## rule IndexSignature: [ BindingIdentifier : string ] TypeAnnotation [ BindingIdentifier : number ] TypeAnnotation
rule IndexSignature: ONEOF(
  '[' + BindingIdentifier + ':' + "string" + ']' + TypeAnnotation,
  '[' + BindingIdentifier + ':' + "number" + ']' + TypeAnnotation)
  attr.action.%1 : BuildStrIndexSig(%2, %6)
  attr.action.%2 : BuildNumIndexSig(%2, %6)

## rule MethodSignature: PropertyName ?opt CallSignature
## I inlined CallSignature
rule MethodSignature: ONEOF(
    PropertyName + ZEROORONE(TypeParameters) + '(' + ZEROORONE(ParameterList)  + ')' + ZEROORONE(TypeAnnotation),
    PropertyName + '?' + ZEROORONE(TypeParameters) + '(' + ZEROORONE(ParameterList)  + ')' + ZEROORONE(TypeAnnotation),
    "return" + ZEROORONE(TypeParameters) + '(' + ZEROORONE(ParameterList)  + ')' + ZEROORONE(TypeAnnotation),
    "throw"  + ZEROORONE(TypeParameters) + '(' + ZEROORONE(ParameterList)  + ')' + ZEROORONE(TypeAnnotation),
    "return" + '?' + ZEROORONE(TypeParameters) + '(' + ZEROORONE(ParameterList)  + ')' + ZEROORONE(TypeAnnotation),
    "throw"  + '?' + ZEROORONE(TypeParameters) + '(' + ZEROORONE(ParameterList)  + ')' + ZEROORONE(TypeAnnotation))
  attr.action.%1,%2,%3,%4,%5,%6 : BuildFunction(%1)
  attr.action.%1,%3,%4 : AddParams(%4)
  attr.action.%1,%3,%4 : AddType(%6)
  attr.action.%1,%3,%4 : AddTypeGenerics(%2)
  attr.action.%2 : SetIsOptional(%1)
  attr.action.%2 : AddParams(%5)
  attr.action.%2 : AddType(%7)
  attr.action.%2 : AddTypeGenerics(%3)
  attr.action.%5,%6 : SetIsOptional()
  attr.action.%5,%6 : AddParams(%5)
  attr.action.%5,%6 : AddType(%7)
  attr.action.%5,%6 : AddTypeGenerics(%3)

## rule TypeAliasDeclaration: type BindingIdentifier TypeParametersopt = Type ;
rule TypeAliasDeclaration: "type" + BindingIdentifier + ZEROORONE(TypeParameters) + '=' + Type + ZEROORONE(';')
  attr.action : BuildTypeAlias(%2, %5)
  attr.action : AddTypeGenerics(%3)


##############################################################################################
##                              A.2 Expression
##############################################################################################

## PropertyDefinition: ( Modified ) IdentifierReference CoverInitializedName PropertyName : AssignmentExpression PropertyName CallSignature { FunctionBody } GetAccessor SetAccessor
rule PropertyDefinition: ONEOF(IdentifierReference,
                               CoverInitializedName,
                               PropertyName + ':' + AssignmentExpression,
                               PropertyName + ZEROORONE(TypeParameters) + '(' + ZEROORONE(ParameterList)  + ')'
                                 + ZEROORONE(TypeAnnotation) + '{' + FunctionBody + '}',
                               GetAccessor,
                               SetAccessor,
                               SpreadElement)
  attr.action.%3 : BuildFieldLiteral(%1, %3)
  attr.action.%4 : BuildFunction(%1)
  attr.action.%4 : AddType(%6)
  attr.action.%4 : AddParams(%4)
  attr.action.%4 : AddFunctionBody(%8)

## GetAccessor: get PropertyName ( ) TypeAnnotationopt { FunctionBody }
rule GetAccessor: ONEOF("get" + PropertyName + '(' + ')' + ZEROORONE(TypeAnnotation) + '{' + FunctionBody + '}',
                        "get" + '(' + "this" + ')' + ZEROORONE(TypeAnnotation) + '{' + FunctionBody + '}')
  attr.action.%1 : BuildFunction(%2)
  attr.action.%1 : SetGetAccessor()
  attr.action.%1 : AddType(%5)
  attr.action.%1 : AddFunctionBody(%7)
  attr.action.%1 : AddModifier(%1)
  attr.action.%2 : BuildFunction()
  attr.action.%2 : SetGetAccessor()
  attr.action.%2 : AddType(%5)
  attr.action.%2 : AddFunctionBody(%7)
  attr.action.%2 : AddModifier(%1)

## SetAccessor: set PropertyName ( BindingIdentifierOrPattern TypeAnnotationopt ) { FunctionBody }
rule SetAccessor: ONEOF("set" + PropertyName + '(' + BindingIdentifierOrPattern + ZEROORONE(TypeAnnotation) + ')' + '{' + FunctionBody + '}',
                        "set" + '(' + "this" + ',' + BindingIdentifierOrPattern + ZEROORONE(TypeAnnotation) + ')' + '{' + FunctionBody + '}')
  attr.action.%1 : AddType(%4, %5)
  attr.action.%1 : BuildFunction(%2)
  attr.action.%1 : SetSetAccessor()
  attr.action.%1 : AddParams(%4)
  attr.action.%1 : AddFunctionBody(%8)
  attr.action.%1 : AddModifier(%1)
  attr.action.%2 : AddType(%5, %6)
  attr.action.%2 : BuildFunction()
  attr.action.%2 : SetSetAccessor()
  attr.action.%2 : AddParams(%5)
  attr.action.%2 : AddFunctionBody(%9)
  attr.action.%2 : AddModifier(%1)

## FunctionExpression: ( Modified ) function BindingIdentifieropt CallSignature { FunctionBody }
## FunctionExpression has the same syntax as FunctionDeclaration. But it appears as an expression. We will build it
## as a FunctionNode in AST.
rule FunctionExpression :
  "function" + ZEROORONE(BindingIdentifier) + ZEROORONE(TypeParameters) + '(' + ZEROORONE(ParameterList)  + ')' + ZEROORONE(TypeAnnotation) + '{' + FunctionBody + '}'
  attr.action : BuildFunction(%2)
  attr.action : AddTypeGenerics(%3)
  attr.action : AddParams(%5)
  attr.action : AddType(%7)
  attr.action : AddFunctionBody(%9)

# ArrowFormalParameter is used in ArrowFunction, and Typescript modified
# it to be CallSignature. As usual, I'd like to inline CallSignature in
# those rules.
## ArrowFormalParameters: ( Modified ) CallSignature

## Arguments: ( Modified ) TypeArgumentsopt ( ArgumentListopt )
## UnaryExpression: ( Modified ) … < Type > UnaryExpression

##############################################################################################
##                              A.4 Functions
##############################################################################################

## FunctionDeclaration: ( Modified )
## function BindingIdentifieropt CallSignature { FunctionBody }
## function BindingIdentifieropt CallSignature ;

# NOTE: Inline Call signature to make it easier to write action.
rule FunctionDeclaration : ONEOF(
  "function" + ZEROORONE(BindingIdentifier) + ZEROORONE(TypeParameters) + '(' + ZEROORONE(ParameterList)  + ')' + ZEROORONE(TypeAnnotation) + '{' + FunctionBody + '}',
  "function" + ZEROORONE(BindingIdentifier) + ZEROORONE(TypeParameters) + '(' + ZEROORONE(ParameterList)  + ')' + ':' + AssertExpression + '{' + FunctionBody + '}',
  "function" + ZEROORONE(BindingIdentifier) + ZEROORONE(TypeParameters) + '(' + ZEROORONE(ParameterList)  + ')' + ':' + IsExpression + '{' + FunctionBody + '}',
  "function" + ZEROORONE(BindingIdentifier) + ZEROORONE(TypeParameters) + '(' + ZEROORONE(ParameterList)  + ')' + ZEROORONE(TypeAnnotation)  + ZEROORONE(';'),
  "function" + ZEROORONE(BindingIdentifier) + ZEROORONE(TypeParameters) + '(' + ZEROORONE(ParameterList)  + ')' + ':' + IsExpression + ZEROORONE(';'))
  attr.action.%1,%2,%3,%4,%5 : BuildFunction(%2)
  attr.action.%1,%2,%3,%4,%5 : AddParams(%5)
  attr.action.%1,%4    : AddType(%7)
  attr.action.%1,%2,%3,%4,%5 : AddTypeGenerics(%3)
  attr.action.%2,%3,%5 : AddAssert(%8)
  attr.action.%1       : AddFunctionBody(%9)
  attr.action.%2,%3    : AddFunctionBody(%10)

##############################################################################################
##                               A.5 Interface
##############################################################################################

##InterfaceDeclaration: interface BindingIdentifier TypeParametersopt InterfaceExtendsClauseopt ObjectType
rule InterfaceDeclaration :
  "interface" + BindingIdentifier + ZEROORONE(TypeParameters) + ZEROORONE(InterfaceExtendsClause) + '{' + ZEROORONE(TypeBody) + '}' 
  attr.action : BuildStruct(%2)
  attr.action : SetTSInterface()
  attr.action : AddTypeGenerics(%3)
  attr.action : AddStructField(%6)
  attr.action : AddSuperInterface(%4)

##InterfaceExtendsClause: extends ClassOrInterfaceTypeList
rule InterfaceExtendsClause: "extends" + ClassOrInterfaceTypeList

##ClassOrInterfaceTypeList: ClassOrInterfaceType ClassOrInterfaceTypeList , ClassOrInterfaceType
rule ClassOrInterfaceTypeList: ONEOF(ClassOrInterfaceType,
                                     ClassOrInterfaceTypeList + ',' + ClassOrInterfaceType)

##ClassOrInterfaceType: TypeReference
rule ClassOrInterfaceType: TypeReference

##############################################################################################
##                             A.6 Class declaration
##############################################################################################

## ClassDeclaration: ( Modified ) class BindingIdentifieropt TypeParametersopt ClassHeritage { ClassBody }
## NOTE. I inlined ClassHeritage to avoid 'lookahead fail'
rule ClassDeclaration:
  ZEROORMORE(Annotation) + ZEROORONE("abstract") + "class" + ZEROORONE(BindingIdentifier) + ZEROORONE(TypeParameters) + ZEROORONE(ClassExtendsClause) + ZEROORONE(ImplementsClause) + '{' + ZEROORONE(ClassBody) + '}'
  attr.action : BuildClass(%4)
  attr.action : AddModifier(%1)
  attr.action : AddModifier(%2)
  attr.action : AddTypeGenerics(%5)
  attr.action : AddSuperClass(%6)
  attr.action : AddSuperInterface(%7)
  attr.action : AddClassBody(%9)

rule Annotation : '@' + JSIdentifier + ZEROORONE(Arguments)
  attr.action : BuildAnnotation(%2)
  attr.action : AddArguments(%3)

## ClassHeritage: ( Modified ) ClassExtendsClauseopt ImplementsClauseopt
rule ClassHeritage: ZEROORONE(ClassExtendsClause) + ZEROORONE(ImplementsClause)

## ClassExtendsClause: extends ClassType
rule ClassExtendsClause: ONEOF("extends" + ZEROORONE('(') + ClassType + ZEROORONE(')'),
                               "extends" + CallExpression)

## ClassType: TypeReference
rule ClassType: TypeReference

## ImplementsClause: implements ClassOrInterfaceTypeList
rule ImplementsClause: "implements" + ClassOrInterfaceTypeList

## ClassElement: ( Modified ) ConstructorDeclaration PropertyMemberDeclaration IndexMemberDeclaration
rule ClassElement: ONEOF(ConstructorDeclaration,
                         PropertyMemberDeclaration,
                         IndexMemberDeclaration)
  attr.property : Single

## ConstructorDeclaration: AccessibilityModifieropt constructor ( ParameterListopt ) { FunctionBody } AccessibilityModifieropt constructor ( ParameterListopt ) ;
rule ConstructorDeclaration: ONEOF(
  ZEROORONE(AccessibilityModifier) + "constructor" + '(' + ZEROORONE(ParameterList) + ')' + '{' + FunctionBody + '}',
  ZEROORONE(AccessibilityModifier) + "constructor" + '(' + ZEROORONE(ParameterList) + ')' + ';')
  attr.action.%1,%2 : BuildConstructor()
  attr.action.%1,%2 : AddParams(%4)
  attr.action.%1 : AddFunctionBody(%7)

## PropertyMemberDeclaration: MemberVariableDeclaration MemberFunctionDeclaration MemberAccessorDeclaration
rule PropertyMemberDeclaration: ONEOF(MemberVariableDeclaration,
                                      MemberFunctionDeclaration + ZEROORONE(';'),
                                      MemberAccessorDeclaration + ZEROORONE(';'),
                                      MemberExternalDeclaration)

## MemberVariableDeclaration: AccessibilityModifieropt staticopt PropertyName TypeAnnotationopt Initializeropt ;
rule MemberVariableDeclaration: ONEOF(
  ZEROORMORE(Annotation) + ZEROORMORE(AccessibilityModifier) + PropertyName + ZEROORONE(TypeAnnotation) + ZEROORONE(Initializer) + ZEROORONE(';'),
  ZEROORMORE(Annotation) + ZEROORMORE(AccessibilityModifier) + PropertyName + '?' + ZEROORONE(TypeAnnotation) + ZEROORONE(Initializer) + ZEROORONE(';'))
  attr.action.%1: AddInitTo(%3, %5)
  attr.action.%1: AddType(%3, %4)
  attr.action.%1: AddModifierTo(%3, %2)
  attr.action.%1: AddModifierTo(%3, %1)
  attr.action.%1: BuildDecl(%4, %3)
  attr.action.%2: AddInitTo(%3, %6)
  attr.action.%2: AddType(%3, %5)
  attr.action.%2: AddModifierTo(%3, %2)
  attr.action.%2: AddModifierTo(%3, %1)
  attr.action.%2: SetIsOptional(%3)
  attr.action.%2: BuildDecl(%4, %3)


## MemberFunctionDeclaration: AccessibilityModifieropt staticopt PropertyName CallSignature { FunctionBody } AccessibilityModifieropt staticopt PropertyName CallSignature ;
#NOTE: I inlined CallSignature to make it easier for building function.
#rule CallSignature: ZEROORONE(TypeParameters) + '(' + ZEROORONE(ParameterList)  + ')' + ZEROORONE(TypeAnnotation)
rule MemberFunctionDeclaration: ONEOF(
  ZEROORONE(Annotation) + ZEROORMORE(AccessibilityModifier) + PropertyName + ZEROORONE(TypeParameters) + '(' + ZEROORONE(ParameterList)  + ')' + ZEROORONE(TypeAnnotation) + '{' + FunctionBody + '}',
  ZEROORONE(Annotation) + ZEROORMORE(AccessibilityModifier) + PropertyName + ZEROORONE(TypeParameters) + '(' + ZEROORONE(ParameterList)  + ')' + ZEROORONE(TypeAnnotation) + ZEROORONE(';'),
  ZEROORONE(Annotation) + ZEROORMORE(AccessibilityModifier) + PropertyName + '?' + ZEROORONE(TypeParameters) + '(' + ZEROORONE(ParameterList)  + ')' + ZEROORONE(TypeAnnotation) + '{' + FunctionBody + '}',
  ZEROORONE(Annotation) + ZEROORMORE(AccessibilityModifier) + PropertyName + '?' + ZEROORONE(TypeParameters) + '(' + ZEROORONE(ParameterList)  + ')' + ZEROORONE(TypeAnnotation) + ZEROORONE(';'),
  ZEROORONE(Annotation) + ZEROORMORE(AccessibilityModifier) + PropertyName + ZEROORONE(TypeParameters) + '(' + ZEROORONE(ParameterList)  + ')' + ':' + IsExpression + '{' + FunctionBody + '}',
  ZEROORONE(Annotation) + ZEROORMORE(AccessibilityModifier) + PropertyName + ZEROORONE(TypeParameters) + '(' + ZEROORONE(ParameterList)  + ')' + ':' + IsExpression + ZEROORONE(';'),
  ZEROORONE(Annotation) + ZEROORMORE(AccessibilityModifier) + "return" + ZEROORONE(TypeParameters) + '(' + ZEROORONE(ParameterList)  + ')' + ZEROORONE(TypeAnnotation) + '{' + FunctionBody + '}')
  attr.action.%1,%2,%3,%4,%5,%6,%7 : BuildFunction(%3)
  attr.action.%1,%2,%3,%4,%5,%6,%7 : AddModifier(%2)
  attr.action.%1,%2,%3,%4,%5,%6,%7 : AddModifier(%1)
  attr.action.%1,%2,%5,%6,%7 : AddTypeGenerics(%4)
  attr.action.%1,%2,%5,%6,%7 : AddParams(%6)
  attr.action.%1,%2,%7 : AddType(%8)
  attr.action.%1,%7    : AddFunctionBody(%10)
  attr.action.%3,%4 : AddTypeGenerics(%5)
  attr.action.%3,%4 : AddParams(%7)
  attr.action.%3,%4 : AddType(%9)
  attr.action.%3    : AddFunctionBody(%11)
  attr.action.%3,%4 : SetIsOptional(%3)
  attr.action.%5,%6 : AddType(%9)
  attr.action.%5    : AddFunctionBody(%11)

## MemberAccessorDeclaration: AccessibilityModifieropt staticopt GetAccessor AccessibilityModifieropt staticopt SetAccessor
rule MemberAccessorDeclaration: ONEOF(
  ZEROORONE(Annotation) + ZEROORMORE(AccessibilityModifier) + GetAccessor,
  ZEROORONE(Annotation) + ZEROORMORE(AccessibilityModifier) + SetAccessor)
  attr.action.%1,%2 : AddModifierTo(%3, %1)

## IndexMemberDeclaration: IndexSignature ;
rule IndexMemberDeclaration: IndexSignature + ZEROORONE(';')

rule MemberExternalDeclaration : ZEROORMORE(AccessibilityModifier) + "declare" + VariableDeclaration + ';'
  attr.action : BuildExternalDeclaration(%3)
  attr.action : AddModifier(%1)
#################################################################################################
#                                          A.7 Enums
#################################################################################################
## EnumDeclaration: constopt enum BindingIdentifier { EnumBodyopt }
rule EnumDeclaration:
  ZEROORONE("const") + "enum" + BindingIdentifier + '{' + ZEROORONE(EnumBody) + '}'
  attr.action : BuildStruct(%3)
  attr.action : SetTSEnum()
  attr.action : AddStructField(%5)

## EnumBody: EnumMemberList ,opt
rule EnumBody: EnumMemberList + ZEROORONE(',')

## EnumMemberList: EnumMember EnumMemberList , EnumMember
rule EnumMemberList: ONEOF(EnumMember,
                           EnumMemberList + ',' + EnumMember)

## EnumMember: PropertyName PropertyName = EnumValue
rule EnumMember: ONEOF(PropertyName,
                       PropertyName + '=' + EnumValue)
  attr.action.%2 : AddInitTo(%1, %3)

## EnumValue: AssignmentExpression
rule EnumValue: AssignmentExpression

#################################################################################################
##                                  A.8 Namespaces
#################################################################################################

##NamespaceDeclaration: namespace IdentifierPath { NamespaceBody }
rule NamespaceDeclaration: "namespace" + IdentifierPath + '{' + NamespaceBody + '}'
  attr.action : BuildNamespace(%2)
  attr.action : AddNamespaceBody(%4)

##IdentifierPath: BindingIdentifier IdentifierPath . BindingIdentifier
rule IdentifierPath: ONEOF(BindingIdentifier,
                           IdentifierPath + '.' + BindingIdentifier)
  attr.action.%2 : BuildField(%1, %3)

##NamespaceBody: NamespaceElementsopt
rule NamespaceBody: ZEROORONE(NamespaceElements)

##NamespaceElements: NamespaceElement NamespaceElements NamespaceElement
rule NamespaceElements: ONEOF(NamespaceElement,
                              NamespaceElements + NamespaceElement)

##NamespaceElement: Statement LexicalDeclaration FunctionDeclaration GeneratorDeclaration ClassDeclaration InterfaceDeclaration TypeAliasDeclaration EnumDeclaration NamespaceDeclaration AmbientDeclaration ImportAliasDeclaration ExportNamespaceElement
rule NamespaceElement: ONEOF(Statement,
                             LexicalDeclaration + ';',
                             FunctionDeclaration,
                             #GeneratorDeclaration,
                             ClassDeclaration,
                             InterfaceDeclaration,
                             TypeAliasDeclaration,
                             EnumDeclaration,
                             NamespaceDeclaration,
                             #AmbientDeclaration,
                             ImportAliasDeclaration,
                             ExportNamespaceElement)

##ExportNamespaceElement: export VariableStatement export LexicalDeclaration export FunctionDeclaration export GeneratorDeclaration export ClassDeclaration export InterfaceDeclaration export TypeAliasDeclaration export EnumDeclaration export NamespaceDeclaration export AmbientDeclaration export ImportAliasDeclaration
rule ExportNamespaceElement: ONEOF("export" + VariableStatement,
                                   "export" + LexicalDeclaration + ZEROORONE(';'),
                                   "export" + FunctionDeclaration,
#                                   "export" + GeneratorDeclaration,
                                   "export" + ClassDeclaration,
                                   "export" + InterfaceDeclaration,
                                   "export" + TypeAliasDeclaration,
                                   "export" + EnumDeclaration,
                                   "export" + NamespaceDeclaration,
#                                   "export" + AmbientDeclaration,
                                   "export" + ImportAliasDeclaration,
                                   ExportDeclaration)
  attr.action.%1,%2,%3,%4,%5,%6,%7,%8,%9 : BuildExport()
  attr.action.%1,%2,%3,%4,%5,%6,%7,%8,%9 : SetPairs(%2)

##ImportAliasDeclaration: import BindingIdentifier = EntityName ;
rule NamespaceImportPair: BindingIdentifier + '=' + EntityName
  attr.action : BuildXXportAsPair(%3, %1)

rule ImportAliasDeclaration: "import" + NamespaceImportPair + ';'
  attr.action : BuildImport()
  attr.action : SetPairs(%2)

##EntityName: NamespaceName NamespaceName . IdentifierReference
rule EntityName: ONEOF(NamespaceName,
                       NamespaceName + '.' + IdentifierReference)
  attr.action.%2 : BuildField(%1, %3)

#################################################################################################
#                      declare syntax
#################################################################################################

rule ExternalDeclaration : ONEOF("declare" + NamespaceDeclaration,
                                 "declare" + LexicalDeclaration + ZEROORONE(';'),
                                 "declare" + ClassDeclaration,
                                 "declare" + FunctionDeclaration,
                                 "declare" + VariableStatement,
                                 "declare" + TypeAliasDeclaration,
                                 "declare" + EnumDeclaration,
                                 "declare" + ExternalModuleDeclaration)
  attr.action.%1,%2,%3,%4,%5,%6,%7,%8 : BuildExternalDeclaration(%2)

#################################################################################################
#                                 A.9 Scripts and Modules
#################################################################################################

# The module name could be an identifier or string literal.
rule ExternalModuleDeclaration : "module" + PrimaryExpression + '{' + DeclarationModule + '}'
  attr.action : BuildModule(%2)
  attr.action : AddModuleBody(%4)

#DeclarationElement: InterfaceDeclaration TypeAliasDeclaration NamespaceDeclaration AmbientDeclaration ImportAliasDeclaration
rule DeclarationElement: ONEOF(InterfaceDeclaration,
                               TypeAliasDeclaration,
                               VariableDeclaration,
                               FunctionDeclaration,
                               ClassDeclaration,
                               EnumDeclaration,
                               NamespaceDeclaration,
                               AmbientDeclaration,
                               ImportAliasDeclaration)

# ImplementationModule: ImplementationModuleElementsopt
# ImplementationModuleElements: ImplementationModuleElement ImplementationModuleElements ImplementationModuleElement
# ImplementationModuleElement: ImplementationElement ImportDeclaration ImportAliasDeclaration ImportRequireDeclaration ExportImplementationElement ExportDefaultImplementationElement ExportListDeclaration ExportAssignment

# DeclarationModule: DeclarationModuleElementsopt
rule DeclarationModule: ZEROORONE(DeclarationModuleElements)

# DeclarationModuleElements: DeclarationModuleElement DeclarationModuleElements DeclarationModuleElement
rule DeclarationModuleElements: ONEOF(DeclarationModuleElement,
                                      DeclarationModuleElements + DeclarationModuleElement)

# DeclarationModuleElement: DeclarationElement ImportDeclaration ImportAliasDeclaration ExportDeclarationElement ExportDefaultDeclarationElement ExportListDeclaration ExportAssignment
rule DeclarationModuleElement: ONEOF(DeclarationElement,
                                     ImportDeclaration,
                                     ImportAliasDeclaration,
                                     ExportDeclaration)
                                     #ExportDefaultDeclarationElement,
                                     #ExportListDeclaration,
                                     #ExportAssignment)

#################################################################################################
#                                 A.10 Ambient
# NOTE : I changed the rules a lot, making it quite different than v1.8 spec.
#################################################################################################

# AmbientDeclaration: declare AmbientVariableDeclaration declare AmbientFunctionDeclaration declare AmbientClassDeclaration declare AmbientEnumDeclaration declare AmbientNamespaceDeclaration
rule AmbientDeclaration: ONEOF("declare" + VariableDeclaration,
                               "declare" + FunctionDeclaration,
                               "declare" + ClassDeclaration,
                               "declare" + EnumDeclaration,
                               "declare" + NamespaceDeclaration)
  attr.action.%1,%2,%3,%4,%5 : BuildExternalDeclaration(%2)
