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

// Position of Def: <stridx, stmtid, nodeid, bbid>
typedef std::tuple<unsigned, unsigned, unsigned, unsigned> DefPosition;
// Position of Use: <stmtidx, nodeid, bbid>
typedef std::tuple<unsigned, unsigned, unsigned> UsePosition;
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

  // def use positions
  SmallVector<DefPosition> mDefPositionVec;
  std::unordered_map<unsigned, std::set<UsePosition>> mUsePositionMap;;

  // followint maps with key BB id
  BVMap mPrsvMap;
  BVMap mGenMap;
  BVMap mRchInMap; // reaching definition bit vector entering bb
  BVMap mRchOutMap;

  std::vector<unsigned> mBbIdVec;                     // bb ids in the function, depth first order
  std::unordered_map<unsigned, AstBasicBlock *> mBbId2BBMap; // bb id to bb map
  std::unordered_set<unsigned> mDefSet;               // def stridx set
  std::unordered_set<unsigned, std::set<UsePosition>> mDefUseMap; // def-use : key is DefPositionVec idx

  friend class CollectUseVisitor;

 public:
  explicit AST_DFA(AST_Handler *h, bool t) : mHandler(h), mTrace(t) {}
  ~AST_DFA();

  void Build();

  void CollectUseNodes();
  void CollectDefNodes();
  void BuildBitVectors();
  void BuildDefUseChain();

  unsigned GetDefStrIdx(TreeNode *node);
  DefPosition *AddDef(TreeNode *node, unsigned &bitnum, unsigned bbid);

  void DumpDefPosition(DefPosition pos);
  void DumpDefPositionVec();
  void DumpReachDefIn();

  void DumpBV(BitVector *bv);
  void DumpBVMap(BVMap &bvmap);
  void DumpAllBVMaps();
  void DumpUse();
  void TestBV();
};

class CollectUseVisitor : public AstVisitor {
 private:
  AST_Handler  *mHandler;
  bool          mTrace;
  unsigned      mStmtIdx;
  unsigned      mStrIdx;
  unsigned      mNodeIdx;
  unsigned      mBbId;
  bool          mFound;

 public:
  explicit CollectUseVisitor(AST_Handler *h, bool t, bool base = false)
    : mHandler(h), mTrace(t), mStrIdx(0), mNodeIdx(0), mFound(false), AstVisitor(t && base) {}
  ~CollectUseVisitor() = default;

  void SetStmtIdx(unsigned id) { mStmtIdx = id; }
  void SetStrIdx(unsigned id)  { mStrIdx  = id; }
  void SetNodeIdx(unsigned id) { mNodeIdx = id; }
  void SetBbId(unsigned id)    { mBbId    = id; }
  void SetFound(bool b)        { mFound   = b; }

  IdentifierNode *VisitIdentifierNode(IdentifierNode *node);
};

}
#endif
