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

// Position of Definition: <stridx, nodeid, bbid>
typedef std::tuple<unsigned, unsigned, unsigned> DefPosition;
// map <bbid, BitVector>
typedef std::unordered_map<unsigned, BitVector*> BVMap;

class AST_DFA {
 private:
  AST_Handler  *mHandler;
  bool          mTrace;
  std::unordered_map<unsigned, unsigned> mVar2DeclMap; // var to decl, both NodeId
  std::unordered_map<unsigned, TreeNode*> mNodeId2NodeMap;
  SmallVector<DefPosition> mDefPositionVec;
  unsigned mDefPositionVecSize;
  StringPool mStringPool;

  // followint maps with key BB id
  BVMap mPrsvMap;
  BVMap mGenMap;
  BVMap mRchInMap; // reaching definition bit vector entering bb
  BVMap mRchOutMap;

  std::unordered_set<unsigned> mBbIdSet;             // bb ids in the function

 public:
  explicit AST_DFA(AST_Handler *h, bool t) : mHandler(h), mTrace(t) {}
  ~AST_DFA();

  void Build();

  void CollectDefNodes();
  void BuildBitVectors();

  unsigned GetDefStrIdx(TreeNode *node);
  DefPosition *AddDef(TreeNode *node, unsigned &bitnum, unsigned bbid);
  // unsigned GetDecl(VarNode *);

  unsigned GetDefPositionVecSize() { return mDefPositionVecSize; }
  void DumpDefPosition(DefPosition pos);
  void DumpDefPositionVec();
  void DumpReachDefIn();

  void DumpBV(BitVector *bv);
  void DumpBVMap(BVMap &bvmap);
  void DumpAllBVMaps();
  void TestBV();
};

}
#endif
