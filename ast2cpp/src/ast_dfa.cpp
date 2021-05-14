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
#include "stringpool.h"
#include "ast_handler.h"
#include "ast_cfg.h"
#include "ast_dfa.h"

namespace maplefe {

void AST_DFA::Build() {
  CollectDefNodes();
  BuildBitVectors();
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
  std::cout << "stridx: " << std::get<1>(pos) << std::endl;
  std::cout << "   nid: " << std::get<2>(pos) << std::endl;
  std::cout << "  bbid: " << std::get<3>(pos) << std::endl;
  std::cout << std::endl;
}

void AST_DFA::DumpPosDefVec() {
  for (unsigned i = 0; i < mDefVecSize; i++) {
    PosDef pos = mDefVec.ValueAtIndex(i);
    DumpPosDef(pos);
  }
}

bool AST_DFA::IsDef(TreeNode *node) {
  unsigned n;
  return AddDef(node, n, 0xffffffff);
}

bool AST_DFA::AddDef(TreeNode *node, unsigned &bitnum, unsigned bbid) {
  const char *name = NULL;
  unsigned nid = 0;
  switch (node->GetKind()) {
    case NK_Decl: {
      DeclNode *decl = static_cast<DeclNode *>(node);
      if (decl->GetInit()) {
        name = decl->GetName();
        nid = decl->GetNodeId();
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
          name = lhs->GetName();
          nid = lhs->GetNodeId();
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
        name = lhs->GetName();
        nid = lhs->GetNodeId();
      }
      break;
    }
    default:
      break;
  }

  // update mDefVec
  if (name && bbid != 0xffffffff) {
    unsigned idx = mStringPool.GetStrIdx(name);
    PosDef pos(bitnum++, idx, nid, bbid);
    mDefVec.PushBack(pos);
  }

  return name != NULL;
}

// this calcuates mDefVec mDefVecSize mBbIdSet
void AST_DFA::CollectDefNodes() {
  if (mTrace) std::cout << "============== CollectDefNodes ==============" << std::endl;
  AST_Function *func = mHandler->GetFunction();
  MASSERT(func && "null func");
  std::deque<AST_BB *> working_list;

  working_list.push_back(func->GetEntryBB());

  unsigned bitnum = 0;

  while(working_list.size()) {
    AST_BB *bb = working_list.front();
    MASSERT(bb && "null BB");
    unsigned bbid = bb->GetId();

    // skip bb already visited
    if (mBbIdSet.find(bbid) != mBbIdSet.end()) {
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

    mBbIdSet.insert(bbid);
    working_list.pop_front();
  }

  mDefVecSize = mDefVec.GetNum();
  if (mTrace) DumpPosDefVec();

}

void AST_DFA::BuildBitVectors() {
  if (mTrace) std::cout << "============== BuildBitVectors ==============" << std::endl;
  AST_Function *func = mHandler->GetFunction();
  MASSERT(func && "null func");
  std::unordered_set<unsigned> doneBbID;
  std::deque<AST_BB *> working_list;

  // init bit vectors
  for (int i = 0; i < mDefVecSize; i++) {
    BitVector bv1;
    bv1.Alloc(mDefVecSize);
    bv1.WipeOff(0);
    mPrsvMap[i] = bv1;

    BitVector bv2;
    bv2.Alloc(mDefVecSize);
    bv2.WipeOff(0xff);      // init with bit 1
    mGenMap[i] = bv2;

    BitVector bv3;
    bv3.Alloc(mDefVecSize);
    bv3.WipeOff(0);
    mRchOutMap[i] = bv3;

    BitVector bv4;
    bv4.Alloc(mDefVecSize);
    bv4.WipeOff(0);
    mRchInMap[i] = bv4;
  }

  working_list.push_back(func->GetEntryBB());

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
      bool isdef = IsDef(node);
      if (isdef) {
      }
    }

    doneBbID.insert(bbid);
    working_list.pop_front();
  }
}
}
