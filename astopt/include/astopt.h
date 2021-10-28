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

#include <list>
#include <unordered_map>
#include <unordered_set>
#include "ast_module.h"
#include "ast.h"
#include "ast_type.h"
#include "ast_common.h"
#include "gen_astvisitor.h"

namespace maplefe {

class AST_Handler;
class Module_Handler;

class AstOpt {
private:
  AST_Handler *mASTHandler;
  unsigned     mFlags;

  std::list<unsigned> mHandlersIdxInOrder;
  std::unordered_map<unsigned, std::unordered_set<unsigned>> mHandlerIdx2DependentHandlerIdxMap;
  std::unordered_map<unsigned, unsigned> mStrIdx2HandlerIdxMap;

public:
  explicit AstOpt(AST_Handler *h, unsigned f);
  ~AstOpt() = default;

  AST_Handler *GetASTHandler() {return mASTHandler;}
  unsigned GetModuleNum();

  virtual void ProcessAST(unsigned trace);

  void SetModuleStrIdx();
  void AddHandler();
  void SortHandler();

  void BuildModuleOrder();
  void CollectInfo();
  void AdjustAST();
  void ScopeAnalysis();

  void BasicAnalysis();
  void BuildCFG();
  void ControlFlowAnalysis();
  void TypeInference();
  void DataFlowAnalysis();

  void AddHandlerIdx2DependentHandlerIdxMap(unsigned hdlIdx, unsigned depHdlIdx) {
    mHandlerIdx2DependentHandlerIdxMap[hdlIdx].insert(depHdlIdx);
  }

  unsigned GetHandleIdxFromStrIdx(unsigned stridx) {
    return mStrIdx2HandlerIdxMap[stridx];
  }
};

class XXportNodeVisitor : public AstVisitor {
 private:
  AstOpt         *mAstOpt;
  Module_Handler *mHandler;
  unsigned       mHandlerIdx;
  unsigned       mFlags;

 public:
  std::unordered_set<ModuleNode *> mImported;

 public:
  explicit XXportNodeVisitor(AstOpt *opt, Module_Handler *h, unsigned i, unsigned f, bool base = false)
    : mAstOpt(opt), mHandler(h), mHandlerIdx(i), mFlags(f), AstVisitor((f & FLG_trace_1) && base) {}
  ~XXportNodeVisitor() = default;

  std::string GetTargetFilename(TreeNode *node);

  ImportNode *VisitImportNode(ImportNode *node);
  ExportNode *VisitExportNode(ExportNode *node);
};

}
#endif
