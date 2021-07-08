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
  Module_Handler *mHandler;
  bool            mTrace;

 public:
  explicit TypeInfer(Module_Handler *h, bool t) : mHandler(h), mTrace(t) {}
  ~TypeInfer() {}

  void TypeInference();
};

class BuildIdNodeToDeclVisitor : public AstVisitor {
  Module_Handler *mHandler;
  bool            mTrace;

  public:
  explicit BuildIdNodeToDeclVisitor(Module_Handler *h, bool t, bool base = false)
    : mHandler(h), mTrace(t), AstVisitor(t && base) {}
  ~BuildIdNodeToDeclVisitor() = default;

  IdentifierNode *VisitIdentifierNode(IdentifierNode *node);
};

class TypeInferBaseVisitor : public AstVisitor {
 private:
  bool            mTrace;

 public:
  explicit TypeInferBaseVisitor(bool t, bool base = false)
    : mTrace(t), AstVisitor(t && base) {}
  ~TypeInferBaseVisitor() = default;

#undef  NODEKIND
#define NODEKIND(K) virtual K##Node *Visit##K##Node(K##Node *node) { \
  (void) AstVisitor::Visit##K##Node(node); \
  return node; \
}
#include "ast_nk.def"
};

class TypeInferVisitor : public TypeInferBaseVisitor {
 private:
  Module_Handler *mHandler;
  bool            mTrace;
  bool            mUpdated;

  std::unordered_set<unsigned> ExportedDeclIds;

 public:
  explicit TypeInferVisitor(Module_Handler *h, bool t, bool base = false)
    : mHandler(h), mTrace(t), TypeInferBaseVisitor(t, base) {}
  ~TypeInferVisitor() = default;

  bool GetUpdated() {return mUpdated;}
  void SetUpdated(bool b = true) {mUpdated = b;}

  void UpdateTypeId(TreeNode *node, TypeId tid);
  void UpdateFuncRetTypeId(FunctionNode *node, TypeId tid);
  void UpdateTypeUseNode(TreeNode *target, TreeNode *input);
  void UpdateArrayElemTypeIdMap(TreeNode *node, TypeId tid);
  TypeId GetArrayElemTypeId(TreeNode *node);

  TypeId MergeTypeId(TypeId tia, TypeId tib);

  bool IsArray(TreeNode *node);

  PrimTypeNode *GetOrClonePrimTypeNode(PrimTypeNode *node, TypeId tid);

  TreeNode *VisitClassField(TreeNode *node);

  ArrayElementNode *VisitArrayElementNode(ArrayElementNode *node);
  ArrayLiteralNode *VisitArrayLiteralNode(ArrayLiteralNode *node);
  BinOperatorNode *VisitBinOperatorNode(BinOperatorNode *node);
  CallNode *VisitCallNode(CallNode *node);
  ClassNode *VisitClassNode(ClassNode *node);
  DeclNode *VisitDeclNode(DeclNode *node);
  ExportNode *VisitExportNode(ExportNode *node);
  FieldLiteralNode *VisitFieldLiteralNode(FieldLiteralNode *node);
  FieldNode *VisitFieldNode(FieldNode *node);
  FunctionNode *VisitFunctionNode(FunctionNode *node);
  IdentifierNode *VisitIdentifierNode(IdentifierNode *node);
  InterfaceNode *VisitInterfaceNode(InterfaceNode *node);
  LambdaNode *VisitLambdaNode(LambdaNode *node);
  LiteralNode *VisitLiteralNode(LiteralNode *node);
  ReturnNode *VisitReturnNode(ReturnNode *node);
  StructLiteralNode *VisitStructLiteralNode(StructLiteralNode *node);
  TemplateLiteralNode *VisitTemplateLiteralNode(TemplateLiteralNode *node);
  TerOperatorNode *VisitTerOperatorNode(TerOperatorNode *node);
  TypeOfNode *VisitTypeOfNode(TypeOfNode *node);
  UnaOperatorNode *VisitUnaOperatorNode(UnaOperatorNode *node);
  UserTypeNode *VisitUserTypeNode(UserTypeNode *node);
};

class ShareUTVisitor : public AstVisitor {
 private:
  Module_Handler *mHandler;
  bool            mTrace;
  bool            mUpdated;

  std::stack<ASTScope *> mScopeStack;
  std::unordered_set<unsigned> ExportedDeclIds;

 public:
  explicit ShareUTVisitor(Module_Handler *h, bool t, bool base = false)
    : mHandler(h), mTrace(t), AstVisitor(t && base) {}
  ~ShareUTVisitor() = default;

  void Push(ASTScope *scope) { mScopeStack.push(scope); }
  void Pop() { mScopeStack.pop(); }

  UserTypeNode *VisitUserTypeNode(UserTypeNode *node);
};

}
#endif
