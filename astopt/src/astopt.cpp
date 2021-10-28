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

#include <filesystem>
#include "astopt.h"
#include "ast_handler.h"
#include "gen_astdump.h"
#include "gen_astgraph.h"
#include "gen_aststore.h"
#include "gen_astload.h"

namespace maplefe {

class ImportedFiles;

// starting point of AST
void AstOpt::ProcessAST(unsigned flags) {
  // loop through module handlers
  unsigned size = mASTHandler->mModuleHandlers.GetNum();
  for (int i = 0; i < size; i++) {
    Module_Handler *handler = mASTHandler->mModuleHandlers.ValueAtIndex(i);
    ModuleNode *module = handler->GetASTModule();

    mFlags = flags;
    if (mFlags & FLG_trace_3) {
      std::cout << "============= in ProcessAST ===========" << std::endl;
      std::cout << "srcLang : " << module->GetSrcLangString() << std::endl;

      for(unsigned i = 0; i < module->GetTreesNum(); i++) {
        TreeNode *tnode = module->GetTree(i);
        tnode->Dump(0);
        std::cout << std::endl;
      }
    }
  }

  // basic analysis
  BasicAnalysis();

  // build CFG
  BuildCFG();

  // control flow analysis
  ControlFlowAnalysis();

  // type inference
  TypeInference();

  // data flow analysis
  DataFlowAnalysis();

  for (int i = 0; i < size; i++) {
    Module_Handler *handler = mASTHandler->mModuleHandlers.ValueAtIndex(i);
    ModuleNode *module = handler->GetASTModule();

    AstStore saveAst(module);
    saveAst.StoreInAstBuf();
  }

  return;
}

void AstOpt::BuildModuleOrder() {
  unsigned size = mASTHandler->mModuleHandlers.GetNum();
  // setup module stridx
  for (int i = 0; i < size; i++) {
    SetModuleStrIdx(i);
  }

  // collect dependent info
  for (int i = 0; i < size; i++) {
    AddHandler(i);
  }

  // sort handlers with dependency
  for (int i = 0; i < size; i++) {
    SortHandler(i);
  }

  if (mFlags & FLG_trace_2) {
    std::cout << "============== Module Order ==============" << std::endl;
    std::list<unsigned>::iterator it = mHandlersIdxInOrder.begin();
    for (; it != mHandlersIdxInOrder.end(); it++) {
      unsigned idx = *it;
      Module_Handler *handler = mASTHandler->mModuleHandlers.ValueAtIndex(idx);
      ModuleNode *module = handler->GetASTModule();
      std::cout << "module : " << gStringPool.GetStringFromStrIdx(module->GetStrIdx()) << std::endl;
    }
  }
}

void AstOpt::CollectInfo() {
  std::list<unsigned>::iterator it = mHandlersIdxInOrder.begin();
  for (; it != mHandlersIdxInOrder.end(); it++) {
    unsigned i = *it;
    Module_Handler *handler = mASTHandler->mModuleHandlers.ValueAtIndex(i);
    handler->CollectInfo();
  }
}

void AstOpt::AdjustAST() {
  std::list<unsigned>::iterator it = mHandlersIdxInOrder.begin();
  for (; it != mHandlersIdxInOrder.end(); it++) {
    unsigned i = *it;
    Module_Handler *handler = mASTHandler->mModuleHandlers.ValueAtIndex(i);
    handler->AdjustAST();
  }
}

void AstOpt::ScopeAnalysis() {
  std::list<unsigned>::iterator it = mHandlersIdxInOrder.begin();
  for (; it != mHandlersIdxInOrder.end(); it++) {
    unsigned i = *it;
    Module_Handler *handler = mASTHandler->mModuleHandlers.ValueAtIndex(i);
    handler->ScopeAnalysis();

    if (mFlags & FLG_trace_2) {
      std::cout << "============== Dump Scope ==============" << std::endl;
      ModuleNode *module = handler->GetASTModule();
      module->GetRootScope()->Dump(0);
    }
  }
}

void AstOpt::BasicAnalysis() {
  // list modules according to dependency
  BuildModuleOrder();

  // collect AST info
  CollectInfo();

  // rewirte some AST nodes
  AdjustAST();

  // scope analysis
  ScopeAnalysis();
}

void AstOpt::BuildCFG() {
  std::list<unsigned>::iterator it = mHandlersIdxInOrder.begin();
  for (; it != mHandlersIdxInOrder.end(); it++) {
    unsigned i = *it;
    Module_Handler *handler = mASTHandler->mModuleHandlers.ValueAtIndex(i);
    handler->BuildCFG();
  }
}

void AstOpt::ControlFlowAnalysis() {
  std::list<unsigned>::iterator it = mHandlersIdxInOrder.begin();
  for (; it != mHandlersIdxInOrder.end(); it++) {
    unsigned i = *it;
    Module_Handler *handler = mASTHandler->mModuleHandlers.ValueAtIndex(i);
    handler->ControlFlowAnalysis();
  }
}

void AstOpt::TypeInference() {
  std::list<unsigned>::iterator it = mHandlersIdxInOrder.begin();
  for (; it != mHandlersIdxInOrder.end(); it++) {
    unsigned i = *it;
    Module_Handler *handler = mASTHandler->mModuleHandlers.ValueAtIndex(i);
    handler->TypeInference();
  }
}

void AstOpt::DataFlowAnalysis() {
  std::list<unsigned>::iterator it = mHandlersIdxInOrder.begin();
  for (; it != mHandlersIdxInOrder.end(); it++) {
    unsigned i = *it;
    Module_Handler *handler = mASTHandler->mModuleHandlers.ValueAtIndex(i);
    handler->DataFlowAnalysis();
  }
}

void AstOpt::SetModuleStrIdx(unsigned i) {
  Module_Handler *handler = mASTHandler->mModuleHandlers.ValueAtIndex(i);
  ModuleNode *module = handler->GetASTModule();

  // setup module node stridx with module file name
  unsigned stridx = gStringPool.GetStrIdx(module->GetFilename());
  module->SetStrIdx(stridx);
  mStrIdx2HandlerIdxMap[stridx] = i;

  if (mFlags & FLG_trace_2) {
    std::cout << "module : " << gStringPool.GetStringFromStrIdx(module->GetStrIdx()) << std::endl;
  }
}

void AstOpt::AddHandler(unsigned i) {
  Module_Handler *handler = mASTHandler->mModuleHandlers.ValueAtIndex(i);
  ModuleNode *module = handler->GetASTModule();

  XXportNodeVisitor visitor(this, handler, i, mFlags);
  visitor.Visit(module);
}

void AstOpt::SortHandler(unsigned i) {
  if (mHandlersIdxInOrder.size() == 0) {
    mHandlersIdxInOrder.push_back(i);
    return;
  }
  std::list<unsigned>::iterator it = mHandlersIdxInOrder.begin();
  for (; it != mHandlersIdxInOrder.end(); it++) {
    unsigned idx = *it;
    // check if handler idx has dependency on i
    if (mHandlerIdx2DependentHandlerIdxMap[idx].find(i) != mHandlerIdx2DependentHandlerIdxMap[i].end()) {
      mHandlersIdxInOrder.insert(it, i);
      return;
    }
  }
  mHandlersIdxInOrder.push_back(i);
}

// set up import node stridx with target module file name stridx
ImportNode *XXportNodeVisitor::VisitImportNode(ImportNode *node) {
  (void) AstVisitor::VisitImportNode(node);

  TreeNode *target = node->GetTarget();
  std::string name = GetTargetFilename(target);

  // store name's string index in node
  unsigned stridx = gStringPool.GetStrIdx(name);
  node->SetStrIdx(stridx);

  // update handler dependency map
  unsigned dep = mAstOpt->GetHandleIdxFromStrIdx(stridx);
  mAstOpt->AddHandlerIdx2DependentHandlerIdxMap(mHandlerIdx, dep);

  return node;
}

// set up export node stridx with target module file name stridx
ExportNode *XXportNodeVisitor::VisitExportNode(ExportNode *node) {
  (void) AstVisitor::VisitExportNode(node);

  TreeNode *target = node->GetTarget();
  std::string name = GetTargetFilename(target);

  // store name's string index in node
  unsigned stridx = gStringPool.GetStrIdx(name);
  node->SetStrIdx(stridx);

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
