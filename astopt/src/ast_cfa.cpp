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
#include "ast_handler.h"
#include "ast_cfa.h"

namespace maplefe {

void AST_CFA::ControlFlowAnalysis() {
  MSGNOLOC0("============== ControlFlowAnalysis ==============");
  CollectReachableBB();
  RemoveUnreachableBB();

  if (mFlags & FLG_trace_3) {
    Dump();
  }
}

void AST_CFA::Dump() {
  ModuleNode *module = mHandler->GetASTModule();
  for(unsigned i = 0; i < module->GetTreesNum(); i++) {
    TreeNode *it = module->GetTree(i);
    it->Dump(0);
    std::cout << std::endl;
  }
}

// this calcuates mNodeId2BbMap
void AST_CFA::CollectReachableBB() {
  MSGNOLOC0("============== CollectReachableBB ==============");
  mReachableBbIdx.clear();
  std::deque<CfgBB *> working_list;

  // process each functions
  for (auto func: mHandler->mModuleFuncs) {
    // initialisze work list with all entry BB
    working_list.push_back(func->GetEntryBB());

    while(working_list.size()) {
      CfgBB *bb = working_list.front();
      MASSERT(bb && "null BB");
      unsigned bbid = bb->GetId();

      // skip bb already visited
      if (mReachableBbIdx.find(bbid) != mReachableBbIdx.end()) {
        working_list.pop_front();
        continue;
      }

      for (int i = 0; i < bb->GetSuccessorsNum(); i++) {
        working_list.push_back(bb->GetSuccessorAtIndex(i));
      }

      for (int i = 0; i < bb->GetStatementsNum(); i++) {
        TreeNode *node = bb->GetStatementAtIndex(i);
        mHandler->SetBbFromNodeId(node->GetNodeId(), bb);
      }

      mHandler->SetBbFromBbId(bbid, bb);
      mHandler->mBbIdVec.push_back(bbid);

      mReachableBbIdx.insert(bbid);
      working_list.pop_front();
    }
  }
}

void AST_CFA::RemoveUnreachableBB() {
  std::set<CfgBB *> deadBb;
  CfgBB *bb = nullptr;
  for (auto id: mReachableBbIdx) {
    bb = mHandler->mBbId2BbMap[id];
    for (int i = 0; i < bb->GetPredecessorsNum(); i++) {
      CfgBB *pred = bb->GetPredecessorAtIndex(i);
      unsigned pid = pred->GetId();
      if (mHandler->mBbId2BbMap.find(pid) == mHandler->mBbId2BbMap.end()) {
        deadBb.insert(pred);
      }
    }
  }
  for (auto it: deadBb) {
    if (mFlags & FLG_trace_3) std::cout << "deleted BB :";
    for (int i = 0; i < it->GetSuccessorsNum(); i++) {
      bb = it->GetSuccessorAtIndex(i);
      bb->mPredecessors.Remove(it);
    }
    if (mFlags & FLG_trace_3) std::cout << " BB" << it->GetId();
    it->~CfgBB();
  }
  if (mFlags & FLG_trace_3) std::cout << std::endl;
}

}
