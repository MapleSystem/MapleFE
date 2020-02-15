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

TreeNode* ASTTree::NewNode(const AppealNode *anode) {
  unsigned size = 0;
  if (anode->IsToken()) {
    Token *token = anode->GetToken();
    if (token->IsIdentifier()) {
      IdentifierToken *t = (IdentifierToken*)token;
      IdentifierNode *n = (IdentifierNode*)mMemPool.NewTreeNode(sizeof(IdentifierNode));
      new (n) IdentifierNode(t->mName);
      return n;
    } else if (token->IsLiteral()) {
      LiteralToken *lt = (LiteralToken*)token;
      LitData data = lt->GetLitData();
      LiteralNode *n = (LiteralNode*)mMemPool.NewTreeNode(sizeof(LiteralNode));
      new (n) LiteralNode(data);
      return n;
    } else if (token->IsOperator()) {
      OperatorToken *t = (OperatorToken*)token;
      OperatorNode *n = (OperatorNode*)mMemPool.NewTreeNode(sizeof(OperatorNode));
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
