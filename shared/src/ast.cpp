/*
* Copyright (C) [2020] Futurewei Technologies, Inc. All rights reverved.
*
* OpenArkFE is licensed under the Mulan PSL v1.
* You can use this software according to the terms and conditions of the Mulan PSL v1.
* You may obtain a copy of Mulan PSL v1 at:
*
*  http://license.coscl.org.cn/MulanPSL
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR
* FIT FOR A PARTICULAR PURPOSE.
* See the Mulan PSL v1 for more details.
*/

#include "ast.h"
#include "ast_builder.h"
#include "parser.h"
#include "container.h"
#include "token.h"
#include "ruletable.h"

#include "massert.h"

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
//                             ASTTree
//////////////////////////////////////////////////////////////////////////////////////

ASTTree::ASTTree() {
  mRootNode = NULL;
  mBuilder = new ASTBuilder(&mTreePool);
}

ASTTree::~ASTTree() {
  delete mBuilder;
}

void ASTTree::SetTraceBuild(bool b) {
  mBuilder->SetTrace(b);
}

// Create tree node. Its children have been created tree nodes.
// There are couple issueshere.
//
// 1. An sorted AppealNode could have NO tree node, because it may have NO RuleAction to
//    create the sub tree. This happens if the RuleTable is just a temporary intermediate
//    table created by Autogen, or its rule is just ONEOF without real syntax. Here
//    is an example.
//
//       The AST after BuildAST() for a simple statment: c=a+b;
//
//       ======= Simplify Trees Dump SortOut =======
//       [1] Table TblExpressionStatement@0: 2,3,
//       [2:1] Table TblAssignment@0: 4,5,6,
//       [3] Token
//       [4:1] Token
//       [5:2] Token
//       [6:3] Table TblArrayAccess_sub1@2: 7,8,  <-- supposed to get a binary expression
//       [7:1] Token                              <-- a
//       [8:2] Table TblUnaryExpression_sub1@3: 9,10, <-- +b
//       [9] Token
//       [10:2] Token
//
//    Node [1] won't have a tree node at all since it has no Rule Action attached.
//    Node [6] won't have a tree node either.
//
// 2. A binary operation like a+b could be parsed as (1) expression: a, and (2) a
//    unary operation: +b. This is because we parse them in favor to ArrayAccess before
//    Binary Operation. Usually to handle this issue, in some system like ANTLR,
//    they require you to list the priority, by writing rules from higher priority to
//    lower priority.
//
//    We are going to do a consolidation of the sub-trees, by converting smaller trees
//    to a more compact bigger trees. However, to do this we want to set some rules.
//    *) The parent AppealNode of these sub-trees has no tree node. So the conversion
//       helps make the tree complete.

TreeNode* ASTTree::NewTreeNode(AppealNode *appeal_node, std::map<AppealNode*, TreeNode*> &map) {
  TreeNode *sub_tree = NULL;

  if (appeal_node->IsToken()) {
    sub_tree = mBuilder->CreateTokenTreeNode(appeal_node->GetToken());
    return sub_tree;
  }

  RuleTable *rule_table = appeal_node->GetTable();

  for (unsigned i = 0; i < rule_table->mNumAction; i++) {
    Action *action = rule_table->mActions + i;
    mBuilder->mActionId = action->mId;
    mBuilder->ClearParams();

    for (unsigned j = 0; j < action->mNumElem; j++) {
      // find the appeal node child
      unsigned elem_idx = action->mElems[j];
      AppealNode *child = appeal_node->GetSortedChildByIndex(elem_idx);

      Param p;
      if (child) {
        p.mIsEmpty = false;
        std::map<AppealNode*, TreeNode*>::iterator it = map.find(child);
        if (it == map.end()) {
          // only token children could have NO tree node.
          MASSERT(child->IsToken() && "A RuleTable node has no tree node?");
          p.mIsTreeNode = false;
          p.mData.mToken = child->GetToken();
        } else {
          p.mIsTreeNode = true;
          p.mData.mTreeNode = it->second;
        }
      } else {
        p.mIsEmpty = true;
      }
      mBuilder->AddParam(p);
    }

    // For multiple actions of a rule, there should be only action which create tree.
    // The others are just for adding attribute or else, and return the same tree
    // with additional attributes.
    sub_tree = mBuilder->Build();
  }

  if (sub_tree)
    return sub_tree;

  // It's possible that the Rule has no action, meaning it cannot create tree node.
  // In this case, we will take the child's tree node if it has one and only one
  // sub tree among all children.
  std::vector<TreeNode*> child_trees;
  std::vector<AppealNode*>::iterator cit = appeal_node->mSortedChildren.begin();
  for (; cit != appeal_node->mSortedChildren.end(); cit++) {
    std::map<AppealNode*, TreeNode*>::iterator child_tree_it = map.find(*cit);
    if (child_tree_it != map.end()) {
      child_trees.push_back(child_tree_it->second);
    }
  }

  if (child_trees.size() == 1) {
    std::cout << "Attached a tree node from Child" << std::endl;
    map.insert(std::pair<AppealNode*, TreeNode*>(appeal_node, child_trees[0]));
    sub_tree = child_trees[0];
    if (sub_tree)
      return sub_tree;
    else
      MERROR("We got a broken AST tree, not connected sub tree.");
  }

  // There are cases like a+b could be parsed as "a" and "+b", a symbol and a
  // unary operation. However, we do prefer binary operation than unary. So a
  // combination is needed here, especially when the parent node is NULL.
  if (child_trees.size() == 2) {
    TreeNode *child_a = child_trees[0];
    TreeNode *child_b = child_trees[1];
    if (child_b->IsUnaOperator()) {
      UnaryOperatorNode *unary = (UnaryOperatorNode*)child_b;
      unsigned property = GetOperatorProperty(unary->mOprId);
      if ((property & Binary) && (property & Unary)) {
        std::cout << "Convert unary --> binary" << std::endl;
        TreeNode *unary_sub = unary->mOpnd;
        TreeNode *binary = BuildBinaryOperation(child_a, unary_sub, unary->mOprId);
        map.insert(std::pair<AppealNode*, TreeNode*>(appeal_node, binary));
        return binary;
      }
    }
  }

  // In the end, we will put subtrees into a PassNode to pass to parent.
  if (child_trees.size() > 0) {
    PassNode *pass = BuildPassNode();
    std::vector<TreeNode*>::iterator child_it = child_trees.begin();
    for (; child_it != child_trees.end(); child_it++)
      pass->AddChild(*child_it);
    map.insert(std::pair<AppealNode*, TreeNode*>(appeal_node, pass));
    return pass;
  }

  MERROR("We got a broken AST tree, not connected sub tree.");
}

