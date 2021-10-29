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
#include "ast_module.h"
#include "ast.h"
#include "gen_astvisitor.h"

namespace maplefe {

class AstOpt;
class AST_Handler;
class Module_Handler;

class AST_Xxport {
 private:
  AstOpt      *mAstOpt;
  AST_Handler *mASTHandler;
  unsigned        mFlags;

  std::list<unsigned> mHandlersIdxInOrder;
  std::unordered_map<unsigned, std::unordered_set<unsigned>> mHandlerIdx2DependentHandlerIdxMap;
  std::unordered_map<unsigned, unsigned> mStrIdx2HandlerIdxMap;

  std::unordered_set<ImportNode *> mImports;;
  std::unordered_set<ExportNode *> mExports;;

 public:
  explicit AST_Xxport(AstOpt *o, unsigned f);
  ~AST_Xxport() {}

  unsigned GetModuleNum();

  void BuildModuleOrder();

  void SetModuleStrIdx();
  void AddHandler();
  void SortHandler();

  void AddHandlerIdx2DependentHandlerIdxMap(unsigned hdlIdx, unsigned depHdlIdx) {
    mHandlerIdx2DependentHandlerIdxMap[hdlIdx].insert(depHdlIdx);
  }

  unsigned GetHandleIdxFromStrIdx(unsigned stridx) {
    return mStrIdx2HandlerIdxMap[stridx];
  }
};

class XXportNodeVisitor : public AstVisitor {
 private:
  AST_Xxport     *mASTXxport;
  Module_Handler *mHandler;
  unsigned       mHandlerIdx;
  unsigned       mFlags;

 public:
  std::unordered_set<ModuleNode *> mImported;

 public:
  explicit XXportNodeVisitor(AST_Xxport *xx, Module_Handler *h, unsigned i, unsigned f, bool base = false)
    : mASTXxport(xx), mHandler(h), mHandlerIdx(i), mFlags(f), AstVisitor((f & FLG_trace_1) && base) {}
  ~XXportNodeVisitor() = default;

  std::string GetTargetFilename(TreeNode *node);

  ImportNode *VisitImportNode(ImportNode *node);
  ExportNode *VisitExportNode(ExportNode *node);
};


}
#endif
