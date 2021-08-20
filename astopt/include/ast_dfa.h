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

#ifndef __AST_DFA_HEADER__
#define __AST_DFA_HEADER__

#include <stack>
#include <utility>

#include "stringpool.h"
#include "ast_module.h"
#include "ast.h"
#include "ast_type.h"
#include "ast_handler.h"
#include "gen_astvisitor.h"

namespace maplefe {

// def positions: <stridx, nodeid>
typedef std::pair<unsigned, unsigned> DefPosition;
// map <bbid, BitVector>
typedef std::unordered_map<unsigned, BitVector*> BVMap;

class AST_DFA {
 private:
  Module_Handler *mHandler;
  unsigned        mFlags;
  std::unordered_map<unsigned, unsigned> mVar2DeclMap; // var to decl, both NodeId

  // stmt id
  SmallVector<unsigned> mStmtIdVec;
  std::unordered_map<unsigned, TreeNode*> mStmtId2StmtMap;
  std::unordered_map<unsigned, CfgFunc*> mEntryBbId2FuncMap;

  // def node id set
  std::unordered_set<unsigned> mDefNodeIdSet;
  // def and use id set: i in i++; i+=j;
  std::unordered_set<unsigned> mDefUseNodeIdSet;
  // def positions, def index
  SmallVector<DefPosition> mDefPositionVec;
  // use stridx to set of node id
  std::unordered_map<unsigned, std::set<unsigned>> mUsePositionMap;

  // followint maps with key BB id
  BVMap mPrsvMap;
  BVMap mGenMap;
  BVMap mRchInMap; // reaching definition bit vector entering bb
  BVMap mRchOutMap;

  // def/use nid --> stmtid
  std::unordered_map<unsigned, unsigned> mNodeId2StmtIdMap;
  // stmtid --> bbid
  std::unordered_map<unsigned, unsigned> mStmtId2BbIdMap;

  // def stridx set
  std::unordered_set<unsigned> mDefStrIdxSet;
  // def-use : key is def node id to a set of use node id
  std::unordered_map<unsigned, std::set<unsigned>> mDefUseMap;

  friend class DefUseChainVisitor;

 public:
  explicit AST_DFA(Module_Handler *h, unsigned f) : mHandler(h), mFlags(f) {}
  ~AST_DFA();

  void DataFlowAnalysis();

  void CollectInfo();
  void CollectDefNodes();
  void BuildBitVectors();
  void BuildDefUseChain();

  bool IsDef(unsigned nid) { return mDefNodeIdSet.find(nid) != mDefNodeIdSet.end();}
  bool IsDefUse(unsigned nid) { return mDefUseNodeIdSet.find(nid) != mDefUseNodeIdSet.end();}
  // return def stridx, return 0 if no def
  unsigned GetDefStrIdx(TreeNode *node);
  // return def nodeId, return 0 if no def
  unsigned AddDef(TreeNode *node, unsigned &bitnum, unsigned bbid);

  void SetNodeId2StmtId(unsigned nid, unsigned sid) { mNodeId2StmtIdMap[nid] = sid; }
  unsigned GetStmtIdFromNodeId(unsigned id) { return mNodeId2StmtIdMap[id]; }
  unsigned GetBbIdFromStmtId(unsigned id) { return mStmtId2BbIdMap[id]; }
  TreeNode *GetStmtFromStmtId(unsigned id) { return mStmtId2StmtMap[id]; }
  CfgBB *GetBbFromBbId(unsigned id) { return mHandler->GetBbFromBbId(id); }

  void DumpDefPosition(unsigned idx, DefPosition pos);
  void DumpDefPositionVec();
  void DumpReachDefIn();

  void DumpBV(BitVector *bv);
  void DumpBVMap(BVMap &bvmap);
  void DumpAllBVMaps();
  void DumpDefUse();
  void TestBV();
  void Clear();
};

class CollectInfoVisitor : public AstVisitor {
 private:
  Module_Handler *mHandler;
  AST_DFA        *mDFA;
  unsigned        mFlags;
  unsigned        mStmtIdx;
  unsigned        mBbId;

 public:
  explicit CollectInfoVisitor(Module_Handler *h, unsigned f, bool base = false)
    : mHandler(h), mDFA(h->GetDFA()), mFlags(f), AstVisitor((f & FLG_trace_1) && base) {}
  ~CollectInfoVisitor() = default;

  void SetStmtIdx(unsigned id) { mStmtIdx = id; }
  void SetBbId(unsigned id)    { mBbId    = id; }

  IdentifierNode *VisitIdentifierNode(IdentifierNode *node);
};

class DefUseChainVisitor : public AstVisitor {
 private:
  Module_Handler *mHandler;
  AST_DFA        *mDFA;
  unsigned        mFlags;
  unsigned        mStmtIdx;
  unsigned        mBbId;

 public:
  unsigned      mDefNodeId;
  unsigned      mDefStrIdx;
  unsigned      mReachDef;
  unsigned      mReachNewDef;

 public:
  explicit DefUseChainVisitor(Module_Handler *h, unsigned f, bool base = false)
    : mHandler(h), mDFA(h->GetDFA()), mFlags(f), AstVisitor((f & FLG_trace_1) && base) {}
  ~DefUseChainVisitor() = default;

  void SetStmtIdx(unsigned id) { mStmtIdx = id; }
  void SetBbId(unsigned id)    { mBbId    = id; }
  void VisitBB(unsigned bbid);

  IdentifierNode *VisitIdentifierNode(IdentifierNode *node);
  BinOperatorNode *VisitBinOperatorNode(BinOperatorNode *node);
};

}
#endif
