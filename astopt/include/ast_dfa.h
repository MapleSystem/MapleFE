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

// DefPosition of Def: <stridx, nodeid>
typedef std::pair<unsigned, unsigned> DefPosition;
// map <bbid, BitVector>
typedef std::unordered_map<unsigned, BitVector*> BVMap;

class AST_DFA {
 private:
  AST_Handler  *mHandler;
  bool          mTrace;
  std::unordered_map<unsigned, unsigned> mVar2DeclMap; // var to decl, both NodeId

  // stmt id
  SmallVector<unsigned> mStmtIdVec;
  std::unordered_map<unsigned, TreeNode*> mStmtId2StmtMap;

  // def node id set
  std::unordered_set<unsigned> mDefNodeIdSet;
  // def use positions, def index
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
  // bbid --> bb
  std::unordered_map<unsigned, AstBasicBlock *> mBbId2BBMap;
  // bbid vec
  std::vector<unsigned> mBbIdVec;

  // def stridx set
  std::unordered_set<unsigned> mDefStrIdxSet;
  // def-use : key is def node id to a set of use node id
  std::unordered_set<unsigned, std::set<unsigned>> mDefUseMap;

  friend class CollectUseVisitor;

 public:
  explicit AST_DFA(AST_Handler *h, bool t) : mHandler(h), mTrace(t) {}
  ~AST_DFA();

  void Build(AstFunction *func);

  void CollectDefNodes(AstFunction *func);
  void CollectUseNodes();
  void BuildBitVectors();
  void BuildDefUseChain();

  bool IsDef(unsigned nid) { return mDefNodeIdSet.find(nid) != mDefNodeIdSet.end();}
  // return def stridx, return 0 if no def
  unsigned GetDefStrIdx(TreeNode *node);
  // return def nodeId, return 0 if no def
  unsigned AddDef(TreeNode *node, unsigned &bitnum, unsigned bbid);

  unsigned GetStmtIdFromNodeId(unsigned id) { return mNodeId2StmtIdMap[id]; }
  unsigned GetBbIdFromStmtId(unsigned id) { return mStmtId2BbIdMap[id]; }
  TreeNode *GetStmtFromStmtId(unsigned id) { return mStmtId2StmtMap[id]; }
  AstBasicBlock *GetBbFromBbId(unsigned id) { return mBbId2BBMap[id]; }

  void DumpDefPosition(DefPosition pos);
  void DumpDefPositionVec();
  void DumpReachDefIn();

  void DumpBV(BitVector *bv);
  void DumpBVMap(BVMap &bvmap);
  void DumpAllBVMaps();
  void DumpUse();
  void TestBV();
  void Clear();
};

class CollectUseVisitor : public AstVisitor {
 private:
  AST_Handler  *mHandler;
  AST_DFA      *mDFA;
  bool          mTrace;
  unsigned      mStmtIdx;
  unsigned      mBbId;

 public:
  explicit CollectUseVisitor(AST_Handler *h, bool t, bool base = false)
    : mHandler(h), mDFA(h->GetDFA()), mTrace(t), AstVisitor(t && base) {}
  ~CollectUseVisitor() = default;

  void SetStmtIdx(unsigned id) { mStmtIdx = id; }
  void SetBbId(unsigned id)    { mBbId    = id; }

  IdentifierNode *VisitIdentifierNode(IdentifierNode *node);
};

}
#endif
