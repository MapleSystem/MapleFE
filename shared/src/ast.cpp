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

TreeNode* ASTTree::NewTokenTreeNode(const AppealNode *anode) {
  unsigned size = 0;
  if (anode->IsToken()) {
    Token *token = anode->GetToken();
    if (token->IsIdentifier()) {
      IdentifierToken *t = (IdentifierToken*)token;
      IdentifierNode *n = (IdentifierNode*)mTreePool.NewTreeNode(sizeof(IdentifierNode));
      new (n) IdentifierNode(t->mName);
      return n;
    } else if (token->IsLiteral()) {
      LiteralToken *lt = (LiteralToken*)token;
      LitData data = lt->GetLitData();
      LiteralNode *n = (LiteralNode*)mTreePool.NewTreeNode(sizeof(LiteralNode));
      new (n) LiteralNode(data);
      return n;
    } else if (token->IsOperator()) {
      OperatorToken *t = (OperatorToken*)token;
      OperatorNode *n = (OperatorNode*)mTreePool.NewTreeNode(sizeof(OperatorNode));
      new (n) OperatorNode(t->mOprId);
      return n;
    } else {
      // Other tokens shouldn't be involved in the tree creation. They should have
      // already been digested and used in the RuleTable actions.
      std::cout << "Unexpected token types to create AST node" << std::endl; 
      return NULL;
    } 
  } else
    std::cout << "RuleTable is not going to create tree node directly." << std::endl; 

  return NULL;
}

// Create action tree node. It could be an operator node, or a function node.
// Its children have been created tree nodes.
// The appeal_node should be a rule table node.
//
// It's caller's job to make sure appeal_node is for RuleTalbe.
TreeNode* ASTTree::NewActionTreeNode(const AppealNode *appeal_node, std::map<AppealNode*, TreeNode*> &map) {
  RuleTable *rule_table = appeal_node->GetTable();

  if (rule_table->mNumAction > 1) {
    std::cout << "Need further implementation!!!" << std::endl;
    exit (1);
  }

  for (unsigned i = 0; i < rule_table->mNumAction; i++) {
    Action *action = rule_table->mActions + i;
    mBuilder->mActionId = action->mId;

    for (unsigned j = 0; j < action->mNumElem; j++) {
      // find the appeal node child
      unsigned elem_idx = action->mElems[j];
      AppealNode *child_app_node = appeal_node->GetSortedChildByIndex(elem_idx);
      MASSERT(child_app_node && "Couldnot find sorted child of an index");
      // find the tree node
      std::map<AppealNode*, TreeNode*>::iterator it = map.find(child_app_node);
      MASSERT(it != map.end() && "Could find the tree node in map?");
      TreeNode *tree_node = it->second;
      mBuilder->AddParam(tree_node);
    }

    TreeNode *sub_tree = mBuilder->Build();
    return sub_tree;
  }
}

////////////////////////////////////////////////////////////////////////////////////////
//                       AST  Builder
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
}
