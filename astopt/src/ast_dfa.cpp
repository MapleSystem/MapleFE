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

void AST_DFA::DataFlowAnalysis() {
  Clear();
  // TestBV();
  CollectInfo();
  CollectDefNodes();
  BuildBitVectors();
  BuildDefUseChain();
  if (mFlags & FLG_trace_3) DumpDefUse();
}

void AST_DFA::Clear() {
  mVar2DeclMap.clear();
  mStmtIdVec.Clear();
  mStmtId2StmtMap.clear();
  mDefNodeIdSet.clear();
  mDefPositionVec.Clear();
  mUsePositionMap.clear();
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
  for (auto it: mPrsvMap) {
    delete it.second;
  }
  mNodeId2StmtIdMap.clear();
  mStmtId2BbIdMap.clear();
  mDefStrIdxSet.clear();
  mDefUseMap.clear();
}

void AST_DFA::DumpDefPosition(unsigned idx, DefPosition pos) {
  std::cout << "  DefPos: " << idx;
  unsigned stridx = pos.first;
  std::cout << " stridx: " << stridx << " " << gStringPool.GetStringFromStrIdx(stridx) << "\t";
  unsigned nid = pos.second;
  std::cout << "nodeid: " << nid << "\t";
  unsigned sid = GetStmtIdFromNodeId(nid);
  std::cout << "stmtid: " << sid << "\t";
  unsigned bbid = GetBbIdFromStmtId(sid);
  std::cout << "  bbid: " << bbid << "\t";
  std::cout << std::endl;
}

void AST_DFA::DumpDefPositionVec() {
  MSGNOLOC0("============== DefPositionVec ==============");
  for (unsigned i = 0; i < mDefPositionVec.GetNum(); i++) {
    DefPosition pos = mDefPositionVec.ValueAtIndex(i);
    DumpDefPosition(i, pos);
  }
}

#define DUMMY_BBID 0xffffffff

unsigned AST_DFA::GetDefStrIdx(TreeNode *node) {
  unsigned stridx = 0;
  // pass DUMMY_BBID to indicate to get stridx only
  AddDef(node, stridx, DUMMY_BBID);
  return stridx;
}

