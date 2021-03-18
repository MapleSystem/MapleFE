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


rule PackageName : ONEOF(Identifier, PackageName + '.' + Identifier)
  attr.action.%2 : BuildField(%1, %3)

rule TypeName    : ONEOF(Identifier, PackageOrTypeName + '.' + Identifier)
  attr.action.%2 : BuildField(%1, %3)

rule PackageOrTypeName : ONEOF(Identifier, PackageOrTypeName + '.' + Identifier)
  attr.action.%2 : BuildField(%1, %3)

rule ExpressionName : ONEOF(Identifier, AmbiguousName + '.' + Identifier)
  attr.action.%2 : BuildField(%1, %3)

rule MethodName     : Identifier

rule AmbiguousName  : ONEOF(Identifier, AmbiguousName + '.' + Identifier)
  attr.action.%2 : BuildField(%1, %3)

rule Class : "class"
rule ClassLiteral : ONEOF(TypeName + ZEROORMORE('[' + ']') + '.' + Class,
                          NumericType + ZEROORMORE('[' + ']') + '.' + Class,
                          "boolean" + ZEROORMORE('[' + ']') + '.' + Class,
                          "void" + '.' + Class)
  attr.property : Single

rule PrimaryNoNewArray_single : ONEOF(
  Literal,
  ClassLiteral,
  "this",
  TypeName + '.' + "this",
  '(' + Expression + ')',
  ClassInstanceCreationExpression)
  attr.action.%4 : BuildField(%1, %3)
  attr.action.%5 : BuildParenthesis(%2)
  attr.property : Single

rule PrimaryNoNewArray : ONEOF(
  PrimaryNoNewArray_single,
  FieldAccess,
  ArrayAccess,
  MethodInvocation,
  MethodReference)

# There was a child rule.
#  ExpressionName + '.' + UnqualifiedClassInstanceCreationExpression,
# But Primary contains ExpressionName. It's a duplication, so I removed it.
rule ClassInstanceCreationExpression : ONEOF(
  UnqualifiedClassInstanceCreationExpression,
  Primary + '.' + UnqualifiedClassInstanceCreationExpression)

rule UnqualifiedClassInstanceCreationExpression :
  "new" + ZEROORONE(TypeArguments) + ClassOrInterfaceTypeToInstantiate +
  '(' + ZEROORONE(ArgumentList) + ')' + ZEROORONE(ClassBody)
  attr.action : BuildNewOperation(%3, %5, %7)

rule ClassOrInterfaceTypeToInstantiate :
  ZEROORMORE(Annotation) + Identifier + ZEROORMORE('.' + ZEROORMORE(Annotation) + Identifier) +
  ZEROORONE(TypeArgumentsOrDiamond)
  attr.action : BuildUserType(%2)
  attr.action : AddTypeArgument(%4)

rule TypeArgumentsOrDiamond : ONEOF(
  TypeArguments,
  "<>")

rule ArgumentList : Expression + ZEROORMORE(',' + Expression)

rule ArrayInitializer : '{' + ZEROORONE(VariableInitializerList) + ZEROORONE(',') + '}'
rule ArrayCreationExpression : ONEOF(
  "new" + PrimitiveType + DimExprs + ZEROORONE(Dims),
  "new" + ClassOrInterfaceType + DimExprs + ZEROORONE(Dims),
  "new" + PrimitiveType + Dims + ArrayInitializer,
  "new" + ClassOrInterfaceType + Dims + ArrayInitializer)

rule DimExprs : DimExpr + ZEROORMORE(DimExpr)

rule DimExpr : ZEROORMORE(Annotation) + '[' + Expression + ']'

rule ArrayAccess : ONEOF(
  ExpressionName + '[' + Expression + ']',
  PrimaryNoNewArray + '[' + Expression + ']')

rule FieldAccess : ONEOF(
  Primary + '.' + Identifier,
  "super" + '.' + Identifier,
  TypeName + '.' + "super" + '.' + Identifier)
  attr.action.%1 : BuildField(%1, %3)

