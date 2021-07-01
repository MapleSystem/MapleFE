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

void AST_DFA::Build(CfgFunc *func) {
  Clear();
  // TestBV();
  CollectDefNodes(func);
  BuildBitVectors();
  CollectUseNodes();
  // DumpUse();
  BuildDefUseChain();
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
  mBbIdVec.clear();
  for (auto it: mPrsvMap) {
    delete it.second;
  }
  mNodeId2StmtIdMap.clear();
  mStmtId2BbIdMap.clear();
  mBbId2BBMap.clear();
  mDefStrIdxSet.clear();
  mDefUseMap.clear();
}

void AST_DFA::DumpDefPosition(DefPosition pos) {
  unsigned stridx = pos.first;
  std::cout << "stridx: " << stridx << " " << gStringPool.GetStringFromStrIdx(stridx) << std::endl;
  unsigned nid = pos.second;
  std::cout << "nodeid: " << nid << "\t";
  unsigned sid = GetStmtIdFromNodeId(nid);
  std::cout << "stmtid: " << sid << "\t";
  unsigned bbid = GetBbIdFromStmtId(sid);
  std::cout << "  bbid: " << bbid << "\t";
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

// return def-node id defined in node
// return 0 if node has no def
unsigned AST_DFA::AddDef(TreeNode *node, unsigned &bitnum, unsigned bbid) {
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
      if (op == OPR_Inc || op == OPR_Dec || op == OPR_PreInc || op == OPR_PreDec) {
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
      DefPosition pos(stridx, nodeid);
      bitnum++;
      mDefStrIdxSet.insert(stridx);
      mDefNodeIdSet.insert(nodeid);
      mDefPositionVec.PushBack(pos);
      return nodeid;
    }
  }

  return nodeid;
}

