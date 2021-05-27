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
#include "ast_cfg.h"
#include "ast_dfa.h"

namespace maplefe {

AST_DFA::~AST_DFA() {
  mVar2DeclMap.clear();
  for (auto it: mStmtId2StmtMap) {
    delete it.second;
  }
  mStmtId2StmtMap.clear();
  for (auto it: mPrsvMap) {
    delete it.second;
  }
  mPrsvMap.clear();
  for (auto it: mGenMap) {
    delete it.second;
  }
  mGenMap.clear();
  for (auto it: mRchInMap) {
    delete it.second;
  }
  mRchInMap.clear();
  for (auto it: mRchOutMap) {
    delete it.second;
  }
  mBbIdVec.clear();
}

void AST_DFA::TestBV() {
  unsigned size = 300;
  BitVector *bv1 = new BitVector(size);
  bv1->WipeOff(0xab);

  BitVector *bv2 = new BitVector(size);
  bv2->WipeOff(0x12);

  DumpBV(bv1);
  DumpBV(bv2);

  bv1->Or(bv2);
  DumpBV(bv1);

  bv1->And(bv2);
  DumpBV(bv1);

  free(bv1);
  free(bv2);
}

void AST_DFA::Build() {
  // TestBV();
  CollectDefNodes();
  BuildBitVectors();
  CollectUseNodes();
  DumpUse();
}

void AST_DFA::DumpDefPosition(DefPosition pos) {
  std::cout << "stridx: " << std::get<0>(pos) << " " << gStringPool.GetStringFromStrIdx(std::get<0>(pos)) << std::endl;
  std::cout << "stmtid: " << std::get<1>(pos) << std::endl;
  std::cout << "nodeid: " << std::get<2>(pos) << std::endl;
  std::cout << "  bbid: " << std::get<3>(pos) << std::endl;
  std::cout << std::endl;
}

void AST_DFA::DumpDefPositionVec() {
  for (unsigned i = 0; i < mDefPositionVec.GetNum(); i++) {
    DefPosition pos = mDefPositionVec.ValueAtIndex(i);
    std::cout << "BitPos: " << i << std::endl;
    DumpDefPosition(pos);
  }
}

#define DUMMY_BBID 0xffffffff

unsigned AST_DFA::GetDefStrIdx(TreeNode *node) {
  unsigned stridx = 0;
  // pass DUMMY_BBID to indicate to get stridx only
  AddDef(node, stridx, DUMMY_BBID);
  return stridx;
}

DefPosition *AST_DFA::AddDef(TreeNode *node, unsigned &bitnum, unsigned bbid) {
  unsigned stridx = 0;
  unsigned nodeid = 0;
  switch (node->GetKind()) {
    case NK_Decl: {
      DeclNode *decl = static_cast<DeclNode *>(node);
      if (decl->GetInit()) {
        stridx = decl->GetStrIdx();
        nodeid = decl->GetNodeId();
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
          stridx = lhs->GetStrIdx();
          nodeid = lhs->GetNodeId();
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
        stridx = lhs->GetStrIdx();
        nodeid = lhs->GetNodeId();
      }
      break;
    }
    default:
      break;
  }

  // update mDefPositionVec
  if (stridx) {
    if (bbid == DUMMY_BBID) {
      // special usage for GetDefStrIdx(): use bitnum to return stridx
      bitnum = stridx;
    } else {
      DefPosition pos(stridx, node->GetNodeId(), nodeid, bbid);
      bitnum++;
      mDefPositionVec.PushBack(pos);
      mDefSet.insert(stridx);
      return &pos;
    }
  }

  return NULL;
}

// this calcuates mDefPositionVec mBbIdVec
void AST_DFA::CollectDefNodes() {
  if (mTrace) std::cout << "============== CollectDefNodes ==============" << std::endl;
  AstFunction *func = mHandler->GetFunction();
  MASSERT(func && "null func");
  std::unordered_set<unsigned> done_list;
  std::deque<AstBasicBlock *> working_list;

  AstBasicBlock *bb = func->GetEntryBB();
  MASSERT(bb && "null BB");
  unsigned bbid = bb->GetId();

  working_list.push_back(bb);

  unsigned bitnum = 0;

  while(working_list.size()) {
    bb = working_list.front();
    MASSERT(bb && "null BB");
    bbid = bb->GetId();

    // process bb not visited
    if (done_list.find(bbid) == done_list.end()) {
      std::cout << "working_list work " << bbid << std::endl;
      for (int i = 0; i < bb->GetStatementsNum(); i++) {
        TreeNode *node = bb->GetStatementAtIndex(i);
        unsigned nid = node->GetNodeId();
        mStmtIdVec.PushBack(nid);
        mStmtId2StmtMap[nid] = node;
        (void) AddDef(node, bitnum, bbid);
      }

      for (int i = 0; i < bb->GetSuccessorsNum(); i++) {
        working_list.push_back(bb->GetSuccessorAtIndex(i));
      }

      done_list.insert(bbid);
      mBbIdVec.push_back(bbid);
      mBbId2BBMap[bbid] = bb;
    }

    working_list.pop_front();
  }

  if (mTrace) DumpDefPositionVec();

}

void AST_DFA::BuildBitVectors() {
  if (mTrace) std::cout << "============== BuildBitVectors ==============" << std::endl;
  AstFunction *func = mHandler->GetFunction();
  MASSERT(func && "null func");
  std::unordered_set<unsigned> done_list;
  std::deque<AstBasicBlock *> working_list;

  // init bit vectors
  unsigned bvsize = mDefPositionVec.GetNum();
  for (auto bbid: mBbIdVec) {
    BitVector *bv1 = new BitVector(bvsize);
    bv1->WipeOff(0xff);                // init with all 1
    mPrsvMap[bbid] = bv1;

    BitVector *bv2 = new BitVector(bvsize);
    bv2->WipeOff(0);
    mGenMap[bbid] = bv2;
  }

  working_list.push_back(func->GetEntryBB());

  while(working_list.size()) {
    AstBasicBlock *bb = working_list.front();
    MASSERT(bb && "null BB");
    unsigned bbid = bb->GetId();

    // skip bb already visited
    if (done_list.find(bbid) != done_list.end()) {
      working_list.pop_front();
      continue;
    }

    for (int it = 0; it < bb->GetSuccessorsNum(); it++) {
      working_list.push_back(bb->GetSuccessorAtIndex(it));
    }

    for (int it = 0; it < bb->GetStatementsNum(); it++) {
      TreeNode *node = bb->GetStatementAtIndex(it);
      unsigned stridx = GetDefStrIdx(node);
      if (stridx != 0) {
        // now loop through all the definition positions
        // mPrsvMap
        for (int i = 0; i < mDefPositionVec.GetNum(); i++) {
          // clear bits for matching stridx
          if (std::get<0>(mDefPositionVec.ValueAtIndex(i)) == stridx) {
            mPrsvMap[bbid]->ClearBit(i);
          }
        }

        // mGenMap
        for (int i = 0; i < mDefPositionVec.GetNum(); i++) {
          // set bits for matching bbid
          if (std::get<3>(mDefPositionVec.ValueAtIndex(i)) == bbid) {
            mGenMap[bbid]->SetBit(i);
          }
        }
      }
    }

    done_list.insert(bbid);
    working_list.pop_front();
  }

  // mRchInMap
  for (auto bbid: mBbIdVec) {
    BitVector *bv = new BitVector(bvsize);
    bv->Alloc(bvsize);
    bv->WipeOff(0);
    mRchInMap[bbid] = bv;
  }

  bool changed = true;
  working_list.clear();
  // initialize work list with all reachable BB
  for (auto it: mHandler->mBbId2BbMap) {
    working_list.push_back(it.second);
  }

  BitVector *old_bv = new BitVector(bvsize);
  BitVector *tmp_bv = new BitVector(bvsize);
  while (working_list.size()) {
    AstBasicBlock *bb = working_list.front();
    unsigned bbid = bb->GetId();

    tmp_bv->WipeOff(0);
    old_bv->WipeOff(0);
    old_bv->Or(mRchInMap[bbid]);
    mRchInMap[bbid]->WipeOff(0);
    for (int i = 0; i < bb->GetPredecessorsNum(); i++){
      AstBasicBlock *pred = bb->GetPredecessorAtIndex(i);
      unsigned pid = pred->GetId();
      tmp_bv->WipeOff(0);
      tmp_bv->Or(mRchInMap[pid]);
      tmp_bv->And(mPrsvMap[pid]);
      tmp_bv->Or(mGenMap[pid]);
      mRchInMap[bbid]->Or(tmp_bv);
    }

    working_list.pop_front();
    if (!mRchInMap[bbid]->Equal(old_bv)) {
      for (int i = 0; i < bb->GetSuccessorsNum(); i++) {
        working_list.push_back(bb->GetSuccessorAtIndex(i));
      }
    }
  }

  bool buildOutMap = false;
  if (buildOutMap) {
    for (auto bbid: mBbIdVec) {
      BitVector *bv = new BitVector(bvsize);
      bv->Alloc(bvsize);
      bv->WipeOff(0);
      mRchOutMap[bbid] = bv;
    }
  }

  delete old_bv;
  delete tmp_bv;
  if (mTrace) DumpAllBVMaps();
}

void AST_DFA::DumpAllBVMaps() {
  std::cout << "=== mPrsvMap ===" << std::endl;
  DumpBVMap(mPrsvMap);
  std::cout << "=== mGenMap ===" << std::endl;
  DumpBVMap(mGenMap);
  std::cout << "=== mRchInMap ===" << std::endl;
  DumpBVMap(mRchInMap);
}

void AST_DFA::DumpBVMap(BVMap &map) {
  if (!map.size()) { return; }
  std::set<unsigned> ordered(mBbIdVec.begin(), mBbIdVec.end());
  for (auto bbid: ordered) {
    std::cout << "BB" << bbid << " : ";
    DumpBV(map[bbid]);
  }
  std::cout << std::endl;
}

void AST_DFA::DumpBV(BitVector *bv) {
  std::cout << "BitVector: ";
  for (int i = 0; i < mDefPositionVec.GetNum(); i++) {
    std::cout << bv->GetBit(i);
    if (i%8 == 7) std::cout << " ";
    if (i%64 == 63) {
      std::cout << std::endl;
      std::cout << "           ";
    }
  }
  std::cout << std::endl;
}

void AST_DFA::DumpUse() {
  for (auto stridx: mDefSet) {
    std::cout << "stridx: " << stridx << " " << gStringPool.GetStringFromStrIdx(stridx) << std::endl;
    for (auto pos: mUsePositionMap[stridx]) {
      std::cout << "stmtid: " << std::get<0>(pos) << "\t";
      std::cout << "nodeid: " << std::get<1>(pos) << "\t";
      std::cout << "  bbid: " << std::get<2>(pos) << "\t";
      std::cout << std::endl;
    }
    std::cout << std::endl;
  }
}

void AST_DFA::CollectUseNodes() {
  if (mTrace) std::cout << "============== CollectUseNodes ==============" << std::endl;
  CollectUseVisitor visitor(mHandler, mTrace, true);
  for (auto bbid: mBbIdVec) {
    visitor.SetBbId(bbid);
    std::cout << " == CollectUseNodes: bbid " << bbid << std::endl;
    AstBasicBlock *bb = mBbId2BBMap[bbid];
    for (int i = 0; i < bb->GetStatementsNum(); i++) {
      TreeNode *node = bb->GetStatementAtIndex(i);
      visitor.SetStmtIdx(node->GetNodeId());
      visitor.Visit(node);
    }
  }
}

void AST_DFA::BuildDefUseChain() {
}

IdentifierNode *CollectUseVisitor::VisitIdentifierNode(IdentifierNode *node) {
  unsigned stridx = node->GetStrIdx();
  UsePosition pos(mStmtIdx, node->GetNodeId(), mBbId);
  mHandler->GetDFA()->mUsePositionMap[stridx].insert(pos);
  return node;
}

}
