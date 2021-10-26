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

#include <cstring>

#include "token.h"
#include "ruletable.h"
#include "stringpool.h"
#include "ast_builder.h"
#include "ast_scope.h"
#include "ast_attr.h"
#include "ast_type.h"
#include "ast_module.h"
#include "massert.h"

namespace maplefe {

////////////////////////////////////////////////////////////////////////////////////////
// For the time being, we simply use a big switch-case. Later on we could use a more
// flexible solution.
////////////////////////////////////////////////////////////////////////////////////////

// Return the sub-tree.
TreeNode* ASTBuilder::Build() {
  TreeNode *tree_node = NULL;
#define ACTION(A) \
  case (ACT_##A): \
    tree_node = A(); \
    break;

  switch (mActionId) {
#include "supported_actions.def"
  }

  return tree_node;
}

TreeNode* ASTBuilder::CreateTokenTreeNode(const Token *token) {
  unsigned size = 0;
  if (token->IsIdentifier()) {
    IdentifierNode *n = (IdentifierNode*)gTreePool.NewTreeNode(sizeof(IdentifierNode));
    unsigned idx = gStringPool.GetStrIdx(token->GetName());
    new (n) IdentifierNode(idx);
    mLastTreeNode = n;
    return n;
  } else if (token->IsLiteral()) {
    LitData data = token->GetLitData();
    LiteralNode *n = (LiteralNode*)gTreePool.NewTreeNode(sizeof(LiteralNode));
    new (n) LiteralNode(data);
    mLastTreeNode = n;
    return n;
  } else if (token->IsTempLit()) {
    TemplateLiteralNode *n = (TemplateLiteralNode*)gTreePool.NewTreeNode(sizeof(TemplateLiteralNode));
    new (n) TemplateLiteralNode();

    // copy mStrings&mPlaceHolders to n
    TempLitData *tld = token->GetTempLitData();
    for (unsigned i = 0; i < tld->mStrings.GetNum(); i++) {
      const char *s = tld->mStrings.ValueAtIndex(i);
      n->AddString(s);
    }

    // release memeory of SmallVector of mStrings.
    tld->mStrings.Release();
    delete tld;

    gTemplateLiteralNodes.PushBack(n);
    mLastTreeNode = n;
    return n;

  } else if (token->IsRegExpr()) {
    RegExprNode *n = (RegExprNode*)gTreePool.NewTreeNode(sizeof(RegExprNode));
    new (n) RegExprNode();

    RegExprData d = token->GetRegExpr();
    n->SetData(d);

    mLastTreeNode = n;
    return n;

  } else if (token->IsKeyword()) {
    mNameForBuildIdentifier = NULL;
    const char *keyword = token->GetName();
    // If it's an attribute
    AttrNode *n = gAttrPool.GetAttrNode(keyword);
    if (n) {
      mLastTreeNode = n;
      return n;
    }
    // If it's a type
    PrimTypeNode *type = gPrimTypePool.FindType(keyword);
    if (type) {
      mLastTreeNode = type;
      mNameForBuildIdentifier = keyword;
      return type;
    }
    // We define special literal tree node for 'this', 'super'.
    if ((strlen(token->GetName()) == 4) && !strncmp(token->GetName(), "this", 4)) {
      LitData data;
      data.mType = LT_ThisLiteral;
      LiteralNode *n = (LiteralNode*)gTreePool.NewTreeNode(sizeof(LiteralNode));
      new (n) LiteralNode(data);
      mLastTreeNode = n;
      return n;
    } else if ((strlen(token->GetName()) == 5) && !strncmp(token->GetName(), "super", 5)) {
      LitData data;
      data.mType = LT_SuperLiteral;
      LiteralNode *n = (LiteralNode*)gTreePool.NewTreeNode(sizeof(LiteralNode));
      new (n) LiteralNode(data);
      mLastTreeNode = n;
      return n;
    }

    // Otherwise, it doesn't create any tree node.
    // But we pass the keyword name to future possible BuildIdentifier.
    mNameForBuildIdentifier = keyword;
  }

  // Other tokens shouldn't be involved in the tree creation.
  return NULL;
}

////////////////////////////////////////////////////////////////////////////////////////
//                      Static Functions help build the tree
////////////////////////////////////////////////////////////////////////////////////////

static void add_attribute_to(TreeNode *tree, TreeNode *attr) {
  MASSERT(attr->IsAttr());
  AttrNode *attr_node = (AttrNode*)attr;
  AttrId aid = attr_node->GetId();

  if (tree->IsPass()) {
    PassNode *pass_node = (PassNode*)tree;
    for (unsigned i = 0; i < pass_node->GetChildrenNum(); i++) {
      TreeNode *child = pass_node->GetChild(i);
      add_attribute_to(child, attr);
    }
  } else if (tree->IsVarList()) {
    VarListNode *vl = (VarListNode*)tree;
    for (unsigned i = 0; i < vl->GetVarsNum(); i++) {
      IdentifierNode *inode = vl->GetVarAtIndex(i);
      inode->AddAttr(aid);
    }
  } else if (tree->IsExprList()) {
    ExprListNode *vl = (ExprListNode*)tree;
    for (unsigned i = 0; i < vl->GetExprsNum(); i++) {
      TreeNode *child = vl->GetExprAtIndex(i);
      add_attribute_to(child, attr);
    }
  } else {
    // The basse TreeNode has a virtual AddAttr().
    tree->AddAttr(aid);
  }
  return;
}

// It's the caller to assure tree is valid, meaning something could carry type.
static void add_type_to(TreeNode *tree, TreeNode *type) {
  if (tree->IsIdentifier()) {
    IdentifierNode *in = (IdentifierNode*)tree;
    in->SetType(type);
  } else if (tree->IsLiteral()) {
    LiteralNode *lit = (LiteralNode*)tree;
    lit->SetType(type);
  } else if (tree->IsLambda()) {
    LambdaNode *lam = (LambdaNode*)tree;
    lam->SetType(type);
  } else if (tree->IsVarList()) {
    VarListNode *vl = (VarListNode*)tree;
    for (unsigned i = 0; i < vl->GetVarsNum(); i++)
      vl->GetVarAtIndex(i)->SetType(type);
  } else if (tree->IsFunction()) {
    FunctionNode *func = (FunctionNode*)tree;
    func->SetType(type);
  } else if (tree->IsBindingPattern()) {
    BindingPatternNode *bp = (BindingPatternNode*)tree;
    bp->SetType(type);
  } else if (tree->IsComputedName()) {
    ComputedNameNode *mp = (ComputedNameNode*)tree;
    mp->SetExtendType(type);
  } else {
    MERROR("Unsupported tree node in add_type_to()");
  }
}

////////////////////////////////////////////////////////////////////////////////////////
//                          BuildModule
////////////////////////////////////////////////////////////////////////////////////////

// Take one argument, the module name
TreeNode* ASTBuilder::BuildModule() {
  ModuleNode *n = (ModuleNode*)gTreePool.NewTreeNode(sizeof(ModuleNode));
  new (n) ModuleNode();

  MASSERT(mParams.size() == 1);
  Param p_a = mParams[0];
  if (!p_a.mIsEmpty && p_a.mIsTreeNode) {
    TreeNode *tree = p_a.mData.mTreeNode;
    if (tree->IsIdentifier()) {
      const char *name = tree->GetName();
      n->SetFilename(name);
    } else if (tree->IsLiteral()) {
      LiteralNode *lit = (LiteralNode*)tree;
      LitData data = lit->GetData();
      MASSERT(data.mType == LT_StringLiteral);
      const char *name = gStringPool.GetStringFromStrIdx(data.mData.mStrIdx);
      n->SetFilename(name);
    } else {
      MERROR("Unsupported module name.");
    }
  }

  mLastTreeNode = n;
  return mLastTreeNode;
}

// Takes one parameter which is the tree of module body.
TreeNode* ASTBuilder::AddModuleBody() {
  if (mTrace)
    std::cout << "In AddModuleBody" << std::endl;

  Param p_body = mParams[0];
  if (!p_body.mIsEmpty) {
    if (!p_body.mIsTreeNode)
      MERROR("The module body is not a tree node.");
    TreeNode *tn = p_body.mData.mTreeNode;

    MASSERT(mLastTreeNode->IsModule());
    ModuleNode *mod = (ModuleNode*)mLastTreeNode;
    mod->AddTree(tn);
  }

  return mLastTreeNode;
}

////////////////////////////////////////////////////////////////////////////////////////
//                          BuildIdentifier
////////////////////////////////////////////////////////////////////////////////////////

// 1. It takes one argument, the target to build into identifier.
// 2. It takes no argument, then mLastTreeNode is the target.
// It could be a token or tree.
TreeNode* ASTBuilder::BuildIdentifier() {
  if (mParams.size() == 1) {
    Param target = mParams[0];
    if (!target.mIsEmpty) {
      if (target.mIsTreeNode) {
        TreeNode *tn = target.mData.mTreeNode;
        return BuildIdentifier(tn);
      } else {
        Token *tn = target.mData.mToken;
        return BuildIdentifier(tn);
      }
    }
    return NULL;
  }

  if (mNameForBuildIdentifier) {
    IdentifierNode *n = (IdentifierNode*)gTreePool.NewTreeNode(sizeof(IdentifierNode));
    unsigned idx = gStringPool.GetStrIdx(mNameForBuildIdentifier);
    new (n) IdentifierNode(idx);
    mLastTreeNode = n;
    mNameForBuildIdentifier = NULL;
    return n;
  } else if (mLastTreeNode->IsIdentifier()) {
    return mLastTreeNode;
  } else if (mLastTreeNode->IsAttr()) {
    AttrNode *an = (AttrNode*)mLastTreeNode;
    AttrId aid = an->GetId();

    IdentifierNode *n = (IdentifierNode*)gTreePool.NewTreeNode(sizeof(IdentifierNode));
    unsigned idx = gStringPool.GetStrIdx(FindAttrKeyword(aid));
    new (n) IdentifierNode(idx);
    mLastTreeNode = n;
    return n;
  } else if (mLastTreeNode->IsPrimType()) {
    PrimTypeNode *prim_type = (PrimTypeNode*)mLastTreeNode;
    IdentifierNode *n = (IdentifierNode*)gTreePool.NewTreeNode(sizeof(IdentifierNode));
    unsigned idx = gStringPool.GetStrIdx(prim_type->GetTypeName());
    new (n) IdentifierNode(idx);
    mLastTreeNode = n;
    return n;
  } else {
    MERROR("Unsupported node type in BuildIdentifier()");
  }
}

// Build IdentifierNode from a token.
TreeNode* ASTBuilder::BuildIdentifier(const Token *token) {
  const char *name = token->GetName();
  MASSERT(name);
  IdentifierNode *n = (IdentifierNode*)gTreePool.NewTreeNode(sizeof(IdentifierNode));
  unsigned idx = gStringPool.GetStrIdx(name);
  new (n) IdentifierNode(idx);
  mLastTreeNode = n;
  return n;
}

// Build IdentifierNode from a TreeNode.
TreeNode* ASTBuilder::BuildIdentifier(const TreeNode *tree) {
  if (!tree)
    return NULL;

  if (tree->IsAttr()) {
    AttrNode *an = (AttrNode*)tree;
    AttrId aid = an->GetId();
    IdentifierNode *n = (IdentifierNode*)gTreePool.NewTreeNode(sizeof(IdentifierNode));
    unsigned idx = gStringPool.GetStrIdx(FindAttrKeyword(aid));
    new (n) IdentifierNode(idx);
    mLastTreeNode = n;
    return n;
  }

  return NULL;
}

////////////////////////////////////////////////////////////////////////////////////////
//                      NameTypePair
////////////////////////////////////////////////////////////////////////////////////////

TreeNode* ASTBuilder::BuildNameTypePair() {
  if (mTrace)
    std::cout << "In BuildNameTypePair" << std::endl;

  MASSERT(mParams.size() == 2);
  Param p_a = mParams[0];
  Param p_b = mParams[1];

  NameTypePairNode *n = (NameTypePairNode*)gTreePool.NewTreeNode(sizeof(NameTypePairNode));
  new (n) NameTypePairNode();
  mLastTreeNode = n;

  if (!p_a.mIsEmpty && p_a.mIsTreeNode)
    n->SetVar(p_a.mData.mTreeNode);

  if (!p_b.mIsEmpty && p_b.mIsTreeNode)
    n->SetType(p_b.mData.mTreeNode);

  return mLastTreeNode;
}

////////////////////////////////////////////////////////////////////////////////////////
//                      Interfaces for Java style package  and import
////////////////////////////////////////////////////////////////////////////////////////

TreeNode* ASTBuilder::BuildPackageName() {
  MASSERT(!mASTModule->mPackage);
  MASSERT(mLastTreeNode->IsField() || mLastTreeNode->IsIdentifier());

  PackageNode *n = (PackageNode*)gTreePool.NewTreeNode(sizeof(PackageNode));
  new (n) PackageNode();
  n->SetPackage(mLastTreeNode);

  mASTModule->SetPackage(n);

  mLastTreeNode = n;
  return mLastTreeNode;
}

TreeNode* ASTBuilder::BuildSingleTypeImport() {
  ImportNode *n = (ImportNode*)gTreePool.NewTreeNode(sizeof(ImportNode));
  new (n) ImportNode();
  n->SetImportSingle();
  n->SetImportType();

  MASSERT(mParams.size() == 1);
  Param p = mParams[0];
  MASSERT(!p.mIsEmpty && p.mIsTreeNode);
  TreeNode *tree = p.mData.mTreeNode;
  MASSERT(tree->IsIdentifier() || tree->IsField());

  n->SetTarget(tree);
  mLastTreeNode = n;
  return mLastTreeNode;
}

TreeNode* ASTBuilder::BuildAllTypeImport() {
  ImportNode *n = (ImportNode*)gTreePool.NewTreeNode(sizeof(ImportNode));
  new (n) ImportNode();
  n->SetImportAll();
  n->SetImportType();

  MASSERT(mParams.size() == 1);
  Param p = mParams[0];
  MASSERT(!p.mIsEmpty && p.mIsTreeNode);
  TreeNode *tree = p.mData.mTreeNode;
  MASSERT(tree->IsIdentifier() || tree->IsField());

  n->SetTarget(tree);
  mLastTreeNode = n;
  return mLastTreeNode;
}

// It takes the mLastTreeNode as parameter
TreeNode* ASTBuilder::BuildSingleStaticImport() {
  ImportNode *n = (ImportNode*)gTreePool.NewTreeNode(sizeof(ImportNode));
  new (n) ImportNode();
  n->SetImportSingle();
  n->SetImportType();

  MASSERT(mLastTreeNode->IsIdentifier() || mLastTreeNode->IsField());
  n->SetTarget(mLastTreeNode);
  n->SetImportStatic();
  mLastTreeNode = n;
  return mLastTreeNode;
}

TreeNode* ASTBuilder::BuildAllStaticImport() {
  BuildAllTypeImport();
  ImportNode *import = (ImportNode*)mLastTreeNode;
  import->SetImportStatic();
  return mLastTreeNode;
}

TreeNode* ASTBuilder::BuildAllImport() {
}

////////////////////////////////////////////////////////////////////////////////////////
//                      Interfaces for Javascript style export and import
////////////////////////////////////////////////////////////////////////////////////////

// Takes no argument.
TreeNode* ASTBuilder::BuildImport() {
  ImportNode *n = (ImportNode*)gTreePool.NewTreeNode(sizeof(ImportNode));
  new (n) ImportNode();

  mLastTreeNode = n;
  return mLastTreeNode;
}

// Takes no argument.
TreeNode* ASTBuilder::BuildExport() {
  ExportNode *n = (ExportNode*)gTreePool.NewTreeNode(sizeof(ExportNode));
  new (n) ExportNode();

  mLastTreeNode = n;
  return mLastTreeNode;
}

// It set mLastTreeNode to a type import/export.
TreeNode* ASTBuilder::SetIsXXportType() {
  if (mLastTreeNode->IsImport()) {
    ImportNode *inode = (ImportNode*)mLastTreeNode;
    inode->SetImportType();
  } else if (mLastTreeNode->IsExport()) {
    ExportNode *enode = (ExportNode*)mLastTreeNode;
    enode->SetIsExportType();
  } else {
    MERROR("Unsupported action.");
  }

  return mLastTreeNode;
}

// It takes one argument, the pairs.
// The pairs could be complicated in Javascript. We will let ImportNode or
// ExportNode to handle by themselves.
TreeNode* ASTBuilder::SetPairs() {
  TreeNode *pairs = NULL;
  Param p = mParams[0];
  if (!p.mIsEmpty && p.mIsTreeNode) {
    pairs = p.mData.mTreeNode;
    if (mLastTreeNode->IsImport()) {
      ImportNode *inode = (ImportNode*)mLastTreeNode;
      inode->AddPair(pairs);
    } else if (mLastTreeNode->IsExport()) {
      ExportNode *enode = (ExportNode*)mLastTreeNode;
      enode->AddPair(pairs);
    }
  }

  return mLastTreeNode;
}

TreeNode* ASTBuilder::SetDefaultPairs() {
  TreeNode *pairs = NULL;
  Param p = mParams[0];
  if (!p.mIsEmpty && p.mIsTreeNode) {
    pairs = p.mData.mTreeNode;
    if (mLastTreeNode->IsImport()) {
      ImportNode *inode = (ImportNode*)mLastTreeNode;
      inode->AddDefaultPair(pairs);
    } else if (mLastTreeNode->IsExport()) {
      ExportNode *enode = (ExportNode*)mLastTreeNode;
      enode->AddDefaultPair(pairs);
    }
  }

  return mLastTreeNode;
}

// It takes
//   1. One argument:  the mBefore object. Usually happens in
//                       export = x
//   2. Two arguments: the mBefore and mAfter. In TS, it has
//                       import x = require(y)
//                     [NOTE] In .spec file, put the arguments in order of
//                            SetSinglePairs(before, after)

TreeNode* ASTBuilder::SetSinglePairs() {
  TreeNode *before = NULL;
  TreeNode *after = NULL;

  if (mParams.size() == 2) {
    Param p = mParams[0];
    if (!p.mIsEmpty && p.mIsTreeNode)
      before = p.mData.mTreeNode;
    p = mParams[1];
    if (!p.mIsEmpty && p.mIsTreeNode)
      after = p.mData.mTreeNode;
  } else {
    MASSERT(mParams.size() == 1);
    Param p = mParams[0];
    if (!p.mIsEmpty && p.mIsTreeNode)
      before = p.mData.mTreeNode;
  }

  if (mLastTreeNode->IsImport()) {
    ImportNode *inode = (ImportNode*)mLastTreeNode;
    inode->AddSinglePair(before, after);
  } else if (mLastTreeNode->IsExport()) {
    ExportNode *enode = (ExportNode*)mLastTreeNode;
    enode->AddSinglePair(before, after);
  }

  return mLastTreeNode;
}

// Takes one argument, the 'from' module
TreeNode* ASTBuilder::SetFromModule() {
  Param p = mParams[0];
  if (!p.mIsEmpty && p.mIsTreeNode) {
    TreeNode *t = p.mData.mTreeNode;
    if (mLastTreeNode->IsImport()) {
      ImportNode *inode = (ImportNode*)mLastTreeNode;
      inode->SetTarget(t);
    } else if (mLastTreeNode->IsExport()) {
      ExportNode *enode = (ExportNode*)mLastTreeNode;
      enode->SetTarget(t);
    }
  }
  return mLastTreeNode;
}

// Take no argument, or one argument.
// (1) If no argument, it applies to all pairs of import/export.
//     This happens right after BuildImport or BuildExport, and there is no existing
//     pairs. In this case, we create a new pair and sets it to *.
// (2) If one argument, it's the new name of '*' (aka the Everything),
//     and is saved in mAfter of the pair.
//
// In either case, we need create a new and the only pair for XXport node.
TreeNode* ASTBuilder::SetIsEverything() {
  XXportAsPairNode *n = (XXportAsPairNode*)gTreePool.NewTreeNode(sizeof(XXportAsPairNode));
  new (n) XXportAsPairNode();
  n->SetIsEverything();

  if (mParams.size() == 1) {
    Param p = mParams[0];
    if (!p.mIsEmpty && p.mIsTreeNode) {
      TreeNode *expr = p.mData.mTreeNode;
      n->SetBefore(expr);
    }
  }

  if (mLastTreeNode->IsImport()) {
    ImportNode *inode = (ImportNode*)mLastTreeNode;
    MASSERT(!inode->GetPairsNum());
    inode->AddPair(n);
  } else if (mLastTreeNode->IsExport()) {
    ExportNode *enode = (ExportNode*)mLastTreeNode;
    MASSERT(!enode->GetPairsNum());
    enode->AddPair(n);
  }

  return mLastTreeNode;
}

// Right now it takes no argument. mLastTreeNode is the implicit argument
TreeNode* ASTBuilder::SetAsNamespace() {
  MASSERT(mLastTreeNode->IsXXportAsPair());
  XXportAsPairNode *pair = (XXportAsPairNode*)mLastTreeNode;
  pair->SetAsNamespace();
  return mLastTreeNode;
}

// 1. It takes two arguments, before and after.
// 2. It takes one argument, before.
TreeNode* ASTBuilder::BuildXXportAsPair() {

  TreeNode *before = NULL;
  TreeNode *after = NULL;

  Param p = mParams[0];
  if (!p.mIsEmpty && p.mIsTreeNode) {
    before = p.mData.mTreeNode;
  }

  if (mParams.size() == 2) {
    p = mParams[1];
    if (!p.mIsEmpty && p.mIsTreeNode) {
      after = p.mData.mTreeNode;
    }
  }

  XXportAsPairNode *n = (XXportAsPairNode*)gTreePool.NewTreeNode(sizeof(XXportAsPairNode));
  new (n) XXportAsPairNode();

  if (before)
    n->SetBefore(before);
  if (after)
    n->SetAfter(after);

  mLastTreeNode = n;
  return mLastTreeNode;
}

// It takes one arguments, the 'x' in the '* as x'.
TreeNode* ASTBuilder::BuildXXportAsPairEverything() {
  MASSERT(mParams.size() == 1);

  TreeNode *tree = NULL;

  Param p = mParams[0];
  if (!p.mIsEmpty && p.mIsTreeNode) {
    tree = p.mData.mTreeNode;
  }

  XXportAsPairNode *n = (XXportAsPairNode*)gTreePool.NewTreeNode(sizeof(XXportAsPairNode));
  new (n) XXportAsPairNode();
  n->SetIsEverything();

  if (tree)
    n->SetBefore(tree);

  mLastTreeNode = n;
  return mLastTreeNode;
}

// It takes one arguments, the name after 'as'.
TreeNode* ASTBuilder::BuildXXportAsPairDefault() {
  MASSERT(mParams.size() == 1);

  TreeNode *tree = NULL;

  Param p = mParams[0];
  if (!p.mIsEmpty && p.mIsTreeNode) {
    tree = p.mData.mTreeNode;
  }

  XXportAsPairNode *n = (XXportAsPairNode*)gTreePool.NewTreeNode(sizeof(XXportAsPairNode));
  new (n) XXportAsPairNode();
  n->SetIsDefault();

  if (tree)
    n->SetBefore(tree);

  mLastTreeNode = n;
  return mLastTreeNode;
}

/////////////////////////////////////////////////////////////////////////////////////////
//          BuildExternalDeclaration and BuildGlobalExternalDeclaration
/////////////////////////////////////////////////////////////////////////////////////////

// It takes one arguments
TreeNode* ASTBuilder::BuildExternalDeclaration() {
  MASSERT(mParams.size() == 1);

  TreeNode *tree = NULL;

  Param p = mParams[0];
  if (!p.mIsEmpty && p.mIsTreeNode) {
    tree = p.mData.mTreeNode;
  }

  DeclareNode *n = (DeclareNode*)gTreePool.NewTreeNode(sizeof(DeclareNode));
  new (n) DeclareNode();
  n->AddDecl(tree);

  mLastTreeNode = n;
  return mLastTreeNode;
}

// It takes one arguments
TreeNode* ASTBuilder::BuildGlobalExternalDeclaration() {
  MASSERT(mParams.size() == 1);

  TreeNode *tree = NULL;

  Param p = mParams[0];
  if (!p.mIsEmpty && p.mIsTreeNode) {
    tree = p.mData.mTreeNode;
  }

  DeclareNode *n = (DeclareNode*)gTreePool.NewTreeNode(sizeof(DeclareNode));
  new (n) DeclareNode();
  n->AddDecl(tree);
  n->SetIsGlobal();

  mLastTreeNode = n;
  return mLastTreeNode;
}

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////

// Takes one argument, the expression of the parenthesis
TreeNode* ASTBuilder::BuildParenthesis() {
  if (mTrace)
    std::cout << "In BuildParenthesis" << std::endl;

  TreeNode *expr = NULL;

  MASSERT(mParams.size() == 1);
  Param p = mParams[0];
  MASSERT(!p.mIsEmpty && p.mIsTreeNode);
  expr = p.mData.mTreeNode;

  ParenthesisNode *n = (ParenthesisNode*)gTreePool.NewTreeNode(sizeof(ParenthesisNode));
  new (n) ParenthesisNode();
  n->SetExpr(expr);

  mLastTreeNode = n;
  return mLastTreeNode;
}

// For first parameter has to be an operator.
TreeNode* ASTBuilder::BuildCast() {
  if (mTrace)
    std::cout << "In BuildCast" << std::endl;

  MASSERT(mParams.size() == 2 && "BuildCast has NO 2 params?");
  Param p_a = mParams[0];
  Param p_b = mParams[1];
  MASSERT(!p_a.mIsEmpty && !p_a.mIsEmpty);
  MASSERT(p_a.mIsTreeNode && p_a.mIsTreeNode);

  TreeNode *desttype = p_a.mData.mTreeNode;
  TreeNode *expr = p_b.mData.mTreeNode;

  CastNode *n = (CastNode*)gTreePool.NewTreeNode(sizeof(CastNode));
  new (n) CastNode();

  n->SetDestType(desttype);
  n->SetExpr(expr);

  mLastTreeNode = n;
  return n;
}

// It takes one parameter, the word tells what literal it is.
TreeNode* ASTBuilder::BuildLiteral() {
  if (mTrace)
    std::cout << "In BuildLiteral" << std::endl;

  MASSERT(mParams.size() == 1);
  Param p_a = mParams[0];
  MASSERT(p_a.mIsTreeNode);

  TreeNode *tree = p_a.mData.mTreeNode;
  bool is_void = false;
  if (tree->IsPrimType()) {
    PrimTypeNode *prim = (PrimTypeNode*)tree;
    if (prim->GetPrimType() == TY_Void)
      is_void = true;
  }

  if (is_void) {
    LitData data;
    data.mType = LT_VoidLiteral;
    LiteralNode *n = (LiteralNode*)gTreePool.NewTreeNode(sizeof(LiteralNode));
    new (n) LiteralNode(data);
    mLastTreeNode = n;
    return n;
  } else {
    MERROR("Unspported in BuildLiteral().");
  }
}


// For first parameter has to be an operator.
TreeNode* ASTBuilder::BuildUnaryOperation() {
  if (mTrace)
    std::cout << "In build unary" << std::endl;

  MASSERT(mParams.size() == 2 && "Binary Operator has NO 2 params?");
  Param p_a = mParams[0];
  Param p_b = mParams[1];
  MASSERT(!p_a.mIsTreeNode && "First param of Unary Operator is not a token?");

  Token *token = p_a.mData.mToken;
  MASSERT(token->IsOperator() && "First param of Unary Operator is not an operator token?");

  // create the sub tree
  UnaOperatorNode *n = (UnaOperatorNode*)gTreePool.NewTreeNode(sizeof(UnaOperatorNode));
  new (n) UnaOperatorNode(token->GetOprId());

  // set 1st param
  if (p_b.mIsTreeNode)
    n->SetOpnd(p_b.mData.mTreeNode);
  else {
    TreeNode *tn = CreateTokenTreeNode(p_b.mData.mToken);
    n->SetOpnd(tn);
  }

  mLastTreeNode = n;
  return n;
}

// This is the same as BuildUnaryOperation, except setting mIsPost to true.
TreeNode* ASTBuilder::BuildPostfixOperation() {
  if (mTrace)
    std::cout << "In BuildPostfixOperation" << std::endl;
  UnaOperatorNode * t = (UnaOperatorNode*)BuildUnaryOperation();
  t->SetIsPost(true);
  mLastTreeNode = t;
  return t;
}

// For second parameter has to be an operator.
TreeNode* ASTBuilder::BuildBinaryOperation() {
  if (mTrace)
    std::cout << "In build binary" << std::endl;

  MASSERT(mParams.size() == 3 && "Binary Operator has NO 3 params?");
  Param p_a = mParams[0];
  Param p_b = mParams[1];
  Param p_c = mParams[2];
  MASSERT(!p_b.mIsTreeNode && "Second param of Binary Operator is not a token?");

  Token *token = p_b.mData.mToken;
  MASSERT(token->IsOperator() && "Second param of Binary Operator is not an operator token?");

  // create the sub tree
  BinOperatorNode *n = (BinOperatorNode*)gTreePool.NewTreeNode(sizeof(BinOperatorNode));
  new (n) BinOperatorNode(token->GetOprId());
  mLastTreeNode = n;

  // set 1st param
  if (p_a.mIsTreeNode)
    n->SetOpndA(p_a.mData.mTreeNode);
  else {
    TreeNode *tn = CreateTokenTreeNode(p_a.mData.mToken);
    n->SetOpndA(tn);
  }

  // set 2nd param
  if (p_c.mIsTreeNode)
    n->SetOpndB(p_c.mData.mTreeNode);
  else {
    TreeNode *tn = CreateTokenTreeNode(p_c.mData.mToken);
    n->SetOpndB(tn);
  }

  return n;
}

// For second parameter has to be an operator.
TreeNode* ASTBuilder::BuildTernaryOperation() {
  if (mTrace)
    std::cout << "In BuildTernaryOperation" << std::endl;

  MASSERT(mParams.size() == 3 && "Ternary Operator has NO 3 params?");
  Param p_a = mParams[0];
  Param p_b = mParams[1];
  Param p_c = mParams[2];

  // create the sub tree
  TerOperatorNode *n = (TerOperatorNode*)gTreePool.NewTreeNode(sizeof(TerOperatorNode));
  new (n) TerOperatorNode();
  mLastTreeNode = n;

  MASSERT(p_a.mIsTreeNode);
  n->SetOpndA(p_a.mData.mTreeNode);

  MASSERT(p_b.mIsTreeNode);
  n->SetOpndB(p_b.mData.mTreeNode);

  MASSERT(p_c.mIsTreeNode);
  n->SetOpndC(p_c.mData.mTreeNode);

  return n;
}

// Takes one argument. Set the tree as a statement.
// We still return the previous mLastTreeNode.
TreeNode* ASTBuilder::SetIsStmt() {
  if (mTrace)
    std::cout << "In SetIsStmt" << std::endl;

  Param p_tree = mParams[0];
  if (!p_tree.mIsEmpty) {
    MASSERT(p_tree.mIsTreeNode);
    TreeNode *treenode = p_tree.mData.mTreeNode;
    treenode->SetIsStmt();
  }

  return mLastTreeNode;
}

// 1. Takes one argument. Set the tree as an optional node.
// 2. Takes no argument. Set mLastTreeNode as optional.
// We still return the previous mLastTreeNode.
TreeNode* ASTBuilder::SetIsOptional() {
  if (mTrace)
    std::cout << "In SetIsOptional" << std::endl;

  TreeNode *treenode = NULL;
  if (mParams.size() > 0) {
    Param p_tree = mParams[0];
    if (!p_tree.mIsEmpty) {
      MASSERT(p_tree.mIsTreeNode);
      treenode = p_tree.mData.mTreeNode;
    }
  } else {
    treenode = mLastTreeNode;
  }

  MASSERT(treenode);
  if (treenode->IsFunction()) {
    FunctionNode *f = (FunctionNode*)mLastTreeNode;
    f->GetFuncName()->SetIsOptional();
  } else {
    treenode->SetIsOptional();
  }

  return mLastTreeNode;
}

// Takes one argument. Set the tree as a non null node.
// We still return the previous mLastTreeNode.
TreeNode* ASTBuilder::SetIsNonNull() {
  if (mTrace)
    std::cout << "In SetIsNonNull" << std::endl;

  Param p_tree = mParams[0];
  if (!p_tree.mIsEmpty) {
    MASSERT(p_tree.mIsTreeNode);
    TreeNode *treenode = p_tree.mData.mTreeNode;
    treenode->SetIsNonNull();
  }

  return mLastTreeNode;
}

// Takes one argument. Set the tree as a rest or spread node.
// We still return the previous mLastTreeNode.
TreeNode* ASTBuilder::SetIsRest() {
  if (mTrace)
    std::cout << "In SetIsRest" << std::endl;

  Param p_tree = mParams[0];
  if (!p_tree.mIsEmpty) {
    MASSERT(p_tree.mIsTreeNode);
    TreeNode *treenode = p_tree.mData.mTreeNode;
    treenode->SetIsRest();
  }

  return mLastTreeNode;
}

// Takes one argument. Set the tree as a constant node.
// Or takes no argument. Set mLastTreeNode as constant.
// We still return the previous mLastTreeNode.
TreeNode* ASTBuilder::SetIsConst() {
  if (mTrace)
    std::cout << "In SetIsConst" << std::endl;

  TreeNode *treenode = NULL;

  if (mParams.size() == 1) {
    Param p_tree = mParams[0];
    if (!p_tree.mIsEmpty) {
      MASSERT(p_tree.mIsTreeNode);
      treenode = p_tree.mData.mTreeNode;
    }
  } else {
    treenode = mLastTreeNode;
  }

  treenode->SetIsConst();
  return mLastTreeNode;
}

// Takes one argument, which is a primary type node. Set the type as unique.
// Or takes no argument. Use mLastTreeNode as the argument.
TreeNode* ASTBuilder::SetIsUnique() {
  if (mTrace)
    std::cout << "In SetIsUnique" << std::endl;

  TreeNode *treenode = NULL;

  if (mParams.size() == 1) {
    Param p_tree = mParams[0];
    if (!p_tree.mIsEmpty) {
      MASSERT(p_tree.mIsTreeNode);
      treenode = p_tree.mData.mTreeNode;
    }
  } else {
    treenode = mLastTreeNode;
  }

  MASSERT(treenode);
  MASSERT(treenode->IsPrimType());
  PrimTypeNode *p = (PrimTypeNode*)treenode;
  p->SetIsUnique();

  return treenode;
}


// Assignment is actually a binary operator.
TreeNode* ASTBuilder::BuildAssignment() {
  if (mTrace)
    std::cout << "In assignment --> BuildBinary" << std::endl;
  return BuildBinaryOperation();
}

// Takes one argument, the result expression
// Or takes 0 argument, and it's a simple return stmt.
TreeNode* ASTBuilder::BuildReturn() {
  if (mTrace)
    std::cout << "In BuildReturn" << std::endl;

  ReturnNode *result = (ReturnNode*)gTreePool.NewTreeNode(sizeof(ReturnNode));
  new (result) ReturnNode();

  if (mParams.size() == 1) {
    Param p_result = mParams[0];
    if (!p_result.mIsEmpty) {
      if (!p_result.mIsTreeNode)
        MERROR("The return value is not a tree node.");
      TreeNode *result_value = p_result.mData.mTreeNode;
      result->SetResult(result_value);
    }
  }

  mLastTreeNode = result;
  return mLastTreeNode;
}

// Takes one argument, the result expression
// Or takes 0 argument, and it's a simple return stmt.
TreeNode* ASTBuilder::BuildYield() {
  if (mTrace)
    std::cout << "In BuildYield" << std::endl;

  YieldNode *result = (YieldNode*)gTreePool.NewTreeNode(sizeof(YieldNode));
  new (result) YieldNode();

  if (mParams.size() == 1) {
    Param p_result = mParams[0];
    if (!p_result.mIsEmpty) {
      if (!p_result.mIsTreeNode)
        MERROR("The return value is not a tree node.");
      TreeNode *result_value = p_result.mData.mTreeNode;
      result->SetResult(result_value);
    }
  }

  mLastTreeNode = result;
  return mLastTreeNode;
}

// Takes one argument, the condition expression
TreeNode* ASTBuilder::BuildCondBranch() {
  if (mTrace)
    std::cout << "In BuildCondBranch" << std::endl;

  CondBranchNode *cond_branch = (CondBranchNode*)gTreePool.NewTreeNode(sizeof(CondBranchNode));
  new (cond_branch) CondBranchNode();

  Param p_cond = mParams[0];
  if (p_cond.mIsEmpty)
    MERROR("The condition expression is empty in building conditional branch.");

  if (!p_cond.mIsTreeNode)
    MERROR("The condition expr is not a tree node.");

  TreeNode *cond_expr = p_cond.mData.mTreeNode;
  cond_branch->SetCond(cond_expr);

  mLastTreeNode = cond_branch;
  return mLastTreeNode;
}

// Takes one argument, the body of true statement/block
TreeNode* ASTBuilder::AddCondBranchTrueStatement() {
  if (mTrace)
    std::cout << "In AddCondBranchTrueStatement" << std::endl;

  CondBranchNode *cond_branch = (CondBranchNode*)mLastTreeNode;
  Param p_true = mParams[0];
  if (!p_true.mIsEmpty) {
    if (!p_true.mIsTreeNode)
      MERROR("The condition expr is not a tree node.");
    TreeNode *true_expr = p_true.mData.mTreeNode;
    cond_branch->SetTrueBranch(true_expr);
  }

  return mLastTreeNode;
}

// Takes one argument, the body of false statement/block
TreeNode* ASTBuilder::AddCondBranchFalseStatement() {
  if (mTrace)
    std::cout << "In AddCondBranchFalseStatement" << std::endl;

  CondBranchNode *cond_branch = (CondBranchNode*)mLastTreeNode;
  Param p_false = mParams[0];
  if (!p_false.mIsEmpty) {
    if (!p_false.mIsTreeNode)
      MERROR("The condition expr is not a tree node.");
    TreeNode *false_expr = p_false.mData.mTreeNode;
    cond_branch->SetFalseBranch(false_expr);
  }

  return mLastTreeNode;
}

// AddLabel tabkes two arguments, target tree, and label
TreeNode* ASTBuilder::AddLabel() {
  if (mTrace)
    std::cout << "In AddLabel " << std::endl;

  MASSERT(mParams.size() == 2 && "AddLabel has NO 2 params?");
  Param p_tree = mParams[0];
  Param p_label = mParams[1];
  MASSERT(p_tree.mIsTreeNode && "Target tree in AddLabel is not a tree.");

  TreeNode *tree = p_tree.mData.mTreeNode;

  if (p_label.mIsEmpty)
    return tree;

  // Label should be an identifier node
  MASSERT(p_label.mIsTreeNode && "Label in AddLabel is not a tree.");
  TreeNode *label = p_label.mData.mTreeNode;
  MASSERT(label->IsIdentifier() && "Label in AddLabel is not an identifier.");

  tree->SetLabel(label);

  mLastTreeNode = tree;
  return tree;
}

// BuildBreak takes 1) one argument, an identifer node
//                  2) empty
TreeNode* ASTBuilder::BuildBreak() {
  if (mTrace)
    std::cout << "In BuildBreak " << std::endl;

  BreakNode *break_node = (BreakNode*)gTreePool.NewTreeNode(sizeof(BreakNode));
  new (break_node) BreakNode();

  TreeNode *target = NULL;

  if (mParams.size() == 1) {
    Param p_target = mParams[0];
    if (!p_target.mIsEmpty) {
      MASSERT(p_target.mIsTreeNode && "Target in BuildBreak is not a tree.");
      target = p_target.mData.mTreeNode;
      MASSERT(target->IsIdentifier() && "Target in BuildBreak is not an identifier.");
      break_node->SetTarget(target);
    }
  }

  mLastTreeNode = break_node;
  return break_node;
}

// BuildContinue takes 1) one argument, an identifer node
//                  2) empty
TreeNode* ASTBuilder::BuildContinue() {
  if (mTrace)
    std::cout << "In BuildContinue " << std::endl;

  ContinueNode *continue_node = (ContinueNode*)gTreePool.NewTreeNode(sizeof(ContinueNode));
  new (continue_node) ContinueNode();

  TreeNode *target = NULL;

  if (mParams.size() == 1) {
    Param p_target = mParams[0];
    if (!p_target.mIsEmpty) {
      MASSERT(p_target.mIsTreeNode && "Target in BuildContinue is not a tree.");
      target = p_target.mData.mTreeNode;
      MASSERT(target->IsIdentifier() && "Target in BuildContinue is not an identifier.");
      continue_node->SetTarget(target);
    }
  }

  mLastTreeNode = continue_node;
  return continue_node;
}

// BuildForLoop takes four arguments.
//  1. init statement, could be a list
//  2. cond expression, should be a boolean expresion.
//  3. update statement, could be a list
//  4. body.
TreeNode* ASTBuilder::BuildForLoop() {
  if (mTrace)
    std::cout << "In BuildForLoop " << std::endl;

  ForLoopNode *for_loop = (ForLoopNode*)gTreePool.NewTreeNode(sizeof(ForLoopNode));
  new (for_loop) ForLoopNode();

  MASSERT(mParams.size() == 4 && "BuildForLoop has NO 4 params?");

  Param p_init = mParams[0];
  if (!p_init.mIsEmpty) {
    MASSERT(p_init.mIsTreeNode && "ForLoop init is not a treenode.");
    TreeNode *init = p_init.mData.mTreeNode;
    if (init->IsPass()) {
      PassNode *pass_node = (PassNode*)init;
      for (unsigned i = 0; i < pass_node->GetChildrenNum(); i++)
        for_loop->AddInit(pass_node->GetChild(i));
    } else {
      for_loop->AddInit(init);
    }
  }

  Param p_cond = mParams[1];
  if (!p_cond.mIsEmpty) {
    MASSERT(p_cond.mIsTreeNode && "ForLoop init is not a treenode.");
    TreeNode *cond = p_cond.mData.mTreeNode;
    for_loop->SetCond(cond);
  }

  Param p_update = mParams[2];
  if (!p_update.mIsEmpty) {
    MASSERT(p_update.mIsTreeNode && "ForLoop update is not a treenode.");
    TreeNode *update = p_update.mData.mTreeNode;
    if (update->IsPass()) {
      PassNode *pass_node = (PassNode*)update;
      for (unsigned i = 0; i < pass_node->GetChildrenNum(); i++)
        for_loop->AddUpdate(pass_node->GetChild(i));
    } else {
      for_loop->AddUpdate(update);
    }
  }

  Param p_body = mParams[3];
  if (!p_body.mIsEmpty) {
    MASSERT(p_body.mIsTreeNode && "ForLoop body is not a treenode.");
    TreeNode *body = p_body.mData.mTreeNode;
    for_loop->SetBody(body);
  }

  mLastTreeNode = for_loop;
  return mLastTreeNode;
}

// BuildForLoop_In takes 3 or 2 arguments.
// If 3 arguments
//  1. The decl of variable.
//  2. The first explicit arg. This is the set of data
//  3. The body.
// If 2 arguments
//  1. The implicit arg, mLastTreeNode. This is a decl of variable.
//  2. The first explicit arg. This is the set of data
//  3. The body.
TreeNode* ASTBuilder::BuildForLoop_In() {
  if (mTrace)
    std::cout << "In BuildForLoop_In " << std::endl;

  ForLoopNode *for_loop = (ForLoopNode*)gTreePool.NewTreeNode(sizeof(ForLoopNode));
  new (for_loop) ForLoopNode();
  for_loop->SetProp(FLP_JSIn);

  TreeNode *the_var = NULL;
  TreeNode *the_set = NULL;
  TreeNode *the_body = NULL;

  if (mParams.size() == 3) {
    Param p_var = mParams[0];
    if (!p_var.mIsEmpty) {
      MASSERT(p_var.mIsTreeNode);
      the_var = p_var.mData.mTreeNode;
    }

    Param p_set = mParams[1];
    if (!p_set.mIsEmpty) {
      MASSERT(p_set.mIsTreeNode);
      the_set = p_set.mData.mTreeNode;
    }

    Param p_body = mParams[2];
    if (!p_body.mIsEmpty) {
      MASSERT(p_body.mIsTreeNode);
      the_body = p_body.mData.mTreeNode;
    }
  } else {
    MASSERT(mParams.size() == 2);

    the_var = mLastTreeNode;

    Param p_set = mParams[0];
    if (!p_set.mIsEmpty) {
      MASSERT(p_set.mIsTreeNode);
      the_set = p_set.mData.mTreeNode;
    }

    Param p_body = mParams[1];
    if (!p_body.mIsEmpty) {
      MASSERT(p_body.mIsTreeNode);
      the_body = p_body.mData.mTreeNode;
    }
  }

  for_loop->SetVariable(the_var);
  for_loop->SetSet(the_set);
  for_loop->SetBody(the_body);

  mLastTreeNode = for_loop;
  return mLastTreeNode;
}

// BuildForLoop_Of takes 3 or 2 arguments.
// If 3 arguments
//  1. The decl of variable.
//  2. The first explicit arg. This is the set of data
//  3. The body.
// If 2 arguments
//  1. The implicit arg, mLastTreeNode. This is a decl of variable.
//  2. The first explicit arg. This is the set of data
//  3. The body.
TreeNode* ASTBuilder::BuildForLoop_Of() {
  if (mTrace)
    std::cout << "In BuildForLoop_Of " << std::endl;

  ForLoopNode *for_loop = (ForLoopNode*)gTreePool.NewTreeNode(sizeof(ForLoopNode));
  new (for_loop) ForLoopNode();
  for_loop->SetProp(FLP_JSOf);

  TreeNode *the_var = NULL;
  TreeNode *the_set = NULL;
  TreeNode *the_body = NULL;

  if (mParams.size() == 3) {
    Param p_var = mParams[0];
    if (!p_var.mIsEmpty) {
      MASSERT(p_var.mIsTreeNode);
      the_var = p_var.mData.mTreeNode;
    }

    Param p_set = mParams[1];
    if (!p_set.mIsEmpty) {
      MASSERT(p_set.mIsTreeNode);
      the_set = p_set.mData.mTreeNode;
    }

    Param p_body = mParams[2];
    if (!p_body.mIsEmpty) {
      MASSERT(p_body.mIsTreeNode);
      the_body = p_body.mData.mTreeNode;
    }
  } else {
    MASSERT(mParams.size() == 2);

    the_var = mLastTreeNode;

    Param p_set = mParams[0];
    if (!p_set.mIsEmpty) {
      MASSERT(p_set.mIsTreeNode);
      the_set = p_set.mData.mTreeNode;
    }

    Param p_body = mParams[1];
    if (!p_body.mIsEmpty) {
      MASSERT(p_body.mIsTreeNode);
      the_body = p_body.mData.mTreeNode;
    }
  }

  for_loop->SetVariable(the_var);
  for_loop->SetSet(the_set);
  for_loop->SetBody(the_body);

  mLastTreeNode = for_loop;
  return mLastTreeNode;
}


TreeNode* ASTBuilder::BuildWhileLoop() {
  if (mTrace)
    std::cout << "In BuildWhileLoop " << std::endl;

  WhileLoopNode *while_loop = (WhileLoopNode*)gTreePool.NewTreeNode(sizeof(WhileLoopNode));
  new (while_loop) WhileLoopNode();

  MASSERT(mParams.size() == 2 && "BuildWhileLoop has NO 2 params?");

  Param p_cond = mParams[0];
  if (!p_cond.mIsEmpty) {
    MASSERT(p_cond.mIsTreeNode && "WhileLoop condition is not a treenode.");
    TreeNode *cond = p_cond.mData.mTreeNode;
    while_loop->SetCond(cond);
  }

  Param p_body = mParams[1];
  if (!p_body.mIsEmpty) {
    MASSERT(p_body.mIsTreeNode && "WhileLoop body is not a treenode.");
    TreeNode *body = p_body.mData.mTreeNode;
    while_loop->SetBody(body);
  }

  mLastTreeNode = while_loop;
  return mLastTreeNode;
}

TreeNode* ASTBuilder::BuildDoLoop() {
  if (mTrace)
    std::cout << "In BuildDoLoop " << std::endl;

  DoLoopNode *do_loop = (DoLoopNode*)gTreePool.NewTreeNode(sizeof(DoLoopNode));
  new (do_loop) DoLoopNode();

  MASSERT(mParams.size() == 2 && "BuildDoLoop has NO 2 params?");

  Param p_cond = mParams[0];
  if (!p_cond.mIsEmpty) {
    MASSERT(p_cond.mIsTreeNode && "DoLoop condition is not a treenode.");
    TreeNode *cond = p_cond.mData.mTreeNode;
    do_loop->SetCond(cond);
  }

  Param p_body = mParams[1];
  if (!p_body.mIsEmpty) {
    MASSERT(p_body.mIsTreeNode && "DoLoop body is not a treenode.");
    TreeNode *body = p_body.mData.mTreeNode;
    do_loop->SetBody(body);
  }

  mLastTreeNode = do_loop;
  return mLastTreeNode;
}

// BuildSwitchLabel takes one argument, the expression telling the value of label.
TreeNode* ASTBuilder::BuildSwitchLabel() {
  if (mTrace)
    std::cout << "In BuildSwitchLabel " << std::endl;

  SwitchLabelNode *label =
    (SwitchLabelNode*)gTreePool.NewTreeNode(sizeof(SwitchLabelNode));
  new (label) SwitchLabelNode();

  MASSERT(mParams.size() == 1 && "BuildSwitchLabel has NO 1 params?");
  Param p_value = mParams[0];
  MASSERT(!p_value.mIsEmpty);
  MASSERT(p_value.mIsTreeNode && "Label in BuildSwitchLabel is not a tree.");

  TreeNode *value = p_value.mData.mTreeNode;
  label->SetValue(value);

  mLastTreeNode = label;
  return label;
}

// BuildDefaultSwitchLabel takes NO argument.
TreeNode* ASTBuilder::BuildDefaultSwitchLabel() {
  if (mTrace)
    std::cout << "In BuildDefaultSwitchLabel " << std::endl;
  SwitchLabelNode *label =
    (SwitchLabelNode*)gTreePool.NewTreeNode(sizeof(SwitchLabelNode));
  new (label) SwitchLabelNode();
  label->SetIsDefault(true);
  mLastTreeNode = label;
  return label;
}

// BuildOneCase takes
// 1. two arguments, the expression of a label and the statements under the label.
// 2. One arguemnt, which is the statements. The label is mLastTreeNode.

TreeNode* ASTBuilder::BuildOneCase() {
  if (mTrace)
    std::cout << "In BuildOneCase " << std::endl;

  SwitchCaseNode *case_node =
    (SwitchCaseNode*)gTreePool.NewTreeNode(sizeof(SwitchCaseNode));
  new (case_node) SwitchCaseNode();

  TreeNode *label = NULL;
  TreeNode *stmt = NULL;

  if (mParams.size() == 2) {
    Param p_label = mParams[0];
    if (!p_label.mIsEmpty) {
      MASSERT(p_label.mIsTreeNode && "Labels in BuildOneCase is not a tree.");
      label = p_label.mData.mTreeNode;
    }
    Param p_stmt = mParams[1];
    if (!p_stmt.mIsEmpty) {
      MASSERT(p_stmt.mIsTreeNode && "Stmts in BuildOneCase is not a tree.");
      stmt = p_stmt.mData.mTreeNode;
    }
  } else {
    label = mLastTreeNode;
    Param p_stmt = mParams[0];
    if (!p_stmt.mIsEmpty) {
      MASSERT(p_stmt.mIsTreeNode && "Stmts in BuildOneCase is not a tree.");
      stmt = p_stmt.mData.mTreeNode;
    }
  }

  if (label)
    case_node->AddLabel(label);
  if (stmt)
    case_node->AddStmt(stmt);

  mLastTreeNode = case_node;
  return case_node;
}

TreeNode* ASTBuilder::BuildSwitch() {
  if (mTrace)
    std::cout << "In BuildSwitch " << std::endl;

  SwitchNode *switch_node =
    (SwitchNode*)gTreePool.NewTreeNode(sizeof(SwitchNode));
  new (switch_node) SwitchNode();

  MASSERT(mParams.size() == 2 && "BuildSwitch has NO 1 params?");

  Param p_expr = mParams[0];
  MASSERT(!p_expr.mIsEmpty);
  MASSERT(p_expr.mIsTreeNode && "Expression in BuildSwitch is not a tree.");
  TreeNode *expr = p_expr.mData.mTreeNode;
  switch_node->SetExpr(expr);

  Param p_cases = mParams[1];
  MASSERT(!p_cases.mIsEmpty);
  MASSERT(p_cases.mIsTreeNode && "Cases in BuildSwitch is not a tree.");
  TreeNode *cases = p_cases.mData.mTreeNode;

  switch_node->AddSwitchCase(cases);

  mLastTreeNode = switch_node;
  return switch_node;
}

////////////////////////////////////////////////////////////////////////////////
// Issues in building declarations.
// 1) First we are going to create an IdentifierNode, which should be attached
//    to a ASTScope.
// 2) The tree is created in order of children-first, so the scope is not there yet.
//    We need have a list of pending declarations until the scope is created.
////////////////////////////////////////////////////////////////////////////////

// AddType takes two parameters, 1) tree; 2) type
// or takes one parameter, type, and apply it to mLastTreeNode
TreeNode* ASTBuilder::AddType() {
  if (mTrace)
    std::cout << "In AddType " << std::endl;

  TreeNode *node = NULL;
  TreeNode *tree_type = NULL;

  if (mParams.size() == 2) {
    Param p_type = mParams[1];
    Param p_name = mParams[0];

    if(!p_type.mIsEmpty && p_type.mIsTreeNode)
      tree_type = p_type.mData.mTreeNode;

    if (!p_name.mIsTreeNode)
      MERROR("The variable name should be a IdentifierNode already, but actually NOT?");
    node = p_name.mData.mTreeNode;

  } else {
    Param p_type = mParams[0];
    if(!p_type.mIsEmpty && p_type.mIsTreeNode)
      tree_type = p_type.mData.mTreeNode;
    node = mLastTreeNode;
  }

  if (tree_type)
    add_type_to(node, tree_type);

  mLastTreeNode = node;
  return mLastTreeNode;
}

// AddType takes two parameters, 1) tree; 2) type
// or takes one parameter, type, and apply it to mLastTreeNode
TreeNode* ASTBuilder::AddAsType() {
  if (mTrace)
    std::cout << "In AddAsType " << std::endl;

  TreeNode *node = NULL;
  TreeNode *tree_type = NULL;

  if (mParams.size() == 2) {
    Param p_type = mParams[1];
    Param p_name = mParams[0];

    if(!p_type.mIsEmpty && p_type.mIsTreeNode)
      tree_type = p_type.mData.mTreeNode;

    if (!p_name.mIsTreeNode)
      MERROR("The variable name should be a IdentifierNode already, but actually NOT?");
    node = p_name.mData.mTreeNode;

  } else {
    Param p_type = mParams[0];
    if(!p_type.mIsEmpty && p_type.mIsTreeNode)
      tree_type = p_type.mData.mTreeNode;
    node = mLastTreeNode;
  }

  if (tree_type) {
    node->AddAsTypes(tree_type);
    mLastTreeNode = node;
  }

  return mLastTreeNode;
}

// BuildDecl usually takes two parameters, 1) type; 2) name
// It can also take only one parameter: name.
// It can also take zero parameter, it's mLastTreeNode handled.
TreeNode* ASTBuilder::BuildDecl() {
  if (mTrace)
    std::cout << "In BuildDecl" << std::endl;

  TreeNode *tree_type = NULL;
  TreeNode *var = NULL;

  if (mParams.size() == 2) {
    Param p_type = mParams[0];
    Param p_name = mParams[1];
    if(!p_type.mIsEmpty && p_type.mIsTreeNode)
      tree_type = p_type.mData.mTreeNode;

    if (!p_name.mIsTreeNode)
      MERROR("The variable name should be a IdentifierNode already, but actually NOT?");
    var = p_name.mData.mTreeNode;

    if (tree_type)
      add_type_to(var, tree_type);

  } else if (mParams.size() == 1) {
    Param p_name = mParams[0];
    if (!p_name.mIsTreeNode)
      MERROR("The variable name should be a IdentifierNode already, but actually NOT?");
    var = p_name.mData.mTreeNode;
  } else {
    var = mLastTreeNode;
  }

  DeclNode *decl = decl = (DeclNode*)gTreePool.NewTreeNode(sizeof(DeclNode));
  new (decl) DeclNode(var);

  mLastTreeNode = decl;
  return decl;
}

TreeNode* ASTBuilder::SetJSVar() {
  MASSERT(mLastTreeNode->IsDecl());
  DeclNode *decl = (DeclNode*)mLastTreeNode;
  decl->SetProp(JS_Var);
  return mLastTreeNode;
}

TreeNode* ASTBuilder::SetJSLet() {
  MASSERT(mLastTreeNode->IsDecl());
  DeclNode *decl = (DeclNode*)mLastTreeNode;
  decl->SetProp(JS_Let);
  return mLastTreeNode;
}

TreeNode* ASTBuilder::SetJSConst() {
  MASSERT(mLastTreeNode->IsDecl());
  DeclNode *decl = (DeclNode*)mLastTreeNode;
  decl->SetProp(JS_Const);
  return mLastTreeNode;
}

//////////////////////////////////////////////////////////////////////////////////
//                         ArrayElement, ArrayLiteral
//////////////////////////////////////////////////////////////////////////////////

// It takes two or more than two params.
// The first is the array.
// The second is the first dimension expression
// So on so forth.

TreeNode* ASTBuilder::BuildArrayElement() {
  if (mTrace)
    std::cout << "In BuildArrayElement" << std::endl;

  MASSERT(mParams.size() >= 2);

  Param p_array = mParams[0];
  MASSERT(p_array.mIsTreeNode);
  TreeNode *array = p_array.mData.mTreeNode;
  MASSERT(array->IsIdentifier() ||
          array->IsArrayElement() ||
          array->IsField() ||
          array->IsUserType() ||
          array->IsBinOperator() ||
          array->IsCall() ||
          (array->IsLiteral() && ((LiteralNode*)array)->IsThis()) ||
          array->IsTupleType() ||
          array->IsStruct() ||
          array->IsNew() ||
          array->IsTypeOf() ||
          array->IsCast() ||
          array->IsPrimType());

  ArrayElementNode *array_element = NULL;
  if (array->IsIdentifier() ||
      array->IsField() ||
      array->IsUserType() ||
      array->IsBinOperator() ||
      array->IsCall() ||
      array->IsTupleType() ||
      (array->IsLiteral() && ((LiteralNode*)array)->IsThis()) ||
      array->IsStruct() ||
      array->IsNew() ||
      array->IsTypeOf() ||
      array->IsCast() ||
      array->IsPrimType()) {
    array_element = (ArrayElementNode*)gTreePool.NewTreeNode(sizeof(ArrayElementNode));
    new (array_element) ArrayElementNode();
    array_element->SetArray(array);
  } else {
    array_element = (ArrayElementNode*)array;
  }

  unsigned num = mParams.size() - 1;
  for (unsigned i = 0; i < num; i++) {
    Param p_index = mParams[i+1];
    MASSERT(p_index.mIsTreeNode);
    TreeNode *index = p_index.mData.mTreeNode;
    array_element->AddExpr(index);
  }

  mLastTreeNode = array_element;
  return mLastTreeNode;
}

// It takes only one parameter, the literals.
TreeNode* ASTBuilder::BuildArrayLiteral() {
  if (mTrace)
    std::cout << "In BuildArrayLiteral" << std::endl;

  MASSERT(mParams.size() == 1);

  // The parameter could be empty, meaning the literal is like: [].
  // But it still is a array literal, and we create one for it with 0 expressions.
  ArrayLiteralNode *array_literal = (ArrayLiteralNode*)gTreePool.NewTreeNode(sizeof(ArrayLiteralNode));
  new (array_literal) ArrayLiteralNode();

  Param p_literals = mParams[0];
  if (!p_literals.mIsEmpty) {
    MASSERT(p_literals.mIsTreeNode);
    TreeNode *literals = p_literals.mData.mTreeNode;
    MASSERT(literals->IsLiteral() ||
            literals->IsIdentifier() ||
            literals->IsNew() ||
            literals->IsExprList() ||
            literals->IsArrayLiteral() ||
            literals->IsStructLiteral() ||
            literals->IsFieldLiteral() ||
            literals->IsCall() ||
            literals->IsArrayElement() ||
            literals->IsField() ||
            literals->IsBinOperator() ||
            literals->IsUnaOperator() ||
            literals->IsTerOperator() ||
            literals->IsRegExpr() ||
            literals->IsFunction() ||
            literals->IsLambda());
    if (literals->IsExprList()) {
      ExprListNode *el = (ExprListNode*)literals;
      for (unsigned i = 0; i < el->GetExprsNum(); i++) {
        TreeNode *expr = el->GetExprAtIndex(i);
        MASSERT(expr->IsLiteral() ||
                expr->IsNew() ||
                expr->IsArrayLiteral() ||
                expr->IsFieldLiteral() ||
                expr->IsStructLiteral() ||
                expr->IsIdentifier() ||
                expr->IsCall() ||
                expr->IsArrayElement() ||
                expr->IsField() ||
                expr->IsBinOperator() ||
                expr->IsUnaOperator() ||
                expr->IsTerOperator() ||
                expr->IsRegExpr() ||
                expr->IsFunction() ||
                expr->IsLambda());
        array_literal->AddLiteral(expr);
      }
    } else {
      array_literal->AddLiteral(literals);
    }
  }

  mLastTreeNode = array_literal;
  return mLastTreeNode;
}

//////////////////////////////////////////////////////////////////////////////////
//                         BindingElement and BindingPattern
//////////////////////////////////////////////////////////////////////////////////

// It could take:
// 1) Two arguments, 'variable' name and 'element' to bind
// 2) one argument,  the 'element'
TreeNode* ASTBuilder::BuildBindingElement() {
  if (mTrace)
    std::cout << "In BuildBindingElement" << std::endl;

  BindingElementNode *be_node = NULL;

  if (mParams.size() == 2) {
    Param p_variable = mParams[0];
    MASSERT(p_variable.mIsTreeNode);
    TreeNode *variable = p_variable.mData.mTreeNode;

    Param p_element = mParams[1];
    MASSERT(p_element.mIsTreeNode);
    TreeNode *element = p_element.mData.mTreeNode;

    // There are a few cases.
    // 1. If element is an existing binding element, we just add the 'variable'.
    // 2. If element is a binding pattern, we need create new binding element.
    // 3. If element is anything else, we need create new binding element.
    if (element->IsBindingElement()) {
      be_node = (BindingElementNode*)element;
      be_node->SetVariable(variable);
    } else {
      be_node = (BindingElementNode*)gTreePool.NewTreeNode(sizeof(BindingElementNode));
      new (be_node) BindingElementNode();
      be_node->SetVariable(variable);
      be_node->SetElement(element);
    }
  } else if (mParams.size() == 1) {
    Param p_element = mParams[0];
    MASSERT(p_element.mIsTreeNode);
    TreeNode *element = p_element.mData.mTreeNode;

    be_node = (BindingElementNode*)gTreePool.NewTreeNode(sizeof(BindingElementNode));
    new (be_node) BindingElementNode();
    be_node->SetElement(element);
  } else {
    MASSERT(0 && "unsupported number of arguments in BuildBindingElemnt.");
  }

  mLastTreeNode = be_node;
  return mLastTreeNode;
}

// It could take:
// 1) zero arguments. it is an empty binding pattern.
// 2) one argument, the 'element' or passnode containing list of elements.
TreeNode* ASTBuilder::BuildBindingPattern() {
  if (mTrace)
    std::cout << "In BuildBindingPattern" << std::endl;

  BindingPatternNode *bp = NULL;

  if (mParams.size() == 1) {
    Param p_element = mParams[0];
    MASSERT(p_element.mIsTreeNode);
    TreeNode *element = p_element.mData.mTreeNode;

    bp = (BindingPatternNode*)gTreePool.NewTreeNode(sizeof(BindingPatternNode));
    new (bp) BindingPatternNode();
    bp->AddElement(element);
  } else if (mParams.size() == 0) {
    // an empty binding pattern
    bp = (BindingPatternNode*)gTreePool.NewTreeNode(sizeof(BindingPatternNode));
    new (bp) BindingPatternNode();
  } else {
    MASSERT(0 && "unsupported number of arguments in BuildBindingElemnt.");
  }

  mLastTreeNode = bp;
  return mLastTreeNode;
}

TreeNode* ASTBuilder::SetArrayBinding() {
  MASSERT(mLastTreeNode->IsBindingPattern());
  BindingPatternNode *b = (BindingPatternNode*)mLastTreeNode;
  b->SetProp(BPP_ArrayBinding);
  return mLastTreeNode;
}

TreeNode* ASTBuilder::SetObjectBinding() {
  MASSERT(mLastTreeNode->IsBindingPattern());
  BindingPatternNode *b = (BindingPatternNode*)mLastTreeNode;
  b->SetProp(BPP_ObjectBinding);
  return mLastTreeNode;
}

//////////////////////////////////////////////////////////////////////////////////
//                         StructNode, StructLiteralNode, FieldLiteralNode
//////////////////////////////////////////////////////////////////////////////////

// It takes two parameters: name of key, the data type.
TreeNode* ASTBuilder::BuildNumIndexSig() {
  if (mTrace)
    std::cout << "In BuildNumIndexSig" << std::endl;

  Param p_key = mParams[0];
  MASSERT(p_key.mIsTreeNode);
  TreeNode *key = p_key.mData.mTreeNode;

  Param p_data = mParams[1];
  MASSERT(p_data.mIsTreeNode);
  TreeNode *data = p_data.mData.mTreeNode;

  NumIndexSigNode *sig = (NumIndexSigNode*)gTreePool.NewTreeNode(sizeof(NumIndexSigNode));
  new (sig) NumIndexSigNode();
  sig->SetKey(key);
  sig->SetDataType(data);

  mLastTreeNode = sig;
  return mLastTreeNode;
}

// It takes two parameters: name of key, the data type.
TreeNode* ASTBuilder::BuildStrIndexSig() {
  if (mTrace)
    std::cout << "In BuildStrIndexSig" << std::endl;

  Param p_key = mParams[0];
  MASSERT(p_key.mIsTreeNode);
  TreeNode *key = p_key.mData.mTreeNode;

  Param p_data = mParams[1];
  MASSERT(p_data.mIsTreeNode);
  TreeNode *data = p_data.mData.mTreeNode;

  StrIndexSigNode *sig = (StrIndexSigNode*)gTreePool.NewTreeNode(sizeof(StrIndexSigNode));
  new (sig) StrIndexSigNode();
  sig->SetKey(key);
  sig->SetDataType(data);

  mLastTreeNode = sig;
  return mLastTreeNode;
}

// It takes only one parameter: name.
// Or it take no param, meaning the name is empty.
TreeNode* ASTBuilder::BuildStruct() {
  if (mTrace)
    std::cout << "In BuildStruct" << std::endl;

  TreeNode *name = NULL;

  if (mParams.size() == 1) {
    Param p_name = mParams[0];
    MASSERT(p_name.mIsTreeNode);
    name = p_name.mData.mTreeNode;
    MASSERT(name->IsIdentifier());
  }

  StructNode *struct_node = (StructNode*)gTreePool.NewTreeNode(sizeof(StructNode));
  new (struct_node) StructNode((IdentifierNode*)name);

  mLastTreeNode = struct_node;
  return mLastTreeNode;
}

// It take no param.
TreeNode* ASTBuilder::BuildTupleType() {
  if (mTrace)
    std::cout << "In BuildTupleType" << std::endl;

  TupleTypeNode *tuple_type = (TupleTypeNode*)gTreePool.NewTreeNode(sizeof(TupleTypeNode));
  new (tuple_type) TupleTypeNode();

  mLastTreeNode = tuple_type;
  return mLastTreeNode;
}

// It takes only one parameter: Field.
TreeNode* ASTBuilder::AddStructField() {
  if (mTrace)
    std::cout << "In AddStructField" << std::endl;
  Param p_field = mParams[0];
  if (!p_field.mIsEmpty) {
    MASSERT(p_field.mIsTreeNode);
    TreeNode *field = p_field.mData.mTreeNode;

    if (mLastTreeNode->IsStruct()) {
      StructNode *struct_node = (StructNode*)mLastTreeNode;
      struct_node->AddChild(field);
    } else if (mLastTreeNode->IsTupleType()) {
      TupleTypeNode *tt = (TupleTypeNode*)mLastTreeNode;
      tt->AddChild(field);
    } else {
      MERROR("Unsupported in AddStructField()");
    }
  }
  return mLastTreeNode;
}

TreeNode* ASTBuilder::SetTSInterface() {
  MASSERT(mLastTreeNode->IsStruct());
  StructNode *s = (StructNode*)mLastTreeNode;
  s->SetProp(SProp_TSInterface);
  return mLastTreeNode;
}

TreeNode* ASTBuilder::SetTSEnum() {
  MASSERT(mLastTreeNode->IsStruct());
  StructNode *s = (StructNode*)mLastTreeNode;
  s->SetProp(SProp_TSEnum);
  return mLastTreeNode;
}

// Build FieldLiteral
// It takes two param, field name and field value (a literal).
TreeNode* ASTBuilder::BuildFieldLiteral() {
  if (mTrace)
    std::cout << "In BuildFieldLiteral" << std::endl;

  Param p_field = mParams[0];
  MASSERT(p_field.mIsTreeNode);
  TreeNode *field = p_field.mData.mTreeNode;

  Param p_value = mParams[1];
  MASSERT(p_value.mIsTreeNode);
  TreeNode *value = p_value.mData.mTreeNode;

  FieldLiteralNode *field_literal = (FieldLiteralNode*)gTreePool.NewTreeNode(sizeof(FieldLiteralNode));
  new (field_literal) FieldLiteralNode();
  field_literal->SetFieldName(field);
  field_literal->SetLiteral(value);

  mLastTreeNode = field_literal;
  return mLastTreeNode;
}

// 1) It takes no param. We create an empty struct litreal.
// 2) It takes one param. The param could a FieldLiteralNode or
//    a PassNode containing multiple FieldLiteralNode.
//
// The param could also be a GetAccessor/SetAccessor in Javascript,
// which is a function node. We take the name of function as field name,
// the FunctionNode as the value.
TreeNode* ASTBuilder::BuildStructLiteral() {
  if (mTrace)
    std::cout << "In BuildStructLiteral" << std::endl;

  TreeNode *literal = NULL;

  if (mParams.size() == 1) {
    Param p_literal = mParams[0];
    MASSERT(p_literal.mIsTreeNode);
    literal = p_literal.mData.mTreeNode;
  }

  StructLiteralNode *struct_literal = (StructLiteralNode*)gTreePool.NewTreeNode(sizeof(StructLiteralNode));
  new (struct_literal) StructLiteralNode();

  if (literal)
    struct_literal->AddField(literal);

  mLastTreeNode = struct_literal;
  return mLastTreeNode;
}

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

// BuildField takes two parameters,
// 1) upper enclosing node, could be another field.
// 2) name of this field.
TreeNode* ASTBuilder::BuildField() {
  if (mTrace)
    std::cout << "In BuildField" << std::endl;

  MASSERT(mParams.size() == 2 && "BuildField has NO 2 params?");
  Param p_var_a = mParams[0];
  Param p_var_b = mParams[1];

  // Both variable should have been created as tree node.
  MASSERT(p_var_a.mIsTreeNode);

  // The second param should be an IdentifierNode
  TreeNode *node_a = p_var_a.mIsEmpty ? NULL : p_var_a.mData.mTreeNode;
  TreeNode *node_b = NULL;
  if (!p_var_b.mIsEmpty) {
    if (p_var_b.mIsTreeNode) {
      node_b = p_var_b.mData.mTreeNode;
      if (!node_b->IsIdentifier() && !node_b->IsComputedName()) {
        TreeNode *id = BuildIdentifier(node_b);
        if (id)
          node_b = id;
      }
    } else {
      node_b = BuildIdentifier(p_var_b.mData.mToken);
    }
  }

  FieldNode *field = NULL;

  if (node_b->IsPass()) {
    TreeNode *upper = node_a;
    PassNode *pass = (PassNode*)node_b;
    for (unsigned i = 0; i < pass->GetChildrenNum(); i++) {
      TreeNode *child = pass->GetChild(i);
      MASSERT(child->IsIdentifier());

      field = (FieldNode*)gTreePool.NewTreeNode(sizeof(FieldNode));
      new (field) FieldNode();
      field->SetUpper(upper);
      field->SetField((IdentifierNode*)child);

      upper = field;
    }
  } else {
    MASSERT(node_b->IsIdentifier() ||
            node_b->IsUserType());
    field = (FieldNode*)gTreePool.NewTreeNode(sizeof(FieldNode));
    new (field) FieldNode();
    field->SetUpper(node_a);
    field->SetField(node_b);
  }

  mLastTreeNode = field;
  return mLastTreeNode;
}

// BuildVariableList takes two parameters, var 1 and var 2
TreeNode* ASTBuilder::BuildVarList() {
  if (mTrace)
    std::cout << "In build Variable List" << std::endl;

  MASSERT(mParams.size() == 2 && "BuildVarList has NO 2 params?");
  Param p_var_a = mParams[0];
  Param p_var_b = mParams[1];

  // Both variable should have been created as tree node.
  if (!p_var_a.mIsTreeNode || !p_var_b.mIsTreeNode) {
    MERROR("The var in BuildVarList is not a treenode");
  }

  TreeNode *node_a = p_var_a.mIsEmpty ? NULL : p_var_a.mData.mTreeNode;
  TreeNode *node_b = p_var_b.mIsEmpty ? NULL : p_var_b.mData.mTreeNode;

  // There are a few different scenarios.
  // (1) node_a is a VarListNode, and we dont care about node_b
  // (2) node_a is an IdentifierNode, node_b is a VarListNode
  // (4) both are IdentifierNode
  // The solution is simple, pick an existing varListNode as the result
  // or create a new one. Merge the remaining node(s) to the result node.

  VarListNode *node_ret = NULL;
  if (node_a && node_a->IsVarList()) {
    node_ret = (VarListNode*)node_a;
    node_ret->Merge(node_b);
  } else if (node_b && node_b->IsVarList()) {
    node_ret = (VarListNode*)node_b;
    node_ret->Merge(node_a);
  } else {
    // both nodes are not VarListNode
    node_ret = (VarListNode*)gTreePool.NewTreeNode(sizeof(VarListNode));
    new (node_ret) VarListNode();
    if (node_a)
      node_ret->Merge(node_a);
    if (node_b)
      node_ret->Merge(node_b);
  }

  // Set last tree node
  mLastTreeNode = node_ret;

  return node_ret;
}

// Attach the modifier(s) to mLastTreeNode.
// It takes:
//   1. One argument, which is the solo modifier.
//   2. Two arguments, which happens mostly in Typescript, like
//        + readonly
//        - readonly
//        + ?
//        - ?
TreeNode* ASTBuilder::AddModifier() {
  if (mTrace)
    std::cout << "In AddModifier" << std::endl;

  if (mParams.size() == 1) {
    Param p_mod = mParams[0];
    if (p_mod.mIsEmpty) {
      if (mTrace)
        std::cout << " do nothing." << std::endl;
      return mLastTreeNode;
    }

    TreeNode *mod = NULL;
    if (!p_mod.mIsTreeNode) {
      Token *token = p_mod.mData.mToken;
      if (token->IsSeparator() && token->GetSepId()==SEP_Pound) {
        // This is a '#' in front of class member in Javascript.
        // This is a 'private' modifier.
        AttrNode *an = gAttrPool.GetAttrNode(ATTR_private);
        MASSERT(an);
        mod = an;
      } else {
        MERROR("The modifier is not a treenode");
      }
    } else {
      mod= p_mod.mData.mTreeNode;
    }

    if (mod->IsPass()) {
      PassNode *pass = (PassNode*)mod;
      for (unsigned i = 0; i < pass->GetChildrenNum(); i++) {
        TreeNode *child = pass->GetChild(i);
        if (child->IsAnnotation()) {
          AnnotationNode *a = (AnnotationNode*)child;
          mLastTreeNode->AddAnnotation(a);
        } else {
          add_attribute_to(mLastTreeNode, child);
        }
      }
    } else if (mod->IsAnnotation()) {
      AnnotationNode *a = (AnnotationNode*)mod;
      mLastTreeNode->AddAnnotation(a);
    } else {
      add_attribute_to(mLastTreeNode, mod);
    }
  } else if (mParams.size() == 2) {
    // the two modifiers are in fixed order, with +/- at first
    // readonly/? the second.
    bool add = false;   // add opr if true, rem if false
    bool readonly = false;
    bool optional = false;
    Param p_opr = mParams[0];
    if (p_opr.mIsEmpty) {
      add = true;
    } else {
      MASSERT(!p_opr.mIsTreeNode);
      Token *token= p_opr.mData.mToken;
      if (token->GetOprId() == OPR_Add)
        add = true;
      else if (token->GetOprId() == OPR_Sub)
        add = false;
      else
        MERROR("unsupported opr id.");
    }

    Param p_prop = mParams[1];
    if (!p_prop.mIsEmpty) {
      if (!p_prop.mIsTreeNode) {
        Token *token= p_prop.mData.mToken;
        MASSERT(token->IsSeparator() && (token->GetSepId() == SEP_Select));
        optional = true;
      } else {
        TreeNode *tree = p_prop.mData.mTreeNode;
        MASSERT(tree->IsAttr());
        AttrNode *attr = (AttrNode*)tree;
        MASSERT(attr->GetId() == ATTR_readonly);
        readonly = true;
      }

      MASSERT(mLastTreeNode->IsComputedName());
      ComputedNameNode *cnn = (ComputedNameNode*)mLastTreeNode;
      if (add) {
        if (readonly)
          cnn->SetProp((unsigned)CNP_Add_ReadOnly);
        else if (optional)
          cnn->SetProp((unsigned)CNP_Add_Optional);
        else
          MERROR("unsupported property.");
      } else {
        if (readonly)
          cnn->SetProp((unsigned)CNP_Rem_ReadOnly);
        else if (optional)
          cnn->SetProp((unsigned)CNP_Rem_Optional);
        else
          MERROR("unsupported property.");
      }
    }
  }

  return mLastTreeNode;
}

// Takes two arguments. 1) target tree node; 2) modifier, which could be
// attr, annotation/pragma.
//
// This function doesn't update mLastTreeNode.
TreeNode* ASTBuilder::AddModifierTo() {
  if (mTrace)
    std::cout << "In AddModifierTo " << std::endl;

  Param p_tree = mParams[0];
  MASSERT(!p_tree.mIsEmpty && "Treenode cannot be empty in AddModifierTo");
  MASSERT(p_tree.mIsTreeNode && "The tree node is not a treenode in AddModifierTo()");
  TreeNode *tree = p_tree.mData.mTreeNode;

  Param p_mod = mParams[1];
  if(p_mod.mIsEmpty)
    return tree;

  MASSERT(p_mod.mIsTreeNode && "The Attr is not a treenode in AddModifierTo()");
  TreeNode *mod = p_mod.mData.mTreeNode;

  if (mod->IsPass()) {
    PassNode *pass = (PassNode*)mod;
    for (unsigned i = 0; i < pass->GetChildrenNum(); i++) {
      TreeNode *child = pass->GetChild(i);
      if (child->IsAnnotation()) {
        AnnotationNode *a = (AnnotationNode*)child;
        tree->AddAnnotation(a);
      } else {
        add_attribute_to(tree, child);
      }
    }
  } else if (mod->IsAnnotation()) {
    AnnotationNode *a = (AnnotationNode*)mod;
    tree->AddAnnotation(a);
  } else {
    add_attribute_to(tree, mod);
  }

  return tree;
}

// It takes one argument, the init.
// Apply init to mLastTreeNode
TreeNode* ASTBuilder::AddInit() {
  if (mTrace)
    std::cout << "In AddInit" << std::endl;

  MASSERT(mParams.size() == 1);

  Param p_init = mParams[0];
  if (p_init.mIsEmpty)
    return NULL;

  if (!p_init.mIsTreeNode)
    MERROR("The init is not a treenode in AddInit()");

  TreeNode *node_init = p_init.mData.mTreeNode;

  if (mLastTreeNode->IsIdentifier()) {
    IdentifierNode *in = (IdentifierNode*)mLastTreeNode;
    in->SetInit(node_init);
    return in;
  } else if (mLastTreeNode->IsBindingPattern()) {
    BindingPatternNode *in = (BindingPatternNode*)mLastTreeNode;
    in->SetInit(node_init);
    return in;
  } else if (mLastTreeNode->IsTypeParameter()) {
    TypeParameterNode *in = (TypeParameterNode*)mLastTreeNode;
    in->SetDefault(node_init);
    return in;
  } else {
    MERROR("The target of AddInit is unsupported.");
  }
}

// It takes (1) two arguments or (2) one argument
TreeNode* ASTBuilder::AddInitTo() {
  if (mTrace)
    std::cout << "In AddInitTo" << std::endl;

  TreeNode *node_decl = NULL;
  TreeNode *node_init = NULL;

  // If there is no init value, return NULL.
  if (mParams.size() == 1) {
    Param p_init = mParams[0];
    if (p_init.mIsEmpty)
      return NULL;
    node_init = p_init.mData.mTreeNode;
    node_decl = mLastTreeNode;
  } else {
    Param p_decl = mParams[0];
    Param p_init;
    p_init = mParams[1];
    if (p_init.mIsEmpty)
      return NULL;

    // Both variable should have been created as tree node.
    if (!p_decl.mIsTreeNode || !p_init.mIsTreeNode)
      MERROR("The decl or init is not a treenode in AddInitTo()");

    node_decl = p_decl.mData.mTreeNode;
    node_init = p_init.mData.mTreeNode;
  }

  if (node_decl->IsIdentifier()) {
    IdentifierNode *in = (IdentifierNode*)node_decl;
    in->SetInit(node_init);
    return in;
  } else if (node_decl->IsBindingPattern()) {
    BindingPatternNode *in = (BindingPatternNode*)node_decl;
    in->SetInit(node_init);
    return in;
  } else if (node_decl->IsComputedName()) {
    ComputedNameNode *in = (ComputedNameNode*)node_decl;
    in->SetInit(node_init);
    return in;
  } else if (node_decl->IsLiteral()) {
    LiteralNode *in = (LiteralNode*)node_decl;
    in->SetInit(node_init);
    return in;
  } else {
    MERROR("The target of AddInitTo is unsupported.");
  }
}

// This takes just one argument which is the namespace name.
TreeNode* ASTBuilder::BuildNamespace() {
  if (mTrace)
    std::cout << "In BuildNamespace" << std::endl;

  Param p_name = mParams[0];
  MASSERT(p_name.mIsTreeNode);
  TreeNode *node_name = p_name.mData.mTreeNode;

  MASSERT(node_name->IsIdentifier() || node_name->IsField());

  NamespaceNode *ns = (NamespaceNode*)gTreePool.NewTreeNode(sizeof(NamespaceNode));
  new (ns) NamespaceNode();
  ns->SetId(node_name);

  mLastTreeNode = ns;
  return mLastTreeNode;
}

// Takes one parameter which is the tree of namespace body.
TreeNode* ASTBuilder::AddNamespaceBody() {
  if (mTrace)
    std::cout << "In AddNamespaceBody" << std::endl;

  Param p_body = mParams[0];
  if (!p_body.mIsEmpty) {
    if(!p_body.mIsTreeNode)
      MERROR("The namespace body is not a tree node.");
    TreeNode *tree = p_body.mData.mTreeNode;

    MASSERT(mLastTreeNode->IsNamespace());
    NamespaceNode *ns = (NamespaceNode*)mLastTreeNode;
    ns->AddBody(tree);
  }

  return mLastTreeNode;
}

// This takes just one argument which is the class name.
TreeNode* ASTBuilder::BuildClass() {
  if (mTrace)
    std::cout << "In BuildClass" << std::endl;

  IdentifierNode *in = NULL;

  Param p_name = mParams[0];
  if (!p_name.mIsEmpty) {
    if (!p_name.mIsTreeNode)
      MERROR("The class name is not a treenode in BuildClass()");
    TreeNode *node_name = p_name.mData.mTreeNode;

    if (!node_name->IsIdentifier())
      MERROR("The class name should be an indentifier node. Not?");
    in = (IdentifierNode*)node_name;
  }

  ClassNode *node_class = (ClassNode*)gTreePool.NewTreeNode(sizeof(ClassNode));
  new (node_class) ClassNode();
  if (in)
    node_class->SetStrIdx(in->GetStrIdx());

  mLastTreeNode = node_class;
  return mLastTreeNode;
}

TreeNode* ASTBuilder::SetClassIsJavaEnum() {
  ClassNode *klass = (ClassNode*)mLastTreeNode;
  klass->SetIsJavaEnum();
  return mLastTreeNode;
}

// This takes just one argument which is the root of sub tree
TreeNode* ASTBuilder::BuildBlock() {
  if (mTrace)
    std::cout << "In BuildBlock" << std::endl;

  BlockNode *block = (BlockNode*)gTreePool.NewTreeNode(sizeof(BlockNode));
  new (block) BlockNode();

  Param p_subtree = mParams[0];
  if (!p_subtree.mIsEmpty) {
    if (!p_subtree.mIsTreeNode)
      MERROR("The subtree is not a treenode in BuildBlock()");

    // If the subtree is PassNode, we need add all children to block
    // If else, simply assign subtree as child.
    TreeNode *subtree = p_subtree.mData.mTreeNode;
    if (subtree->IsPass()) {
      PassNode *pass_node = (PassNode*)subtree;
      for (unsigned i = 0; i < pass_node->GetChildrenNum(); i++)
        block->AddChild(pass_node->GetChild(i));
    } else {
      block->AddChild(subtree);
    }
  }

  mLastTreeNode = block;
  return mLastTreeNode;
}

// This takes just one argument which is the root of tree to be added.
TreeNode* ASTBuilder::AddToBlock() {
  if (mTrace)
    std::cout << "In AddToBlock" << std::endl;

  MASSERT(mLastTreeNode->IsBlock());
  BlockNode *block = (BlockNode*)mLastTreeNode;

  Param p_subtree = mParams[0];
  if (!p_subtree.mIsEmpty) {
    if (!p_subtree.mIsTreeNode)
      MERROR("The subtree is not a treenode in BuildBlock()");

    // If the subtree is PassNode, we need add all children to block
    // If else, simply assign subtree as child.
    TreeNode *subtree = p_subtree.mData.mTreeNode;
    if (subtree->IsPass()) {
      PassNode *pass_node = (PassNode*)subtree;
      for (unsigned i = 0; i < pass_node->GetChildrenNum(); i++)
        block->AddChild(pass_node->GetChild(i));
    } else {
      block->AddChild(subtree);
    }
  }

  // set last tree node
  mLastTreeNode = block;
  return mLastTreeNode;
}

// This takes just two arguments. First is the sync object, second the block
// It returns the block with sync added.
TreeNode* ASTBuilder::AddSyncToBlock() {
  if (mTrace)
    std::cout << "In AddSyncToBlock" << std::endl;

  Param p_sync = mParams[0];
  MASSERT(!p_sync.mIsEmpty && p_sync.mIsTreeNode);
  TreeNode *sync_tree = p_sync.mData.mTreeNode;

  Param p_block = mParams[1];
  MASSERT(!p_block.mIsEmpty && p_block.mIsTreeNode);
  TreeNode *b = p_block.mData.mTreeNode;
  MASSERT(b->IsBlock());
  BlockNode *block = (BlockNode*)b;

  block->SetSync(sync_tree);

  // set last tree node
  mLastTreeNode = block;
  return mLastTreeNode;
}

// This takes just one argument which either a block node, or the root of sub tree
TreeNode* ASTBuilder::BuildInstInit() {
  if (mTrace)
    std::cout << "In BuildInstInit" << std::endl;

  BlockNode *b = NULL;

  Param p_subtree = mParams[0];
  if (!p_subtree.mIsEmpty) {
    if (!p_subtree.mIsTreeNode)
      MERROR("The subtree is not a treenode in BuildInstInit()");

    TreeNode *subtree = p_subtree.mData.mTreeNode;
    if (subtree->IsBlock()) {
      b = (BlockNode*)subtree;
      b->SetIsInstInit();
    }
  }

  if (!b) {
    b = (BlockNode*)BuildBlock();
    b->SetIsInstInit();
  }

  // set last tree node
  mLastTreeNode = b;
  return mLastTreeNode;
}

TreeNode* ASTBuilder::AddSuperClass() {
  if (mTrace)
    std::cout << "In AddSuperClass" << std::endl;
  Param p_super = mParams[0];
  if (p_super.mIsEmpty)
    return mLastTreeNode;

  MASSERT(p_super.mIsTreeNode);
  TreeNode *t_super = p_super.mData.mTreeNode;

  if (mLastTreeNode->IsClass()) {
    ClassNode *sn = (ClassNode*)mLastTreeNode;
    sn->AddSuperClass(t_super);
  }

  return mLastTreeNode;
}

// It takes one argument, the super interface.
// Add it to the mLastTreeNode.
TreeNode* ASTBuilder::AddSuperInterface() {
  if (mTrace)
    std::cout << "In AddSuperInterface" << std::endl;
  Param p_super = mParams[0];
  if (p_super.mIsEmpty)
    return mLastTreeNode;

  MASSERT(p_super.mIsTreeNode);
  TreeNode *t_super = p_super.mData.mTreeNode;

  if (mLastTreeNode->IsStruct()) {
    StructNode *sn = (StructNode*)mLastTreeNode;
    sn->AddSuper(t_super);
  } else if (mLastTreeNode->IsClass()) {
    ClassNode *sn = (ClassNode*)mLastTreeNode;
    sn->AddSuperInterface(t_super);
  }

  return mLastTreeNode;
}

// Takes one parameter which is the tree of class body.
TreeNode* ASTBuilder::AddClassBody() {
  if (mTrace)
    std::cout << "In AddClassBody" << std::endl;

  Param p_body = mParams[0];
  if (p_body.mIsEmpty)
    return mLastTreeNode;

  if (!p_body.mIsTreeNode)
    MERROR("The class body is not a tree node.");

  TreeNode *tn = p_body.mData.mTreeNode;
  if (!tn->IsBlock()) {
    BlockNode *block = (BlockNode*)gTreePool.NewTreeNode(sizeof(BlockNode));
    new (block) BlockNode();
    block->AddChild(tn);
    tn = block;
  }
  BlockNode *block = (BlockNode*)(tn);

  MASSERT(mLastTreeNode->IsClass() && "Class is not a ClassNode?");
  ClassNode *klass = (ClassNode*)mLastTreeNode;
  klass->Construct(block);

  return mLastTreeNode;
}

// This takes just one argument which is the annotation type name.
TreeNode* ASTBuilder::BuildAnnotationType() {
  if (mTrace)
    std::cout << "In BuildAnnotationType" << std::endl;

  Param p_name = mParams[0];

  if (!p_name.mIsTreeNode)
    MERROR("The annotationtype name is not a treenode in BuildAnnotationtType()");
  TreeNode *node_name = p_name.mData.mTreeNode;

  if (!node_name->IsIdentifier())
    MERROR("The annotation type name should be an indentifier node. Not?");
  IdentifierNode *in = (IdentifierNode*)node_name;

  AnnotationTypeNode *annon_type = (AnnotationTypeNode*)gTreePool.NewTreeNode(sizeof(AnnotationTypeNode));
  new (annon_type) AnnotationTypeNode();
  annon_type->SetId(in);

  // set last tree node and return it.
  mLastTreeNode = annon_type;
  return mLastTreeNode;
}

TreeNode* ASTBuilder::AddAnnotationTypeBody() {
  if (mTrace)
    std::cout << "In AddAnnotationTypeBody" << std::endl;
  Param p_attr = mParams[0];
  return mLastTreeNode;
}

TreeNode* ASTBuilder::BuildAnnotation() {
  if (mTrace)
    std::cout << "In BuildAnnotation" << std::endl;
  Param p_name = mParams[0];

  if (!p_name.mIsTreeNode)
    MERROR("The annotationtype name is not a treenode in BuildAnnotation()");
  TreeNode *iden = p_name.mData.mTreeNode;

  AnnotationNode *annot = (AnnotationNode*)gTreePool.NewTreeNode(sizeof(AnnotationNode));
  new (annot) AnnotationNode();
  annot->SetId(iden);

  // set last tree node and return it.
  mLastTreeNode = annot;
  return mLastTreeNode;
}

TreeNode* ASTBuilder::BuildInterface() {
  if (mTrace)
    std::cout << "In BuildInterface" << std::endl;
  Param p_name = mParams[0];

  if (!p_name.mIsTreeNode)
    MERROR("The name is not a treenode in BuildInterface()");
  TreeNode *node_name = p_name.mData.mTreeNode;

  if (!node_name->IsIdentifier())
    MERROR("The name is NOT an indentifier node.");
  IdentifierNode *in = (IdentifierNode*)node_name;

  InterfaceNode *interf = (InterfaceNode*)gTreePool.NewTreeNode(sizeof(InterfaceNode));
  new (interf) InterfaceNode();
  interf->SetStrIdx(in->GetStrIdx());

  // set last tree node and return it.
  mLastTreeNode = interf;
  return mLastTreeNode;
}

// Takes one parameter which is the tree of interface body.
TreeNode* ASTBuilder::AddInterfaceBody() {
  if (mTrace)
    std::cout << "In AddInterfaceBody" << std::endl;

  Param p_body = mParams[0];
  if (!p_body.mIsTreeNode)
    MERROR("The interface body is not a tree node.");
  TreeNode *tree_node = p_body.mData.mTreeNode;
  MASSERT(tree_node->IsBlock() && "Interface body is not a BlockNode?");
  BlockNode *block = (BlockNode*)tree_node;

  MASSERT(mLastTreeNode->IsInterface() && "Interface is not a InterfaceNode?");
  InterfaceNode *interf = (InterfaceNode*)mLastTreeNode;
  interf->Construct(block);
  return interf;
}

// This takes just one argument which is the length of this dimension
// [TODO] Don't support yet.
TreeNode* ASTBuilder::BuildDim() {
  if (mTrace)
    std::cout << "In BuildDim" << std::endl;

  DimensionNode *dim = (DimensionNode*)gTreePool.NewTreeNode(sizeof(DimensionNode));
  new (dim) DimensionNode();
  dim->AddDimension();

  // set last tree node and return it.
  mLastTreeNode = dim;
  return mLastTreeNode;
}

// BuildDims() takes two parameters. Each contains a set of dimension info.
// Each is a DimensionNode.
TreeNode* ASTBuilder::BuildDims() {
  if (mTrace)
    std::cout << "In build dimension List" << std::endl;

  MASSERT(mParams.size() == 2 && "BuildDims has NO 2 params?");
  Param p_dims_a = mParams[0];
  Param p_dims_b = mParams[1];

  // Both variable should have been created as tree node.
  if (!p_dims_a.mIsTreeNode || !p_dims_b.mIsTreeNode) {
    MERROR("The var in BuildVarList is not a treenode");
  }

  TreeNode *node_a = p_dims_a.mIsEmpty ? NULL : p_dims_a.mData.mTreeNode;
  TreeNode *node_b = p_dims_b.mIsEmpty ? NULL : p_dims_b.mData.mTreeNode;

  // Pick an existing node, merge the other into to.
  DimensionNode *node_ret = NULL;
  if (node_a) {
    node_ret = (DimensionNode*)node_a;
    node_ret->Merge(node_b);
  } else if (node_b) {
    node_ret = (DimensionNode*)node_b;
    node_ret->Merge(node_a);
  } else {
    // both nodes are NULL
    MERROR("BuildDims() has two NULL parameters?");
  }

  // Set last tree node
  mLastTreeNode = node_ret;
  return node_ret;
}

// AddDimsTo() takes two parameters. The first is the variable,
// the second is the dims.
TreeNode* ASTBuilder::AddDimsTo() {
  if (mTrace)
    std::cout << "In AddDimsTo " << std::endl;

  MASSERT(mParams.size() == 2 && "AddDimsTo has NO 2 params?");
  Param p_dims_a = mParams[0];
  Param p_dims_b = mParams[1];

  // Both variable should have been created as tree node.
  if (p_dims_a.mIsEmpty)
    MERROR("The var in AddDimsTo() is empty?");

  TreeNode *node_a = p_dims_a.mData.mTreeNode;
  TreeNode *node_b = p_dims_b.mIsEmpty ? NULL : p_dims_b.mData.mTreeNode;

  if (node_b) {
    MASSERT(node_b->IsDimension() && "Expected a DimensionNode.");
    DimensionNode *dim = (DimensionNode*)node_b;
    if (node_a->IsIdentifier()) {
      IdentifierNode *inode = (IdentifierNode*)node_a;
      inode->SetDims(dim);
      mLastTreeNode = node_a;
    } else if (node_a->IsPrimType()) {
      PrimTypeNode *pt = (PrimTypeNode*)node_a;
      PrimArrayTypeNode *pat = (PrimArrayTypeNode*)gTreePool.NewTreeNode(sizeof(PrimArrayTypeNode));
      new (pat) PrimArrayTypeNode();
      pat->SetPrim(pt);
      pat->SetDims(dim);
      mLastTreeNode = pat;
    }
  }

  return mLastTreeNode;
}

// AddDims() takes one parameters, the dims.
// Add to mLastTreeNode
TreeNode* ASTBuilder::AddDims() {
  if (mTrace)
    std::cout << "In AddDims " << std::endl;

  Param p_dims = mParams[0];

  if (p_dims.mIsEmpty)
    return mLastTreeNode;

  TreeNode *param_tree = p_dims.mData.mTreeNode;
  DimensionNode *dims = NULL;
  if (param_tree) {
    MASSERT(param_tree->IsDimension() && "Expected a DimensionNode.");
    dims = (DimensionNode*)param_tree;
  }

  if (mLastTreeNode->IsIdentifier()) {
    IdentifierNode *node = (IdentifierNode*)mLastTreeNode;
    node->SetDims(dims);
  } else if (mLastTreeNode->IsFunction()) {
    FunctionNode *node = (FunctionNode*)mLastTreeNode;
    node->SetDims(dims);
  }

  return mLastTreeNode;
}

// This is a help function which adds parameters to a function decl.
// It's the caller's duty to assure 'func' and 'params' are non null.
void ASTBuilder::AddParams(TreeNode *func, TreeNode *decl_params) {
  if (decl_params->IsDecl()) {
    DeclNode *decl = (DeclNode*)decl_params;
    TreeNode *params = decl->GetVar();
    // a param could be a 'this' literal, binding pattern, etc
    if (params->IsIdentifier() || params->IsLiteral() || params->IsBindingPattern()) {
      // one single parameter at call site
      if (func->IsFunction())
        ((FunctionNode*)func)->AddParam(params);
      else if (func->IsLambda())
        ((LambdaNode*)func)->AddParam(params);
      else
        MERROR("Unsupported yet.");
    } else if (params->IsVarList()) {
      // a list of decls at function declaration
      VarListNode *vl = (VarListNode*)params;
      for (unsigned i = 0; i < vl->GetVarsNum(); i++) {
        IdentifierNode *inode = vl->GetVarAtIndex(i);
        if (func->IsFunction())
          ((FunctionNode*)func)->AddParam(inode);
        else if (func->IsLambda())
          ((LambdaNode*)func)->AddParam(inode);
        else
          MERROR("Unsupported yet.");
      }
    } else {
      MERROR("Unsupported yet.");
    }
  } else if (decl_params->IsPass()) {
    PassNode *pass = (PassNode*)decl_params;
    for (unsigned i = 0; i < pass->GetChildrenNum(); i++) {
      TreeNode *child = pass->GetChild(i);
      AddParams(func, child);
    }
  } else if (decl_params->IsIdentifier()) {
    // sometimes, the parameter is just an identifier in Javascript.
    // Like the SetAccessor
    if (func->IsFunction())
      ((FunctionNode*)func)->AddParam(decl_params);
    else if (func->IsLambda())
      ((LambdaNode*)func)->AddParam(decl_params);
    else
      MERROR("Unsupported yet.");
  } else if (decl_params->IsStruct()) {
    if (func->IsFunction())
      ((FunctionNode*)func)->AddParam(decl_params);
    else if (func->IsLambda())
      ((LambdaNode*)func)->AddParam(decl_params);
    else
      MERROR("Unsupported yet.");
  } else {
    MERROR("Unsupported yet.");
  }
}

// AddAssert takes one parameter, the asserts expression, and apply it to mLastTreeNode
TreeNode* ASTBuilder::AddAssert() {
  if (mTrace)
    std::cout << "In AddAssert " << std::endl;

  FunctionNode *f = NULL;
  TreeNode *a = NULL;

  Param p_type = mParams[0];
  if(!p_type.mIsEmpty && p_type.mIsTreeNode) {
    a = p_type.mData.mTreeNode;
    MASSERT(mLastTreeNode->IsFunction());
    f = (FunctionNode*)mLastTreeNode;
  }

  if (f && a)
    f->SetAssert(a);

  return mLastTreeNode;
}
////////////////////////////////////////////////////////////////////////////////
//                    New & Delete operation related
////////////////////////////////////////////////////////////////////////////////

// This function takes one, two or three arguments.
// 1. The id of the class/interface/function/..., or a lambda
// 2. The arguments, could be empty
// 3. In some cases there is a third argument, for function body.
TreeNode* ASTBuilder::BuildNewOperation() {
  if (mTrace)
    std::cout << "In BuildNewOperation " << std::endl;

  NewNode *new_node = (NewNode*)gTreePool.NewTreeNode(sizeof(NewNode));
  new (new_node) NewNode();

  // Name could not be empty
  Param p_a = mParams[0];
  if (p_a.mIsEmpty)
    MERROR("The name in BuildNewOperation() is empty?");
  MASSERT(p_a.mIsTreeNode && "Name of new expression is not a tree?");
  TreeNode *name = p_a.mData.mTreeNode;
  new_node->SetId(name);

  if (mParams.size() > 1) {
    Param p_b = mParams[1];
    TreeNode *node_b = p_b.mIsEmpty ? NULL : p_b.mData.mTreeNode;
    if (node_b)
      AddArguments(new_node, node_b);
  }

  if (mParams.size() > 2) {
    Param p_c = mParams[2];
    TreeNode *node_c = p_c.mIsEmpty ? NULL : p_c.mData.mTreeNode;
    if (node_c) {
      MASSERT(node_c->IsBlock() && "ClassBody is not a block?");
      BlockNode *b = (BlockNode*)node_c;
      new_node->SetBody(b);
    }
  }

  mLastTreeNode = new_node;
  return new_node;
}

TreeNode* ASTBuilder::BuildDeleteOperation() {
  if (mTrace)
    std::cout << "In BuildDelete" << std::endl;

  Param l_param = mParams[0];
  MASSERT(!l_param.mIsEmpty);
  MASSERT(l_param.mIsTreeNode);
  TreeNode *expr = l_param.mData.mTreeNode;

  DeleteNode *d_node = (DeleteNode*)gTreePool.NewTreeNode(sizeof(DeleteNode));
  new (d_node) DeleteNode();
  d_node->SetExpr(expr);

  mLastTreeNode = d_node;
  return mLastTreeNode;
}

////////////////////////////////////////////////////////////////////////////////
//                    Building AssertNode related
// The node could take two parameters, one expression and one message.
// It also could take only one parameter, the expression.
////////////////////////////////////////////////////////////////////////////////

TreeNode* ASTBuilder::BuildAssert() {
  if (mTrace)
    std::cout << "In BuildAssert " << std::endl;

  AssertNode *assert_node = (AssertNode*)gTreePool.NewTreeNode(sizeof(AssertNode));
  new (assert_node) AssertNode();

  MASSERT(mParams.size() >= 1 && "BuildAssert has NO expression?");
  Param p_a, p_b;

  p_a = mParams[0];
  if (p_a.mIsEmpty)
    MERROR("The expression in BuildAssert() is empty?");
  MASSERT(p_a.mIsTreeNode && "Expression is not a tree?");
  TreeNode *expr = p_a.mData.mTreeNode;
  assert_node->SetExpr(expr);

  if (mParams.size() == 2) {
    p_b = mParams[1];
    if (!p_b.mIsEmpty) {
      MASSERT(p_b.mIsTreeNode && "Messge of assert is not a tree?");
      TreeNode *node_b = p_b.mData.mTreeNode;
      if (node_b)
        assert_node->SetMsg(node_b);
    }
  }

  mLastTreeNode = assert_node;
  return assert_node;
}

////////////////////////////////////////////////////////////////////////////////
//                    CallSite related
////////////////////////////////////////////////////////////////////////////////

// There are two formats of BuildCall, one with one param, the other no param.
TreeNode* ASTBuilder::BuildCall() {
  if (mTrace)
    std::cout << "In BuildCall" << std::endl;

  CallNode *call = (CallNode*)gTreePool.NewTreeNode(sizeof(CallNode));
  new (call) CallNode();

  // The default is having no param.
  TreeNode *method = mLastTreeNode;

  if (!ParamsEmpty()) {
    Param p_method = mParams[0];
    if (!p_method.mIsTreeNode)
      MERROR("The function name is not a treenode in BuildCall()");
    method = p_method.mData.mTreeNode;
  }

  // In Typescript, get/set are keywords of attributes. But it also allowed to be
  // function name. So we need transfer this AttrNode to IdentifierNode.
  if (method && method->IsAttr())
    method = BuildIdentifier(method);

  call->SetMethod(method);

  mLastTreeNode = call;
  return mLastTreeNode;
}

// The argument could be any kind of expression, like arithmetic expression,
// identifier, call, or any valid expression.
//
// This AddArguments can be used for CallNode, NewNode, etc.

TreeNode* ASTBuilder::AddArguments() {
  if (mTrace)
    std::cout << "In AddArguments" << std::endl;

  Param p_params = mParams[0];
  TreeNode *args = NULL;
  if (!p_params.mIsEmpty) {
    if (!p_params.mIsTreeNode)
      MERROR("The parameters is not a treenode in AddArguments()");
    args = p_params.mData.mTreeNode;
  }

  if (!args)
    return mLastTreeNode;

  AddArguments(mLastTreeNode, args);

  return mLastTreeNode;
}

// 'call' could be a CallNode or NewNode.
// 'args' could be identifier, literal, expr, etc.
void ASTBuilder::AddArguments(TreeNode *call, TreeNode *args) {
  CallNode *callnode = NULL;
  NewNode *newnode = NULL;
  AnnotationNode *annotation = NULL;
  if (call->IsCall())
    callnode = (CallNode*)call;
  else if (call->IsNew())
    newnode = (NewNode*)call;
  else if (call->IsAnnotation())
    annotation = (AnnotationNode*)call;
  else
    MERROR("Unsupported call node.");

  if (args->IsVarList()) {
    VarListNode *vl = (VarListNode*)args;
    for (unsigned i = 0; i < vl->GetVarsNum(); i++) {
      IdentifierNode *inode = vl->GetVarAtIndex(i);
      if (callnode)
        callnode->AddArg(inode);
      else if (newnode)
        newnode->AddArg(inode);
      else if (annotation)
        annotation->AddArg(inode);
    }
  } else if (args->IsPass()) {
    PassNode *pass = (PassNode*)args;
    for (unsigned i = 0; i < pass->GetChildrenNum(); i++) {
      TreeNode *child = pass->GetChild(i);
      AddArguments(call, child);
    }
  } else {
    if (callnode)
      callnode->AddArg(args);
    else if (newnode)
      newnode->AddArg(args);
    else if (annotation)
      annotation->AddArg(args);
  }
}

// BuildVariableList takes two parameters, var 1 and var 2
TreeNode* ASTBuilder::BuildExprList() {
  if (mTrace)
    std::cout << "In build Expr List" << std::endl;

  MASSERT(mParams.size() == 2 && "BuildExprList has NO 2 params?");
  Param p_var_a = mParams[0];
  Param p_var_b = mParams[1];

  // Both variable should have been created as tree node.
  if (!p_var_a.mIsTreeNode || !p_var_b.mIsTreeNode) {
    MERROR("The expr in BuildExprList is not a treenode");
  }

  TreeNode *node_a = p_var_a.mIsEmpty ? NULL : p_var_a.mData.mTreeNode;
  TreeNode *node_b = p_var_b.mIsEmpty ? NULL : p_var_b.mData.mTreeNode;

  ExprListNode *node_ret = NULL;
  if (node_a && node_a->IsExprList()) {
    node_ret = (ExprListNode*)node_a;
    node_ret->Merge(node_b);
  } else if (node_b && node_b->IsExprList()) {
    node_ret = (ExprListNode*)node_b;
    node_ret->Merge(node_a);
  } else {
    // both nodes are not ExprListNode
    node_ret = (ExprListNode*)gTreePool.NewTreeNode(sizeof(ExprListNode));
    new (node_ret) ExprListNode();
    if (node_a)
      node_ret->Merge(node_a);
    if (node_b)
      node_ret->Merge(node_b);
  }

  // Set last tree node
  mLastTreeNode = node_ret;

  return node_ret;
}

////////////////////////////////////////////////////////////////////////////////
//                    FunctionNode related
////////////////////////////////////////////////////////////////////////////////

// Takes only one argument, the params, and add it to mLastTreeNode
TreeNode* ASTBuilder::AddParams() {
  if (mTrace)
    std::cout << "In AddParams" << std::endl;

  Param p_params = mParams[0];
  if (!p_params.mIsEmpty) {
    if (!p_params.mIsTreeNode)
      MERROR("The parameters is not a treenode in AddParams()");
    TreeNode *params = p_params.mData.mTreeNode;
    AddParams(mLastTreeNode, params);
  }

  return mLastTreeNode;
}

// Takes one argument, set it as optional param.
TreeNode* ASTBuilder::SetOptionalParam() {
  if (mTrace)
    std::cout << "In SetOptionalParam" << std::endl;

  MASSERT(mParams.size() == 1);
  Param p_param = mParams[0];
  MASSERT(!p_param.mIsEmpty && p_param.mIsTreeNode);
  TreeNode *param = p_param.mData.mTreeNode;
  if (param->IsIdentifier()) {
    IdentifierNode *id = (IdentifierNode*)param;
    id->SetOptionalParam(true);
  } else if (param->IsBindingPattern()) {
    BindingPatternNode *id = (BindingPatternNode*)param;
    id->SetIsOptional();
  } else {
    MERROR("Unsupported optional param.");
  }

  mLastTreeNode = param;
  return mLastTreeNode;
}

// This takes just one argument which is the function name.
// The name could empty which is allowed in languages like JS.
TreeNode* ASTBuilder::BuildFunction() {
  if (mTrace)
    std::cout << "In BuildFunction" << std::endl;

  TreeNode *node_name = NULL;

  if (mParams.size() > 0) {
    Param p_name = mParams[0];
    // In JS/TS the name could be empty.
    if (!p_name.mIsEmpty) {
      if (p_name.mIsTreeNode) {
        node_name = p_name.mData.mTreeNode;
        if (node_name->IsAttr()) {
          node_name = BuildIdentifier(node_name);
        } else if (!node_name->IsIdentifier() &&
                   !node_name->IsComputedName() &&
                   !node_name->IsLiteral())
          MERROR("The function name should be an indentifier node. Not?");
      } else {
        node_name = BuildIdentifier(p_name.mData.mToken);
      }
    }
  }

  FunctionNode *f = (FunctionNode*)gTreePool.NewTreeNode(sizeof(FunctionNode));
  new (f) FunctionNode();

  if (node_name) {
    f->SetFuncName(node_name);
    f->SetStrIdx(node_name->GetStrIdx());
  }

  mLastTreeNode = f;
  return mLastTreeNode;
}

// This takes just one argument which is the function name.
TreeNode* ASTBuilder::BuildConstructor() {
  TreeNode *t = BuildFunction();
  FunctionNode *cons = (FunctionNode*)t;
  cons->SetIsConstructor();

  mLastTreeNode = cons;
  return cons;
}

// Takes func_body as argument, to mLastTreeNode which is a function.
TreeNode* ASTBuilder::AddFunctionBody() {
  if (mTrace)
    std::cout << "In AddFunctionBody" << std::endl;

  FunctionNode *func = (FunctionNode*)mLastTreeNode;

  // It's possible that the func body is empty, such as in the
  // function header declaration. Usually it's just a token ';'.
  Param p_body = mParams[0];
  if (!p_body.mIsEmpty) {
    MASSERT(p_body.mIsTreeNode);
    TreeNode *tree_node = p_body.mData.mTreeNode;
    MASSERT(tree_node->IsBlock() && "Class body is not a BlockNode?");
    BlockNode *block = (BlockNode*)tree_node;
    func->SetBody(block);
  } else {
    // It is an 'empty' function body. Not a NULL pointer of function body.
    BlockNode *block = (BlockNode*)gTreePool.NewTreeNode(sizeof(BlockNode));
    new (block) BlockNode();
    func->SetBody(block);
  }

  mLastTreeNode = func;
  return mLastTreeNode;
}

// Takes two arguments.
// 1st: Function
// 2nd: body
TreeNode* ASTBuilder::AddFunctionBodyTo() {
  if (mTrace)
    std::cout << "In AddFunctionBodyTo" << std::endl;

  Param p_func = mParams[0];
  if (!p_func.mIsTreeNode)
    MERROR("The Function is not a tree node.");
  TreeNode *func_node = p_func.mData.mTreeNode;
  MASSERT(func_node->IsFunction() && "Function is not a FunctionNode?");
  FunctionNode *func = (FunctionNode*)func_node;

  // It's possible that the func body is empty, such as in the
  // function header declaration. Usually it's just a token ';'.
  Param p_body = mParams[1];
  if (!p_body.mIsEmpty && p_body.mIsTreeNode) {
    TreeNode *tree_node = p_body.mData.mTreeNode;
    MASSERT(tree_node->IsBlock() && "Class body is not a BlockNode?");
    BlockNode *block = (BlockNode*)tree_node;
    func->SetBody(block);
  }

  mLastTreeNode = func;
  return mLastTreeNode;
}

// It take no arugment. It uses mLastTreeNode which is
// a function node.
TreeNode* ASTBuilder::SetIsGenerator() {
  MASSERT(mLastTreeNode->IsFunction());
  FunctionNode *node = (FunctionNode*)mLastTreeNode;
  node->SetIsGenerator();
  return mLastTreeNode;
}

// It take no arugment. It uses mLastTreeNode which is
// a function node.
TreeNode* ASTBuilder::SetGetAccessor() {
  MASSERT(mLastTreeNode->IsFunction());
  FunctionNode *node = (FunctionNode*)mLastTreeNode;
  node->SetIsGetAccessor();
  return mLastTreeNode;
}

// It take no arugment. It uses mLastTreeNode which is
// a function node.
TreeNode* ASTBuilder::SetSetAccessor() {
  MASSERT(mLastTreeNode->IsFunction());
  FunctionNode *node = (FunctionNode*)mLastTreeNode;
  node->SetIsSetAccessor();
  return mLastTreeNode;
}

// It take no arugment. It uses mLastTreeNode which is
// a function node.
TreeNode* ASTBuilder::SetCallSignature() {
  MASSERT(mLastTreeNode->IsFunction());
  FunctionNode *node = (FunctionNode*)mLastTreeNode;
  node->SetIsCallSignature();
  return mLastTreeNode;
}

// It take no arugment. It uses mLastTreeNode which is
// a function node.
TreeNode* ASTBuilder::SetConstructSignature() {
  MASSERT(mLastTreeNode->IsFunction());
  FunctionNode *node = (FunctionNode*)mLastTreeNode;
  node->SetIsConstructSignature();
  return mLastTreeNode;
}

////////////////////////////////////////////////////////////////////////////////
//                   Try, Catch, Throw
////////////////////////////////////////////////////////////////////////////////

// Takes one argument which is the block.
TreeNode* ASTBuilder::BuildTry() {
  if (mTrace)
    std::cout << "In BuildTry" << std::endl;

  Param p_block = mParams[0];

  MASSERT(p_block.mIsTreeNode);
  TreeNode *block = p_block.mData.mTreeNode;

  TryNode *try_node = (TryNode*)gTreePool.NewTreeNode(sizeof(TryNode));
  new (try_node) TryNode();
  try_node->SetBlock((BlockNode*)block);

  mLastTreeNode = try_node;
  return mLastTreeNode;
}

// Takes one arguments, the catch clause
// Add to mLastTreeNode which is a TryNode.
TreeNode* ASTBuilder::AddCatch() {
  if (mTrace)
    std::cout << "In AddCatch " << std::endl;

  Param p_catch = mParams[0];
  MASSERT(p_catch.mIsTreeNode);
  TreeNode *catch_node = p_catch.mData.mTreeNode;

  TryNode *try_node = (TryNode*)mLastTreeNode;
  try_node->AddCatch(catch_node);

  return mLastTreeNode;
}

// Takes one arguments, the finally clause
// Add to mLastTreeNode which is a TryNode.
TreeNode* ASTBuilder::AddFinally() {
  if (mTrace)
    std::cout << "In AddFinally " << std::endl;

  Param p_finally = mParams[0];
  MASSERT(p_finally.mIsTreeNode);
  TreeNode *finally_node = p_finally.mData.mTreeNode;
  MASSERT(finally_node->IsFinally());

  TryNode *try_node = (TryNode*)mLastTreeNode;
  try_node->SetFinally((FinallyNode*)finally_node);

  return mLastTreeNode;
}

// Takes one argument which is the block.
TreeNode* ASTBuilder::BuildFinally() {
  if (mTrace)
    std::cout << "In BuildFinally" << std::endl;

  Param p_block = mParams[0];

  MASSERT(p_block.mIsTreeNode);
  TreeNode *block = p_block.mData.mTreeNode;

  FinallyNode *finally_node = (FinallyNode*)gTreePool.NewTreeNode(sizeof(FinallyNode));
  new (finally_node) FinallyNode();
  finally_node->SetBlock((BlockNode*)block);

  mLastTreeNode = finally_node;
  return mLastTreeNode;
}

// Takes two arguments, the parameters and the block
TreeNode* ASTBuilder::BuildCatch() {
  if (mTrace)
    std::cout << "In BuildCatch" << std::endl;

  Param p_params = mParams[0];
  Param p_block = mParams[1];

  MASSERT(p_params.mIsTreeNode);
  TreeNode *params = p_params.mData.mTreeNode;

  MASSERT(p_block.mIsTreeNode);
  TreeNode *block = p_block.mData.mTreeNode;

  CatchNode *catch_node = (CatchNode*)gTreePool.NewTreeNode(sizeof(CatchNode));
  new (catch_node) CatchNode();

  catch_node->AddParam(params);
  catch_node->SetBlock((BlockNode*)block);

  mLastTreeNode = catch_node;
  return mLastTreeNode;
}


////////////////////////////////////////////////////////////////////////////////
//                   Throw Functions
////////////////////////////////////////////////////////////////////////////////

// This takes just one argument which is the exception(s) thrown.
TreeNode* ASTBuilder::BuildThrows() {
  if (mTrace)
    std::cout << "In BuildThrows" << std::endl;

  Param p_throws = mParams[0];

  if (!p_throws.mIsTreeNode)
    MERROR("The exceptions is not a treenode in BuildThrows()");
  TreeNode *exceptions = p_throws.mData.mTreeNode;

  ThrowNode *throw_node = (ThrowNode*)gTreePool.NewTreeNode(sizeof(ThrowNode));
  new (throw_node) ThrowNode();

  throw_node->AddException(exceptions);

  mLastTreeNode = throw_node;
  return mLastTreeNode;
}

// Takes two arguments.
// 1st: Function
// 2nd: throws
TreeNode* ASTBuilder::AddThrowsTo() {
  if (mTrace)
    std::cout << "In AddThrowsTo" << std::endl;

  Param p_func = mParams[0];
  if (!p_func.mIsTreeNode)
    MERROR("The Function is not a tree node.");
  TreeNode *func_node = p_func.mData.mTreeNode;
  MASSERT(func_node->IsFunction() && "Function is not a FunctionNode?");
  FunctionNode *func = (FunctionNode*)func_node;

  // It's possible that the throws is a single identifier node,
  // or a pass node.
  Param p_body = mParams[1];
  if (p_body.mIsTreeNode) {
    TreeNode *tree_node = p_body.mData.mTreeNode;
    if (tree_node->IsIdentifier()) {
      IdentifierNode *id = (IdentifierNode*)tree_node;
      ExceptionNode *exception = (ExceptionNode*)gTreePool.NewTreeNode(sizeof(ExceptionNode));
      new (exception) ExceptionNode(id);
      func->AddThrow(exception);
    } else if (tree_node->IsPass()) {
      PassNode *pass = (PassNode*)tree_node;
      for (unsigned i = 0; i < pass->GetChildrenNum(); i++) {
        TreeNode *child = pass->GetChild(i);
        if (child->IsIdentifier()) {
          IdentifierNode *id = (IdentifierNode*)child;
          ExceptionNode *exception = (ExceptionNode*)gTreePool.NewTreeNode(sizeof(ExceptionNode));
          new (exception) ExceptionNode(id);
          func->AddThrow(exception);
        } else {
          MERROR("The to-be-added exception is not an identifier?");
        }
      }
    }
  }

  mLastTreeNode = func;
  return mLastTreeNode;
}

////////////////////////////////////////////////////////////////////////////////
//                   Pass a Child
// We only pass tree node. It should not be a token.
////////////////////////////////////////////////////////////////////////////////

TreeNode* ASTBuilder::PassChild() {
  if (mTrace)
    std::cout << "In PassChild" << std::endl;

  TreeNode *node = NULL;
  Param p = mParams[0];
  if (!p.mIsEmpty) {
    if (!p.mIsTreeNode)
      MERROR("The child is not a treenode.");
    node = p.mData.mTreeNode;
  }

  mLastTreeNode = node;
  return mLastTreeNode;
}

////////////////////////////////////////////////////////////////////////////////
//                   User Type Functions
////////////////////////////////////////////////////////////////////////////////

TreeNode* ASTBuilder::BuildUserType() {
  if (mTrace)
    std::cout << "In BuildUserType" << std::endl;

  Param p_id = mParams[0];
  if (!p_id.mIsTreeNode)
    MERROR("The Identifier of user type is not a treenode.");
  TreeNode *id = p_id.mData.mTreeNode;

  UserTypeNode *user_type = (UserTypeNode*)gTreePool.NewTreeNode(sizeof(UserTypeNode));
  new (user_type) UserTypeNode(id);
  mLastTreeNode = user_type;
  return mLastTreeNode;
}

TreeNode* ASTBuilder::BuildTypeParameter() {
  if (mTrace)
    std::cout << "In BuildTypeParameter" << std::endl;

  Param p_id = mParams[0];
  if (!p_id.mIsTreeNode)
    MERROR("The Identifier of type parameter is not a treenode.");
  TreeNode *id = p_id.mData.mTreeNode;

  TypeParameterNode *tp = (TypeParameterNode*)gTreePool.NewTreeNode(sizeof(TypeParameterNode));
  new (tp) TypeParameterNode();
  tp->SetId(id);

  mLastTreeNode = tp;
  return mLastTreeNode;
}

// It takes one argument, the constraint which could be empty.
TreeNode* ASTBuilder::AddTypeParameterExtends() {
  if (mTrace)
    std::cout << "In AddTypeParameterExtends" << std::endl;

  Param p_id = mParams[0];
  if (p_id.mIsEmpty)
    return mLastTreeNode;

  MASSERT(p_id.mIsTreeNode);
  TreeNode *id = p_id.mData.mTreeNode;

  MASSERT(mLastTreeNode->IsTypeParameter());
  TypeParameterNode *tp = (TypeParameterNode*)mLastTreeNode;
  tp->SetExtends(id);

  mLastTreeNode = tp;
  return mLastTreeNode;
}


// Takes one argument, the as 'type'.
TreeNode* ASTBuilder::BuildAsType() {
  if (mTrace)
    std::cout << "In BuildAsType" << std::endl;

  Param p_id = mParams[0];
  if (!p_id.mIsTreeNode)
    MERROR("The Identifier of type parameter is not a treenode.");
  TreeNode *id = p_id.mData.mTreeNode;

  AsTypeNode *tp = (AsTypeNode*)gTreePool.NewTreeNode(sizeof(AsTypeNode));
  new (tp) AsTypeNode();
  tp->SetType(id);

  mLastTreeNode = tp;
  return mLastTreeNode;
}

// Takes four argument, type a, type b, type c , type d.
TreeNode* ASTBuilder::BuildConditionalType() {
  if (mTrace)
    std::cout << "In BuildConditionalType" << std::endl;

  Param p_a = mParams[0];
  TreeNode *type_a = p_a.mData.mTreeNode;
  Param p_b = mParams[1];
  TreeNode *type_b = p_b.mData.mTreeNode;
  Param p_c = mParams[2];
  TreeNode *type_c = p_c.mData.mTreeNode;
  Param p_d = mParams[3];
  TreeNode *type_d = p_d.mData.mTreeNode;

  ConditionalTypeNode *tp = (ConditionalTypeNode*)gTreePool.NewTreeNode(sizeof(ConditionalTypeNode));
  new (tp) ConditionalTypeNode();
  tp->SetTypeA(type_a);
  tp->SetTypeB(type_b);
  tp->SetTypeC(type_c);
  tp->SetTypeD(type_d);

  mLastTreeNode = tp;
  return mLastTreeNode;
}


// It takes one argument, the type param or type arg
TreeNode* ASTBuilder::AddTypeGenerics() {
  if (mTrace)
    std::cout << "In AddTypeGenerics" << std::endl;

  if (mParams.size() == 0)
    return mLastTreeNode;

  Param p_args = mParams[0];
  if (p_args.mIsEmpty)
    return mLastTreeNode;

  // Some language allows special syntax as type arguments, like <> in Java.
  // It's just a token.
  if (!p_args.mIsTreeNode)
    return mLastTreeNode;

  TreeNode *args = p_args.mData.mTreeNode;
  MASSERT(args);

  if (mLastTreeNode->IsTypeAlias()) {
    TypeAliasNode *type_alias = (TypeAliasNode*)mLastTreeNode;
    UserTypeNode *n = type_alias->GetId();
    n->AddTypeGeneric(args);
  } else if (mLastTreeNode->IsUserType()) {
    UserTypeNode *type_node = (UserTypeNode*)mLastTreeNode;
    type_node->AddTypeGeneric(args);
  } else if (mLastTreeNode->IsCall()) {
    CallNode *call = (CallNode*)mLastTreeNode;
    call->AddTypeArgument(args);
  } else if (mLastTreeNode->IsFunction()) {
    FunctionNode *func = (FunctionNode*)mLastTreeNode;
    func->AddTypeParam(args);
  } else if (mLastTreeNode->IsClass()) {
    ClassNode *c = (ClassNode*)mLastTreeNode;
    c->AddTypeParam(args);
  } else if (mLastTreeNode->IsStruct()) {
    StructNode *c = (StructNode*)mLastTreeNode;
    c->AddTypeParam(args);
  } else if (mLastTreeNode->IsLambda()) {
    LambdaNode *c = (LambdaNode*)mLastTreeNode;
    c->AddTypeParam(args);
  } else {
    MERROR("Unsupported node in AddTypeGenerics()");
  }

  return mLastTreeNode;
}

// It takes two arguments to build a union type, child-a and child-b
// A child could be a prim type or user type, or even a union user type.
TreeNode* ASTBuilder::BuildUnionUserType() {
  if (mTrace)
    std::cout << "In BuildUnionUserType" << std::endl;

  UserTypeNode *user_type = NULL;

  Param p_a = mParams[0];
  MASSERT (p_a.mIsTreeNode);
  TreeNode *child_a = p_a.mData.mTreeNode;

  Param p_b = mParams[1];
  MASSERT (p_b.mIsTreeNode);
  TreeNode *child_b = p_b.mData.mTreeNode;

  if (child_a->IsUserType()) {
    UserTypeNode *ut = (UserTypeNode*)child_a;
    // for case like : (a | b)[] | c
    // We won't merge c into the array type.
    if (ut->GetType() == UT_Union && !ut->GetDims()) {
      user_type = ut;
      user_type->AddUnionInterType(child_b);
    }
  }

  if (child_b->IsUserType()) {
    UserTypeNode *ut = (UserTypeNode*)child_b;
    if (ut->GetType() == UT_Union && !ut->GetDims()) {
      // assert, both children cannot be UnionUserType at the same time.
      MASSERT(!user_type);
      user_type = ut;
      user_type->AddUnionInterType(child_a);
    }
  }

  if (!user_type) {
    user_type = (UserTypeNode*)gTreePool.NewTreeNode(sizeof(UserTypeNode));
    new (user_type) UserTypeNode();
    user_type->SetType(UT_Union);
    user_type->AddUnionInterType(child_a);
    user_type->AddUnionInterType(child_b);
  }

  mLastTreeNode = user_type;
  return mLastTreeNode;
}

// It takes two arguments to build a intersection type, child-a and child-b
// A child could be a prim type or user type.
TreeNode* ASTBuilder::BuildInterUserType() {
  if (mTrace)
    std::cout << "In BuildInterUserType" << std::endl;

  UserTypeNode *user_type = NULL;

  Param p_a = mParams[0];
  MASSERT (p_a.mIsTreeNode);
  TreeNode *child_a = p_a.mData.mTreeNode;

  Param p_b = mParams[1];
  MASSERT (p_b.mIsTreeNode);
  TreeNode *child_b = p_b.mData.mTreeNode;

  if (child_a->IsUserType()) {
    UserTypeNode *ut = (UserTypeNode*)child_a;
    if (ut->GetType() == UT_Inter) {
      user_type = ut;
      user_type->AddUnionInterType(child_b);
    }
  }

  if (child_b->IsUserType()) {
    UserTypeNode *ut = (UserTypeNode*)child_b;
    if (ut->GetType() == UT_Inter) {
      // assert, both children cannot be UnionUserType at the same time.
      MASSERT(!user_type);
      user_type = ut;
      user_type->AddUnionInterType(child_a);
    }
  }

  if (!user_type) {
    user_type = (UserTypeNode*)gTreePool.NewTreeNode(sizeof(UserTypeNode));
    new (user_type) UserTypeNode();
    user_type->SetType(UT_Inter);
    user_type->AddUnionInterType(child_a);
    user_type->AddUnionInterType(child_b);
  }

  mLastTreeNode = user_type;
  return mLastTreeNode;
}

// It takes two arguments. The alias name, and they orig type.
TreeNode* ASTBuilder::BuildTypeAlias() {
  if (mTrace)
    std::cout << "In BuildTypeAlias" << std::endl;

  Param p_name = mParams[0];
  MASSERT (p_name.mIsTreeNode);
  TreeNode *name = p_name.mData.mTreeNode;
  MASSERT(name->IsIdentifier());
  IdentifierNode *id = (IdentifierNode*)name;

  UserTypeNode *user_type = (UserTypeNode*)gTreePool.NewTreeNode(sizeof(UserTypeNode));
  new (user_type) UserTypeNode();
  user_type->SetId(id);

  Param p_orig = mParams[1];
  MASSERT (p_orig.mIsTreeNode);
  TreeNode *orig = p_orig.mData.mTreeNode;

  TypeAliasNode *type_alias = (TypeAliasNode*)gTreePool.NewTreeNode(sizeof(TypeAliasNode));
  new (type_alias) TypeAliasNode();
  type_alias->SetId(user_type);
  type_alias->SetAlias(orig);

  mLastTreeNode = type_alias;
  return mLastTreeNode;
}

// It takes at least one argument, the basic type.
TreeNode* ASTBuilder::BuildNeverArrayType() {
  if (mTrace)
    std::cout << "In BuildNeverArrayType" << std::endl;

  PrimTypeNode *prim_type = gPrimTypePool.FindType(TY_Never);
  PrimArrayTypeNode *prim_array_type = (PrimArrayTypeNode*)gTreePool.NewTreeNode(sizeof(PrimArrayTypeNode));
  new (prim_array_type) PrimArrayTypeNode();
  prim_array_type->SetPrim(prim_type);

  DimensionNode *dims = (DimensionNode*)gTreePool.NewTreeNode(sizeof(DimensionNode));
  new (dims) DimensionNode();
  dims->AddDimension(0);

  prim_array_type->SetDims(dims);
  mLastTreeNode = prim_array_type;

  return mLastTreeNode;
}

// It takes at least one argument, the basic type.
// The rest argument represent the dimensions.
//
// [NOTE] For each dimension, we are using a trick. If the size of a dimension is unknown,
//        we use the same tree node of 'basic type'.

TreeNode* ASTBuilder::BuildArrayType() {
  if (mTrace)
    std::cout << "In BuildArrayType" << std::endl;

  Param p_basic = mParams[0];
  MASSERT (p_basic.mIsTreeNode);
  TreeNode *basic = p_basic.mData.mTreeNode;

  UserTypeNode *user_type = NULL;       // we return either user_type
  PrimTypeNode *prim_type = NULL;       //
  PrimArrayTypeNode *prim_array_type = NULL; // or prim_array_type

  DimensionNode *dims = NULL;

  // This is a weird behavior in Typescript. A type key word can be identifier also.
  // I need check here.
  if (basic->IsIdentifier()) {
    IdentifierNode *id = (IdentifierNode*)basic;
    const char *id_name = id->GetName();
    if (id_name) {
      PrimTypeNode *pt = gPrimTypePool.FindType(id_name);
      if (pt)
        basic = pt;
    }
  }

  if (basic->IsPrimArrayType()) {
    prim_array_type = (PrimArrayTypeNode*)basic;
    dims = prim_array_type->GetDims();
  } else if (basic->IsUserType()) {
    user_type = (UserTypeNode*)basic;
    dims = user_type->GetDims();
  } else if (basic->IsPrimType()) {
    prim_type = (PrimTypeNode*)basic;
    prim_array_type = (PrimArrayTypeNode*)gTreePool.NewTreeNode(sizeof(PrimArrayTypeNode));
    new (prim_array_type) PrimArrayTypeNode();
    prim_array_type->SetPrim(prim_type);
  } else {
    user_type = (UserTypeNode*)gTreePool.NewTreeNode(sizeof(UserTypeNode));
    new (user_type) UserTypeNode();
    user_type->SetId(basic);
  }

  if (!dims) {
    dims = (DimensionNode*)gTreePool.NewTreeNode(sizeof(DimensionNode));
    new (dims) DimensionNode();
  }

  for (unsigned i = 1; i < mParams.size(); i++) {
    Param p_dim = mParams[i];
    MASSERT (p_dim.mIsTreeNode);
    TreeNode *dim = p_dim.mData.mTreeNode;
    // Right now we just add all 0 to dim.
    if (dim == basic)
      dims->AddDimension(0);
    else
      dims->AddDimension(0);
  }

  if (user_type) {
    if (!user_type->GetDims())
      user_type->SetDims(dims);
    mLastTreeNode = user_type;
  } else {
    MASSERT(prim_array_type);
    if (!prim_array_type->GetDims())
      prim_array_type->SetDims(dims);
    mLastTreeNode = prim_array_type;
  }

  return mLastTreeNode;
}

////////////////////////////////////////////////////////////////////////////////
//                       LambdaNode
// As stated in the ast.h, LambdaNode could be different syntax construct in
// different languages.
////////////////////////////////////////////////////////////////////////////////

// It could take
//   1) One parameter, which is the parameter list.
//   2) two parameters, the parameter list and the body
TreeNode* ASTBuilder::BuildLambda() {
  if (mTrace)
    std::cout << "In BuildLambda" << std::endl;

  TreeNode *params_node = NULL;
  TreeNode *body_node = NULL;

  Param p_params = mParams[0];
  if (!p_params.mIsEmpty) {
    if (!p_params.mIsTreeNode)
      MERROR("Lambda params is not a tree node.");
    else
      params_node = p_params.mData.mTreeNode;
  }

  if (mParams.size() == 2) {
    Param p_body = mParams[1];
    if (!p_body.mIsEmpty) {
      if (!p_body.mIsTreeNode)
        MERROR("Lambda Body is not a tree node.");
      else
        body_node = p_body.mData.mTreeNode;
    }
  }

  LambdaNode *lambda = (LambdaNode*)gTreePool.NewTreeNode(sizeof(LambdaNode));
  new (lambda) LambdaNode();

  if (params_node) {
    if (params_node->IsIdentifier()) {
      lambda->AddParam((IdentifierNode*)params_node);
    } else {
      AddParams(lambda, params_node);
    }
  }

  if (body_node)
    lambda->SetBody(body_node);

  mLastTreeNode = lambda;
  return mLastTreeNode;
}

// It take no arugment. It uses mLastTreeNode which is
// a lambda node.
TreeNode* ASTBuilder::SetJavaLambda() {
  MASSERT(mLastTreeNode->IsLambda());
  LambdaNode *node = (LambdaNode*)mLastTreeNode;
  node->SetProperty(LP_JavaLambda);
  return mLastTreeNode;
}

// It take no arugment. It uses mLastTreeNode which is
// a lambda node.
TreeNode* ASTBuilder::SetArrowFunction() {
  MASSERT(mLastTreeNode->IsLambda());
  LambdaNode *node = (LambdaNode*)mLastTreeNode;
  node->SetProperty(LP_JSArrowFunction);
  return mLastTreeNode;
}

////////////////////////////////////////////////////////////////////////////////
//                       InstanceOf Expression
////////////////////////////////////////////////////////////////////////////////

TreeNode* ASTBuilder::BuildInstanceOf() {
  if (mTrace)
    std::cout << "In BuildInstanceOf" << std::endl;

  Param l_param = mParams[0];
  MASSERT(!l_param.mIsEmpty);
  MASSERT(l_param.mIsTreeNode);
  TreeNode *left = l_param.mData.mTreeNode;

  Param r_param = mParams[1];
  MASSERT(!r_param.mIsEmpty);
  MASSERT(r_param.mIsTreeNode);
  TreeNode *right = r_param.mData.mTreeNode;

  InstanceOfNode *instanceof = (InstanceOfNode*)gTreePool.NewTreeNode(sizeof(InstanceOfNode));
  new (instanceof) InstanceOfNode();

  instanceof->SetLeft(left);
  instanceof->SetRight(right);

  mLastTreeNode = instanceof;
  return mLastTreeNode;
}

////////////////////////////////////////////////////////////////////////////////
//                       In Expression
////////////////////////////////////////////////////////////////////////////////

TreeNode* ASTBuilder::BuildIn() {
  if (mTrace)
    std::cout << "In BuildIn" << std::endl;

  Param l_param = mParams[0];
  MASSERT(!l_param.mIsEmpty);
  MASSERT(l_param.mIsTreeNode);
  TreeNode *left = l_param.mData.mTreeNode;

  Param r_param = mParams[1];
  MASSERT(!r_param.mIsEmpty);
  MASSERT(r_param.mIsTreeNode);
  TreeNode *right = r_param.mData.mTreeNode;

  InNode *innode = (InNode*)gTreePool.NewTreeNode(sizeof(InNode));
  new (innode) InNode();

  innode->SetLeft(left);
  innode->SetRight(right);

  mLastTreeNode = innode;
  return mLastTreeNode;
}

////////////////////////////////////////////////////////////////////////////////
//                       ComputedNameNode Expression
////////////////////////////////////////////////////////////////////////////////

TreeNode* ASTBuilder::BuildComputedName() {
  if (mTrace)
    std::cout << "In BuildComputedName" << std::endl;

  Param l_param = mParams[0];
  MASSERT(!l_param.mIsEmpty);
  MASSERT(l_param.mIsTreeNode);
  TreeNode *in = l_param.mData.mTreeNode;

  ComputedNameNode *innode = (ComputedNameNode*)gTreePool.NewTreeNode(sizeof(ComputedNameNode));
  new (innode) ComputedNameNode();
  innode->SetExpr(in);

  mLastTreeNode = innode;
  return mLastTreeNode;
}

////////////////////////////////////////////////////////////////////////////////
//                       Is Expression
////////////////////////////////////////////////////////////////////////////////

TreeNode* ASTBuilder::BuildIs() {
  if (mTrace)
    std::cout << "In BuildIs" << std::endl;

  Param l_param = mParams[0];
  MASSERT(!l_param.mIsEmpty);
  MASSERT(l_param.mIsTreeNode);
  TreeNode *left = l_param.mData.mTreeNode;

  Param r_param = mParams[1];
  MASSERT(!r_param.mIsEmpty);
  MASSERT(r_param.mIsTreeNode);
  TreeNode *right = r_param.mData.mTreeNode;

  IsNode *isnode = (IsNode*)gTreePool.NewTreeNode(sizeof(IsNode));
  new (isnode) IsNode();

  isnode->SetLeft(left);
  isnode->SetRight(right);

  mLastTreeNode = isnode;
  return mLastTreeNode;
}

////////////////////////////////////////////////////////////////////////////////
//                       TypeOf Expression
////////////////////////////////////////////////////////////////////////////////

// It takes (1) one argument
//          (2) zero argument. Use mLastTreeNode as the argument.
TreeNode* ASTBuilder::BuildTypeOf() {
  if (mTrace)
    std::cout << "In BuildTypeOf" << std::endl;

  TreeNode *expr = NULL;

  if (mParams.size() == 0) {
    expr = mLastTreeNode;
  } else {
    Param l_param = mParams[0];
    MASSERT(l_param.mIsTreeNode);
    expr = l_param.mData.mTreeNode;
  }

  TypeOfNode *typeof = (TypeOfNode*)gTreePool.NewTreeNode(sizeof(TypeOfNode));
  new (typeof) TypeOfNode();

  typeof->SetExpr(expr);

  mLastTreeNode = typeof;
  return mLastTreeNode;
}

////////////////////////////////////////////////////////////////////////////////
//                       KeyOf Expression
////////////////////////////////////////////////////////////////////////////////

TreeNode* ASTBuilder::BuildKeyOf() {
  if (mTrace)
    std::cout << "In BuildKeyOf" << std::endl;

  Param l_param = mParams[0];
  MASSERT(!l_param.mIsEmpty);
  MASSERT(l_param.mIsTreeNode);
  TreeNode *expr = l_param.mData.mTreeNode;

  KeyOfNode *keyof = (KeyOfNode*)gTreePool.NewTreeNode(sizeof(KeyOfNode));
  new (keyof) KeyOfNode();

  keyof->SetExpr(expr);

  mLastTreeNode = keyof;
  return mLastTreeNode;
}

////////////////////////////////////////////////////////////////////////////////
//                       Infer Expression
////////////////////////////////////////////////////////////////////////////////

TreeNode* ASTBuilder::BuildInfer() {
  if (mTrace)
    std::cout << "In BuildInfer" << std::endl;

  Param l_param = mParams[0];
  MASSERT(!l_param.mIsEmpty);
  MASSERT(l_param.mIsTreeNode);
  TreeNode *expr = l_param.mData.mTreeNode;

  InferNode *infer = (InferNode*)gTreePool.NewTreeNode(sizeof(InferNode));
  new (infer) InferNode();

  infer->SetExpr(expr);

  mLastTreeNode = infer;
  return mLastTreeNode;
}

////////////////////////////////////////////////////////////////////////////////
//                       Await
////////////////////////////////////////////////////////////////////////////////

// For first parameter has to be an operator.
TreeNode* ASTBuilder::BuildAwait() {
  if (mTrace)
    std::cout << "In BuildAwait" << std::endl;

  MASSERT(mParams.size() == 1);
  Param p_a = mParams[0];
  MASSERT(!p_a.mIsEmpty && p_a.mIsTreeNode);
  TreeNode *expr = p_a.mData.mTreeNode;

  AwaitNode *n = (AwaitNode*)gTreePool.NewTreeNode(sizeof(AwaitNode));
  new (n) AwaitNode();
  n->SetExpr(expr);

  mLastTreeNode = n;
  return n;
}

}
