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

AST_XXport::AST_XXport(AstOpt *o, unsigned f) {
  mAstOpt = o;
  mASTHandler = o->GetASTHandler();
  mFlags = f;
}

unsigned AST_XXport::GetModuleNum() {
  return mASTHandler->GetSize();
}

void AST_XXport::BuildModuleOrder() {
  // setup module stridx
  SetModuleStrIdx();

  // collect dependent info
  CollectXXportNodes();

  // collect dependent info
  AddHandler();

  // sort handlers with dependency
  SortHandler();

  // collect import/export info
  CollectXXportInfo();

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

void AST_XXport::SetModuleStrIdx() {
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

void AST_XXport::CollectXXportNodes() {
  for (int i = 0; i < GetModuleNum(); i++) {
    Module_Handler *handler = mASTHandler->GetModuleHandler(i);
    ModuleNode *module = handler->GetASTModule();

    XXportBasicVisitor visitor(this, handler, i, mFlags);
    visitor.Visit(module);
  }
}

void AST_XXport::AddHandler() {
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

void AST_XXport::SortHandler() {
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

void AST_XXport::CollectXXportInfo() {
  for (unsigned hidx = 0; hidx < GetModuleNum(); hidx++) {
    Module_Handler *handler = mASTHandler->GetModuleHandler(hidx);
    ModuleNode *module = handler->GetASTModule();

    for (auto it : mImportNodeSets[hidx]) {
      ImportNode *node = it;
      TreeNode *target = GetTarget(node);
      unsigned stridx = (target && target->GetStrIdx()) ? target->GetStrIdx() : module->GetStrIdx();
      XXportInfo *info = new XXportInfo(stridx, node->GetNodeId());;

      for (unsigned i = 0; i < node->GetPairsNum(); i++) {
        XXportAsPairNode *p = node->GetPair(i);
        TreeNode *bfnode = p->GetBefore();
        TreeNode *afnode = p->GetAfter();
        MASSERT(bfnode && "before node NULL for default");
        if (p->IsEverything()) {
          info->SetEverything();

          MASSERT(target && "everything export no target");
          SetIdStrIdx2ModuleStrIdx(bfnode->GetStrIdx(), target->GetStrIdx());
        }

        // reformat default import
        if (!p->IsDefault()) {
          if (IsDefault(bfnode) ) {
            p->SetIsDefault(true);
            p->SetBefore(afnode);
            p->SetAfter(NULL);
          }
        }

        if (p->IsDefault()) {
          info->mDefaultNodeId = bfnode->GetNodeId();
        } else {
          std::pair<unsigned, unsigned> pnid(bfnode->GetNodeId(), afnode ? afnode->GetNodeId() : 0);
          info->mNodeIdPairs.insert(pnid);
        }
      }

      mImports[hidx].insert(info);
    }

    for (auto it : mExportNodeSets[hidx]) {
      ExportNode *node = it;
      TreeNode *target = GetTarget(node);
      unsigned stridx = (target && target->GetStrIdx()) ? target->GetStrIdx() : module->GetStrIdx();
      XXportInfo *info = new XXportInfo(stridx, node->GetNodeId());;

      for (unsigned i = 0; i < node->GetPairsNum(); i++) {
        XXportAsPairNode *p = node->GetPair(i);
        TreeNode *bfnode = p->GetBefore();
        TreeNode *afnode = p->GetAfter();
        if (p->IsEverything()) {
          info->SetEverything();
          if (bfnode) {
            // export * as MM from "./M";
            // bfnode represents a module
            MASSERT(target && "everything export no target");
            SetIdStrIdx2ModuleStrIdx(bfnode->GetStrIdx(), target->GetStrIdx());

            unsigned hidx = GetHandleIdxFromStrIdx(target->GetStrIdx());
            Module_Handler *handler = mASTHandler->GetModuleHandler(hidx);
            ModuleNode *module = handler->GetASTModule();
            bfnode->SetTypeId(TY_Module);
            bfnode->SetTypeIdx(module->GetTypeIdx());
          } else {
            // export * from "./M"
            continue;
          }
        }

        // reformat default export
        if (p->IsDefault()) {
          p->SetIsRef(false);
        } else if (afnode && IsDefault(afnode)) {
          p->SetIsDefault(true);
          p->SetBefore(bfnode);
          p->SetAfter(NULL);
        }

        bfnode = p->GetBefore();
        afnode = p->GetAfter();
        if (p->IsDefault()) {
          if (afnode) {
            info->mDefaultNodeId = afnode->GetNodeId();
          } else {
            info->mDefaultNodeId = bfnode->GetNodeId();
          }
        } else if (mExportNodeSets[hidx].size() == 1 && node->GetPairsNum() == 1) {
          info->mDefaultNodeId = bfnode->GetNodeId();
          std::pair<unsigned, unsigned> pnid(bfnode->GetNodeId(), afnode ? afnode->GetNodeId() : 0);
          info->mNodeIdPairs.insert(pnid);
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
  mASTXXport->mImportNodeSets[mHandlerIdx].insert(node);
  return node;
}

ExportNode *XXportBasicVisitor::VisitExportNode(ExportNode *node) {
  (void) AstVisitor::VisitExportNode(node);
  mASTXXport->mExportNodeSets[mHandlerIdx].insert(node);
  return node;
}

TreeNode *AST_XXport::GetTarget(TreeNode *node) {
  TreeNode *tree = NULL;
  if (node->IsImport()) {
    tree = static_cast<ImportNode *>(node)->GetTarget();
  } else if (node->IsExport()) {
    tree = static_cast<ExportNode *>(node)->GetTarget();
  }
  return tree;
}

// borrowed from ast2cpp
std::string AST_XXport::GetTargetFilename(unsigned hidx, TreeNode *node) {
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
void AST_XXport::UpdateDependency(unsigned hidx, TreeNode *node) {
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

unsigned AST_XXport::ExtractTargetStrIdx(TreeNode *node) {
  unsigned stridx = 0;
  if (node->IsField()) {
    FieldNode *fld = static_cast<FieldNode *>(node);
    TreeNode *upper = fld->GetUpper();
    stridx = GetModuleStrIdxFromIdStrIdx(upper->GetStrIdx());
    if (stridx) {
      unsigned hidx = GetHandleIdxFromStrIdx(stridx);
      Module_Handler *handler = mASTHandler->GetModuleHandler(hidx);
      ModuleNode *module = handler->GetASTModule();
      upper->SetTypeId(TY_Module);
      upper->SetTypeIdx(module->GetTypeIdx());
    }
  }
  return stridx;
}

// hstridx is the string index of handler/module name with full path
TreeNode *AST_XXport::GetExportedDefault(unsigned hstridx) {
  if (mStrIdx2HandlerIdxMap.find(hstridx) != mStrIdx2HandlerIdxMap.end()) {
    unsigned hidx = GetHandleIdxFromStrIdx(hstridx);
    for (auto it : mExports[hidx]) {
      if (it->mDefaultNodeId) {
        return mAstOpt->GetNodeFromNodeId(it->mDefaultNodeId);
      }
    }
  }
  return NULL;
}

// hidx is the index of handler, string is the string index of identifier
TreeNode *AST_XXport::GetExportedNamedNode(unsigned hidx, unsigned stridx) {
  for (auto it : mExports[hidx]) {
    for (auto it1 : it->mNodeIdPairs) {
      unsigned nid = it1.first;
      TreeNode *node = mAstOpt->GetNodeFromNodeId(nid);
      if (node->GetStrIdx() == stridx ) {
        return node;
      }
    }
  }
  return NULL;
}

// hidx is the index of handler, string is the string index of identifier
TreeNode *AST_XXport::GetExportedNodeFromImportedNode(unsigned hidx, unsigned nid) {
  TreeNode *node = mAstOpt->GetNodeFromNodeId(nid);
  unsigned stridx = node->GetStrIdx();

  for (auto it : mImports[hidx]) {
    if (it->mDefaultNodeId == nid) {
      TreeNode *node = GetExportedDefault(it->mModuleStrIdx);
      return node;
    }
    for (auto it1 : it->mNodeIdPairs) {
      unsigned nid2 = it1.second;
      if (nid2 == nid) {
        unsigned nid1 = it1.first;
        TreeNode *node1 = mAstOpt->GetNodeFromNodeId(nid1);
        if (node1->GetStrIdx() == stridx ) {
          unsigned hexpidx = GetHandleIdxFromStrIdx(it->mModuleStrIdx);
          TreeNode *n = GetExportedNamedNode(hexpidx, stridx);
          return n;
        }
      }
    }
  }
  return NULL;
}

}
