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

void ASTTree::Dump() {
  DUMP0("== Sub Tree ==");
  mRootNode->Dump();
}

//////////////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////////////

#undef  OPERATOR
#define OPERATOR(T, D)  {OPR_##T, D},
OperatorDesc gOperatorDesc[OPR_NA] = {
#include "supported_operators.def"
};

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
