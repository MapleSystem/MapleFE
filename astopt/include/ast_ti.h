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
  unsigned        mFlags;

 public:
  explicit TypeInfer(Module_Handler *h, unsigned f) : mHandler(h), mFlags(f) {}
  ~TypeInfer() {}

  void TypeInference();
  void CheckType();
};

class BuildIdNodeToDeclVisitor : public AstVisitor {
  Module_Handler *mHandler;
  unsigned        mFlags;

  public:
  explicit BuildIdNodeToDeclVisitor(Module_Handler *h, unsigned f, bool base = false)
    : mHandler(h), mFlags(f), AstVisitor((f & FLG_trace_1) && base) {}
  ~BuildIdNodeToDeclVisitor() = default;

  IdentifierNode *VisitIdentifierNode(IdentifierNode *node);
};

class TypeInferBaseVisitor : public AstVisitor {
 private:
  unsigned mFlags;

 public:
  explicit TypeInferBaseVisitor(unsigned f, bool base = false)
    : mFlags(f), AstVisitor((f & FLG_trace_1) && base) {}
  ~TypeInferBaseVisitor() = default;

#undef  NODEKIND
#define NODEKIND(K) virtual K##Node *Visit##K##Node(K##Node *node) { \
  (void) AstVisitor::Visit##K##Node(node); \
  return node; \
}
#include "ast_nk.def"
};

class ChangeTypeIdxVisitor : public AstVisitor {
 private:
  Module_Handler *mHandler;
  unsigned        mFlags;
  unsigned        mStrIdx;
  unsigned        mTypeIdx;
  bool            mUpdated;

 public:
  explicit ChangeTypeIdxVisitor(Module_Handler *h, unsigned f, bool base = false)
    : mHandler(h), mFlags(f), AstVisitor((f & FLG_trace_1) && base) {}
  ~ChangeTypeIdxVisitor() = default;

  void Setup(unsigned stridx, unsigned tidx) { mStrIdx = stridx; mTypeIdx = tidx;}

  IdentifierNode *VisitIdentifierNode(IdentifierNode *node);
};

class TypeInferVisitor : public TypeInferBaseVisitor {
 private:
  Module_Handler *mHandler;
  unsigned        mFlags;
  bool            mUpdated;
  AST_INFO       *mInfo;

  ChangeTypeIdxVisitor *mChangeTypeIdxVisitor;

  // node ids
  std::unordered_set<unsigned> ImportedDeclIds;
  std::unordered_set<unsigned> ExportedDeclIds;

  std::unordered_map<unsigned, std::unordered_set<TreeNode *>> mParam2ArgArrayDeclMap;;

  // func nodeid to typeidx
  std::unordered_map<unsigned, unsigned> mFuncIsNodeMap;;
  std::unordered_map<unsigned, std::unordered_set<unsigned>> mCbFuncIsDone;

 public:
  explicit TypeInferVisitor(Module_Handler *h, unsigned f, bool base = false)
    : mHandler(h), mFlags(f), mInfo(h->GetINFO()), TypeInferBaseVisitor(f, base) {
      mChangeTypeIdxVisitor = new ChangeTypeIdxVisitor(h, f, true);
    }

  ~TypeInferVisitor() = default;

  bool IsPrimTypeId(TypeId tid);

  bool GetUpdated() {return mUpdated;}
  void SetUpdated(bool b = true) {mUpdated = b;}

  void SetTypeId(TreeNode *node, TypeId tid);
  void UpdateTypeId(TreeNode *node, TypeId tid);
  void UpdateTypeId(TreeNode *node1, TreeNode *node2);
  void UpdateFuncRetTypeId(FunctionNode *node, TypeId tid);
  void UpdateTypeUseNode(TreeNode *target, TreeNode *input);
  void UpdateArgArrayDecls(unsigned nid, TypeId tid);
  void UpdateArrayElemTypeIdMap(TreeNode *node, TypeId tid);
  bool UpdateVarTypeWithInit(TreeNode *var, TreeNode *init);
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
  CondBranchNode *VisitCondBranchNode(CondBranchNode *node);
  DeclNode *VisitDeclNode(DeclNode *node);
  ExportNode *VisitExportNode(ExportNode *node);
  FieldLiteralNode *VisitFieldLiteralNode(FieldLiteralNode *node);
  FieldNode *VisitFieldNode(FieldNode *node);
  ForLoopNode *VisitForLoopNode(ForLoopNode *node);
  FunctionNode *VisitFunctionNode(FunctionNode *node);
  IdentifierNode *VisitIdentifierNode(IdentifierNode *node);
  ImportNode *VisitImportNode(ImportNode *node);
  InterfaceNode *VisitInterfaceNode(InterfaceNode *node);
  IsNode *VisitIsNode(IsNode *node);
  LambdaNode *VisitLambdaNode(LambdaNode *node);
  LiteralNode *VisitLiteralNode(LiteralNode *node);
  NewNode *VisitNewNode(NewNode *node);
  ReturnNode *VisitReturnNode(ReturnNode *node);
  StructLiteralNode *VisitStructLiteralNode(StructLiteralNode *node);
  StructNode *VisitStructNode(StructNode *node);
  TemplateLiteralNode *VisitTemplateLiteralNode(TemplateLiteralNode *node);
  TerOperatorNode *VisitTerOperatorNode(TerOperatorNode *node);
  TypeAliasNode *VisitTypeAliasNode(TypeAliasNode *node);
  TypeOfNode *VisitTypeOfNode(TypeOfNode *node);
  UnaOperatorNode *VisitUnaOperatorNode(UnaOperatorNode *node);
  UserTypeNode *VisitUserTypeNode(UserTypeNode *node);
};

class ShareUTVisitor : public AstVisitor {
 private:
  Module_Handler *mHandler;
  unsigned        mFlags;
  bool            mUpdated;

  std::stack<ASTScope *> mScopeStack;

 public:
  explicit ShareUTVisitor(Module_Handler *h, unsigned f, bool base = false)
    : mHandler(h), mFlags(f), AstVisitor((f & FLG_trace_1) && base) {}
  ~ShareUTVisitor() = default;

  void Push(ASTScope *scope) { mScopeStack.push(scope); }
  void Pop() { mScopeStack.pop(); }

  UserTypeNode *VisitUserTypeNode(UserTypeNode *node);
};

class CheckTypeVisitor : public AstVisitor {
 private:
  Module_Handler *mHandler;
  unsigned        mFlags;
  bool            mUpdated;

  std::stack<ASTScope *> mScopeStack;

 public:
  explicit CheckTypeVisitor(Module_Handler *h, unsigned f, bool base = false)
    : mHandler(h), mFlags(f), AstVisitor((f & FLG_trace_1) && base) {}
  ~CheckTypeVisitor() = default;

  // check if typeid "tid" is compatible with "target"
  bool IsCompatible(TypeId tid, TypeId target);

  IdentifierNode *VisitIdentifierNode(IdentifierNode *node);
};

}
#endif
