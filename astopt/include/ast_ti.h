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

#ifndef __AST_TYPE_INFERENCE_HEADER__
#define __AST_TYPE_INFERENCEHEADER__

#include <stack>
#include <utility>
#include "ast_module.h"
#include "ast.h"
#include "ast_type.h"
#include "gen_astvisitor.h"

namespace maplefe {

class Module_Handler;

class TypeInfer {
 private:
  Module_Handler  *mHandler;
  bool          mTrace;

 public:
  explicit TypeInfer(Module_Handler *h, bool t) : mHandler(h), mTrace(t) {}
  ~TypeInfer() {}

  void TypeInference();
};

class TypeInferVisitor : public AstVisitor {
 private:
  Module_Handler  *mHandler;
  bool          mTrace;
  bool          mUpdated;

  AstFunction   *mCurrentFunction;
  AstBasicBlock *mCurrentBB;

 public:
  explicit TypeInferVisitor(Module_Handler *h, bool t, bool base = false)
    : mHandler(h), mTrace(t), AstVisitor(t && base) {}
  ~TypeInferVisitor() = default;

  TypeId MergeTypeId(TypeId tia,  TypeId tib);

  DeclNode *VisitDeclNode(DeclNode *node);
  LiteralNode *VisitLiteralNode(LiteralNode *node);
  IdentifierNode *VisitIdentifierNode(IdentifierNode *node);
  BinOperatorNode *VisitBinOperatorNode(BinOperatorNode *node);
  UnaOperatorNode *VisitUnaOperatorNode(UnaOperatorNode *node);
  FunctionNode *VisitFunctionNode(FunctionNode *node);
  LambdaNode *VisitLambdaNode(LambdaNode *node);
  ClassNode *VisitClassNode(ClassNode *node);
  InterfaceNode *VisitInterfaceNode(InterfaceNode *node);
  ReturnNode *VisitReturnNode(ReturnNode *node);
};

}
#endif