# It's possible MethodInvocation includes a MethodReference, like
#  A::B(a,b)
rule MethodInvocation : ONEOF(
  MethodName + '(' + ZEROORONE(ArgumentList) + ')',
  TypeName + '.' + ZEROORONE(TypeArguments) + Identifier + '(' + ZEROORONE(ArgumentList) + ')',
  ExpressionName + '.' + ZEROORONE(TypeArguments) + Identifier + '(' + ZEROORONE(ArgumentList) + ')',
  Primary + '.' + ZEROORONE(TypeArguments) + Identifier + '(' + ZEROORONE(ArgumentList) + ')',
  "super" + '.' + ZEROORONE(TypeArguments) + Identifier + '(' + ZEROORONE(ArgumentList) + ')',
  TypeName + '.' + "super" + '.' + ZEROORONE(TypeArguments) + Identifier + '(' + ZEROORONE(ArgumentList) + ')')
  attr.action.%1 : BuildCall(%1)
  attr.action.%1 : AddArguments(%3)
  attr.action.%2 : BuildField(%1, %4)
  attr.action.%2 : BuildCall()
  attr.action.%2 : AddArguments(%6)
  attr.action.%3,%4,%5 : BuildField(%1, %4)
  attr.action.%3,%4,%5 : BuildCall()
  attr.action.%3,%4,%5 : AddArguments(%6)

rule ArgumentList : Expression + ZEROORMORE(',' + Expression)
  attr.action.%1: BuildExprList(%1, %2)

rule MethodReference : ONEOF(
  ExpressionName + "::" + ZEROORONE(TypeArguments) + Identifier,
  Primary + "::" + ZEROORONE(TypeArguments) + Identifier,
  ReferenceType + "::" + ZEROORONE(TypeArguments) + Identifier,
  "super" + "::" + ZEROORONE(TypeArguments) + Identifier,
  TypeName + '.' + "super" + "::" + ZEROORONE(TypeArguments) + Identifier,
  ClassType + "::" + ZEROORONE(TypeArguments) "new",
  ArrayType + "::" + "new")

rule PostfixExpression : ONEOF(
  Primary,
  ExpressionName,
  PostIncrementExpression,
  PostDecrementExpression)

rule PostIncrementExpression : PostfixExpression + "++"
  attr.action : BuildPostfixOperation(%2, %1)
rule PostDecrementExpression : PostfixExpression + "--"
  attr.action : BuildPostfixOperation(%2, %1)

rule UnaryExpression : ONEOF(
  PreIncrementExpression,
  PreDecrementExpression,
  '+' + UnaryExpression,
  '-' + UnaryExpression,
  UnaryExpressionNotPlusMinus)
  attr.action.%3,%4 : BuildUnaryOperation(%1, %2)

rule PreIncrementExpression : "++" + UnaryExpression
  attr.action : BuildUnaryOperation(%1, %2)

rule PreDecrementExpression : "--" + UnaryExpression
  attr.action : BuildUnaryOperation(%1, %2)

rule UnaryExpressionNotPlusMinus : ONEOF(
  PostfixExpression,
  '~' + UnaryExpression,
  '!' + UnaryExpression,
  CastExpression)

rule CastExpression : ONEOF(
  '(' + PrimitiveType + ')' + UnaryExpression,
  '(' + ReferenceType + ZEROORMORE(AdditionalBound) + ')' + UnaryExpressionNotPlusMinus,
  '(' + ReferenceType + ZEROORMORE(AdditionalBound) + ')' + LambdaExpression)
  attr.action.%1 : BuildCast(%2, %4)
  attr.action.%2,%3 : BuildCast(%2, %5)

rule MultiplicativeExpression : ONEOF(
  UnaryExpression,
  MultiplicativeExpression + '*' + UnaryExpression,
  MultiplicativeExpression + '/' + UnaryExpression,
  MultiplicativeExpression + '%' + UnaryExpression)
  attr.action.%2,%3,%4 : BuildBinaryOperation(%1, %2, %3)

rule AdditiveExpression : ONEOF(
  MultiplicativeExpression,
  AdditiveExpression + '+' + MultiplicativeExpression,
  AdditiveExpression + '-' + MultiplicativeExpression)
  attr.action.%2,%3 : BuildBinaryOperation(%1, %2, %3)

rule ShiftExpression : ONEOF(
  AdditiveExpression,
  ShiftExpression + "<<" + AdditiveExpression,
  ShiftExpression + ">>" + AdditiveExpression,
  ShiftExpression + ">>>" + AdditiveExpression)
  attr.action.%2,%3,%4 : BuildBinaryOperation(%1, %2, %3)

