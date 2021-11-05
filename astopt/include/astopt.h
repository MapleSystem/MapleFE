/*
* Copyright (C) [2020] Futurewei Technologies, Inc. All rights reverved.
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

//////////////////////////////////////////////////////////////////////////////////////////////
//                This is the interface to translate AST to C++
//////////////////////////////////////////////////////////////////////////////////////////////

#ifndef __ASTOPT_HEADER__
#define __ASTOPT_HEADER__

#include <vector>
#include <unordered_map>
#include "ast_module.h"
#include "ast.h"
#include "gen_astvisitor.h"
#include "ast_common.h"

namespace maplefe {

class AST_Handler;
class Module_Handler;
class AST_XXport;

class AstOpt {
private:
  AST_Handler *mASTHandler;
  AST_XXport  *mASTXXport;
  unsigned     mFlags;

  // module handlers in mASTHandler sorted by import/export dependency
  std::vector<Module_Handler *> mHandlersIdxInOrder;

  // nodeid to node map for all nodes in all modules
  std::unordered_map<unsigned, TreeNode*> mNodeId2NodeMap;

public:
  explicit AstOpt(AST_Handler *h, unsigned f);
  ~AstOpt() { delete mASTHandler; }

  AST_Handler *GetASTHandler() {return mASTHandler;}
  AST_XXport *GetASTXXport() {return mASTXXport;}
  unsigned GetModuleNum();
  Module_Handler *GetModuleHandler(unsigned i) { return mHandlersIdxInOrder[i]; }
  void AddModuleHandler(Module_Handler *h) { mHandlersIdxInOrder.push_back(h); }

  virtual void ProcessAST(unsigned trace);

  void PreprocessModules();
  void CollectInfo();
  void AdjustAST();
  void ScopeAnalysis();

  void BasicAnalysis();
  void BuildCFG();
  void ControlFlowAnalysis();
  void TypeInference();
  void DataFlowAnalysis();

  TreeNode *GetNodeFromNodeId(unsigned nid) { return mNodeId2NodeMap[nid]; }
  void AddNodeId2NodeMap(TreeNode *node) { mNodeId2NodeMap[node->GetNodeId()] = node; }
};

class BuildNodeIdToNodeVisitor : public AstVisitor {
  AstOpt   *mAstOpt;
  unsigned  mFlags;

  public:
  explicit BuildNodeIdToNodeVisitor(AstOpt *opt, unsigned f, bool base = false)
    : mAstOpt(opt), mFlags(f), AstVisitor((f & FLG_trace_1) && base) {}
  ~BuildNodeIdToNodeVisitor() = default;

  TreeNode *VisitTreeNode(TreeNode *node) {
    if (mFlags & FLG_trace_3) std::cout << "nodeid2node:   " << node->GetNodeId() << std::endl;
    (void) AstVisitor::VisitTreeNode(node);
    mAstOpt->AddNodeId2NodeMap(node);
    return node;
  }

  TreeNode *BaseTreeNode(TreeNode *node) {
    if (mFlags & FLG_trace_3) std::cout << "nodeid2node: b " << node->GetNodeId() << std::endl;
    (void) AstVisitor::BaseTreeNode(node);
    mAstOpt->AddNodeId2NodeMap(node);
    return node;
  }
};

}
#endif