// this calcuates mDefPositionVec mBbIdVec
void AST_DFA::CollectDefNodes(CfgFunc *func) {
  if (mTrace) std::cout << "============== CollectDefNodes ==============" << std::endl;
  std::unordered_set<unsigned> done_list;
  std::deque<CfgBB *> working_list;

  CfgBB *bb = func->GetEntryBB();
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
      if (mTrace) std::cout << "working_list work " << bbid << std::endl;
      for (int i = 0; i < bb->GetStatementsNum(); i++) {
        TreeNode *stmt = bb->GetStatementAtIndex(i);
        unsigned sid = stmt->GetNodeId();
        mStmtIdVec.PushBack(sid);
        mStmtId2StmtMap[sid] = stmt;
        mStmtId2BbIdMap[sid] = bbid;
        unsigned nid = AddDef(stmt, bitnum, bbid);
        if (nid) {
          mNodeId2StmtIdMap[nid] = sid;
        }
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
  std::unordered_set<unsigned> done_list;
  std::deque<CfgBB *> working_list;

  // init bit vectors
  unsigned bvsize = mDefPositionVec.GetNum();
  for (auto bbid: mBbIdVec) {
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
  for (auto bbid: mBbIdVec) {
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
  if (mTrace) std::cout << "============== Dump Use ==============" << std::endl;
  for (auto stridx: mDefStrIdxSet) {
    std::cout << "stridx: " << stridx << " " << gStringPool.GetStringFromStrIdx(stridx) << std::endl;
    for (auto nid: mUsePositionMap[stridx]) {
      std::cout << "nodeid: " << nid << "\t";
      unsigned sid = GetStmtIdFromNodeId(nid);
      std::cout << "stmtid: " << sid << "\t";
      unsigned bbid = GetBbIdFromStmtId(sid);
      std::cout << "  bbid: " << bbid << "\t";
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
    if (mTrace) std::cout << " == CollectUseNodes: bbid " << bbid << std::endl;
    CfgBB *bb = mBbId2BBMap[bbid];
    for (int i = 0; i < bb->GetStatementsNum(); i++) {
      TreeNode *node = bb->GetStatementAtIndex(i);
      visitor.SetStmtIdx(node->GetNodeId());
      visitor.Visit(node);
    }
  }
}

void AST_DFA::BuildDefUseChain() {
  for (int i = 0; i < mDefPositionVec.GetNum(); i++) {
    DefPosition pos = mDefPositionVec.ValueAtIndex(i);
  }
}

IdentifierNode *CollectUseVisitor::VisitIdentifierNode(IdentifierNode *node) {
  unsigned stridx = node->GetStrIdx();
  // only collect use with def in the function
  if (mDFA->mDefStrIdxSet.find(stridx) == mDFA->mDefStrIdxSet.end()) {
    return node;
  }

  // exclude def
  unsigned nid = node->GetNodeId();
  if (mHandler->GetDFA()->IsDef(nid))
    return node;

  // exclude its own decl
  TreeNode *p = node->GetParent();
  if (p && p->IsDecl()) {
    DeclNode *dn = static_cast<DeclNode *>(p);
    if (dn->GetVar() == node) {
      return node;
    }
  }

  mDFA->mUsePositionMap[stridx].insert(nid);
  mDFA->mNodeId2StmtIdMap[nid] = mStmtIdx;
  return node;
}

void AST_DFA::BuildScope() {
  if (mTrace) std::cout << "============== BuildScope ==============" << std::endl;
  BuildScopeVisitor visitor(mHandler, mTrace, true);
  while(!visitor.mScopeStack.empty()) {
    visitor.mScopeStack.pop();
  }
  ModuleNode *module = mHandler->GetASTModule();
  visitor.mScopeStack.push(module->GetRootScope());
  mHandler->mNodeId2Scope[module->GetNodeId()] = module->GetRootScope();

  for(unsigned i = 0; i < module->GetTreesNum(); i++) {
    TreeNode *it = module->GetTree(i);
    visitor.Visit(it);
  }
}

BlockNode *BuildScopeVisitor::VisitBlockNode(BlockNode *node) {
  ASTScope *parent = mScopeStack.top();
  ASTScope *scope = mASTModule->NewScope(parent);
  scope->SetTree(node);
  mHandler->mNodeId2Scope[node->GetNodeId()] = scope;
  parent->AddChild(scope);

  mScopeStack.push(scope);

  AstVisitor::VisitBlockNode(node);

  mScopeStack.pop();
  return node;
}

FunctionNode *BuildScopeVisitor::VisitFunctionNode(FunctionNode *node) {
  ASTScope *parent = mScopeStack.top();
  // function is a decl
  parent->AddDecl(node);
  ASTScope *scope = mASTModule->NewScope(parent);
  scope->SetTree(node);
  mHandler->mNodeId2Scope[node->GetNodeId()] = scope;
  parent->AddChild(scope);

  // add parameters as decl
  for(unsigned i = 0; i < node->GetParamsNum(); i++) {
    TreeNode *it = node->GetParam(i);
    scope->AddDecl(it);
  }

  mScopeStack.push(scope);

  AstVisitor::VisitFunctionNode(node);

  mScopeStack.pop();
  return node;
}

LambdaNode *BuildScopeVisitor::VisitLambdaNode(LambdaNode *node) {
  ASTScope *parent = mScopeStack.top();
  ASTScope *scope = mASTModule->NewScope(parent);
  scope->SetTree(node);
  mHandler->mNodeId2Scope[node->GetNodeId()] = scope;
  parent->AddChild(scope);

  // add parameters as decl
  for(unsigned i = 0; i < node->GetParamsNum(); i++) {
    TreeNode *it = node->GetParam(i);
    scope->AddDecl(it);
  }

  mScopeStack.push(scope);

  AstVisitor::VisitLambdaNode(node);

  mScopeStack.pop();
  return node;
}

ClassNode *BuildScopeVisitor::VisitClassNode(ClassNode *node) {
  ASTScope *parent = mScopeStack.top();
  // inner class is a decl
  if (parent) {
    parent->AddDecl(node);
    parent->AddType(node);
  }
  ASTScope *scope = mASTModule->NewScope(parent);
  scope->SetTree(node);
  mHandler->mNodeId2Scope[node->GetNodeId()] = scope;
  parent->AddChild(scope);

  // add fields as decl
  for(unsigned i = 0; i < node->GetFieldsNum(); i++) {
    TreeNode *it = node->GetField(i);
    if (it->IsIdentifier())
      scope->AddDecl(it);
  }

  mScopeStack.push(scope);

  AstVisitor::VisitClassNode(node);

  mScopeStack.pop();
  return node;
}

InterfaceNode *BuildScopeVisitor::VisitInterfaceNode(InterfaceNode *node) {
  ASTScope *parent = mScopeStack.top();
  // inner interface is a decl
  if (parent) {
    parent->AddDecl(node);
    parent->AddType(node);
  }

  ASTScope *scope = mASTModule->NewScope(parent);
  scope->SetTree(node);
  mHandler->mNodeId2Scope[node->GetNodeId()] = scope;
  parent->AddChild(scope);

  // add fields as decl
  for(unsigned i = 0; i < node->GetFieldsNum(); i++) {
    TreeNode *it = node->GetFieldAtIndex(i);
    if (it->IsIdentifier())
      scope->AddDecl(it);
  }
  mScopeStack.push(scope);

  AstVisitor::VisitInterfaceNode(node);

  mScopeStack.pop();
  return node;
}

DeclNode *BuildScopeVisitor::VisitDeclNode(DeclNode *node) {
  ASTScope *parent = mScopeStack.top();
  AstVisitor::VisitDeclNode(node);
  parent->AddDecl(node);
  return node;
}

UserTypeNode *BuildScopeVisitor::VisitUserTypeNode(UserTypeNode *node) {
  ASTScope *parent = mScopeStack.top();
  AstVisitor::VisitUserTypeNode(node);
  TreeNode *p = node->GetParent();
  if (p) {
    if (p->IsFunction()) {
      // exclude function return type
      FunctionNode *f = static_cast<FunctionNode *>(p);
      if (f->GetType() == node) {
        return node;
      }
    } else if (p->IsTypeAlias()) {
      // typalias id
      TypeAliasNode *ta = static_cast<TypeAliasNode *>(p);
      if (ta->GetId() == node) {
        parent->AddType(node);
        return node;
      }
    } else if (p->IsScope()) {
      // normal type decl
      parent->AddType(node);
    }
  }
  return node;
}

TypeAliasNode *BuildScopeVisitor::VisitTypeAliasNode(TypeAliasNode *node) {
  ASTScope *parent = mScopeStack.top();
  AstVisitor::VisitTypeAliasNode(node);
  UserTypeNode *ut = node->GetId();
  if (ut) {
    parent->AddType(ut);
  }
  return node;
}

}