rule RelationalExpression : ONEOF(
  ShiftExpression,
  RelationalExpression + '<' + ShiftExpression,
  RelationalExpression + '>' + ShiftExpression,
  RelationalExpression + "<=" + ShiftExpression,
  RelationalExpression + ">=" + ShiftExpression,
  RelationalExpression + "instanceof" + ReferenceType)
  attr.action.%2,%3,%4,%5 : BuildBinaryOperation(%1, %2, %3)

rule EqualityExpression : ONEOF(
  RelationalExpression,
  EqualityExpression + "==" + RelationalExpression,
  EqualityExpression + "!=" + RelationalExpression)
  attr.action.%2,%3 : BuildBinaryOperation(%1, %2, %3)

rule AndExpression : ONEOF(
  EqualityExpression,
  AndExpression + '&' + EqualityExpression)
  attr.action.%2 : BuildBinaryOperation(%1, %2, %3)

rule ExclusiveOrExpression : ONEOF(
  AndExpression,
  ExclusiveOrExpression + '^' + AndExpression)
  attr.action.%2 : BuildBinaryOperation(%1, %2, %3)

rule InclusiveOrExpression : ONEOF(
  ExclusiveOrExpression,
  InclusiveOrExpression + '|' + ExclusiveOrExpression)
  attr.action.%2 : BuildBinaryOperation(%1, %2, %3)

rule ConditionalAndExpression : ONEOF(
  InclusiveOrExpression,
  ConditionalAndExpression + "&&" + InclusiveOrExpression)
  attr.action.%2 : BuildBinaryOperation(%1, %2, %3)

rule ConditionalOrExpression : ONEOF(
  ConditionalAndExpression,
  ConditionalOrExpression + "||" + ConditionalAndExpression)
  attr.action.%2 : BuildBinaryOperation(%1, %2, %3)

rule ConditionalExpression : ONEOF(
  ConditionalOrExpression,
  ConditionalOrExpression + '?' + Expression + ':' + ConditionalExpression,
  ConditionalOrExpression + '?' + Expression + ':' + LambdaExpression)

rule AssignmentExpression : ONEOF(
  ConditionalExpression,
  Assignment)

rule Assignment : LeftHandSide + AssignmentOperator + Expression
  attr.action : BuildAssignment(%1, %2, %3)

rule LeftHandSide : ONEOF(
  ExpressionName,
  FieldAccess,
  ArrayAccess)

rule AssignmentOperator : ONEOF('=', "*=", "/=", "%=", "+=", "-=", "<<=", ">>=", ">>>=", "&=", "^=", "|=")

rule LambdaExpression : LambdaParameters + "->" + LambdaBody
  attr.action : BuildLambda(%1, %3)

rule LambdaParameters : ONEOF(
  '(' + ZEROORONE(LambdaParameterList) + ')',
  Identifier)

rule LambdaParameterList : ONEOF(
  LambdaParameter + ZEROORMORE(',' + LambdaParameter),
  Identifier + ZEROORMORE(',' + Identifier))

rule LambdaParameter : ONEOF(
  ZEROORMORE(VariableModifier) + LambdaParameterType + VariableDeclaratorId,
  VariableArityParameter)

rule LambdaParameterType : ONEOF(UnannType, "var")

rule VariableArityParameter : ZEROORMORE(VariableModifier) + UnannType + ZEROORMORE(Annotation) + "..." + Identifier

rule LambdaBody : ONEOF(Expression, Block)

rule ConstantExpression : Expression

rule Primary : ONEOF(
  PrimaryNoNewArray,
  ArrayCreationExpression)

rule Expression : ONEOF(
  ExpressionName,
  Primary,
  UnaryExpression,
  BinaryExpression,
  ConditionalExpression,
  LambdaExpression,
  AssignmentExpression)

rule BinaryExpression : ONEOF (
  MultiplicativeExpression,
  AdditiveExpression,
  ShiftExpression,
  RelationalExpression,
  EqualityExpression,
  AndExpression,
  ExclusiveOrExpression,
  InclusiveOrExpression,
  ConditionalAndExpression,
  ConditionalOrExpression)

