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
#include <algorithm>
#include <filesystem>
#include "ast_handler.h"
#include "ast_util.h"
#include "astopt.h"
#include "ast_xxport.h"
#include "gen_astdump.h"

namespace maplefe {

AST_Xxport::AST_Xxport(AstOpt *o, unsigned f) {
  mAstOpt = o;
  mASTHandler = o->GetASTHandler();
  mFlags = f;
}

unsigned AST_Xxport::GetModuleNum() {
  return mASTHandler->GetSize();
}

void AST_Xxport::BuildModuleOrder() {
  // setup module stridx
  SetModuleStrIdx();

  // collect dependent info
  AddHandler();

  // sort handlers with dependency
  SortHandler();

  if (mFlags & FLG_trace_2) {
    std::cout << "============== Module Order ==============" << std::endl;
    std::list<unsigned>::iterator it = mHandlersIdxInOrder.begin();
    for (; it != mHandlersIdxInOrder.end(); it++) {
      unsigned idx = *it;
      Module_Handler *handler = mASTHandler->GetModuleHandler(idx);
      ModuleNode *module = handler->GetASTModule();
      std::cout << "module : " << gStringPool.GetStringFromStrIdx(module->GetStrIdx()) << std::endl;
    }
  }
}

void AST_Xxport::SetModuleStrIdx() {
  for (int i = 0; i < GetModuleNum(); i++) {
    Module_Handler *handler = mASTHandler->GetModuleHandler(i);
    ModuleNode *module = handler->GetASTModule();

    // setup module node stridx with module file name
    unsigned stridx = gStringPool.GetStrIdx(module->GetFilename());
    module->SetStrIdx(stridx);
    mStrIdx2HandlerIdxMap[stridx] = i;

    if (mFlags & FLG_trace_2) {
      std::cout << "module : " << gStringPool.GetStringFromStrIdx(module->GetStrIdx()) << std::endl;
    }
  }
}

void AST_Xxport::AddHandler() {
  for (int i = 0; i < GetModuleNum(); i++) {
    Module_Handler *handler = mASTHandler->GetModuleHandler(i);
    ModuleNode *module = handler->GetASTModule();

    XXportNodeVisitor visitor(this, handler, i, mFlags);
    visitor.Visit(module);
  }
}

void AST_Xxport::SortHandler() {
  for (int i = 0; i < GetModuleNum(); i++) {
    if (mHandlersIdxInOrder.size() == 0) {
      mHandlersIdxInOrder.push_back(i);
      continue;
    }

    bool added = false;
    std::list<unsigned>::iterator it = mHandlersIdxInOrder.begin();
    for (; it != mHandlersIdxInOrder.end(); it++) {
      unsigned idx = *it;
      // check if handler idx has dependency on i
      if (mHandlerIdx2DependentHandlerIdxMap[idx].find(i) != mHandlerIdx2DependentHandlerIdxMap[i].end()) {
        mHandlersIdxInOrder.insert(it, i);
        added = true;
        break;
      }
    }

    if (!added) {
      mHandlersIdxInOrder.push_back(i);
    }
  }

  // copy result to AstOpt
  std::list<unsigned>::iterator it = mHandlersIdxInOrder.begin();
  for (; it != mHandlersIdxInOrder.end(); it++) {
    unsigned idx = *it;
    Module_Handler *h = mASTHandler->GetModuleHandler(idx);
    mAstOpt->AddModuleHandler(h);
  }
}

// set up import node stridx with target module file name stridx
ImportNode *XXportNodeVisitor::VisitImportNode(ImportNode *node) {
  (void) AstVisitor::VisitImportNode(node);

  TreeNode *target = node->GetTarget();
  if (target) {
    std::string name = GetTargetFilename(target);

    // store name's string index in node
    unsigned stridx = gStringPool.GetStrIdx(name);
    node->SetStrIdx(stridx);

    // update handler dependency map
    unsigned dep = mASTXxport->GetHandleIdxFromStrIdx(stridx);
    mASTXxport->AddHandlerIdx2DependentHandlerIdxMap(mHandlerIdx, dep);
  }

  return node;
}

// set up export node stridx with target module file name stridx
ExportNode *XXportNodeVisitor::VisitExportNode(ExportNode *node) {
  (void) AstVisitor::VisitExportNode(node);

  TreeNode *target = node->GetTarget();
  if (target) {
    std::string name = GetTargetFilename(target);

    // store name's string index in node
    unsigned stridx = gStringPool.GetStrIdx(name);
    node->SetStrIdx(stridx);

    // update handler dependency map
    unsigned dep = mASTXxport->GetHandleIdxFromStrIdx(stridx);
    mASTXxport->AddHandlerIdx2DependentHandlerIdxMap(mHandlerIdx, dep);
  }

  return node;
}

std::string XXportNodeVisitor::GetTargetFilename(TreeNode *node) {
  std::string filename;
  if (node && node->IsLiteral()) {
    LiteralNode *lit = static_cast<LiteralNode *>(node);
    LitData data = lit->GetData();
    filename = AstDump::GetEnumLitData(data);
    filename += ".ts"s;
    if(filename.front() != '/') {
      ModuleNode *module = mHandler->GetASTModule();
      std::filesystem::path p = module->GetFilename();
      try {
        p = std::filesystem::canonical(p.parent_path() / filename);
        filename = p.string();
      }
      catch(std::filesystem::filesystem_error const& ex) {
        // Ignore std::filesystem::filesystem_error exception
        // keep filename without converting it to a cannonical path
      }
    }
  }
  return filename;
}

}
