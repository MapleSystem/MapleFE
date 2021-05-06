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

#include <stack>
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
    BK_Yield,       // Yield BB eneded with a yield statement
    BK_Terminated,  // Return BB endded with a return/break/continue statement
    BK_Join         // BB at join point
  };

  enum BBAttribute {
    AK_Entry    = 1 << 0,
    AK_Exit     = 1 << 1,
    AK_Try      = 1 << 2,
    AK_Catch    = 1 << 3,
    AK_Throw    = 1 << 4,
    AK_Finally  = 1 << 5,
    AK_InLoop   = 1 << 6,
    AK_HasCall  = 1 << 7
  };

  using BBIndex = unsigned;

  class AST_BB {
    private:
      BBKind                   mKind;
      BBAttribute              mAttr;
      BBIndex                  mId;             // unique BB id
      TreeNode                *mPredicate;      // a predicate for true/false branches
      TreeNode                *mSwitchExpr;     // switch/case expression or nullptr
      SmallVector<TreeNode *>  mStatements;     // all statement nodes
      SmallList<AST_BB *>      mSuccessors;     // for BK_Branch: [0] true branch, [1] false branch
      SmallList<AST_BB *>      mPredecessors;

    public:
      explicit AST_BB() : mKind(BK_Unknown), mId(GetNextId()), mPredicate(nullptr) {}
      explicit AST_BB(BBKind k) : mKind(k), mId(GetNextId()), mPredicate(nullptr) {}
      ~AST_BB() {mStatements.Release(); mSuccessors.Release(); mPredecessors.Release();}

      void   SetKind(BBKind k) {mKind = k;}
      BBKind GetKind()         {return mKind;}

      void    SetAttr(BBAttribute a) {mAttr = a;}
      BBIndex GetAttr()              {return mAttr;}

      void    SetId(BBIndex id) {mId = id;}
      BBIndex GetId()           {return mId;}

      void      SetPredicate(TreeNode *node) {mPredicate = node;}
      TreeNode *GetPredicate()               {return mPredicate;}

      void      SetSwitchExpr(TreeNode *node) {mSwitchExpr = node;}
      TreeNode *GetSwitchExpr()               {return mSwitchExpr;}

      void AddStatement(TreeNode *stmt) {if(mKind != BK_Terminated) mStatements.PushBack(stmt);}

      unsigned  GetStatementsNum()              {return mStatements.GetNum();}
      TreeNode* GetStatementAtIndex(unsigned i) {return mStatements.ValueAtIndex(i);}

      void AddSuccessor(AST_BB *succ) {
        if(mKind == BK_Terminated)
          return;
        mSuccessors.PushBack(succ);
        succ->mPredecessors.PushBack(this);
      }
      unsigned  GetSuccessorsNum()                {return mSuccessors.GetNum();}
      AST_BB   *GetSuccessorAtIndex(unsigned i)   {return mSuccessors.ValueAtIndex(i);}
      unsigned  GetPredecessorsNum()              {return mPredecessors.GetNum();}
      AST_BB   *GetPredecessorAtIndex(unsigned i) {return mPredecessors.ValueAtIndex(i);}

      static BBIndex GetLastId() {return GetNextId(false);}

      void Dump();

    private:
      static BBIndex GetNextId(bool inc = true) {static BBIndex id = 0; return inc ? ++id : id; }
  };

  class AST_Function {
    private:
      FunctionNode              *mFunction;        // nullptr if it is an init function
      SmallList<AST_Function *>  mNestedFunctions; // nested functions
      AST_Function              *mParent;
      AST_BB                    *mEntryBB;
      AST_BB                    *mExitBB;
      BBIndex                    mLastBBId;

    public:
      explicit AST_Function() : mParent(nullptr), mEntryBB(nullptr), mExitBB(nullptr), mFunction(nullptr) {}
      ~AST_Function() {mNestedFunctions.Release();}

      void          SetFunction(FunctionNode *func) {mFunction = func;}
      FunctionNode *GetFunction()                   {return mFunction;}

      void          AddNestedFunction(AST_Function *func) {mNestedFunctions.PushBack(func);}
      unsigned      GetNestedFunctionsNum()               {return mNestedFunctions.GetNum();}
      AST_Function *GetNestedFunctionAtIndex(unsigned i)  {return mNestedFunctions.ValueAtIndex(i);}

      void          SetParent(AST_Function *func) {mParent = func;}
      AST_Function *GetParent()                   {return mParent;}

      void     SetEntryBB(AST_BB *bb) {mEntryBB = bb; bb->SetAttr(AK_Entry);}
      AST_BB  *GetEntryBB()           {return mEntryBB;}

      void     SetExitBB(AST_BB *bb)  {mExitBB = bb; bb->SetAttr(AK_Exit);}
      AST_BB  *GetExitBB()            {return mExitBB;}

      void    SetLastBBId(BBIndex id) {mLastBBId = id;}
      BBIndex GetLastBBId()           {return mLastBBId;}

      void Dump();
  };

  // Each TS source file is a module
  class AST_Module {
    private:
      MemPool       mMemPool;     // Memory pool for all AST_Function and AST_BB
      ASTModule    *mASTModule;   // for an AST module
      AST_Function *mFunction;    // an init function for statements in module scope
      bool          mTraceModule;

    public:
      explicit AST_Module(ASTModule *module, bool trace) :
        mASTModule(module),
        mFunction(nullptr),
        mTraceModule(trace) {}
      ~AST_Module() {mMemPool.Release();}

      void BuildCFG();

      ASTModule* GetASTModule() {return mASTModule;}

      void          SetFunction(AST_Function *func) {mFunction = func;}
      AST_Function *GetFunction()                   {return mFunction;}

      bool GetTraceModule() {return mTraceModule;}

      AST_Function *NewFunction()   {return new(mMemPool.Alloc(sizeof(AST_Function))) AST_Function;}
      AST_BB       *NewBB(BBKind k) {return new(mMemPool.Alloc(sizeof(AST_BB))) AST_BB(k);}

      void Dump(char *msg);
  };

  using TargetBBStack = std::stack<std::pair<AST_BB*,AST_BB*>>;
  class ModuleVisitor : public AstVisitor {
    private:
      AST_Module   *mModule;
      bool          mTrace;

      AST_Function *mCurrentFunction;
      AST_BB       *mCurrentBB;
      TargetBBStack mTargetBBs;

    public:
      explicit ModuleVisitor(AST_Module *m, bool t, bool base = false)
        : mModule(m), mTrace(t), AstVisitor(t && base) {}
      ~ModuleVisitor() = default;

      void InitializeFunction(AST_Function *func);
      void FinalizeFunction();

      FunctionNode *VisitFunctionNode(FunctionNode *node);
      ReturnNode *VisitReturnNode(ReturnNode *node);
      CondBranchNode *VisitCondBranchNode(CondBranchNode *node);
      ForLoopNode *VisitForLoopNode(ForLoopNode *node);
      WhileLoopNode *VisitWhileLoopNode(WhileLoopNode *node);
      DoLoopNode *VisitDoLoopNode(DoLoopNode *node);
      ContinueNode *VisitContinueNode(ContinueNode *node);
      BreakNode *VisitBreakNode(BreakNode *node);
      SwitchNode *VisitSwitchNode(SwitchNode *node);
      BlockNode *VisitBlockNode(BlockNode *node);

      DeclNode *VisitDeclNode(DeclNode *node);
      CallNode *VisitCallNode(CallNode *node);
  };
}
#endif
