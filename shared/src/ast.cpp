/*
* Copyright (C) [2020] Futurewei Technologies, Inc. All rights reverved.
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

#include "ast.h"
#include "ast_type.h"
#include "ast_builder.h"
#include "parser.h"
#include "container.h"
#include "token.h"
#include "ruletable.h"

#include "massert.h"

namespace maplefe {

//////////////////////////////////////////////////////////////////////////////////////
//                          Utility    Functions
//////////////////////////////////////////////////////////////////////////////////////

#undef  OPERATOR
#define OPERATOR(T, D)  {OPR_##T, D},
OperatorDesc gOperatorDesc[OPR_NA] = {
#include "supported_operators.def"
};

unsigned GetOperatorProperty(OprId id) {
  for (unsigned i = 0; i < OPR_NA; i++) {
    if (gOperatorDesc[i].mOprId == id)
      return gOperatorDesc[i].mDesc;
  }
  MERROR("I shouldn't reach this point.");
}

#undef  OPERATOR
#define OPERATOR(T, D) case OPR_##T: return #T;
static const char* GetOperatorName(OprId opr) {
  switch (opr) {
#include "supported_operators.def"
  default:
    return "NA";
  }
};

//////////////////////////////////////////////////////////////////////////////////////
//                               TreeNode
//////////////////////////////////////////////////////////////////////////////////////

void TreeNode::AddAsTypes(TreeNode *type) {
  if (!type)
    return;

  if (type->IsPass()) {
    PassNode *pass_node = (PassNode*)type;
    for (unsigned i = 0; i < pass_node->GetChildrenNum(); i++)
      AddAsTypes(pass_node->GetChild(i));
  } else if (type->IsAsType()) {
    AsTypeNode *asn = (AsTypeNode*)type;
    AddAsType(asn);
  } else {
    MERROR("unsupported as-type in AddAsType.");
  }
}

// return true iff:
//   both are type nodes, either UserTypeNode or PrimTypeNode, and
//   they are type equal.
bool TreeNode::TypeEquivalent(TreeNode *t) {
  if (IsUserType() && t->IsUserType()) {
    UserTypeNode *this_t = (UserTypeNode *)this;
    UserTypeNode *that_t = (UserTypeNode *)t;
    if (this_t->TypeEquivalent(that_t))
      return true;
  }

  if (IsPrimType() && t->IsPrimType() && (this == t))
    return true;

  return false;
}

void TreeNode::DumpLabel(unsigned ind) {
  TreeNode *label = GetLabel();
  if (label) {
    MASSERT(label->IsIdentifier() && "Label is not an identifier.");
    IdentifierNode *inode = (IdentifierNode*)label;
    for (unsigned i = 0; i < ind; i++)
      DUMP0_NORETURN(' ');
    DUMP0_NORETURN(inode->GetName());
    DUMP0_NORETURN(':');
    DUMP_RETURN();
  }
}

void TreeNode::DumpIndentation(unsigned ind) {
  for (unsigned i = 0; i < ind; i++)
    DUMP0_NORETURN(' ');
}

//////////////////////////////////////////////////////////////////////////////////////
//                          PackageNode
//////////////////////////////////////////////////////////////////////////////////////

void PackageNode::Dump(unsigned indent) {
  DumpIndentation(indent);
  DUMP0_NORETURN("package ");
  mPackage->Dump(0);
}

//////////////////////////////////////////////////////////////////////////////////////
//                          DeclareNode
//////////////////////////////////////////////////////////////////////////////////////

void DeclareNode::Dump(unsigned indent) {
  DumpIndentation(indent);
  DUMP0_NORETURN("declare ");

  if (mDecl)
    mDecl->Dump(0);
}

//////////////////////////////////////////////////////////////////////////////////////
//                          ImportNode
//////////////////////////////////////////////////////////////////////////////////////

void ImportNode::AddPair(TreeNode *t) {
  if (t->IsPass()) {
    PassNode *n = (PassNode*)t;
    for (unsigned i = 0; i < n->GetChildrenNum(); i++) {
      TreeNode *child = n->GetChild(i);
      AddPair(child);
    }
  } else if (t->IsXXportAsPair()) {
    mPairs.PushBack((XXportAsPairNode*)t);
  } else {
    // We create a new pair to save 't'.
    XXportAsPairNode *n = (XXportAsPairNode*)gTreePool.NewTreeNode(sizeof(XXportAsPairNode));
    new (n) XXportAsPairNode();
    n->SetBefore(t);
    mPairs.PushBack(n);
  }
}

void ImportNode::Dump(unsigned indent) {
  DumpIndentation(indent);
  DUMP0_NORETURN("import ");
  if (IsImportStatic())
    DUMP0_NORETURN("static ");

  if (mPairs.GetNum() > 0) {
    DUMP0_NORETURN('{');
    for (unsigned i = 0; i < mPairs.GetNum(); i++) {
      XXportAsPairNode *p = GetPair(i);
      p->Dump(0);
      if (i < mPairs.GetNum() - 1)
        DUMP0_NORETURN(',');
    }
    DUMP0_NORETURN("} ");
  }

  if (mTarget)
    mTarget->Dump(0);
}

//////////////////////////////////////////////////////////////////////////////////////
//                          ExportNode
//////////////////////////////////////////////////////////////////////////////////////

void ExportNode::AddPair(TreeNode *t) {
  if (t->IsPass()) {
    PassNode *n = (PassNode*)t;
    for (unsigned i = 0; i < n->GetChildrenNum(); i++) {
      TreeNode *child = n->GetChild(i);
      AddPair(child);
    }
  } else if (t->IsXXportAsPair()) {
    mPairs.PushBack((XXportAsPairNode*)t);
    SETPARENT(t);
  } else {
    // We create a new pair to save 't'.
    XXportAsPairNode *n = (XXportAsPairNode*)gTreePool.NewTreeNode(sizeof(XXportAsPairNode));
    new (n) XXportAsPairNode();
    n->SetBefore(t);
    mPairs.PushBack(n);
    SETPARENT(n);
  }
}

void ExportNode::Dump(unsigned indent) {
  DumpIndentation(indent);
  DUMP0_NORETURN("export ");

  if (mPairs.GetNum() > 0) {
    DUMP0_NORETURN('{');
    for (unsigned i = 0; i < mPairs.GetNum(); i++) {
      XXportAsPairNode *p = GetPair(i);
      p->Dump(0);
      if (i < mPairs.GetNum() - 1)
        DUMP0_NORETURN(',');
    }
    DUMP0_NORETURN("} ");
  }

  if (mTarget)
    mTarget->Dump(0);
}

//////////////////////////////////////////////////////////////////////////////////////
//                          XXportAsPair
//////////////////////////////////////////////////////////////////////////////////////

void XXportAsPairNode::Dump(unsigned indent) {
  DumpIndentation(indent);
  if (IsEverything()) {
    DUMP0_NORETURN(" *");
    if (mBefore) {
      DUMP0_NORETURN(" as ");
      mBefore->Dump(0);
    }
  } else if (IsDefault()) {
    DUMP0_NORETURN(" default");
    if (mBefore) {
      DUMP0_NORETURN(" as ");
      mBefore->Dump(0);
    }
  } else {
    MASSERT(mBefore);
    mBefore->Dump(0);
    if (mAfter) {
      DUMP0_NORETURN(" as ");
      mAfter->Dump(0);
    }
  }
}

//////////////////////////////////////////////////////////////////////////////////////
//                          ParenthesisNode
//////////////////////////////////////////////////////////////////////////////////////

void ParenthesisNode::Dump(unsigned indent) {
  DumpIndentation(indent);
  DUMP0_NORETURN('(');
  mExpr->Dump(0);
  DUMP0_NORETURN(')');
}

//////////////////////////////////////////////////////////////////////////////////////
//                          AnnotationTypeNode
//////////////////////////////////////////////////////////////////////////////////////

void AnnotationTypeNode::Dump(unsigned indent) {
  DumpIndentation(indent);
  DUMP0_NORETURN("annotation type : ");
  mId->Dump(0);
}

//////////////////////////////////////////////////////////////////////////////////////
//                          CastNode
//////////////////////////////////////////////////////////////////////////////////////

const char* CastNode::GetDumpName() {
  std::string name = "(";
  name += mDestType->GetName();
  name += ")";
  name += mExpr->GetName();
  return gStringPool.FindString(name);
}

void CastNode::Dump(unsigned indent) {
  DumpIndentation(indent);
  DUMP0_NORETURN('(');
  mDestType->Dump(0);
  DUMP0_NORETURN(')');
  mExpr->Dump(0);
}

//////////////////////////////////////////////////////////////////////////////////////
//                          AssertNode
//////////////////////////////////////////////////////////////////////////////////////

void AssertNode::Dump(unsigned indent) {
  DumpIndentation(indent);
  DUMP0_NORETURN("assert ");
  mExpr->Dump(0);
  DUMP0_NORETURN(" : ");
  if (mMsg)
    mMsg->Dump(0);
}

//////////////////////////////////////////////////////////////////////////////////////
//                          TerOpeartorNode
//////////////////////////////////////////////////////////////////////////////////////

void TerOperatorNode::Dump(unsigned indent) {
  DumpIndentation(indent);
  mOpndA->Dump(0);
  DUMP0_NORETURN(" ? ");
  mOpndB->Dump(0);
  DUMP0_NORETURN(" : ");
  mOpndC->Dump(0);
}

//////////////////////////////////////////////////////////////////////////////////////
//                          BinOperatorNode
//////////////////////////////////////////////////////////////////////////////////////

// It's caller's duty to assure old_child is a child
void BinOperatorNode::ReplaceChild(TreeNode *old_child, TreeNode *new_child) {
  if (mOpndA == old_child) {
    mOpndA = new_child;
  } else if (mOpndB == old_child) {
    mOpndB = new_child;
  } else {
    MERROR("To-be-replaced node is not a child of BinOperatorNode?");
  }
}

void BinOperatorNode::Dump(unsigned indent) {
  const char *name = GetOperatorName(mOprId);
  DumpIndentation(indent);
  mOpndA->Dump(0);
  DUMP0_NORETURN(' ');
  DUMP0_NORETURN(name);
  DUMP0_NORETURN(' ');
  mOpndB->Dump(0);
}

//////////////////////////////////////////////////////////////////////////////////////
//                           UnaOperatorNode
//////////////////////////////////////////////////////////////////////////////////////

// It's caller's duty to assure old_child is a child
void UnaOperatorNode::ReplaceChild(TreeNode *old_child, TreeNode *new_child) {
  MASSERT((mOpnd == old_child) && "To-be-replaced node is not a child?");
  SetOpnd(new_child);
}

void UnaOperatorNode::Dump(unsigned indent) {
  const char *name = GetOperatorName(mOprId);
  DumpIndentation(indent);
  if (IsPost()) {
    mOpnd->Dump(0);
    DUMP0_NORETURN(' ');
    DUMP0(name);
  } else {
    DUMP0_NORETURN(name);
    DUMP0_NORETURN(' ');
    mOpnd->Dump(0);
  }
}

//////////////////////////////////////////////////////////////////////////////////////
//                           FieldNode
//////////////////////////////////////////////////////////////////////////////////////

void FieldNode::Dump(unsigned indent) {
  DumpIndentation(indent);
  mUpper->Dump(0);
  DUMP0_NORETURN('.');
  mField->Dump(0);
}

//////////////////////////////////////////////////////////////////////////////////////
//                           TypeAlias Node
//////////////////////////////////////////////////////////////////////////////////////

void TypeAliasNode::Dump(unsigned indent) {
  DumpIndentation(indent);
  DUMP0_NORETURN(" type ");
  mId->Dump(0);
  DUMP0_NORETURN(" = ");
  mAlias->Dump(0);
}

//////////////////////////////////////////////////////////////////////////////////////
//                           AsType Node
//////////////////////////////////////////////////////////////////////////////////////

void AsTypeNode::Dump(unsigned indent) {
  DumpIndentation(indent);
  DUMP0_NORETURN(" as ");
  TreeNode *type = GetType();
  type->Dump(0);
}

//////////////////////////////////////////////////////////////////////////////////////
//                           type parameter & type argument
//////////////////////////////////////////////////////////////////////////////////////

void TypeParameterNode::Dump(unsigned indent) {
  DumpIndentation(indent);
  TreeNode *id = GetId();
  DUMP0_NORETURN(id->GetName());
  if (mDefault) {
    DUMP0_NORETURN("=");
    DUMP0_NORETURN(mDefault->GetName());
  }
}

//////////////////////////////////////////////////////////////////////////////////////
//                           NewNode
//////////////////////////////////////////////////////////////////////////////////////

// It's caller's duty to assure old_child is a child
void NewNode::ReplaceChild(TreeNode *old_child, TreeNode *new_child) {
  if (mId == old_child) {
    SetId(new_child);
    return;
  } else {
    for (unsigned i = 0; i < GetArgsNum(); i++) {
      if (GetArg(i) == old_child)
        mArgs.SetElem(i, new_child);
    }
  }
}

void NewNode::Dump(unsigned indent) {
  DumpIndentation(indent);
  DUMP0_NORETURN("new ");
  TreeNode *id = GetId();
  DUMP0_NORETURN(id->GetName());
  DUMP0_NORETURN("(");
  for (unsigned i = 0; i < GetArgsNum(); i++) {
    TreeNode *arg = GetArg(i);
    arg->Dump(0);
    if (i < GetArgsNum() - 1)
      DUMP0_NORETURN(",");
  }
  DUMP0_NORETURN(")");
}

//////////////////////////////////////////////////////////////////////////////////////
//                          DeleteNode
//////////////////////////////////////////////////////////////////////////////////////

void DeleteNode::Dump(unsigned indent) {
  DumpIndentation(indent);
  DUMP0_NORETURN(" delete ");
  if (mExpr)
    mExpr->Dump(0);
}

//////////////////////////////////////////////////////////////////////////////////////
//                          CallNode
//////////////////////////////////////////////////////////////////////////////////////

void CallNode::AddTypeArgument(TreeNode *arg) {
  if (arg->IsPass()) {
    PassNode *n = (PassNode*)arg;
    for (unsigned i = 0; i < n->GetChildrenNum(); i++) {
      TreeNode *child = n->GetChild(i);
      AddTypeArgument(child);
    }
  } else {
    mTypeArguments.PushBack(arg);
  }
}

void CallNode::Dump(unsigned indent) {
  DumpIndentation(indent);
  mMethod->Dump(0);
  if (GetTypeArgumentsNum() > 0) {
    DUMP0_NORETURN("<");
    for (unsigned i = 0; i < GetTypeArgumentsNum(); i++) {
      TreeNode *arg = GetTypeArgumentAtIndex(i);
      arg->Dump(0);
      if (i < GetTypeArgumentsNum() - 1)
        DUMP0_NORETURN(",");
    }
    DUMP0_NORETURN(">");
  }

  DUMP0_NORETURN("(");
  mArgs.Dump(0);
  DUMP0_NORETURN(")");
}

//////////////////////////////////////////////////////////////////////////////////////
//                          DimensionNode
//////////////////////////////////////////////////////////////////////////////////////

// Merge 'node' into 'this'.
void DimensionNode::Merge(const TreeNode *node) {
  if (!node)
    return;

  if (node->IsDimension()) {
    DimensionNode *n = (DimensionNode *)node;
    for (unsigned i = 0; i < n->GetDimensionsNum(); i++)
      AddDimension(n->GetDimension(i));
  } else if (node->IsPass()) {
    PassNode *n = (PassNode*)node;
    for (unsigned i = 0; i < n->GetChildrenNum(); i++) {
      TreeNode *child = n->GetChild(i);
      Merge(child);
    }
  } else {
    MERROR("DimensionNode.Merge() cannot handle the node");
  }
}

//////////////////////////////////////////////////////////////////////////////////////
//                          IdentifierNode
//////////////////////////////////////////////////////////////////////////////////////

void IdentifierNode::Release() {
  if (mDims)
     mDims->Release();
  mAttrs.Release();
  mAnnotations.Release();
}

void IdentifierNode::Dump(unsigned indent) {
  DumpIndentation(indent);
  if (mRestParam)
    DUMP0_NORETURN("...");
  DUMP0_NORETURN(GetName());
  if (mOptionalParam || mIsOptional)
    DUMP0_NORETURN('?');
  if (IsNonNull())
    DUMP0_NORETURN('!');
  if (mInit) {
    DUMP0_NORETURN('=');
    mInit->Dump(0);
  }

  if (IsArray()){
    for (unsigned i = 0; i < GetDimsNum(); i++)
      DUMP0_NORETURN("[]");
  }
}

//////////////////////////////////////////////////////////////////////////////////////
//                          DeclNode
//////////////////////////////////////////////////////////////////////////////////////

void DeclNode::Dump(unsigned indent) {
  DumpIndentation(indent);
  switch (mProp) {
  case JS_Var:
    DUMP0_NORETURN("js_var ");
    break;
  case JS_Let:
    DUMP0_NORETURN("js_let ");
    break;
  case JS_Const:
    DUMP0_NORETURN("js_const ");
    break;
  default:
    break;
  }
  DUMP0_NORETURN("Decl: ");
  mVar->Dump(0);
}

//////////////////////////////////////////////////////////////////////////////////////
//                          ArrayElement and ArrayLiteral
//////////////////////////////////////////////////////////////////////////////////////

void ArrayElementNode::Dump(unsigned indent) {
  DumpIndentation(indent);
  mArray->Dump(0);
  for (unsigned i = 0; i < mExprs.GetNum(); i++) {
    DUMP0_NORETURN("[");
    mExprs.ValueAtIndex(i)->Dump(0);
    DUMP0_NORETURN("]");
  }
}

void ArrayLiteralNode::Dump(unsigned indent) {
  DumpIndentation(indent);
  DUMP0_NORETURN("[");
  for (unsigned i = 0; i < GetLiteralsNum(); i++) {
    GetLiteral(i)->Dump(0);
    if (i < GetLiteralsNum() - 1)
      DUMP0_NORETURN(",");
  }
  DUMP0_NORETURN("]");
}

//////////////////////////////////////////////////////////////////////////////////////
//                          BindingElement and BindingPattern
//////////////////////////////////////////////////////////////////////////////////////

void BindingElementNode::Dump(unsigned indent) {
  DumpIndentation(indent);
  if (mVariable)
    mVariable->Dump(0);
  DUMP0_NORETURN(":");
  if (mElement)
    mElement->Dump(0);
}

void BindingPatternNode::AddElement(TreeNode *tree) {
  if (tree->IsBindingElement()) {
    mElements.PushBack(tree);
  } else if (tree->IsPass()) {
    PassNode *pass = (PassNode*)tree;
    for (unsigned i = 0; i < pass->GetChildrenNum(); i++) {
      TreeNode *child = pass->GetChild(i);
      AddElement(child);
    }
  } else {
    MERROR("Unsupported element of binding pattern.");
  }
}

void BindingPatternNode::Dump(unsigned indent) {
  DumpIndentation(indent);
  DUMP0_NORETURN("{");
  for (unsigned i = 0; i < mElements.GetNum(); i++) {
    TreeNode *elem = GetElement(i);
    elem->Dump(0);
    if (i != mElements.GetNum()-1)
      DUMP0_NORETURN(", ");
  }
  DUMP0_NORETURN("}");
}

//////////////////////////////////////////////////////////////////////////////////////
//                          StructNode
//////////////////////////////////////////////////////////////////////////////////////

// Child could be a field or index signature.
void StructNode::AddChild(TreeNode *field) {
  if (field->IsPass()) {
    PassNode *pass = (PassNode*)field;
    for (unsigned i = 0; i < pass->GetChildrenNum(); i++) {
      TreeNode *child = pass->GetChild(i);
      AddChild(child);
    }
  } else if (field->IsIdentifier()) {
    AddField((IdentifierNode*)field);
  } else if (field->IsNumIndexSig()) {
    SetNumIndexSig((NumIndexSigNode*)field);
  } else if (field->IsStrIndexSig()) {
    SetStrIndexSig((StrIndexSigNode*)field);
  } else
    MERROR("Unsupported struct field type.");
}

void NumIndexSigNode::Dump(unsigned indent) {
  DumpIndentation(indent);
  if (mDataType)
    mDataType->Dump(0);
}

void StrIndexSigNode::Dump(unsigned indent) {
  DumpIndentation(indent);
  if (mDataType)
    mDataType->Dump(0);
}

void StructNode::Dump(unsigned indent) {
  DumpIndentation(indent);
  switch (mProp) {
  case SProp_CStruct:
    DUMP0_NORETURN("struct: ");
    break;
  case SProp_TSInterface:
    DUMP0_NORETURN("ts_interface: ");
    break;
  case SProp_TSEnum:
    DUMP0_NORETURN("ts_enum: ");
    break;
  default:
    break;
  }
  mStructId->Dump(0);
  DUMP0_NORETURN(" {");

  if (mNumIndexSig) {
    DUMP0_NORETURN("numeric index type: ");
    mNumIndexSig->Dump(0);
  }

  if (mStrIndexSig) {
    DUMP0_NORETURN("string index type: ");
    mStrIndexSig->Dump(0);
  }

  for (unsigned i = 0; i < mFields.GetNum(); i++) {
    mFields.ValueAtIndex(i)->Dump(0);
    if (i != mFields.GetNum()-1)
      DUMP0_NORETURN(";");
  }
  DUMP0_NORETURN(" }");
}

//////////////////////////////////////////////////////////////////////////////////////
//                          StructLiteralNode
//////////////////////////////////////////////////////////////////////////////////////

void StructLiteralNode::AddField(TreeNode *tree) {
  if (tree->IsFieldLiteral()) {
    FieldLiteralNode *fl = (FieldLiteralNode*)tree;
    mFields.PushBack(fl);
  } else if (tree->IsFunction()) {
    FunctionNode *node = (FunctionNode*)tree;
    FieldLiteralNode *func_lit = (FieldLiteralNode*)gTreePool.NewTreeNode(sizeof(FieldLiteralNode));
    new (func_lit) FieldLiteralNode();
    MASSERT(node->GetFuncName()->IsIdentifier());
    func_lit->SetFieldName((IdentifierNode*)(node->GetFuncName()));
    func_lit->SetLiteral(node);
    mFields.PushBack(func_lit);
  } else if (tree->IsLiteral() || tree->IsIdentifier()) {
    FieldLiteralNode *fln = (FieldLiteralNode*)gTreePool.NewTreeNode(sizeof(FieldLiteralNode));
    new (fln) FieldLiteralNode();
    fln->SetLiteral(tree);
    mFields.PushBack(fln);
  } else if (tree->IsPass()) {
    PassNode *pass = (PassNode*)tree;
    for (unsigned i = 0; i < pass->GetChildrenNum(); i++) {
      TreeNode *child = pass->GetChild(i);
      AddField(child);
    }
  } else {
    MASSERT(0 && "unsupported.");
  }
}

void StructLiteralNode::Dump(unsigned indent) {
  DumpIndentation(indent);
  DUMP0_NORETURN(" {");
  for (unsigned i = 0; i < mFields.GetNum(); i++) {
    FieldLiteralNode *fl = GetField(i);
    if (fl->mFieldName)
      fl->mFieldName->Dump(0);
    DUMP0_NORETURN(":");
    fl->mLiteral->Dump(0);
    if (i != mFields.GetNum()-1)
      DUMP0_NORETURN(", ");
  }
  DUMP0_NORETURN("}");
}

//////////////////////////////////////////////////////////////////////////////////////
//                          VarListNode
//////////////////////////////////////////////////////////////////////////////////////

void VarListNode::AddVar(IdentifierNode *n) {
  mVars.PushBack(n);
}

// Merge a node.
// 'n' could be either IdentifierNode or another VarListNode.
void VarListNode::Merge(TreeNode *n) {
  if (n->IsIdentifier()) {
    AddVar((IdentifierNode*)n);
  } else if (n->IsVarList()) {
    VarListNode *varlist = (VarListNode*)n;
    for (unsigned i = 0; i < varlist->mVars.GetNum(); i++)
      AddVar(varlist->mVars.ValueAtIndex(i));
  } else if (n->IsPass()) {
    PassNode *p = (PassNode*)n;
    for (unsigned i = 0; i < p->GetChildrenNum(); i++) {
      TreeNode *child = p->GetChild(i);
      Merge(child);
    }
  } else {
    MERROR("VarListNode cannot merge a non-identifier or non-varlist node");
  }
}

void VarListNode::Dump(unsigned indent) {
  DumpIndentation(indent);
  for (unsigned i = 0; i < mVars.GetNum(); i++) {
    //DUMP0_NORETURN(mVars.ValueAtIndex(i)->GetName());
    mVars.ValueAtIndex(i)->Dump(0);
    if (i != mVars.GetNum()-1)
      DUMP0_NORETURN(",");
  }
}

//////////////////////////////////////////////////////////////////////////////////////
//                          NamespaceNode
//////////////////////////////////////////////////////////////////////////////////////

void NamespaceNode::AddBody(TreeNode *tree) {
  if (tree->IsPass()) {
    PassNode *p = (PassNode*)tree;
    for (unsigned i = 0; i < p->GetChildrenNum(); i++) {
      TreeNode *child = p->GetChild(i);
      AddBody(child);
    }
  } else {
    AddElement(tree);
  }
}

void NamespaceNode::Dump(unsigned indent) {
  DumpIndentation(indent);
  DUMP1_NORETURN("namespace ", GetName());
  DUMP_RETURN();
  for (unsigned i = 0; i < mElements.GetNum(); i++) {
    DumpIndentation(indent + 2);
    mElements.ValueAtIndex(i)->Dump(0);
    DUMP_RETURN();
  }
}

//////////////////////////////////////////////////////////////////////////////////////
//                          ExprListNode
//////////////////////////////////////////////////////////////////////////////////////

// Merge a node.
// 'n' could be either TreeNode or another ExprListNode.
void ExprListNode::Merge(TreeNode *n) {
  if (n->IsExprList()) {
    ExprListNode *expr_list = (ExprListNode*)n;
    for (unsigned i = 0; i < expr_list->GetExprsNum(); i++)
      AddExpr(expr_list->GetExprAtIndex(i));
  } else if (n->IsPass()) {
    PassNode *p = (PassNode*)n;
    for (unsigned i = 0; i < p->GetChildrenNum(); i++) {
      TreeNode *child = p->GetChild(i);
      Merge(child);
    }
  } else
    AddExpr(n);
}

void ExprListNode::Dump(unsigned indent) {
  DumpIndentation(indent);
  for (unsigned i = 0; i < mExprs.GetNum(); i++) {
    mExprs.ValueAtIndex(i)->Dump(0);
    if (i != mExprs.GetNum()-1)
      DUMP0_NORETURN(",");
  }
}

//////////////////////////////////////////////////////////////////////////////////////
//                          TemplateLiteralNode
//////////////////////////////////////////////////////////////////////////////////////

void TemplateLiteralNode::Dump(unsigned indent) {
  DumpIndentation(indent);
  DUMP0_NORETURN(" template-literal: ");
  for (unsigned i = 0; i < mTrees.GetNum(); i++) {
    TreeNode *n = mTrees.ValueAtIndex(i);
    if (n)
      n->Dump(0);
    else
      DUMP0_NORETURN("NULL");
    if (i < mStrings.GetNum() - 1)
      DUMP0_NORETURN(",");
  }
}

//////////////////////////////////////////////////////////////////////////////////////
//                          LiteralNode
//////////////////////////////////////////////////////////////////////////////////////

void LiteralNode::InitName() {
  std::string s;
  switch (mData.mType) {
  case LT_NullLiteral:
    s = "null";
    mStrIdx = gStringPool.GetStrIdx(s);
    break;
  case LT_ThisLiteral:
    s = "this";
    mStrIdx = gStringPool.GetStrIdx(s);
    break;
  case LT_IntegerLiteral:
  case LT_DoubleLiteral:
  case LT_FPLiteral:
  case LT_StringLiteral:
  case LT_BooleanLiteral:
  case LT_CharacterLiteral:
  case LT_NA:
  default:
    s = "<NA>";
    mStrIdx = gStringPool.GetStrIdx(s);
    break;
  }
}

void LiteralNode::Dump(unsigned indent) {
  DumpIndentation(indent);
  switch (mData.mType) {
  case LT_IntegerLiteral:
    DUMP0_NORETURN(mData.mData.mInt);
    break;
  case LT_DoubleLiteral:
    DUMP0_NORETURN(mData.mData.mDouble);
    break;
  case LT_FPLiteral:
    DUMP0_NORETURN(mData.mData.mFloat);
    break;
  case LT_StringLiteral:
    DUMP0_NORETURN(gStringPool.GetStringFromStrIdx(mData.mData.mStrIdx));
    break;
  case LT_BooleanLiteral:
    if(mData.mData.mBool == true)
      DUMP0_NORETURN("true");
    else if(mData.mData.mBool == false)
      DUMP0_NORETURN("false");
    break;
  case LT_CharacterLiteral: {
    Char the_char = mData.mData.mChar;
    if (the_char.mIsUnicode)
      DUMP0_NORETURN(the_char.mData.mUniValue);
    else
      DUMP0_NORETURN(the_char.mData.mChar);
    break;
  }
  case LT_NullLiteral:
    DUMP0_NORETURN("null");
    break;
  case LT_ThisLiteral:
    DUMP0_NORETURN("this");
    break;
  case LT_SuperLiteral:
    DUMP0_NORETURN("super");
    break;
  case LT_NA:
  default:
    DUMP0_NORETURN("NA Token:");
    break;
  }
}

//////////////////////////////////////////////////////////////////////////////////////
//                          ThrowNode
//////////////////////////////////////////////////////////////////////////////////////

void ThrowNode::AddException(TreeNode *t) {
  if (t->IsPass()) {
    PassNode *pass_node = (PassNode*)t;
    for (unsigned i = 0; i < pass_node->GetChildrenNum(); i++)
      AddException(pass_node->GetChild(i));
  } else {
    mExceptions.PushBack(t);
  }
}

void ThrowNode::Dump(unsigned indent) {
  DumpIndentation(indent);
  DUMP0_NORETURN("throw ");
  for (unsigned i = 0; i < GetExceptionsNum(); i++) {
    TreeNode *t = GetExceptionAtIndex(i);
    t->Dump(0);
    if (i < GetExceptionsNum() - 1)
      DUMP0_NORETURN(", ");
  }
  DUMP_RETURN();
}

//////////////////////////////////////////////////////////////////////////////////////
//                          Try, Catch, Finally nodes
//////////////////////////////////////////////////////////////////////////////////////

void TryNode::AddCatch(TreeNode *t) {
  if (t->IsPass()) {
    PassNode *pass_node = (PassNode*)t;
    for (unsigned i = 0; i < pass_node->GetChildrenNum(); i++)
      AddCatch(pass_node->GetChild(i));
  } else {
    MASSERT(t->IsCatch());
    mCatches.PushBack((CatchNode*)t);
  }
}

void TryNode::Dump(unsigned indent) {
  DumpIndentation(indent);
  DUMP0_NORETURN("try ");
  if (mBlock)
    mBlock->Dump(indent + 2);
  for (unsigned i = 0; i < GetCatchesNum(); i++) {
    CatchNode *c = GetCatchAtIndex(i);
    c->Dump(indent);
  }
  if (mFinally)
    mFinally->Dump(indent);
}

void CatchNode::AddParam(TreeNode *t) {
  if (t->IsPass()) {
    PassNode *pass_node = (PassNode*)t;
    for (unsigned i = 0; i < pass_node->GetChildrenNum(); i++)
      AddParam(pass_node->GetChild(i));
  } else {
    mParams.PushBack(t);
  }
}

void CatchNode::Dump(unsigned indent) {
  DumpIndentation(indent);
  DUMP0_NORETURN("catch(");
  DUMP0_NORETURN(")");
  if (mBlock)
    mBlock->Dump(indent + 2);
}

void FinallyNode::Dump(unsigned indent) {
  DumpIndentation(indent);
  DUMP0_NORETURN("finally ");
  if (mBlock)
    mBlock->Dump(indent + 2);
}

//////////////////////////////////////////////////////////////////////////////////////
//                          ExceptionNode
//////////////////////////////////////////////////////////////////////////////////////

void ExceptionNode::Dump(unsigned indent) {
  GetException()->Dump(indent);
}

//////////////////////////////////////////////////////////////////////////
//          Statement Node, Control Flow related nodes
//////////////////////////////////////////////////////////////////////////

void ReturnNode::Dump(unsigned ind) {
  DumpLabel(ind);
  DumpIndentation(ind);
  DUMP0_NORETURN("return ");
  if (GetResult())
    GetResult()->Dump(0);
}

CondBranchNode::CondBranchNode() : TreeNode(NK_CondBranch),
  mCond(NULL), mTrueBranch(NULL), mFalseBranch(NULL) {}

void CondBranchNode::Dump(unsigned ind) {
  DumpLabel(ind);
  DumpIndentation(ind);
  DUMP0_NORETURN("cond-branch cond:");
  mCond->Dump(0);
  DUMP_RETURN();
  DumpIndentation(ind);
  DUMP0("true branch :");
  if (mTrueBranch)
    mTrueBranch->Dump(ind+2);
  DumpIndentation(ind);
  DUMP0("false branch :");
  if (mFalseBranch)
    mFalseBranch->Dump(ind+2);
}

void BreakNode::Dump(unsigned ind) {
  DumpLabel(ind);
  DumpIndentation(ind);
  DUMP0_NORETURN("break:");
  if (GetTarget())
    GetTarget()->Dump(0);
  DUMP_RETURN();
}

void ContinueNode::Dump(unsigned ind) {
  DumpLabel(ind);
  DumpIndentation(ind);
  DUMP0_NORETURN("continue:");
  if (GetTarget())
    GetTarget()->Dump(0);
  DUMP_RETURN();
}

void ForLoopNode::Dump(unsigned ind) {
  DumpLabel(ind);
  DumpIndentation(ind);
  DUMP0_NORETURN("for ( ");

  DUMP0_NORETURN(")");
  DUMP_RETURN();
  if (GetBody())
    GetBody()->Dump(ind +2);
}

void WhileLoopNode::Dump(unsigned ind) {
  DumpIndentation(ind);
  DUMP0_NORETURN("while ");
  if (mCond)
    mCond->Dump(0);
  if (GetBody())
    GetBody()->Dump(ind +2);
}

void DoLoopNode::Dump(unsigned ind) {
  DumpIndentation(ind);
  DUMP0_NORETURN("do ");
  if (GetBody())
    GetBody()->Dump(ind +2);
  DUMP0_NORETURN("while ");
  if (mCond)
    mCond->Dump(0);
}

void SwitchLabelNode::Dump(unsigned ind) {
}

void SwitchCaseNode::AddLabel(TreeNode *t) {
  if (t->IsPass()) {
    PassNode *pass = (PassNode*)t;
    for (unsigned i = 0; i < pass->GetChildrenNum(); i++)
      AddLabel(pass->GetChild(i));
  } else {
    MASSERT(t->IsSwitchLabel());
    mLabels.PushBack((SwitchLabelNode*)t);
  }
}

void SwitchCaseNode::AddStmt(TreeNode *t) {
  if (t->IsPass()) {
    PassNode *pass = (PassNode*)t;
    for (unsigned i = 0; i < pass->GetChildrenNum(); i++)
      AddStmt(pass->GetChild(i));
  } else {
    mStmts.PushBack(t);
  }
}

void SwitchCaseNode::Dump(unsigned ind) {
}

void SwitchNode::Dump(unsigned ind) {
  DumpIndentation(ind);
  DUMP0("A switch");
}

//////////////////////////////////////////////////////////////////////////////////////
//                          BlockNode
//////////////////////////////////////////////////////////////////////////////////////

void BlockNode::AddChild(TreeNode *tree) {
  if (tree->IsPass()) {
    PassNode *passnode = (PassNode*)tree;
    for (unsigned j = 0; j < passnode->GetChildrenNum(); j++) {
      TreeNode *child = passnode->GetChild(j);
      AddChild(child);
    }
  } else {
    mChildren.PushBack(tree);
    tree->SetParent(this);
  }
}

void BlockNode::InsertStmtAfter(TreeNode *new_stmt, TreeNode *exist_stmt) {
  mChildren.LocateValue(exist_stmt);
  mChildren.InsertAfter(new_stmt);
}

void BlockNode::InsertStmtBefore(TreeNode *new_stmt, TreeNode *exist_stmt) {
  mChildren.LocateValue(exist_stmt);
  mChildren.InsertBefore(new_stmt);
}

void BlockNode::Dump(unsigned ind) {
  DumpLabel(ind);
  for (unsigned i = 0; i < GetChildrenNum(); i++) {
    TreeNode *child = GetChildAtIndex(i);
    child->Dump(ind);
    DUMP_RETURN();
  }
}

//////////////////////////////////////////////////////////////////////////////////////
//                          PassNode
//////////////////////////////////////////////////////////////////////////////////////

void PassNode::Dump(unsigned ind) {
  return; // disable the Dump
  DumpLabel(ind);
  for (unsigned i = 0; i < GetChildrenNum(); i++) {
    TreeNode *child = GetChild(i);
    child->Dump(ind);
    DUMP_RETURN();
  }
}

//////////////////////////////////////////////////////////////////////////////////////
//                          ClassNode
//////////////////////////////////////////////////////////////////////////////////////

// When the class body, a BlockNode, is added to the ClassNode, we need further
// categorize the subtrees into members, methods, local classes, interfaces, etc.
void ClassNode::Construct(BlockNode *block) {
  for (unsigned i = 0; i < block->GetChildrenNum(); i++) {
    TreeNode *tree_node = block->GetChildAtIndex(i);
    tree_node->SetParent(this);
    if (tree_node->IsDecl()) {
      DeclNode *decl = (DeclNode*)tree_node;
      TreeNode *var = decl->GetVar();
      if (var->IsVarList()) {
        VarListNode *vlnode = (VarListNode*)var;
        for (unsigned i = 0; i < vlnode->GetVarsNum(); i++) {
          IdentifierNode *inode = vlnode->GetVarAtIndex(i);
          inode->SetParent(this);
          mFields.PushBack(inode);
        }
      } else if (var->IsIdentifier()) {
        IdentifierNode *inode = (IdentifierNode*)var;
        mFields.PushBack(inode);
        inode->SetParent(this);
      } else
        MERROR("Unsupported class field.");
    } else if (tree_node->IsFunction()) {
      FunctionNode *f = (FunctionNode*)tree_node;
      if (f->IsConstructor())
        mConstructors.PushBack(f);
      else
        mMethods.PushBack(f);
      f->SetParent(this);
    } else if (tree_node->IsClass()) {
      mLocalClasses.PushBack((ClassNode*)tree_node);
      tree_node->SetParent(this);
    } else if (tree_node->IsInterface()) {
      mLocalInterfaces.PushBack((InterfaceNode*)tree_node);
      tree_node->SetParent(this);
    } else if (tree_node->IsBlock()) {
      BlockNode *block = (BlockNode*)tree_node;
      MASSERT(block->IsInstInit() && "unnamed block in class is not inst init?");
      mInstInits.PushBack(block);
      tree_node->SetParent(this);
    } else if (tree_node->IsImport()) {
      mImports.PushBack((ImportNode*)tree_node);
      tree_node->SetParent(this);
    } else if (tree_node->IsExport()) {
      mExports.PushBack((ExportNode*)tree_node);
      tree_node->SetParent(this);
    } else
      MASSERT("Unsupported tree node in class body.");
  }
}

// Release() only takes care of those container memory. The release of all tree nodes
// is taken care by the tree node pool.
void ClassNode::Release() {
  mSuperClasses.Release();
  mSuperInterfaces.Release();
  mAttributes.Release();
  mAnnotations.Release();
  mFields.Release();
  mMethods.Release();
  mLocalClasses.Release();
  mLocalInterfaces.Release();
  mImports.Release();
  mExports.Release();
}

void ClassNode::Dump(unsigned indent) {
  DumpIndentation(indent);
  if (IsJavaEnum())
    DUMP1_NORETURN("class[JavaEnum] ", GetName());
  else
    DUMP1_NORETURN("class ", GetName());
  DUMP_RETURN();

  DumpIndentation(indent + 2);
  DUMP0("Fields: ");
  for (unsigned i = 0; i < mFields.GetNum(); i++) {
    TreeNode *node = mFields.ValueAtIndex(i);
    node->Dump(indent + 4);
  }
  DUMP_RETURN();

  DumpIndentation(indent + 2);
  DUMP0("Instance Initializer: ");
  for (unsigned i = 0; i < mInstInits.GetNum(); i++) {
    TreeNode *node = mInstInits.ValueAtIndex(i);
    DumpIndentation(indent + 4);
    DUMP1("InstInit-", i);
  }

  DumpIndentation(indent + 2);
  DUMP0("Constructors: ");
  for (unsigned i = 0; i < mConstructors.GetNum(); i++) {
    TreeNode *node = mConstructors.ValueAtIndex(i);
    node->Dump(indent + 4);
  }

  DumpIndentation(indent + 2);
  DUMP0("Methods: ");
  for (unsigned i = 0; i < mMethods.GetNum(); i++) {
    TreeNode *node = mMethods.ValueAtIndex(i);
    node->Dump(indent + 4);
  }

  DumpIndentation(indent + 2);
  DUMP0("LocalClasses: ");
  for (unsigned i = 0; i < mLocalClasses.GetNum(); i++) {
    TreeNode *node = mLocalClasses.ValueAtIndex(i);
    node->Dump(indent + 4);
  }

  DumpIndentation(indent + 2);
  DUMP0("LocalInterfaces: ");
  for (unsigned i = 0; i < mLocalInterfaces.GetNum(); i++) {
    TreeNode *node = mLocalInterfaces.ValueAtIndex(i);
    node->Dump(indent + 4);
  }

  if (mImports.GetNum() > 0) {
    DumpIndentation(indent + 2);
    DUMP0("Imports: ");
  }
  for (unsigned i = 0; i < mImports.GetNum(); i++) {
    TreeNode *node = mImports.ValueAtIndex(i);
    node->Dump(indent + 4);
  }

  if (mExports.GetNum() > 0) {
    DumpIndentation(indent + 2);
    DUMP0("Exports: ");
  }
  for (unsigned i = 0; i < mExports.GetNum(); i++) {
    TreeNode *node = mExports.ValueAtIndex(i);
    node->Dump(indent + 4);
  }
}

//////////////////////////////////////////////////////////////////////////////////////
//                          FunctionNode
//////////////////////////////////////////////////////////////////////////////////////

FunctionNode::FunctionNode() : TreeNode(NK_Function),
  mFuncName(NULL), mType(NULL), mBody(NULL), mDims(NULL), mIsConstructor(false) {}

void FunctionNode::AddTypeParam(TreeNode *param) {
  if (param->IsPass()) {
    PassNode *n = (PassNode*)param;
    for (unsigned i = 0; i < n->GetChildrenNum(); i++) {
      TreeNode *child = n->GetChild(i);
      AddTypeParam(child);
    }
  } else {
    mTypeParams.PushBack(param);
  }
}

// This is to tell if both FunctionNodes have same return type
// and parameter types. So languages require Type Erasure at first, like Java.
// Type erasure should be done earlier in language specific process.
bool FunctionNode::OverrideEquivalent(FunctionNode *fun) {
  if (!mType->TypeEquivalent(fun->GetType()))
    return false;
  if (GetStrIdx() != fun->GetStrIdx())
    return false;
  if (GetParamsNum() != fun->GetParamsNum())
    return false;
  for (unsigned i = 0; i < GetParamsNum(); i++) {
    TreeNode *this_p = GetParam(i);
    TreeNode *that_p = fun->GetParam(i);
    MASSERT(this_p->IsIdentifier());
    MASSERT(that_p->IsIdentifier());
    TreeNode *this_ty = ((IdentifierNode*)this_p)->GetType();
    TreeNode *that_ty = ((IdentifierNode*)that_p)->GetType();
    if (!this_ty->TypeEquivalent(that_ty))
      return false;
  }
  return true;
}

// When BlockNode is added to the FunctionNode, we need further
// cleanup, i.e. clean up the PassNode.
void FunctionNode::CleanUp() {
  TreeNode *prev = NULL;
  TreeNode *next = NULL;
  unsigned num = mBody->GetChildrenNum();
  if (num == 0)
    return;

  bool changed = true;
  while(changed) {
    changed = false;
    for (unsigned i = 0; i < num; i++) {
      TreeNode *tree = mBody->GetChildAtIndex(i);
      if (tree->IsPass()) {
        PassNode *passnode = (PassNode*)tree;

        // Body has only one PassNode. Remove it, and add all its children
        if (num == 1) {
          mBody->ClearChildren();
          for (unsigned j = 0; j < passnode->GetChildrenNum(); j++) {
            TreeNode *child = passnode->GetChild(j);
            mBody->AddChild(child);
          }
        } else {
          // If pass node is the header, insert before next.
          // If pass node is the last or any one else, insert after prev.
          if (i == 0) {
            next = mBody->GetChildAtIndex(1);
          } else {
            prev = mBody->GetChildAtIndex(i-1);
          }

          // remove the passnode
          mBody->mChildren.Remove(tree);

          // set anchor, and add children
          if (next) {
            mBody->mChildren.LocateValue(next);
            for (unsigned j = 0; j < passnode->GetChildrenNum(); j++) {
              TreeNode *child = passnode->GetChild(j);
              mBody->mChildren.InsertBefore(child);
            }
          } else {
            MASSERT(prev);
            mBody->mChildren.LocateValue(prev);
            // [NOTE] We need start from the last child to the first,
            //        because the earlier inserted child will be pushed back
            //        by the later one.
            for (int j = passnode->GetChildrenNum() - 1; j >= 0; j--) {
              TreeNode *child = passnode->GetChild(j);
              mBody->mChildren.InsertAfter(child);
            }
          }
        }

        // Exit the for iteration. One pass node each time.
        changed = true;
        break;
      }
    } // end for
  } // end while
}

void FunctionNode::Dump(unsigned indent) {
  DumpIndentation(indent);
  if (mIsConstructor)
    DUMP0_NORETURN("constructor  ");
  else
    DUMP0_NORETURN("func  ");

  if (mStrIdx)
    DUMP0_NORETURN(GetName());

  if (GetTypeParamsNum() > 0) {
    DUMP0_NORETURN("<");
    for (unsigned i = 0; i < GetTypeParamsNum(); i++) {
      TreeNode *arg = GetTypeParamAtIndex(i);
      arg->Dump(0);
      if (i < GetTypeParamsNum() - 1)
        DUMP0_NORETURN(",");
    }
    DUMP0_NORETURN(">");
  }

  // dump parameters
  DUMP0_NORETURN("(");
  for (unsigned i = 0; i < GetParamsNum(); i++) {
    TreeNode *param = GetParam(i);
    param->Dump(0);
    if (i < GetParamsNum() - 1)
      DUMP0_NORETURN(",");
  }
  DUMP0_NORETURN(")");

  // dump throws
  DUMP0_NORETURN("  throws: ");
  for (unsigned i = 0; i < mThrows.GetNum(); i++) {
    TreeNode *node = mThrows.ValueAtIndex(i);
    node->Dump(4);
  }
  DUMP_RETURN();

  // dump function body
  if (GetBody())
    GetBody()->Dump(indent+2);
}

//////////////////////////////////////////////////////////////////////////////////////
//                              LambdaNode
//////////////////////////////////////////////////////////////////////////////////////

void LambdaNode::Dump(unsigned indent) {
  DumpIndentation(indent);
  std::string dump;
  dump += "(";
  for (unsigned i = 0; i < mParams.GetNum(); i++) {
    TreeNode *in = mParams.ValueAtIndex(i);
    if(in->GetKind() == NK_Decl)
      dump += static_cast<DeclNode*>(in)->GetVar()->GetName();
    else
      dump += in->GetName();
    if (i < mParams.GetNum() - 1)
      dump += ",";
  }
  dump += ") -> ";
  DUMP0_NORETURN(dump.c_str());
  if (mBody)
    mBody->Dump(0);
}

//////////////////////////////////////////////////////////////////////////////////////
//                              InstanceOfNode
//////////////////////////////////////////////////////////////////////////////////////

void InstanceOfNode::Dump(unsigned indent) {
  DumpIndentation(indent);
  mLeft->Dump(0);
  DUMP0_NORETURN(" instanceof ");
  mRight->Dump(0);
}

//////////////////////////////////////////////////////////////////////////////////////
//                              InNode
//////////////////////////////////////////////////////////////////////////////////////

void InNode::Dump(unsigned indent) {
  DumpIndentation(indent);
  mLeft->Dump(0);
  DUMP0_NORETURN(" in ");
  mRight->Dump(0);
}

//////////////////////////////////////////////////////////////////////////////////////
//                              TypeOfNode
//////////////////////////////////////////////////////////////////////////////////////

void TypeOfNode::Dump(unsigned indent) {
  DumpIndentation(indent);
  DUMP0_NORETURN(" typeof ");
  mExpr->Dump(0);
}

//////////////////////////////////////////////////////////////////////////////////////
//                              KeyOfNode
//////////////////////////////////////////////////////////////////////////////////////

void KeyOfNode::Dump(unsigned indent) {
  DumpIndentation(indent);
  DUMP0_NORETURN(" keyof ");
  mExpr->Dump(0);
}

//////////////////////////////////////////////////////////////////////////////////////
//                             InterfaceNode
//////////////////////////////////////////////////////////////////////////////////////

void InterfaceNode::Construct(BlockNode *block) {
  for (unsigned i = 0; i < block->GetChildrenNum(); i++) {
    TreeNode *tree_node = block->GetChildAtIndex(i);
    tree_node->SetParent(this);
    if (tree_node->IsVarList()) {
      VarListNode *vlnode = (VarListNode*)tree_node;
      for (unsigned i = 0; i < vlnode->GetVarsNum(); i++) {
        IdentifierNode *inode = vlnode->GetVarAtIndex(i);
        inode->SetParent(this);
        mFields.PushBack(inode);
      }
    } else if (tree_node->IsIdentifier())
      mFields.PushBack((IdentifierNode*)tree_node);
    else if (tree_node->IsFunction()) {
      FunctionNode *f = (FunctionNode*)tree_node;
      mMethods.PushBack(f);
    } else
      MASSERT("Unsupported tree node in interface body.");
  }
}

void InterfaceNode::Dump(unsigned indent) {
  DumpIndentation(indent);
  DUMP1_NORETURN("interface ", GetName());
  DUMP_RETURN();
  DumpIndentation(indent + 2);

  DUMP0("Fields: ");
  for (unsigned i = 0; i < mFields.GetNum(); i++) {
    TreeNode *node = mFields.ValueAtIndex(i);
    node->Dump(indent + 4);
  }
  DUMP_RETURN();

  DumpIndentation(indent + 2);
  DUMP0("Methods: ");
  for (unsigned i = 0; i < mMethods.GetNum(); i++) {
    TreeNode *node = mMethods.ValueAtIndex(i);
    node->Dump(indent + 4);
  }
}
}