// return def-node id defined in node
// return 0 if node has no def
unsigned AST_DFA::AddDef(TreeNode *node, unsigned &bitnum, unsigned bbid) {
  unsigned stridx = 0;
  unsigned nodeid = 0;
  bool isDefUse = false;
  switch (node->GetKind()) {
    case NK_Decl: {
      DeclNode *decl = static_cast<DeclNode *>(node);
      if (decl->GetInit()) {
        stridx = decl->GetStrIdx();
        nodeid = decl->GetVar()->GetNodeId();
      }
      break;
    }
    case NK_BinOperator: {
      BinOperatorNode *bon = static_cast<BinOperatorNode *>(node);
      OprId op = bon->GetOprId();
      switch (op) {
        case OPR_Assign: {
          TreeNode *lhs = bon->GetOpndA();
          if (lhs->IsField()) {
            lhs = static_cast<FieldNode *>(lhs)->GetUpper();
          }
          stridx = lhs->GetStrIdx();
          nodeid = lhs->GetNodeId();
          break;
        }
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
          if (lhs->IsField()) {
            lhs = static_cast<FieldNode *>(lhs)->GetUpper();
          }
          stridx = lhs->GetStrIdx();
          nodeid = lhs->GetNodeId();
          isDefUse = true;
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
      if (op == OPR_Inc || op == OPR_Dec || op == OPR_PreInc || op == OPR_PreDec) {
        TreeNode *lhs = uon->GetOpnd();
        stridx = lhs->GetStrIdx();
        nodeid = lhs->GetNodeId();
        isDefUse = true;
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
      DefPosition pos(stridx, nodeid);
      bitnum++;
      mDefStrIdxSet.insert(stridx);
      mDefNodeIdSet.insert(nodeid);
      if (isDefUse) {
        mDefUseNodeIdSet.insert(nodeid);
      }
      mDefPositionVec.PushBack(pos);
      return nodeid;
    }
  }

  return nodeid;
}

// this calcuates mDefPositionVec
void AST_DFA::CollectDefNodes() {
  MSGNOLOC0("============== CollectDefNodes ==============");
  std::unordered_set<unsigned> done_list;
  std::deque<CfgBB *> working_list;

  unsigned bitnum = 0;

  // process each functions
  for (auto func: mHandler->mModuleFuncs) {
    // add arguments as def
    TreeNode *f = static_cast<CfgFunc *>(func)->GetFuncNode();
    if (f->IsFunction()) {
      FunctionNode *fn = static_cast<FunctionNode *>(f);
      for (unsigned i = 0; i < fn->GetParamsNum(); i++) {
        TreeNode *arg = fn->GetParam(i);
        if (arg->IsIdentifier()) {
          unsigned stridx = arg->GetStrIdx();
          unsigned nodeid = arg->GetNodeId();
          DefPosition pos(stridx, nodeid);
          bitnum++;
          mDefStrIdxSet.insert(stridx);
          mDefNodeIdSet.insert(nodeid);
          mDefPositionVec.PushBack(pos);
          SetNodeId2StmtId(nodeid, fn->GetNodeId());
        }
      }
    }

    CfgBB *bb = func->GetEntryBB();
    MASSERT(bb && "null BB");
    unsigned bbid = bb->GetId();

    working_list.push_back(bb);

    while(working_list.size()) {
      bb = working_list.front();
      MASSERT(bb && "null BB");
      bbid = bb->GetId();

      // process bb not visited
      if (done_list.find(bbid) == done_list.end()) {
        if (mFlags & FLG_trace_3) std::cout << "working_list work " << bbid << std::endl;
        for (int i = 0; i < bb->GetStatementsNum(); i++) {
          TreeNode *stmt = bb->GetStatementAtIndex(i);
          unsigned nid = AddDef(stmt, bitnum, bbid);
        }

        for (int i = 0; i < bb->GetSuccessorsNum(); i++) {
          working_list.push_back(bb->GetSuccessorAtIndex(i));
        }

        done_list.insert(bbid);
      }

      working_list.pop_front();
    }
  }

  if (mFlags & FLG_trace_3) DumpDefPositionVec();
}

void AST_DFA::BuildBitVectors() {
  MSGNOLOC0("============== BuildBitVectors ==============");
  std::unordered_set<unsigned> done_list;
  std::deque<CfgBB *> working_list;

  // init bit vectors
  unsigned bvsize = mDefPositionVec.GetNum();
  for (auto bbid: mHandler->mBbIdVec) {
    BitVector *bv1 = new BitVector(bvsize);
    bv1->WipeOff(0xff);                // init with all 1
    mPrsvMap[bbid] = bv1;

    BitVector *bv2 = new BitVector(bvsize);
    bv2->WipeOff(0);
    mGenMap[bbid] = bv2;

    working_list.push_back(mHandler->mBbId2BbMap[bbid]);
  }

  while(working_list.size()) {
    CfgBB *bb = working_list.front();
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

    // function parameters are considered defined at function entry bb
    if (bb->GetAttr() == AK_Entry) {
      CfgFunc *func = mEntryBbId2FuncMap[bbid];
      if(func) {
        FunctionNode *fn = static_cast<FunctionNode *>(func->GetFuncNode());
        for (unsigned i = 0; i < fn->GetParamsNum(); i++) {
          TreeNode *arg = fn->GetParam(i);
          if (arg->IsIdentifier()) {
            for (int i = 0; i < mDefPositionVec.GetNum(); i++) {
              // set bits for matching bbid
              unsigned nid = mDefPositionVec.ValueAtIndex(i).second;
              unsigned sid = GetStmtIdFromNodeId(nid);
              unsigned bid = GetBbIdFromStmtId(sid);
              if (bid == bbid) {
                mGenMap[bbid]->SetBit(i);
              }
            }
          }
        }
      }
    }

    for (int it = 0; it < bb->GetStatementsNum(); it++) {
      TreeNode *node = bb->GetStatementAtIndex(it);
      unsigned stridx = GetDefStrIdx(node);
      if (stridx != 0) {
        // now loop through all the definition positions
        // mPrsvMap
        for (int i = 0; i < mDefPositionVec.GetNum(); i++) {
          // clear bits for matching stridx
          if (mDefPositionVec.ValueAtIndex(i).first == stridx) {
            mPrsvMap[bbid]->ClearBit(i);
          }
        }

        // mGenMap
        for (int i = 0; i < mDefPositionVec.GetNum(); i++) {
          // set bits for matching bbid
          unsigned nid = mDefPositionVec.ValueAtIndex(i).second;
          unsigned sid = GetStmtIdFromNodeId(nid);
          unsigned bid = GetBbIdFromStmtId(sid);
          if (bid == bbid) {
            mGenMap[bbid]->SetBit(i);
          }
        }
      }
    }

    done_list.insert(bbid);
    working_list.pop_front();
  }

  // mRchInMap
  for (auto bbid: mHandler->mBbIdVec) {
    BitVector *bv = new BitVector(bvsize);
    bv->Alloc(bvsize);
    bv->WipeOff(0);
    mRchInMap[bbid] = bv;
  }

  bool changed = true;
  working_list.clear();
  // initialize work list with all reachable BB
  for (auto it: done_list) {
    CfgBB *bb = mHandler->mBbId2BbMap[it];
    working_list.push_back(bb);
  }

  BitVector *old_bv = new BitVector(bvsize);
  BitVector *tmp_bv = new BitVector(bvsize);
  while (working_list.size()) {
    CfgBB *bb = working_list.front();
    unsigned bbid = bb->GetId();

    tmp_bv->WipeOff(0);
    old_bv->WipeOff(0);
    old_bv->Or(mRchInMap[bbid]);
    mRchInMap[bbid]->WipeOff(0);
    for (int i = 0; i < bb->GetPredecessorsNum(); i++){
      CfgBB *pred = bb->GetPredecessorAtIndex(i);
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
    for (auto bbid: mHandler->mBbIdVec) {
      BitVector *bv = new BitVector(bvsize);
      bv->Alloc(bvsize);
      bv->WipeOff(0);
      mRchOutMap[bbid] = bv;
    }
  }

  delete old_bv;
  delete tmp_bv;
  if (mFlags & FLG_trace_3) DumpAllBVMaps();
}

void AST_DFA::DumpAllBVMaps() {
  MSGNOLOC0("=== mPrsvMap ===");
  DumpBVMap(mPrsvMap);
  MSGNOLOC0("=== mGenMap ===");
  DumpBVMap(mGenMap);
  MSGNOLOC0("=== mRchInMap ===");
  DumpBVMap(mRchInMap);
}

void AST_DFA::DumpBVMap(BVMap &map) {
  if (!map.size()) { return; }
  std::set<unsigned> ordered(mHandler->mBbIdVec.begin(), mHandler->mBbIdVec.end());
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

void AST_DFA::DumpDefUse() {
  MSGNOLOC0("============== Dump DefUse ==============");
  for (unsigned i = 0; i < mDefPositionVec.GetNum(); i++) {
    DefPosition pos = mDefPositionVec.ValueAtIndex(i);
    DumpDefPosition(i, pos);

    std::cout << "Use: ";
    for (auto nid: mDefUseMap[pos.second]) {
      std::cout << nid << " ";
    }
    std::cout << std::endl;
  }
}

void AST_DFA::CollectInfo() {
  MSGNOLOC0("============== CollectInfo ==============");
  // process each functions for arguments
  for (auto func: mHandler->mModuleFuncs) {
    // add arguments as def
    TreeNode *f = static_cast<CfgFunc *>(func)->GetFuncNode();
    if (f && f->IsFunction()) {
      unsigned sid = f->GetNodeId();
      mStmtIdVec.PushBack(sid);

      // use entry bbid for function and its arguments
      unsigned bbid = func->GetEntryBB()->GetId();
      mStmtId2BbIdMap[sid] = bbid;
      mEntryBbId2FuncMap[bbid] = func;

      FunctionNode *fn = static_cast<FunctionNode *>(f);
      for (unsigned i = 0; i < fn->GetParamsNum(); i++) {
        TreeNode *arg = fn->GetParam(i);
        if (arg->IsIdentifier()) {
          SetNodeId2StmtId(arg->GetNodeId(), sid);
        }
      }
    }
  }

  // loop through each BB and each statement
  CollectInfoVisitor visitor(mHandler, mFlags, true);
  for (auto bbid: mHandler->mBbIdVec) {
    visitor.SetBbId(bbid);
    if (mFlags & FLG_trace_3) std::cout << " == bbid " << bbid << std::endl;
    CfgBB *bb = mHandler->mBbId2BbMap[bbid];
    for (int i = 0; i < bb->GetStatementsNum(); i++) {
      TreeNode *stmt = bb->GetStatementAtIndex(i);
      // function nodes are handled above
      // so do not override with real bbid
      if (stmt->IsFunction()) {
        continue;
      }
      unsigned sid = stmt->GetNodeId();
      mStmtIdVec.PushBack(sid);
      mStmtId2StmtMap[sid] = stmt;
      mStmtId2BbIdMap[sid] = bbid;
      visitor.SetStmtIdx(stmt->GetNodeId());
      visitor.SetBbId(bbid);
      visitor.Visit(stmt);
    }
  }
}

IdentifierNode *CollectInfoVisitor::VisitIdentifierNode(IdentifierNode *node) {
  AstVisitor::VisitIdentifierNode(node);
  mDFA->SetNodeId2StmtId(node->GetNodeId(), mStmtIdx);
  return node;
}

void AST_DFA::BuildDefUseChain() {
  MSGNOLOC0("============== BuildDefUseChain ==============");
  DefUseChainVisitor visitor(mHandler, mFlags, true);
  std::unordered_set<unsigned> defStrIdxs;
  std::unordered_set<unsigned> done_list;
  CfgBB *bb;

  // loop through each variable def
  for (int i = 0; i < mDefPositionVec.GetNum(); i++) {
    DefPosition pos = mDefPositionVec.ValueAtIndex(i);
    if (mFlags & FLG_trace_3) DumpDefPosition(i, pos);
    std::deque<CfgBB *> working_list;

    // def stridx
    visitor.mDefStrIdx = pos.first;
    // def nodeid
    visitor.mDefNodeId = pos.second;
    visitor.mReachNewDef = false;
    unsigned sid = GetStmtIdFromNodeId(visitor.mDefNodeId);
    unsigned bid = GetBbIdFromStmtId(sid);
    // check if func entry bb
    if (mEntryBbId2FuncMap.find(bid) != mEntryBbId2FuncMap.end()) {
      // func arguments are defined in entry bb, mark defined
      visitor.mReachDef = true;
    } else {
      visitor.mReachDef = false;
    }

    bb = mHandler->mBbId2BbMap[bid];

    // loop through each BB and each statement
    working_list.push_back(bb);
    done_list.clear();

    while(working_list.size()) {
      bb = working_list.front();
      MASSERT(bb && "null BB");
      unsigned bbid = bb->GetId();
      MSGNOLOC("BB", bbid);

      // check if def is either alive at bb entry or created in bb
      bool alive = mRchInMap[bbid]->GetBit(i);
      bool gen = mGenMap[bbid]->GetBit(i);
      if (!(alive || gen)) {
        done_list.insert(bbid);
        working_list.pop_front();
        continue;
      }

      // process bb
      visitor.VisitBB(bbid);

      // add successors to working_list if not in done_list
      if (done_list.find(bbid) == done_list.end()) {
        // if new def in bb, then no need to visit successors for the current def
        if (!visitor.mReachNewDef) {
          for (int i = 0; i < bb->GetSuccessorsNum(); i++) {
            working_list.push_back(bb->GetSuccessorAtIndex(i));
          }
        }
      }

      done_list.insert(bbid);
      working_list.pop_front();
    }
  }
}

void DefUseChainVisitor::VisitBB(unsigned bbid) {
  SetBbId(bbid);
  CfgBB *bb = mHandler->mBbId2BbMap[bbid];
  for (int i = 0; i < bb->GetStatementsNum(); i++) {
    TreeNode *node = bb->GetStatementAtIndex(i);
    SetStmtIdx(node->GetNodeId());
    if (node->IsFunction()) {
      FunctionNode *func = static_cast<FunctionNode *>(node);
      for (unsigned i = 0; i < func->GetParamsNum(); i++) {
        TreeNode *arg = func->GetParam(i);
        Visit(arg);
      }
    } else {
      Visit(node);
    }
    if (mReachNewDef) {
      return;
    }
  }
  return;
}

IdentifierNode *DefUseChainVisitor::VisitIdentifierNode(IdentifierNode *node) {
  if (mFlags & FLG_trace_1) std::cout << "Visiting IdentifierNode, id=" << node->GetNodeId() << "..." << std::endl;

  // only deal with use with same stridx of current def
  unsigned stridx = node->GetStrIdx();
  if (stridx != mDefStrIdx) {
    return node;
  }

  // new def, quit
  unsigned nid = node->GetNodeId();
  bool isDef = mHandler->GetDFA()->IsDef(nid);
  bool isUse = mHandler->GetDFA()->IsDefUse(nid);
  if (isDef) {
    if (mReachDef) {
      mReachNewDef = true;
      if (!isUse) {
        return node;
      }
    } else {
      mReachDef = true;
      isUse = false;
    }
  } else if (mReachDef) {
    isUse = true;
  }

  // exclude its own decl
  TreeNode *p = node->GetParent();
  if (p && p->IsDecl()) {
    DeclNode *dn = static_cast<DeclNode *>(p);
    if (dn->GetVar() == node) {
      return node;
    }
  }

  // add to mDefUseMap
  if (isUse) {
    mDFA->mDefUseMap[mDefNodeId].insert(nid);
  }
  return node;
}

BinOperatorNode *DefUseChainVisitor::VisitBinOperatorNode(BinOperatorNode *node) {
  if (mFlags & FLG_trace_1) std::cout << "Visiting BinOperatorNode, id=" << node->GetNodeId() << "..." << std::endl;

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
      // visit rhs first due to computation use/def order
      TreeNode *rhs = bon->GetOpndB();
      (void) AstVisitor::VisitTreeNode(rhs);

      TreeNode *lhs = bon->GetOpndA();
      (void) AstVisitor::VisitTreeNode(lhs);
      break;
    }
    default:
      TreeNode *lhs = bon->GetOpndA();
      (void) AstVisitor::VisitTreeNode(lhs);

      TreeNode *rhs = bon->GetOpndB();
      (void) AstVisitor::VisitTreeNode(rhs);
      break;
  }
  return node;
}


}
