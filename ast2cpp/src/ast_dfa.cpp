/*
* Copyright (C) [2021] Futurewei Technologies, Inc. All rights reverved.
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

#include <stack>
#include <set>
#include <tuple>
#include "ast_handler.h"
#include "ast_cfg.h"
#include "ast_dfa.h"

namespace maplefe {

void AST_DFA::Build() {
  CollectDefNodes();
  BuildReachDefIn();
}

void AST_DFA::BuildReachDefIn() {
  if (mTrace) std::cout << "============== BuildReachDefIn ==============" << std::endl;
  ReachDefInVisitor visitor(mHandler, mTrace, true);
  AST_Function *func = mHandler->GetFunction();
  visitor.SetCurrentFunction(mHandler->GetFunction());
  visitor.SetCurrentBB(func->GetEntryBB());
  for(auto it: mHandler->GetASTModule()->mTrees) {
    visitor.Visit(it->mRootNode);
  }
}

DeclNode *ReachDefInVisitor::VisitDeclNode(DeclNode *node) {
  // AstVisitor::VisitDeclNode(node);
  // node->Dump(0);
  return node;
}

void AST_DFA::DumpPosDef(PosDef pos) {
  std::cout << "BitPos: " << std::get<0>(pos) << std::endl;
  std::cout << "  name: " << std::get<1>(pos) << std::endl;
  std::cout << "   nid: " << std::get<2>(pos) << std::endl;
  std::cout << "  bbid: " << std::get<3>(pos) << std::endl;
  std::cout << std::endl;
}

void AST_DFA::DumpPosDefVec() {
  for (unsigned i = 0; i < mDefVec.GetNum(); i++) {
    PosDef pos = mDefVec.ValueAtIndex(i);
    DumpPosDef(pos);
  }
}

void AST_DFA::AddDef(TreeNode *node, unsigned &bitnum, unsigned bbid) {
  switch (node->GetKind()) {
    case NK_Decl: {
      DeclNode *decl = static_cast<DeclNode *>(node);
      if (decl->GetInit()) {
        PosDef pos(bitnum++, decl->GetName(), decl->GetNodeId(), bbid);
        mDefVec.PushBack(pos);
      }
      break;
    }
    case NK_BinOperator: {
      BinOperatorNode *bon = static_cast<BinOperatorNode *>(node);
      OprId op = bon->GetOprId();
      switch (op) {
        case OPR_Assign:
        case OPR_AddAssign:
        case OPR_SubAssign:
        case OPR_MulAssign:
        case OPR_DivAssign:
        case OPR_ModAssign:
        case OPR_ShlAssign:
        case OPR_ShrAssign:
        case OPR_BandAssign:
        case OPR_BorAssign:
        case OPR_BxorAssign:
        case OPR_ZextAssign: {
          TreeNode *lhs = bon->GetOpndA();
          PosDef pos(bitnum++, lhs->GetName(), lhs->GetNodeId(), bbid);
          mDefVec.PushBack(pos);
          break;
        }
        default:
          break;
      }
      break;
    }
    case NK_UnaOperator: {
      UnaOperatorNode *uon = static_cast<UnaOperatorNode *>(node);
      OprId op = uon->GetOprId();
      if (op == OPR_Inc || op == OPR_Dec) {
        TreeNode *lhs = uon->GetOpnd();
        PosDef pos(bitnum++, lhs->GetName(), lhs->GetNodeId(), bbid);
        mDefVec.PushBack(pos);
      }
      break;
    }
    default:
      break;
  }
}

void AST_DFA::CollectDefNodes() {
  if (mTrace) std::cout << "============== CollectDefNodes ==============" << std::endl;
  AST_Function *func = mHandler->GetFunction();
  MASSERT(func && "null func");
  std::set<unsigned> worklist;;
  std::unordered_set<unsigned> doneBbID;
  std::deque<AST_BB *> working_list;

  working_list.push_back(func->GetEntryBB());

  unsigned bitnum = 0;

  while(working_list.size()) {
    AST_BB *bb = working_list.front();
    MASSERT(bb && "null BB");
    unsigned bbid = bb->GetId();

    // skip bb already visited
    if (doneBbID.find(bbid) != doneBbID.end()) {
      working_list.pop_front();
      continue;
    }

    for (int i = 0; i < bb->GetSuccessorsNum(); i++) {
      working_list.push_back(bb->GetSuccessorAtIndex(i));
    }

    for (int i = 0; i < bb->GetStatementsNum(); i++) {
      TreeNode *node = bb->GetStatementAtIndex(i);
      mNodeId2NodeMap[node->GetNodeId()] = node;
      AddDef(node, bitnum, bbid);
    }
    doneBbID.insert(bbid);
    working_list.pop_front();
  }

  // DumpPosDefVec();

}

}
