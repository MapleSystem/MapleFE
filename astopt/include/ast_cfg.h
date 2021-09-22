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

#ifndef __AST_CFG_HEADER__
#define __AST_CFG_HEADER__

#include <utility>
#include "ast_module.h"
#include "ast.h"
#include "ast_type.h"
#include "gen_astvisitor.h"

namespace maplefe {

enum BBKind {
  BK_Unknown,     // Uninitialized
  BK_Uncond,      // BB for unconditional branch
  BK_Block,       // BB for a block/compound statement
  BK_Branch,      // BB ends up with a predicate for true/false branches
  BK_LoopHeader,  // BB for a loop header of a for, for/in, for/of, while, or do/while statement
  BK_Switch,      // BB for a switch statement
  BK_Case,        // BB for a case in switch statement
  BK_Try,         // BB for a try block
  BK_Catch,       // BB for a catch block
  BK_Finally,     // BB for a finally block
  BK_Yield,       // Yield BB eneded with a yield statement
  BK_Terminated,  // Return BB endded with a return/break/continue statement
  BK_Join,        // BB at join point for loops and switch
  BK_Join2,       // BB at join point for if-stmt and block
};

enum BBAttribute : unsigned {
  AK_None     = 0,
  AK_Entry    = 1 << 0,
  AK_Exit     = 1 << 1,
  AK_Break    = 1 << 2,
  AK_Return   = 1 << 3,
  AK_Throw    = 1 << 4,
  AK_Cont     = 1 << 5,
  AK_InLoop   = 1 << 6,
  AK_HasCall  = 1 << 7,
  AK_ALL      = 0xffffffff
};

inline BBAttribute operator|(BBAttribute x, BBAttribute y) {
  return static_cast<BBAttribute>(static_cast<unsigned>(x) | static_cast<unsigned>(y));
}
inline BBAttribute operator&(BBAttribute x, BBAttribute y) {
  return static_cast<BBAttribute>(static_cast<unsigned>(x) & static_cast<unsigned>(y));
}

using BBIndex = unsigned;
class Module_Handler;
class AST_AST;

class CfgBB {
 private:
  BBKind                 mKind;
  BBAttribute            mAttr;
  BBIndex                mId;             // unique BB id
  TreeNode              *mPredicate;      // a predicate for true/false branches
  TreeNode              *mAuxNode;        // the auxiliary node of current BB
  SmallList<TreeNode *>  mStatements;     // all statement nodes
  SmallList<CfgBB *>     mSuccessors;     // for BK_Branch: [0] true branch, [1] false branch
  SmallList<CfgBB *>     mPredecessors;

  friend class AST_AST;
  friend class AST_CFA;

 public:
  explicit CfgBB(BBKind k)
    : mKind(k), mAttr(AK_None), mId(GetNextId()), mPredicate(nullptr), mAuxNode(nullptr) {}
  ~CfgBB() {mStatements.Release(); mSuccessors.Release(); mPredecessors.Release();}

  void   SetKind(BBKind k) {mKind = k;}
  BBKind GetKind()         {return mKind;}

  void    SetAttr(BBAttribute a)  {mAttr = mAttr | a;}
  BBIndex GetAttr()               {return mAttr;}
  bool    TestAttr(BBAttribute a) {return mAttr & a;}

  void    SetId(BBIndex id) {mId = id;}
  BBIndex GetId()           {return mId;}

  void      SetPredicate(TreeNode *node) {mPredicate = node;}
  TreeNode *GetPredicate()               {return mPredicate;}

  void      SetAuxNode(TreeNode *node)  {mAuxNode = node;}
  TreeNode *GetAuxNode()                {return mAuxNode;}

  unsigned  GetStatementsNum()              {return mStatements.GetNum();}
  TreeNode* GetStatementAtIndex(unsigned i) {return mStatements.ValueAtIndex(i);}

  void AddStatement(TreeNode *stmt) {
    if(mKind != BK_Terminated) {
      mStatements.PushBack(stmt);
      stmt->SetIsStmt();
    }
  }

  void InsertStmtAfter(TreeNode *new_stmt, TreeNode *exist_stmt) {
    mStatements.LocateValue(exist_stmt);
    mStatements.InsertAfter(new_stmt);
  }

  void InsertStmtBefore(TreeNode *new_stmt, TreeNode *exist_stmt) {
    mStatements.LocateValue(exist_stmt);
    mStatements.InsertBefore(new_stmt);
  }

  void AddSuccessor(CfgBB *succ) {
    if(mKind == BK_Terminated) {
      return;
    }
    mSuccessors.PushBack(succ);
    succ->mPredecessors.PushBack(this);
  }
  unsigned  GetSuccessorsNum()                     {return mSuccessors.GetNum();}
  CfgBB *GetSuccessorAtIndex(unsigned i)   {return mSuccessors.ValueAtIndex(i);}
  unsigned  GetPredecessorsNum()                   {return mPredecessors.GetNum();}
  CfgBB *GetPredecessorAtIndex(unsigned i) {return mPredecessors.ValueAtIndex(i);}

  static BBIndex GetLastId() {return GetNextId(false);}

  void Dump();

 private:
  static BBIndex GetNextId(bool inc = true) {static BBIndex id = 0; return inc ? ++id : id; }
};

class CfgFunc {
 private:
  TreeNode             *mFuncNode;    // ModuleNode, FunctionNode or LambdaNode
  SmallList<CfgFunc *>  mNestedFuncs; // nested functions
  CfgFunc              *mParent;
  CfgBB                *mEntryBB;
  CfgBB                *mExitBB;
  BBIndex               mLastBBId;

 public:
  explicit CfgFunc() : mParent(nullptr), mEntryBB(nullptr), mExitBB(nullptr), mFuncNode(nullptr) {}
  ~CfgFunc() {mNestedFuncs.Release();}

