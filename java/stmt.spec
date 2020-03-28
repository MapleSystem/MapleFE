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

rule LocalVariableDeclarationStatement : LocalVariableDeclaration + ';'

rule LocalVariableDeclaration : ZEROORMORE(VariableModifier) + UnannType + VariableDeclaratorList
  attr.action: BuildDecl(%2, %3)
  attr.action: AddAttribute(%1)

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

rule Statement : ONEOF(
  LocalVariableDeclarationStatement,
  StatementWithoutTrailingSubstatement,
  LabeledStatement,
  IfThenStatement,
  IfThenElseStatement,
  WhileStatement,
  ForStatement)

rule StatementNoShortIf : ONEOF(
  StatementWithoutTrailingSubstatement,
  LabeledStatementNoShortIf,
  IfThenElseStatementNoShortIf,
  WhileStatementNoShortIf,
  ForStatementNoShortIf)

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

rule ExpressionStatement : StatementExpression + ';'

rule StatementExpression : ONEOF(
  Assignment,
  PreIncrementExpression,
  PreDecrementExpression,
  PostIncrementExpression,
  PostDecrementExpression,
  MethodInvocation,
  ClassInstanceCreationExpression)

rule IfThenStatement : "if" + '(' + Expression + ')' + Statement

rule IfThenElseStatement : "if" + '(' + Expression + ')' + StatementNoShortIf + "else" + Statement

rule IfThenElseStatementNoShortIf : "if" + '(' + Expression + ')' + StatementNoShortIf + "else" + StatementNoShortIf

rule AssertStatement : ONEOF(
  "assert" + Expression + ';',
  "assert" + Expression + ':' + Expression + ';')

rule SwitchStatement : "switch" + '(' + Expression + ')' + SwitchBlock

rule SwitchBlock : ZEROORMORE(ZEROORMORE(SwitchBlockStatementGroup) + ZEROORMORE(SwitchLabel))

rule SwitchBlockStatementGroup : SwitchLabels + BlockStatements

rule SwitchLabels : SwitchLabel + ZEROORMORE(SwitchLabel)

rule SwitchLabel : ONEOF(
  "case" + ConstantExpression + ':',
  "case" + EnumConstantName + ':',
  "default" + ':')

rule EnumConstantName : Identifier

rule WhileStatement : "while" + '(' + Expression + ')' + Statement

rule WhileStatementNoShortIf : "while" + '(' + Expression + ')' + StatementNoShortIf

rule DoStatement : "do" + Statement + "while" + '(' + Expression + ')' + ';'

rule ForStatement : ONEOF(
  BasicForStatement,
  EnhancedForStatement)

rule ForStatementNoShortIf : ONEOF(
  BasicForStatementNoShortIf,
  EnhancedForStatementNoShortIf)

rule BasicForStatement : "for" + '(' + ZEROORONE(ForInit) + ';' + ZEROORONE(Expression) + ';' + ZEROORONE(ForUpdate) + ')' + Statement

rule BasicForStatementNoShortIf : "for" + '(' + ZEROORONE(ForInit) + ';' + ZEROORONE(Expression) + ';' + ZEROORONE(ForUpdate) + ')' + StatementNoShortIf

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

