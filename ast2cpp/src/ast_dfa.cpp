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
  for (auto it: mNodeId2NodeMap) {
    free(it.second);
  }
  mNodeId2NodeMap.clear();
  for (auto it: mPrsvMap) {
    free(it.second);
  }
  for (auto it: mGenMap) {
    free(it.second);
  }
  for (auto it: mRchInMap) {
    free(it.second);
  }
  for (auto it: mRchOutMap) {
    free(it.second);
  }
  mBbIdSet.clear();
}

void AST_DFA::TestBV() {
  unsigned size = 300;
  BitVector *bv1 = new BitVector(size);
  bv1->WipeOff(0xab);                // init with all 1

  BitVector *bv2 = new BitVector(size);
  bv2->WipeOff(0x12);

  DumpBV(bv1);
  DumpBV(bv2);

  bv1->Or(bv2);
  DumpBV(bv1);

  bv1->And(bv2);
  DumpBV(bv1);
}

void AST_DFA::Build() {
  // TestBV();
  CollectDefNodes();
  BuildBitVectors();
}

void AST_DFA::DumpDefPosition(DefPosition pos) {
  std::cout << "stridx: " << std::get<0>(pos) << std::endl;
  std::cout << "nodeid: " << std::get<1>(pos) << std::endl;
  std::cout << "  bbid: " << std::get<2>(pos) << std::endl;
  std::cout << std::endl;
}

void AST_DFA::DumpDefPositionVec() {
  for (unsigned i = 0; i < mDefPositionVecSize; i++) {
    DefPosition pos = mDefPositionVec.ValueAtIndex(i);
    std::cout << "BitPos: " << i << std::endl;
    DumpDefPosition(pos);
  }
}

unsigned AST_DFA::GetDefStrIdx(TreeNode *node) {
  unsigned stridx = 0;
  AddDef(node, stridx, 0xffffffff);
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
    if (bbid != 0xffffffff) {
      DefPosition pos(stridx, nodeid, bbid);
      bitnum++;
      mDefPositionVec.PushBack(pos);
      return &pos;
    } else {
      // special usage for GetDefStrIdx(): use bitnum as a return value
      bitnum = stridx;
    }
  }

  return NULL;
}

// this calcuates mDefPositionVec mDefPositionVecSize mBbIdSet
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

  mDefPositionVecSize = mDefPositionVec.GetNum();
  if (mTrace) DumpDefPositionVec();

}

void AST_DFA::BuildBitVectors() {
  if (mTrace) std::cout << "============== BuildBitVectors ==============" << std::endl;
  AST_Function *func = mHandler->GetFunction();
  MASSERT(func && "null func");
  std::unordered_set<unsigned> done_list;
  std::deque<AST_BB *> working_list;

  // init bit vectors
  for (auto bbid: mBbIdSet) {
    BitVector *bv1 = new BitVector(mDefPositionVecSize);
    bv1->WipeOff(0xff);                // init with all 1
    mPrsvMap[bbid] = bv1;

    BitVector *bv2 = new BitVector(mDefPositionVecSize);
    bv2->WipeOff(0);
    mGenMap[bbid] = bv2;
  }

  working_list.push_back(func->GetEntryBB());

  while(working_list.size()) {
    AST_BB *bb = working_list.front();
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
          if (std::get<2>(mDefPositionVec.ValueAtIndex(i)) == bbid) {
            mGenMap[bbid]->SetBit(i);
          }
        }
      }
    }

    done_list.insert(bbid);
    working_list.pop_front();
  }

  // mRchInMap
  for (auto bbid: mBbIdSet) {
    BitVector *bv = new BitVector(mDefPositionVecSize);
    bv->Alloc(mDefPositionVecSize);
    bv->WipeOff(0);
    mRchInMap[bbid] = bv;
  }

  bool changed = true;
  working_list.clear();
  // initialize work list with all reachable BB
  for (auto it: mHandler->mBbId2BbMap) {
    working_list.push_back(it.second);
  }

  BitVector *old_bv = new BitVector(mDefPositionVecSize);
  BitVector *tmp_bv = new BitVector(mDefPositionVecSize);
  while (working_list.size()) {
    AST_BB *bb = working_list.front();
    unsigned bbid = bb->GetId();

    tmp_bv->WipeOff(0);
    old_bv->WipeOff(0);
    old_bv->Or(mRchInMap[bbid]);
    mRchInMap[bbid]->WipeOff(0);
    for (int i = 0; i < bb->GetPredecessorsNum(); i++){
      AST_BB *pred = bb->GetPredecessorAtIndex(i);
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
    for (auto bbid: mBbIdSet) {
      BitVector *bv = new BitVector(mDefPositionVecSize);
      bv->Alloc(mDefPositionVecSize);
      bv->WipeOff(0);
      mRchOutMap[bbid] = bv;
    }
  }

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
  std::set<unsigned> ordered(mBbIdSet.begin(), mBbIdSet.end());
  for (auto bbid: ordered) {
    std::cout << "BB" << bbid << " : ";
    DumpBV(map[bbid]);
  }
  std::cout << std::endl;
}

void AST_DFA::DumpBV(BitVector *bv) {
  std::cout << "BitVector: ";
  for (int i = 0; i < mDefPositionVecSize; i++) {
    std::cout << bv->GetBit(i);
    if (i%8 == 7) std::cout << " ";
    if (i%64 == 63) {
      std::cout << std::endl;
      std::cout << "           ";
    }
  }
  std::cout << std::endl;
}
}
