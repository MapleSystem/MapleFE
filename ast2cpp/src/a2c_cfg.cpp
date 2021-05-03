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

#include "a2c_cfg.h"

namespace maplefe {

  FunctionNode *CfgVisitor::VisitFunctionNode(FunctionNode *node) {
    if(mTrace)
      std::cout << "CfgVisitor: enter FunctionNode, id=" << node->GetNodeId() << std::endl;

    AstVisitor::VisitFunctionNode(node);

    if(mTrace)
      std::cout << "CfgVisitor: exit FunctionNode, id=" << node->GetNodeId() << std::endl;
    return node;
  }



  void A2C_CFG::BuildCFG() {
    CfgVisitor visitor(mTraceCFG, true);
    for(auto it: mModule->mTrees)
          visitor.Visit(it->mRootNode);
  }

  void A2C_CFG::Dump() {
  }


}

