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
  unsigned mXXportNodeId;
  unsigned mDefaultNodeId;
  bool     mEverything;
  std::set<std::pair<unsigned, unsigned>> mNodeIdPairs;

 public:
  explicit XXportInfo(unsigned mod, unsigned nid) :
    mModuleStrIdx(mod), mXXportNodeId(nid), mDefaultNodeId(0), mEverything(false) {}
  ~XXportInfo() = default;

  void SetEverything() {mEverything = true;}
  void Dump();
};

class AST_XXport {
 private:
  AstOpt      *mAstOpt;
  AST_Handler *mASTHandler;
  unsigned     mFlags;

  std::list<unsigned> mHandlersIdxInOrder;
  std::unordered_map<unsigned, std::unordered_set<unsigned>> mHandlerIdx2DependentHandlerIdxMap;
  std::unordered_map<unsigned, unsigned> mStrIdx2HandlerIdxMap;

  std::unordered_map<unsigned, unsigned> mIdStrIdx2ModuleStrIdxMap;

 public:
  // module handler idx to set of XXportInfo
  std::unordered_map<unsigned, std::unordered_set<XXportInfo *>> mImports;
  std::unordered_map<unsigned, std::unordered_set<XXportInfo *>> mExports;

  // module handler idx to import/export nodes in the module
  std::unordered_map<unsigned, std::unordered_set<ImportNode *>> mImportNodeSets;
  std::unordered_map<unsigned, std::unordered_set<ExportNode *>> mExportNodeSets;

  std::unordered_map<unsigned, std::unordered_set<unsigned>> mImportedDeclIds;
  std::unordered_map<unsigned, std::unordered_set<unsigned>> mExportedDeclIds;

 public:
  explicit AST_XXport(AstOpt *o, unsigned f);
  ~AST_XXport() {}

  unsigned GetModuleNum();

  void BuildModuleOrder();

  void SetModuleStrIdx();
  void CollectXXportNodes();

  TreeNode *GetTarget(TreeNode *node);
  std::string GetTargetFilename(unsigned hidx, TreeNode *node);
  void UpdateDependency(unsigned hidx, TreeNode *node);
  void AddHandler();

  void SortHandler();

  void CollectXXportInfo();

  void AddHandlerIdx2DependentHandlerIdxMap(unsigned hdlIdx, unsigned depHdlIdx) {
    mHandlerIdx2DependentHandlerIdxMap[hdlIdx].insert(depHdlIdx);
  }

  unsigned GetHandleIdxFromStrIdx(unsigned stridx) { return mStrIdx2HandlerIdxMap[stridx]; }

  bool IsDefault(TreeNode *node) { return node->GetStrIdx() == gStringPool.GetStrIdx("default"); }

  bool IsImportExportDeclId(unsigned hidx, unsigned id) {
    return (mImportedDeclIds[hidx].find(id) != mImportedDeclIds[hidx].end() ||
            mExportedDeclIds[hidx].find(id) != mExportedDeclIds[hidx].end());
  }

  void AddImportedDeclIds(unsigned hidx, unsigned nid) {mImportedDeclIds[hidx].insert(nid);}
  void AddExportedDeclIds(unsigned hidx, unsigned nid) {mExportedDeclIds[hidx].insert(nid);}

  unsigned ExtractTargetStrIdx(TreeNode *node);
  unsigned GetModuleStrIdxFromIdStrIdx(unsigned stridx) {return mIdStrIdx2ModuleStrIdxMap[stridx];}
  void SetIdStrIdx2ModuleStrIdx(unsigned id, unsigned mod) {mIdStrIdx2ModuleStrIdxMap[id] = mod;}

  TreeNode *GetExportedDefault(unsigned hstridx);
  TreeNode *GetExportedNamedNode(unsigned hidx, unsigned stridx);

  void Dump();
};

class XXportBasicVisitor : public AstVisitor {
 private:
  AST_XXport     *mASTXXport;
  Module_Handler *mHandler;
  unsigned       mHandlerIdx;
  unsigned       mFlags;

 public:
  std::unordered_set<ModuleNode *> mImported;

 public:
  explicit XXportBasicVisitor(AST_XXport *xx, Module_Handler *h, unsigned i, unsigned f, bool base = false)
    : mASTXXport(xx), mHandler(h), mHandlerIdx(i), mFlags(f), AstVisitor((f & FLG_trace_1) && base) {}
  ~XXportBasicVisitor() = default;

  ImportNode *VisitImportNode(ImportNode *node);
  ExportNode *VisitExportNode(ExportNode *node);
};

}
#endif
