/*
* Copyright (CtNode) [2020] Futurewei Technologies, Inc. All rights reverved.
*
* OpenArkFE is licensed under the Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*
*  http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR
* FIT FOR A PARTICULAR PURPOSE.
* See the Mulan PSL v2 for more details.
*/

#include "ast2mpl.h"

#define NOTYETIMPL(K) { if (mVerbose) { MNYI(K); }}

void A2M::ProcessPackage(TreeNode *tnode) {
  NOTYETIMPL("ProcessPackage()");
  return;
}

void A2M::ProcessImport(TreeNode *tnode) {
  NOTYETIMPL("ProcessImport()");
  return;
}

void A2M::ProcessIdentifier(TreeNode *tnode) {
  NOTYETIMPL("ProcessIdentifier()");
  return;
}

void A2M::ProcessField(TreeNode *tnode) {
  NOTYETIMPL("ProcessField()");
  return;
}

void A2M::ProcessDimension(TreeNode *tnode) {
  NOTYETIMPL("ProcessDimension()");
  return;
}

void A2M::ProcessAttr(TreeNode *tnode) {
  NOTYETIMPL("ProcessAttr()");
  return;
}

void A2M::ProcessPrimType(TreeNode *tnode) {
  NOTYETIMPL("ProcessPrimType()");
  return;
}

void A2M::ProcessUserType(TreeNode *tnode) {
  NOTYETIMPL("ProcessUserType()");
  return;
}

void A2M::ProcessCast(TreeNode *tnode) {
  NOTYETIMPL("ProcessCast()");
  return;
}

void A2M::ProcessParenthesis(TreeNode *tnode) {
  NOTYETIMPL("ProcessParenthesis()");
  return;
}

void A2M::ProcessVarList(TreeNode *tnode) {
  NOTYETIMPL("ProcessVarList()");
  return;
}

void A2M::ProcessExprList(TreeNode *tnode) {
  NOTYETIMPL("ProcessExprList()");
  return;
}

void A2M::ProcessLiteral(TreeNode *tnode) {
  NOTYETIMPL("ProcessLiteral()");
  return;
}

void A2M::ProcessUnaOperator(TreeNode *tnode) {
  NOTYETIMPL("ProcessUnaOperator()");
  return;
}

void A2M::ProcessBinOperator(TreeNode *tnode) {
  NOTYETIMPL("ProcessBinOperator()");
  return;
}

void A2M::ProcessTerOperator(TreeNode *tnode) {
  NOTYETIMPL("ProcessTerOperator()");
  return;
}

void A2M::ProcessLambda(TreeNode *tnode) {
  NOTYETIMPL("ProcessLambda()");
  return;
}

void A2M::ProcessBlock(TreeNode *tnode) {
  NOTYETIMPL("ProcessBlock()");
  return;
}

void A2M::ProcessFunction(TreeNode *tnode) {
  NOTYETIMPL("ProcessFunction()");
  return;
}

void A2M::ProcessClass(TreeNode *tnode) {
  NOTYETIMPL("ProcessClass()");
  ClassNode *classnode = static_cast<ClassNode *>(tnode);
  const char *name = classnode->GetName();
  GStrIdx strIdx = GlobalTables::GetStrTable().GetOrCreateStrIdxFromName(name);
  // MIRType *type = GlobalTables::GetTypeTable().GetOrCreateClassType(name, mMirModule);
  for (int i=0; i < classnode->GetMethodsNum(); i++) {
    ProcessFunction(classnode->GetMethod(i));
  }
  for (int i=0; i < classnode->GetFieldsNum(); i++) {
    ProcessField(classnode->GetField(i));
  }
  return;
}

void A2M::ProcessInterface(TreeNode *tnode) {
  NOTYETIMPL("ProcessInterface()");
  return;
}

void A2M::ProcessAnnotationType(TreeNode *tnode) {
  NOTYETIMPL("ProcessAnnotationType()");
  return;
}

void A2M::ProcessAnnotation(TreeNode *tnode) {
  NOTYETIMPL("ProcessAnnotation()");
  return;
}

void A2M::ProcessException(TreeNode *tnode) {
  NOTYETIMPL("ProcessException()");
  return;
}

void A2M::ProcessReturn(TreeNode *tnode) {
  NOTYETIMPL("ProcessReturn()");
  return;
}

void A2M::ProcessCondBranch(TreeNode *tnode) {
  NOTYETIMPL("ProcessCondBranch()");
  return;
}

void A2M::ProcessBreak(TreeNode *tnode) {
  NOTYETIMPL("ProcessBreak()");
  return;
}

void A2M::ProcessForLoop(TreeNode *tnode) {
  NOTYETIMPL("ProcessForLoop()");
  return;
}

void A2M::ProcessWhileLoop(TreeNode *tnode) {
  NOTYETIMPL("ProcessWhileLoop()");
  return;
}

void A2M::ProcessDoLoop(TreeNode *tnode) {
  NOTYETIMPL("ProcessDoLoop()");
  return;
}

void A2M::ProcessNew(TreeNode *tnode) {
  NOTYETIMPL("ProcessNew()");
  return;
}

void A2M::ProcessDelete(TreeNode *tnode) {
  NOTYETIMPL("ProcessDelete()");
  return;
}

void A2M::ProcessCall(TreeNode *tnode) {
  NOTYETIMPL("ProcessCall()");
  return;
}

void A2M::ProcessSwitchLabel(TreeNode *tnode) {
  NOTYETIMPL("ProcessSwitchLabel()");
  return;
}

void A2M::ProcessSwitchCase(TreeNode *tnode) {
  NOTYETIMPL("ProcessSwitchCase()");
  return;
}

void A2M::ProcessSwitch(TreeNode *tnode) {
  NOTYETIMPL("ProcessSwitch()");
  return;
}

void A2M::ProcessPass(TreeNode *tnode) {
  NOTYETIMPL("ProcessPass()");
  return;
}

