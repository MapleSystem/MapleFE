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

#ifndef __AST_XXPORT_HEADER__
#define __AST_XXPORT_HEADER__

#include <list>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include "ast_module.h"
#include "ast.h"
#include "gen_astvisitor.h"

namespace maplefe {

class AstOpt;
class AST_Handler;
class Module_Handler;

class XXportInfo {
 public:
  unsigned mModuleStrIdx;
  unsigned mDefaultNodeId;
  bool     mEverything;
  std::set<std::pair<unsigned, unsigned>> mNodeIdPairs;

 public:
  explicit XXportInfo(unsigned m) : mModuleStrIdx(m), mDefaultNodeId(0), mEverything(false) {}
  ~XXportInfo() = default;

  void SetEverything() {mEverything = true;}
  void Dump();
};

class AST_Xxport {
 private:
  AstOpt      *mAstOpt;
  AST_Handler *mASTHandler;
  unsigned     mFlags;

  std::list<unsigned> mHandlersIdxInOrder;
  std::unordered_map<unsigned, std::unordered_set<unsigned>> mHandlerIdx2DependentHandlerIdxMap;
  std::unordered_map<unsigned, unsigned> mStrIdx2HandlerIdxMap;

  // module handler idx to set of XXportInfo
  std::unordered_map<unsigned, std::unordered_set<XXportInfo *>> mImports;
  std::unordered_map<unsigned, std::unordered_set<XXportInfo *>> mExports;

 public:
  // module handler idx to import/export nodes in the module
  std::unordered_map<unsigned, std::unordered_set<ImportNode *>> mImportNodeSets;
  std::unordered_map<unsigned, std::unordered_set<ExportNode *>> mExportNodeSets;

  std::unordered_map<unsigned, std::unordered_set<unsigned>> ImportedDeclIds;
  std::unordered_map<unsigned, std::unordered_set<unsigned>> ExportedDeclIds;

 public:
  explicit AST_Xxport(AstOpt *o, unsigned f);
  ~AST_Xxport() {}

  unsigned GetModuleNum();

  void BuildModuleOrder();

  void SetModuleStrIdx();
  void CollectXxportNodes();

  TreeNode *GetTarget(TreeNode *node);
  std::string GetTargetFilename(unsigned hidx, TreeNode *node);
  void UpdateDependency(unsigned hidx, TreeNode *node);
  void AddHandler();

  void SortHandler();

  void CollectXxportInfo();

  void AddHandlerIdx2DependentHandlerIdxMap(unsigned hdlIdx, unsigned depHdlIdx) {
    mHandlerIdx2DependentHandlerIdxMap[hdlIdx].insert(depHdlIdx);
  }

  unsigned GetHandleIdxFromStrIdx(unsigned stridx) {
    return mStrIdx2HandlerIdxMap[stridx];
  }

  bool IsImportExportDeclId(unsigned hidx, unsigned id) {
    return (ImportedDeclIds[hidx].find(id) != ImportedDeclIds[hidx].end() ||
            ExportedDeclIds[hidx].find(id) != ExportedDeclIds[hidx].end());
  }

  void Dump();
};

class XXportBasicVisitor : public AstVisitor {
 private:
  AST_Xxport     *mASTXxport;
  Module_Handler *mHandler;
  unsigned       mHandlerIdx;
  unsigned       mFlags;

 public:
  std::unordered_set<ModuleNode *> mImported;

 public:
  explicit XXportBasicVisitor(AST_Xxport *xx, Module_Handler *h, unsigned i, unsigned f, bool base = false)
    : mASTXxport(xx), mHandler(h), mHandlerIdx(i), mFlags(f), AstVisitor((f & FLG_trace_1) && base) {}
  ~XXportBasicVisitor() = default;

  ImportNode *VisitImportNode(ImportNode *node);
  ExportNode *VisitExportNode(ExportNode *node);
};

}
#endif