void ASTTree::Dump(unsigned indent) {
  DUMP0("== Sub Tree ==");
  mRootNode->Dump(indent);
  std::cout << std::endl;
}

TreeNode* ASTTree::BuildBinaryOperation(TreeNode *childA, TreeNode *childB, OprId id) {
  BinaryOperatorNode *n = (BinaryOperatorNode*)mTreePool.NewTreeNode(sizeof(BinaryOperatorNode));
  new (n) BinaryOperatorNode(id);
  n->mOpndA = childA;
  n->mOpndB = childB;
  childA->SetParent(n);
  childB->SetParent(n);
  return n;
}

TreeNode* ASTTree::BuildPassNode() {
  PassNode *n = (PassNode*)mTreePool.NewTreeNode(sizeof(PassNode));
  new (n) PassNode();
  return n;
}

//////////////////////////////////////////////////////////////////////////////////////
//                               TreeNode
//////////////////////////////////////////////////////////////////////////////////////

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
//                          BinaryOperatorNode
//////////////////////////////////////////////////////////////////////////////////////

void BinaryOperatorNode::Dump(unsigned indent) {
  const char *name = GetOperatorName(mOprId);
  DumpIndentation(indent);
  DUMP0(name);
  mOpndA->Dump(indent + 2);
  DUMP_RETURN();
  mOpndB->Dump(indent + 2);
  DUMP_RETURN();
}

//////////////////////////////////////////////////////////////////////////////////////
//                           UnaryOperatorNode
//////////////////////////////////////////////////////////////////////////////////////