  void      SetFuncNode(TreeNode *func) {mFuncNode = func;}
  TreeNode *GetFuncNode()               {return mFuncNode;}

  const char *GetName() {
    return mFuncNode->IsModule() ? "_init_" :
      (mFuncNode->GetStrIdx() ? mFuncNode->GetName() : "_anonymous_");
  }

  void     AddNestedFunc(CfgFunc *func)     {mNestedFuncs.PushBack(func); func->SetParent(this);}
  unsigned GetNestedFuncsNum()              {return mNestedFuncs.GetNum();}
  CfgFunc *GetNestedFuncAtIndex(unsigned i) {return mNestedFuncs.ValueAtIndex(i);}

  void     SetParent(CfgFunc *func) {mParent = func;}
  CfgFunc *GetParent()              {return mParent;}

  void   SetEntryBB(CfgBB *bb) {mEntryBB = bb; bb->SetAttr(AK_Entry);}
  CfgBB *GetEntryBB()          {return mEntryBB;}

  void   SetExitBB(CfgBB *bb)  {mExitBB = bb; bb->SetAttr(AK_Exit);}
  CfgBB *GetExitBB()           {return mExitBB;}

  void    SetLastBBId(BBIndex id) {mLastBBId = id;}
  BBIndex GetLastBBId()           {return mLastBBId;}

  void Dump();
};

class CfgBuilder : public AstVisitor {

  using TargetLabel = unsigned;
  using TargetBB = std::pair<CfgBB*, unsigned>;
  using TargetBBStack = std::vector<TargetBB>;

 private:
  Module_Handler *mHandler;
  unsigned        mFlags;

  CfgFunc   *mCurrentFunction;
  CfgBB *mCurrentBB;

  TargetBBStack  mBreakBBs;
  TargetBBStack  mContinueBBs;
  TargetBBStack  mThrowBBs;

 public:
  explicit CfgBuilder(Module_Handler *h, unsigned f)
    : mHandler(h), mFlags(f), AstVisitor(false) {}
  ~CfgBuilder() = default;

  void Build();

  // Create CfgFunc nodes for a module
  CfgFunc *InitCfgFunc(ModuleNode *module);

  CfgFunc *NewFunction(TreeNode *);
  CfgBB   *NewBB(BBKind k);

  static void Push(TargetBBStack &stack, CfgBB* bb, TreeNode *label);
  static CfgBB *LookUp(TargetBBStack &stack, TreeNode *label);
  static void Pop(TargetBBStack &stack);

  void InitializeFunction(CfgFunc *func);
  void FinalizeFunction();

  // For function and lambda
  FunctionNode *VisitFunctionNode(FunctionNode *node);
  LambdaNode *VisitLambdaNode(LambdaNode *node);

  // For class and interface
  ClassNode *VisitClassNode(ClassNode *node);
  InterfaceNode *VisitInterfaceNode(InterfaceNode *node);
  StructNode *VisitStructNode(StructNode *node);

  // For statements of control flow
  ReturnNode *VisitReturnNode(ReturnNode *node);
  CondBranchNode *VisitCondBranchNode(CondBranchNode *node);
  ForLoopNode *VisitForLoopNode(ForLoopNode *node);
  WhileLoopNode *VisitWhileLoopNode(WhileLoopNode *node);
  DoLoopNode *VisitDoLoopNode(DoLoopNode *node);
  ContinueNode *VisitContinueNode(ContinueNode *node);
  BreakNode *VisitBreakNode(BreakNode *node);
  SwitchNode *VisitSwitchNode(SwitchNode *node);
  TryNode *VisitTryNode(TryNode *node);
  ThrowNode *VisitThrowNode(ThrowNode *node);
  BlockNode *VisitBlockNode(BlockNode *node);
  NamespaceNode *VisitNamespaceNode(NamespaceNode *node);

  // For statements of a BB
  PassNode *VisitPassNode(PassNode *node);
  TemplateLiteralNode *VisitTemplateLiteralNode(TemplateLiteralNode *node);
  ImportNode *VisitImportNode(ImportNode *node);
  ExportNode *VisitExportNode(ExportNode *node);
  DeclNode *VisitDeclNode(DeclNode *node);
  ParenthesisNode *VisitParenthesisNode(ParenthesisNode *node);
  CastNode *VisitCastNode(CastNode *node);
  ArrayElementNode *VisitArrayElementNode(ArrayElementNode *node);
  VarListNode *VisitVarListNode(VarListNode *node);
  ExprListNode *VisitExprListNode(ExprListNode *node);
  UnaOperatorNode *VisitUnaOperatorNode(UnaOperatorNode *node);
  BinOperatorNode *VisitBinOperatorNode(BinOperatorNode *node);
  TerOperatorNode *VisitTerOperatorNode(TerOperatorNode *node);
  InstanceOfNode *VisitInstanceOfNode(InstanceOfNode *node);
  TypeOfNode *VisitTypeOfNode(TypeOfNode *node);
  NewNode *VisitNewNode(NewNode *node);
  DeleteNode *VisitDeleteNode(DeleteNode *node);
  CallNode *VisitCallNode(CallNode *node);
  AssertNode *VisitAssertNode(AssertNode *node);
  UserTypeNode *VisitUserTypeNode(UserTypeNode *node);
  IdentifierNode *VisitIdentifierNode(IdentifierNode *node);
  LiteralNode *VisitLiteralNode(LiteralNode *node);
  TypeAliasNode *VisitTypeAliasNode(TypeAliasNode *node);
  FieldNode *VisitFieldNode(FieldNode *node);

  TreeNode *BaseTreeNode(TreeNode *node);
};
}
#endif
