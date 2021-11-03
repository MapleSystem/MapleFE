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
  CollectXxportNodes();

  // collect dependent info
  AddHandler();

  // sort handlers with dependency
  SortHandler();

  // collect import/export info
  CollectXxportInfo();

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

void AST_Xxport::CollectXxportNodes() {
  for (int i = 0; i < GetModuleNum(); i++) {
    Module_Handler *handler = mASTHandler->GetModuleHandler(i);
    ModuleNode *module = handler->GetASTModule();

    XXportBasicVisitor visitor(this, handler, i, mFlags);
    visitor.Visit(module);
  }
}

void AST_Xxport::AddHandler() {
  for (unsigned hidx = 0; hidx < GetModuleNum(); hidx++) {
    Module_Handler *handler = mASTHandler->GetModuleHandler(hidx);
    for (auto it : mImportNodeSets[hidx]) {
      ImportNode *node = it;
      UpdateDependency(hidx, node);
    }
    for (auto it : mExportNodeSets[hidx]) {
      ExportNode *node = it;
      UpdateDependency(hidx, node);
    }
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

void AST_Xxport::CollectXxportInfo() {
  for (unsigned hidx = 0; hidx < GetModuleNum(); hidx++) {
    Module_Handler *handler = mASTHandler->GetModuleHandler(hidx);
    ModuleNode *module = handler->GetASTModule();

    for (auto it : mImportNodeSets[hidx]) {
      ImportNode *node = it;
      XXportInfo *info = new XXportInfo(module->GetStrIdx());;

      for (unsigned i = 0; i < node->GetPairsNum(); i++) {
        XXportAsPairNode *p = node->GetPair(i);
        TreeNode *bfnode = p->GetBefore();
        TreeNode *afnode = p->GetAfter();
        MASSERT(bfnode && "before node NULL for default");
        if (p->IsDefault()) {
          info->mDefaultNodeId = bfnode->GetNodeId();
        } else if (bfnode) {
          std::pair<unsigned, unsigned> pnid(bfnode->GetNodeId(), afnode ? afnode->GetNodeId() : 0);
          info->mNodeIdPairs.insert(pnid);
        }
      }

      mImports[hidx].insert(info);
    }

    for (auto it : mExportNodeSets[hidx]) {
      ExportNode *node = it;
      XXportInfo *info = new XXportInfo(module->GetStrIdx());;

      for (unsigned i = 0; i < node->GetPairsNum(); i++) {
        XXportAsPairNode *p = node->GetPair(i);
        TreeNode *bfnode = p->GetBefore();
        TreeNode *afnode = p->GetAfter();
        if (p->IsEverything()) {
          info->SetEverything();
          if (bfnode) {
            // export * as MM from "./M";
            // bfnode represents a module
            bfnode->SetTypeId(TY_Module);
          } else {
            // export * from "./M"
            continue;
          }
        }
        if (afnode && afnode->GetStrIdx() == gStringPool.GetStrIdx("default")) {
          info->mDefaultNodeId = bfnode->GetNodeId();
        } else {
          std::pair<unsigned, unsigned> pnid(bfnode->GetNodeId(), afnode ? afnode->GetNodeId() : 0);
          info->mNodeIdPairs.insert(pnid);
        }
      }

      mExports[hidx].insert(info);
    }
  }
}

ImportNode *XXportBasicVisitor::VisitImportNode(ImportNode *node) {
  (void) AstVisitor::VisitImportNode(node);
  mASTXxport->mImportNodeSets[mHandlerIdx].insert(node);
  return node;
}

ExportNode *XXportBasicVisitor::VisitExportNode(ExportNode *node) {
  (void) AstVisitor::VisitExportNode(node);
  mASTXxport->mExportNodeSets[mHandlerIdx].insert(node);
  return node;
}

TreeNode *AST_Xxport::GetTarget(TreeNode *node) {
  TreeNode *tree = NULL;
  if (node->IsImport()) {
    tree = static_cast<ImportNode *>(node)->GetTarget();
  } else if (node->IsExport()) {
    tree = static_cast<ExportNode *>(node)->GetTarget();
  }
  return tree;
}

// borrowed from ast2cpp
std::string AST_Xxport::GetTargetFilename(unsigned hidx, TreeNode *node) {
  std::string filename;
  if (node && node->IsLiteral()) {
    LiteralNode *lit = static_cast<LiteralNode *>(node);
    LitData data = lit->GetData();
    filename = AstDump::GetEnumLitData(data);
    filename += ".ts"s;
    if(filename.front() != '/') {
      Module_Handler *handler = mASTHandler->GetModuleHandler(hidx);
      ModuleNode *module = handler->GetASTModule();
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

// set up import/export node stridx with target module file name stridx
void AST_Xxport::UpdateDependency(unsigned hidx, TreeNode *node) {
  TreeNode *target = GetTarget(node);
  if (target) {
    std::string name = GetTargetFilename(hidx, target);

    // store name's string index in node
    unsigned stridx = gStringPool.GetStrIdx(name);
    node->SetStrIdx(stridx);
    target->SetStrIdx(stridx);

    // update handler dependency map
    unsigned dep = GetHandleIdxFromStrIdx(stridx);
    mHandlerIdx2DependentHandlerIdxMap[hidx].insert(dep);
  }
}

}
