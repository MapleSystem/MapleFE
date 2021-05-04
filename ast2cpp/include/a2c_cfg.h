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

#ifndef __A2C_CFG_HEADER__
#define __A2C_CFG_HEADER__

#include "ast_module.h"
#include "ast.h"
#include "ast_type.h"
#include "gen_astvisitor.h"

namespace maplefe {

  enum BBKind {
    BK_Unknown,     // Uninitialized
    BK_Uncond,      // BB for unconditional branch
    BK_Block,       // BB for block/compound statement
    BK_Branch,      // BB ends up with a predicate for true/false branches
    BK_LoopHeader,  // BB for a loop header of for, for/in, for/of, while, do/while statements
    BK_Switch,      // BB for a switch statement
    BK_Yield,       // Yield BB eneded with a yield statement
    BK_Return       // Return BB endded with a return statement
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

  class A2C_BB {
    private:
      BBKind                   mKind;
      BBAttribute              mAttr;
      BBIndex                  mId;             // unique BB id
      TreeNode                *mPredicate;      // a predicate for true/false branches
      TreeNode                *mSwitchCaseExpr; // switch/case expression or nullptr
      SmallVector<TreeNode *>  mStatements;     // all statement nodes
      SmallList<A2C_BB *>      mSuccessors;     // for BK_Branch: [0] true branch, [1] false branch
      SmallList<A2C_BB *>      mPredecessors;

    public:
      explicit A2C_BB() : mKind(BK_Unknown), mId(GetNextId()), mPredicate(nullptr) {}
      ~A2C_BB() {mStatements.Release(); mSuccessors.Release(); mPredecessors.Release();}

      void   SetKind(BBKind k) {mKind = k;}
      BBKind GetKind()         {return mKind;}

      void    SetAttr(BBAttribute a) {mAttr = a;}
      BBIndex GetAttr()           {return mAttr;}

      void    SetId(BBIndex id) {mId = id;}
      BBIndex GetId()           {return mId;}

      void      SetPredicate(TreeNode *node) {mPredicate = node;}
      TreeNode *GetPredicate()               {return mPredicate;}

      void      SetSwitchCaseExpr(TreeNode *node) {mSwitchCaseExpr = node;}
      TreeNode *GetSwitchCaseExpr()               {return mSwitchCaseExpr;}

      void AddStatement(TreeNode *stmt)         {if(mKind != BK_Return) mStatements.PushBack(stmt);}
      unsigned  GetStatementsNum()              {return mStatements.GetNum();}
      TreeNode* GetStatementAtIndex(unsigned i) {return mStatements.ValueAtIndex(i);}

      void AddSuccessor(A2C_BB *succ) {
        if(mKind == BK_Return)
          return;
        mSuccessors.PushBack(succ);
        succ->mPredecessors.PushBack(this);
      }
      unsigned  GetSuccessorsNum()                {return mSuccessors.GetNum();}
      A2C_BB   *GetSuccessorAtIndex(unsigned i)   {return mSuccessors.ValueAtIndex(i);}
      unsigned  GetPredecessorsNum()              {return mPredecessors.GetNum();}
      A2C_BB   *GetPredecessorAtIndex(unsigned i) {return mPredecessors.ValueAtIndex(i);}

      void Dump();

    private:
      static unsigned GetNextId() {static unsigned id = 1; return id++; }
  };

  class A2C_Function {
    private:
      FunctionNode                *mFunction;        // nullptr if it is an init function
      SmallList<A2C_Function *>    mNestedFunctions; // nested functions
      A2C_Function                *mParent;
      A2C_BB                      *mEntryBB;
      A2C_BB                      *mExitBB;

    public:
      explicit A2C_Function() : mParent(nullptr), mEntryBB(nullptr), mExitBB(nullptr), mFunction(nullptr) {}
      ~A2C_Function() {mNestedFunctions.Release();}

      void          SetFunction(FunctionNode *func) {mFunction = func;}
      FunctionNode *GetFunction()                   {return mFunction;}

      void          AddNestedFunction(A2C_Function *func) {mNestedFunctions.PushBack(func);}
      unsigned      GetNestedFunctionsNum()               {return mNestedFunctions.GetNum();}
      A2C_Function *GetNestedFunctionAtIndex(unsigned i)  {return mNestedFunctions.ValueAtIndex(i);}

      void          SetParent(A2C_Function *func) {mParent = func;}
      A2C_Function *GetParent()                   {return mParent;}

      void          SetEntryBB(A2C_BB *bb) {mEntryBB = bb; bb->SetKind(BK_Uncond); bb->SetAttr(AK_Entry);}
      A2C_BB       *GetEntryBB()           {return mEntryBB;}

      void          SetExitBB(A2C_BB *bb)  {mExitBB = bb; bb->SetKind(BK_Uncond); bb->SetAttr(AK_Exit);}
      A2C_BB       *GetExitBB()            {return mExitBB;}

      void Dump();
  };

  // Each TS source file is a module
  class A2C_Module {
    private:
      MemPool       mMemPool;     // Memory pool for all A2C_Function and A2C_BB

      ASTModule    *mASTModule;      // for an AST module
      A2C_Function *mFunction;    // an init function for statements in module scope
      bool          mTraceModule;

    public:
      explicit A2C_Module(ASTModule *module, bool trace) :
        mASTModule(module),
        mFunction(nullptr),
        mTraceModule(trace) {}
      ~A2C_Module() {mMemPool.Release();}

      void BuildCFG();

      ASTModule* GetASTModule() {return mASTModule;}

      void          SetFunction(A2C_Function *func) {mFunction = func;}
      A2C_Function *GetFunction()                   {return mFunction;}

      bool GetTraceModule() {return mTraceModule;}

      A2C_Function *NewFunction() {return new(mMemPool.Alloc(sizeof(A2C_Function))) A2C_Function;}
      A2C_BB       *NewBB()       {return new(mMemPool.Alloc(sizeof(A2C_BB))) A2C_BB;}

      void Dump(char *msg);
  };

  class ModuleVisitor : public AstVisitor {
    private:
      A2C_Module   *mModule;
      bool          mTrace;

      A2C_Function *mCurrentFunction;
      A2C_BB       *mCurrentBB;

    public:

      explicit ModuleVisitor(A2C_Module *m, bool t, bool base = false) : mModule(m), mTrace(t), AstVisitor(t && base) {}
      ~ModuleVisitor() = default;

      void InitializeVisitor();
      void FinalizeVisitor();

      FunctionNode *VisitFunctionNode(FunctionNode *node);
      ReturnNode *VisitReturnNode(ReturnNode *node);
      CondBranchNode *VisitCondBranchNode(CondBranchNode *node);
      BlockNode *VisitBlockNode(BlockNode *node);
      ForLoopNode *VisitForLoopNode(ForLoopNode *node);

      DeclNode *VisitDeclNode(DeclNode *node);
      CallNode *VisitCallNode(CallNode *node);

  };

}
#endif
