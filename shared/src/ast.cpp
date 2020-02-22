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
#include "token.h"
#include "ruletable.h"

#include "massert.h"

ASTTree::ASTTree() {
  mRootNode = NULL;
  mBuilder = new ASTBuilder(&mTreePool);
}

ASTTree::~ASTTree() {
  delete mBuilder;
}

// Create tree node. Its children have been created tree nodes.
// The appeal_node should be a rule table node.
//
// It's caller's job to make sure appeal_node is for RuleTalbe.
TreeNode* ASTTree::NewTreeNode(const AppealNode *appeal_node, std::map<AppealNode*, TreeNode*> &map) {
  RuleTable *rule_table = appeal_node->GetTable();

  if (rule_table->mNumAction > 1) {
    std::cout << "Need further implementation!!!" << std::endl;
    exit (1);
  }

  for (unsigned i = 0; i < rule_table->mNumAction; i++) {
    Action *action = rule_table->mActions + i;
    mBuilder->mActionId = action->mId;
    mBuilder->ClearParams();

    for (unsigned j = 0; j < action->mNumElem; j++) {
      // find the appeal node child
      unsigned elem_idx = action->mElems[j];
      AppealNode *child = appeal_node->GetSortedChildByIndex(elem_idx);
      MASSERT(child && "Couldnot find sorted child of an index");

      Param p;
      if (child->IsToken()) {
        p.mIsTreeNode = false;
        p.mData.mToken = child->GetToken();
      } else {
        // find the tree node
        std::map<AppealNode*, TreeNode*>::iterator it = map.find(child);
        MASSERT(it != map.end() && "Could find the child-s tree node in map?");
        p.mIsTreeNode = true;
        p.mData.mTreeNode = it->second;
      }
      mBuilder->AddParam(p);
    }

    TreeNode *sub_tree = mBuilder->Build();
    return sub_tree;
  }
}

// SimplifySubTree only works if there is no tree nodes for 'parent'.
//
// It does two jobs.
// 1) Move sub-trees upwards to parent if parent has no tree node, under a few conditions.
// 2) Convert couple of smaller sub-trees to a bigger sub-tree

TreeNode* ASTTree::SimplifySubTree(AppealNode *parent, std::map<AppealNode*, TreeNode*> &map) {
  std::vector<TreeNode*> child_trees;
  std::map<AppealNode*, TreeNode*>::iterator it = map.find(parent);
  if (it != map.end()) {
    return it->second;
  } else {
    // Simplification 1. If there is only child having tree node, attach it to parent.
    std::vector<AppealNode*>::iterator cit = parent->mSortedChildren.begin();
    for (; cit != parent->mSortedChildren.end(); cit++) {
      std::map<AppealNode*, TreeNode*>::iterator child_tree_it = map.find(parent);
      if (child_tree_it != map.end()) {
        child_trees.push_back(child_tree_it->second);
      }
    }

    if (child_trees.size() == 1) {
      std::cout << "Attached a tree node" << std::endl;
      map.insert(std::pair<AppealNode*, TreeNode*>(parent, child_trees[0]));
      return child_trees[0];
    } else {
      MERROR("We got a broken AST tree, not connected sub tree.");
    }

    // Simplification 2. If the two children can be converted to one through operation conversion.
    //          [NOTE]   Right now we are just starting from the simplest case, since I have no idea
    //                   what else situation I'll hit. Just do it step by step. Later maybe will
    //                   come up a nice algorithm.
    if (child_trees.size() == 2) {
      TreeNode *child_a = child_trees[0];
      TreeNode *child_b = child_trees[1];
      if (child_b->IsUnaOperator()) {
        UnaryOperatorNode *unary = (UnaryOperatorNode*)child_b;
        unsigned property = GetOperatorProperty(unary->mOprId);
        if ((property & Binary) && (property & Pre)) {
          std::cout << "Convert unary --> binary" << std::endl;
          TreeNode *new_tree = BuildBinaryOperation(child_a, unary, unary->mOprId);
          map.insert(std::pair<AppealNode*, TreeNode*>(parent, new_tree));
          return new_tree;
        }
      }
    }
  }

  return NULL;
}

void ASTTree::Dump() {
  DUMP0("== Sub Tree ==");
  mRootNode->Dump();
}

//////////////////////////////////////////////////////////////////////////////////////
//                          Tree building functions
//////////////////////////////////////////////////////////////////////////////////////

TreeNode* ASTTree::BuildBinaryOperation(TreeNode *childA, TreeNode *childB, OprId id) {
  BinaryOperatorNode *n = (BinaryOperatorNode*)mTreePool.NewTreeNode(sizeof(BinaryOperatorNode));
  new (n) BinaryOperatorNode(id);
  n->mOpndA = childA;
  n->mOpndB = childB;
  childA->SetParent(n);
  childB->SetParent(n);
  return n;
}

//////////////////////////////////////////////////////////////////////////////////////
//                          Misc    Functions
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

void BinaryOperatorNode::Dump() {
  const char *name = GetOperatorName(mOprId);
  mOpndA->Dump();
  DUMP0(name);
  mOpndB->Dump();
}

void UnaryOperatorNode::Dump() {
  const char *name = GetOperatorName(mOprId);
  DUMP0(name);
  mOpnd->Dump();
}

void IdentifierNode::Dump() {
  DUMP0(mName);
}

void LiteralNode::Dump() {
}