void UnaryOperatorNode::Dump(unsigned indent) {
  const char *name = GetOperatorName(mOprId);
  DumpIndentation(indent);
  DUMP0(name);
  mOpnd->Dump(indent + 2);
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
    for (unsigned i = 0; i < n->GetDimsNum(); i++)
      AddDim(n->GetNthDim(i));
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

void IdentifierNode::Dump(unsigned indent) {
  DumpIndentation(indent);
  DUMP0_NORETURN(mName);
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
//                          LiteralNode
//////////////////////////////////////////////////////////////////////////////////////

void LiteralNode::Dump(unsigned indent) {
  DumpIndentation(indent);
  switch (mData.mType) {
  case LT_IntegerLiteral:
    DUMP0(mData.mData.mInt);
    break;
  case LT_FPLiteral:
    DUMP0(mData.mData.mFloat);
    break;
  case LT_BooleanLiteral:
    DUMP0(mData.mData.mBool);
    break;
  case LT_CharacterLiteral:
    DUMP0(mData.mData.mChar);
    break;
  case LT_NullLiteral:
    DUMP0("null");
    break;
  case LT_NA:
  default:
    DUMP0("NA Token:");
    break;
  }
}

//////////////////////////////////////////////////////////////////////////
//          Statement Node, Control Flow related nodes
//////////////////////////////////////////////////////////////////////////

void ReturnNode::Dump(unsigned ind) {
  DumpLabel(ind);
  DumpIndentation(ind);
  DUMP0("return:");
  GetResult()->Dump(ind + 2);
}

CondBranchNode::CondBranchNode() {
  mCond = NULL;
  mTrueBranch = NULL;
  mFalseBranch = NULL;
}

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

//////////////////////////////////////////////////////////////////////////////////////
//                          BlockNode
//////////////////////////////////////////////////////////////////////////////////////

void BlockNode::Dump(unsigned ind) {
  DumpLabel(ind);
  for (unsigned i = 0; i < GetChildrenNum(); i++) {
    TreeNode *child = GetChildAtIndex(i);
    child->Dump(ind);
  }
}

//////////////////////////////////////////////////////////////////////////////////////
//                          ClassNode
//////////////////////////////////////////////////////////////////////////////////////

// When the class body, a BlockNode, is added to the ClassNode, we need further
// categorize the subtrees into members, methods, local classes, interfaces, etc.
void ClassNode::Construct() {
  for (unsigned i = 0; i < mBody->GetChildrenNum(); i++) {
    TreeNode *tree_node = mBody->GetChildAtIndex(i);
    if (tree_node->IsVarList()) {
      VarListNode *vlnode = (VarListNode*)tree_node;
      for (unsigned i = 0; i < vlnode->GetNum(); i++) {
        IdentifierNode *inode = vlnode->VarAtIndex(i);
        mMembers.PushBack(inode);
      }
    } else if (tree_node->IsIdentifier())
      mMembers.PushBack(tree_node);
    else if (tree_node->IsFunction()) {
      FunctionNode *f = (FunctionNode*)tree_node;
      if (f->IsConstructor())
        mConstructors.PushBack(tree_node);
      else
        mMethods.PushBack(tree_node);
    } else if (tree_node->IsClass())
      mLocalClasses.PushBack(tree_node);
    else if (tree_node->IsInterface())
      mLocalInterfaces.PushBack(tree_node);
    else if (tree_node->IsBlock()) {
      BlockNode *block = (BlockNode*)tree_node;
      MASSERT(block->IsInstInit() && "unnamed block in class is not inst init?");
      mInstInits.PushBack(tree_node);
    } else
      MERROR("Unsupported tree node in class body.");
  }
}

// Release() only takes care of those container memory. The release of all tree nodes
// is taken care by the tree node pool.
void ClassNode::Release() {
  mSuperClasses.Release();
  mSuperInterfaces.Release();
  mAttributes.Release();
  mMembers.Release();
  mMethods.Release();
  mLocalClasses.Release();
  mLocalInterfaces.Release();
}

void ClassNode::Dump(unsigned indent) {
  DumpIndentation(indent);
  DUMP1_NORETURN("class ", mName);
  DUMP_RETURN();

  DUMP0("  Members: ");
  for (unsigned i = 0; i < mMembers.GetNum(); i++) {
    TreeNode *node = mMembers.ValueAtIndex(i);
    node->Dump(4);
  }
  DUMP_RETURN();

  DUMP0("  Instance Initializer: ");
  for (unsigned i = 0; i < mInstInits.GetNum(); i++) {
    TreeNode *node = mInstInits.ValueAtIndex(i);
    DUMP1("    InstInit-", i);
  }
  DUMP_RETURN();

  DUMP0("  Constructors: ");
  for (unsigned i = 0; i < mConstructors.GetNum(); i++) {
    TreeNode *node = mConstructors.ValueAtIndex(i);
    node->Dump(4);
  }
  DUMP_RETURN();

  DUMP0("  Methods: ");
  for (unsigned i = 0; i < mMethods.GetNum(); i++) {
    TreeNode *node = mMethods.ValueAtIndex(i);
    node->Dump(4);
  }

  DUMP0("  LocalClasses: ");
  for (unsigned i = 0; i < mLocalClasses.GetNum(); i++) {
    TreeNode *node = mLocalClasses.ValueAtIndex(i);
    node->Dump(4);
  }

  DUMP0("  LocalInterfaces: ");
  for (unsigned i = 0; i < mLocalInterfaces.GetNum(); i++) {
    TreeNode *node = mLocalInterfaces.ValueAtIndex(i);
    node->Dump(4);
  }
}

//////////////////////////////////////////////////////////////////////////////////////
//                          FunctionNode
//////////////////////////////////////////////////////////////////////////////////////

FunctionNode::FunctionNode() {
  mKind = NK_Function;
  mName = NULL;
  mType = NULL;
  mParams = NULL;
  mScope = NULL;
  mBody = NULL;
  mDims = NULL;
  mIsConstructor = false;
}

// When BlockNode is added to the ClassNode, we need further
// categorize the subtrees into members, methods, local classes, interfaces, etc.
void FunctionNode::Construct() {
  for (unsigned i = 0; i < mBody->GetChildrenNum(); i++) {
    TreeNode *tree_node = mBody->GetChildAtIndex(i);
  }
}

void FunctionNode::Dump(unsigned indent) {
  DumpIndentation(indent);
  if (mIsConstructor)
    DUMP1_NORETURN("constructor ", mName);
  else 
    DUMP1_NORETURN("func ", mName);
  DUMP_RETURN();

  // dump parameters

  // dump function body
  if (GetBody())
    GetBody()->Dump(indent);
}
