/*
* Copyright (C) [2020] Futurewei Technologies, Inc. All rights reverved.
*
* OpenArkFE is licensed under the Mulan PSL v1.
* You can use this software according to the terms and conditions of the Mulan PSL v1.
* You may obtain a copy of Mulan PSL v1 at:
*
*  http://license.coscl.org.cn/MulanPSL
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR
* FIT FOR A PARTICULAR PURPOSE.
* See the Mulan PSL v1 for more details.
*/
//////////////////////////////////////////////////////////////////////////////////////////////
// I decided to build AST tree after the parsing, instead of generating expr, stmt and etc.
//
// A module (compilation unit) could have multiple trees, depending on how many top level
// syntax constructs in it. Take Java for example, the top level constructs have Class, Interface.
// For C, it has only functions.
//
// AST is generated from the Appeal tree after SortOut. We walk through the appeal tree, and on
// each node, we apply the 'action' of the rule table which help build the tree node.
//////////////////////////////////////////////////////////////////////////////////////////////

#ifndef __AST_HEADER__
#define __AST_HEADER__

// A node in AST could be one of the following types.
//
// 1. Token
//    This is the leaf node in an AST. It could be a variable, a literal.
//
// 2. Operator
//    This is one of the operators defined in supported_operators.def
//    As you may know, operators, literals and variables (identifiers) are all preprocessed
//    to be a token. But we also define 'Token' as a NodeKind. So please keep in mind, we
//    catagorize operator to a dedicated NodeKind.
//
// 3. Construct
//    This is syntax construct, such as for(..) loop construct. This is language specific,
//    and most of these nodes are defined under each language, such as java/ directory.
//    However, I do define some popular construct in shared/ directory.
//
// 4. Function
//    A Function node have its arguments as children node. The return value is not counted.
//

#include "ast_mempool.h"
#include "container.h"

#include "supported.h"
#include "token.h"

enum NodeKind {
#undef  NODEKIND
#define NODEKIND(K) NK_##K,
#include "ast_nk.def"
  NK_Null,
};

// TreeNode has a VIRTUAL destructor. Why?
//
// Derived TreeNodes are allocated in a mempool and are referenced identically as TreeNode*,
// no matter what derived class they are. So when free the mempool, we call the destructor
// of each node through a TreeNode pointer. In this way, a virtual destructor of TreeNode
// if needed in order to invoke the derived class destructor.

class TreeNode {
protected:
  NodeKind  mKind;
  TreeNode *mParent;
  TreeNode *mLabel;   // label of a statement, or expression.
public:
  TreeNode() {mKind = NK_Null; mLabel = NULL;}
  virtual ~TreeNode() {}

#undef  NODEKIND
#define NODEKIND(K) bool Is##K() {return mKind == NK_##K;}
#include "ast_nk.def"

  bool IsScope() {return IsBlock();}

  void SetParent(TreeNode *p) {mParent = p;}
  void SetLabel (TreeNode *p) {mLabel = p;}
  TreeNode* GetParent() {return mParent;}
  TreeNode* GetLabel()  {return mLabel;}

  virtual const char* GetName() {return NULL;}
  virtual void Dump(unsigned){}
  void DumpIndentation(unsigned);
  void DumpLabel(unsigned);

  // Release the dynamically allocated memory by this tree node.
  virtual void Release(){}
};

//////////////////////////////////////////////////////////////////////////
//              Operator Nodes
//////////////////////////////////////////////////////////////////////////

enum OperatorProperty {
  Unary = 1,   // Unary operator, in front of symbol, like +a, -a
  Binary= 2,
  Ternary = 4,
  Pre = 8,     // PreCompute, compute before evaluate value, --, ++
  Post = 16,   // PostCompute,evaluate before compute,
  OperatorProperty_NA = 32
};

struct OperatorDesc {
  OprId             mOprId;
  OperatorProperty  mDesc;
};
extern OperatorDesc gOperatorDesc[OPR_NA];
extern unsigned GetOperatorProperty(OprId);

class UnaOperatorNode : public TreeNode {
private:
  bool      mIsPost;  // if it's a postfix operation?
  OprId     mOprId;
  TreeNode *mOpnd;
public:
  UnaOperatorNode(OprId id) : mOprId(id), mOpnd(NULL), mIsPost(false)
    {mKind = NK_UnaOperator;}
  UnaOperatorNode() : mOpnd(NULL), mIsPost(false) {mKind = NK_UnaOperator;}
  ~UnaOperatorNode() {}

  void SetIsPost(bool b)    {mIsPost = b;}
  void SetOpnd(TreeNode* t) {mOpnd = t;}
  void SetOprId(OprId o)    {mOprId = o;}

  bool      IsPost()  {return mIsPost;}
  TreeNode* GetOpnd() {return mOpnd;}
  OprId     GetOprId(){return mOprId;}

  void Dump(unsigned);
};

class BinOperatorNode : public TreeNode {
public:
  OprId     mOprId;
  TreeNode *mOpndA;
  TreeNode *mOpndB;
public:
  BinOperatorNode(OprId id) : mOprId(id) {mKind = NK_BinOperator;}
  BinOperatorNode() {mKind = NK_BinOperator;}
  ~BinOperatorNode() {}

  void Dump(unsigned);
};

class TerOperatorNode : public TreeNode {
public:
  OprId     mOprId;
  TreeNode *mOpndA;
  TreeNode *mOpndB;
  TreeNode *mOpndC;
public:
  TerOperatorNode(OprId id) : mOprId(id) {mKind = NK_TerOperator;}
  TerOperatorNode() {mKind = NK_TerOperator;}
  ~TerOperatorNode() {}

  void Dump(unsigned);
};

//////////////////////////////////////////////////////////////////////////
//                         Identifier Nodes
// Everything having a name will be treated as an Identifier node at the
// first place. There are some issues here.
// 1) Each appearance of identifier will be given a new IdentifierNode at
//    the first place. Later, when the scope tree is ready, we can reduce
//    the nodes by merging multiple nodes into one if they are actually
//    semantically equal. For example, a same variable appears in multiple
//    places in a function, and these multiple appearance will be same node.
// 2) The IdentifierNode at the declaration point will have mType. The other
//    appearance of the identifier node won't have mType. After merging, they
//    will point to the same node.
// 3) The name is pointing to the stringpool, which is residing in Lexer.
//    [TODO] we'll have standalone stringpool...
//
//
//                        Dimension of IdentifierNode
//
// For a simple case like, int a[], the dimension info could be attached to
// type info or identifier. I finally decided to put in the identifier since
// it's actually a feature of the object not the type. The type only tells
// what each element is. But anyway, it doesn't matter much in either way.
// A slight advantage puting dimension in identifier is we don't need so many
// types.
//////////////////////////////////////////////////////////////////////////

// mDimensions[N] tells the Nst dimension.
// mDimensions[N] = 0, means the length of that dimension is unspecified.
class DimensionNode : public TreeNode {
private:
  SmallVector<unsigned> mDimensions;
public:
  DimensionNode() {mKind = NK_Dimension;}
  ~DimensionNode(){Release();}

  unsigned GetDimsNum() {return mDimensions.GetNum();}
  unsigned GetNthDim(unsigned n) {return mDimensions.ValueAtIndex(n);} // 0 means unspecified.
  void     SetNthDim(unsigned n, unsigned i) {
    unsigned *addr = mDimensions.RefAtIndex(n);
    *addr = i;
  }
  unsigned AddDim(unsigned i = 0) {mDimensions.PushBack(i);}
  void     Merge(const TreeNode*);

  void Release() {mDimensions.Release();}
  void Dump();
};

// IdentifierNode is the most important node, and we usually call it symbol.
// It has most of the syntax information.
class IdentifierNode : public TreeNode {
private:
  SmallVector<AttrId> mAttrs;
  const char    *mName; // In the lexer's StringPool
  TreeNode      *mType; // PrimTypeNode, or IdentifierNode
  TreeNode      *mInit; // Init value
  DimensionNode *mDims;
public:
  IdentifierNode(const char *s) : mName(s), mType(NULL), mInit(NULL), mDims(NULL) {
    mKind = NK_Identifier; }
  IdentifierNode(const char *s, TreeNode *t) : mName(s), mType(t) {mKind = NK_Identifier;}
  ~IdentifierNode(){}

  const char* GetName() {return mName;}
  TreeNode*   GetType() {return mType;}
  TreeNode*   GetInit() {return mInit;}

  void SetType(TreeNode *t)      {mType = t;}
  void SetInit(TreeNode *t)      {mInit = t;}
  void SetDims(DimensionNode *t) {mDims = t;}

  unsigned GetDimsNum()          {return mDims->GetDimsNum();}
  bool     IsArray()             {return mDims && GetDimsNum() > 0;}
  unsigned AddDim(unsigned i = 0){mDims->AddDim(i);}           // 0 means unspecified
  unsigned GetNthNum(unsigned n) {return mDims->GetNthDim(n);} // 0 means unspecified.
  void     SetNthNum(unsigned n, unsigned i) {mDims->SetNthDim(n, i);}

  // Attributes related
  unsigned GetAttrsNum()           {return mAttrs.GetNum();}
  void     AddAttr(AttrId a)       {mAttrs.PushBack(a);}
  AttrId   AttrAtIndex(unsigned i) {return mAttrs.ValueAtIndex(i);}

  void Release() { if (mDims) mDims->Release();}
  void Dump(unsigned);
};

//////////////////////////////////////////////////////////////////////////
//                           VarList Node
// Why do we need a VarListNode? Often in the program we have multiple
// variables like parameters in function, or varable list in declaration.
//////////////////////////////////////////////////////////////////////////

class VarListNode : public TreeNode {
private:
  SmallVector<IdentifierNode*> mVars;
public:
  VarListNode() {mKind = NK_VarList;}
  ~VarListNode() {}

  unsigned GetNum() {return mVars.GetNum();}
  IdentifierNode* VarAtIndex(unsigned i) {return mVars.ValueAtIndex(i);}

  void AddVar(IdentifierNode *n);
  void Merge(TreeNode*);
  void Release() {mVars.Release();}
  void Dump(unsigned);
};

//////////////////////////////////////////////////////////////////////////
//                         Literal Nodes
//////////////////////////////////////////////////////////////////////////

class LiteralNode : public TreeNode {
public:
  LitData mData;
public:
  LiteralNode(LitData d) : mData(d) {mKind = NK_Literal;}
  ~LiteralNode(){}

  void Dump(unsigned);
};

//////////////////////////////////////////////////////////////////////////
//                         Exception Node
//////////////////////////////////////////////////////////////////////////

class ExceptionNode : public TreeNode {
private:
  // right now I only put a name for it. It could have more properties.
  IdentifierNode *mException;
public:
  ExceptionNode() : mException(NULL) {mKind = NK_Exception;}
  ExceptionNode(IdentifierNode *inode) : mException(inode) {mKind = NK_Exception;}
  ~ExceptionNode(){}

  IdentifierNode* GetException()       {return mException;}
  void SetException(IdentifierNode *n) {mException = n;}

  void Dump(unsigned);
};

//////////////////////////////////////////////////////////////////////////
//          Statement Node, Control Flow related nodes
//////////////////////////////////////////////////////////////////////////

class ReturnNode : public TreeNode {
private:
  TreeNode *mResult;
public:
  ReturnNode() : mResult(NULL) {mKind = NK_Return;}
  ~ReturnNode(){}

  void SetResult(TreeNode *t) {mResult = t;}
  TreeNode* GetResult() { return mResult; }
  void Dump(unsigned);
};

class CondBranchNode : public TreeNode {
private:
  TreeNode *mCond;
  TreeNode *mTrueBranch;
  TreeNode *mFalseBranch;
public:
  CondBranchNode();
  ~CondBranchNode(){}

  void SetCond(TreeNode *t)       {mCond = t;}
  void SetTrueBranch(TreeNode *t) {mTrueBranch = t;}
  void SetFalseBranch(TreeNode *t){mFalseBranch = t;}

  TreeNode* GetCond()        {return mCond;}
  TreeNode* GetTrueBranch()  {return mTrueBranch;}
  TreeNode* GetFalseBranch() {return mFalseBranch;}

  void Dump(unsigned);
};

// Break statement. Break targets could be one identifier or empty.
class BreakNode : public TreeNode {
private:
  TreeNode* mTarget;
public:
  BreakNode() {mKind = NK_Break; mTarget = NULL;}
  ~BreakNode(){}

  TreeNode* GetTarget()           {return mTarget;}
  void      SetTarget(TreeNode* t){mTarget = t;}
  void      Dump(unsigned);
};

class ForLoopNode : public TreeNode {
private:
  SmallVector<TreeNode *> mInit;
  TreeNode               *mCond;
  SmallVector<TreeNode *> mUpdate;
  TreeNode               *mBody;   // This could be a single statement, or a block node
public:
  ForLoopNode() {mCond = NULL; mBody = NULL; mKind = NK_ForLoop;}
  ~ForLoopNode() {Release();}

  void AddInit(TreeNode *t)   {mInit.PushBack(t);}
  void AddUpdate(TreeNode *t) {mUpdate.PushBack(t);}
  void SetCond(TreeNode *t)   {mCond = t;}
  void SetBody(TreeNode *t)   {mBody = t;}

  unsigned GetInitNum()       {return mInit.GetNum();}
  unsigned GetUpdateNum()     {return mUpdate.GetNum();}
  TreeNode* InitAtIndex(unsigned i)   {return mInit.ValueAtIndex(i);}
  TreeNode* UpdateAtIndex(unsigned i) {return mUpdate.ValueAtIndex(i);}
  TreeNode* GetCond() {return mCond;}
  TreeNode* GetBody() {return mBody;}

  void Release() {mInit.Release(); mUpdate.Release();}
  void Dump(unsigned);
};

class WhileLoopNode : public TreeNode {
private:
  TreeNode *mCond;
  TreeNode *mBody; // This could be a single statement, or a block node
public:
  WhileLoopNode() {mCond = NULL; mBody = NULL; mKind = NK_WhileLoop;}
  ~WhileLoopNode() {Release();}

  void SetCond(TreeNode *t) {mCond = t;}
  void SetBody(TreeNode *t) {mBody = t;}
  TreeNode* GetCond()       {return mCond;}
  TreeNode* GetBody()       {return mBody;}

  void Release(){}
  void Dump(unsigned);
};

class DoLoopNode : public TreeNode {
private:
  TreeNode *mCond;
  TreeNode *mBody; // This could be a single statement, or a block node
public:
  DoLoopNode() {mCond = NULL; mBody = NULL; mKind = NK_DoLoop;}
  ~DoLoopNode(){Release();}

  void SetCond(TreeNode *t) {mCond = t;}
  void SetBody(TreeNode *t) {mBody = t;}
  TreeNode* GetCond()       {return mCond;}
  TreeNode* GetBody()       {return mBody;}

  void Release(){}
  void Dump(unsigned);
};

// The switch-case statement is divided into three components,
// SwitchLabelNode  : The expression of each case. The reason it's called a label is
//                    it is written as
//                       case 123:
//                    which looks like a label.
// SwitchCaseNode   : SwitchLabelNode + the statements under this label
// SwitchNode       : The main node of all.

class SwitchLabelNode : public TreeNode {
private:
  bool      mIsDefault; // default lable
  TreeNode *mValue;     // the constant expression
public:
  SwitchLabelNode() : mIsDefault(false), mValue(NULL) {mKind = NK_SwitchLabel;}
  ~SwitchLabelNode(){}

  void SetIsDefault(bool b) {mIsDefault = b;}
  void SetValue(TreeNode *t){mValue = t;}
  bool IsDefault()     {return mIsDefault;}
  TreeNode* GetValue() {return mValue;}

  void Dump(unsigned);
};

// One thing to note is about the mStmts. When creating this node, we should
// dig into subtree and resolve all the PassNode and make sure every node
// in mStmts and mLabels has semanteme. PassNode has no semanteme.
class SwitchCaseNode : public TreeNode {
private:
  SmallVector<SwitchLabelNode*> mLabels;
  SmallVector<TreeNode*>    mStmts;

public:
  SwitchCaseNode() {mKind = NK_SwitchCase;}
  ~SwitchCaseNode() {Release();}

  unsigned  GetLabelsNum()            {return mLabels.GetNum();}
  TreeNode* GetLabelAtIndex(unsigned i) {return mLabels.ValueAtIndex(i);}
  void      AddLabel(TreeNode*);

  unsigned  GetStmtsNum()            {return mStmts.GetNum();}
  TreeNode* GetStmtAtIndex(unsigned i) {return mStmts.ValueAtIndex(i);}
  void      AddStmt(TreeNode*);

  void Release() {mStmts.Release(); mLabels.Release();}
  void Dump(unsigned);
};

class SwitchNode : public TreeNode {
private:
  TreeNode *mCond;
  SmallVector<SwitchCaseNode*> mCases;
public:
  SwitchNode() : mCond(NULL) {mKind = NK_Switch;}
  ~SwitchNode() {Release();}

  TreeNode* GetCond() {return mCond;}
  void SetCond(TreeNode *c) {mCond = c;}

  unsigned  GetCasesNum() {return mCases.GetNum();}
  TreeNode* GetCaseAtIndex(unsigned i) {return mCases.ValueAtIndex(i);}
  void      AddCase(TreeNode *c);

  void Release() {mCases.Release();}
  void Dump(unsigned);
};

//////////////////////////////////////////////////////////////////////////
//                         Block Nodes
// A block is common in all languages, and it serves different function in
// different context. Here is a few examples.
//   1) As function body, it could contain variable decl, statement, lambda,
//      inner function, inner class, instance initializer....
//   2) As a function block, it could contain variable declaration, statement
//      lambda, ...
//
//                       Instance Initializer
//
// Java instance initializer is a block node, and it's a shared part of all
// constructors. It's always executed. We simply let BlockNode describe
// instance initializer.
// C++ has similiar instance initializer, but its bound to a specific constructor.
// We may move those initializer into the constructor body.
//////////////////////////////////////////////////////////////////////////

class BlockNode : public TreeNode {
private:
  SmallVector<TreeNode*> mChildren;

  // Some blocks are instance initializer in languages like Java,
  // and they can have attributes, such as static instance initializer.
  bool                mIsInstInit; // Instance Initializer
  SmallVector<AttrId> mAttrs;

public:
  BlockNode(){mKind = NK_Block; mIsInstInit = false;}
  ~BlockNode() {Release();}

  // Instance Initializer and Attributes related
  bool IsInstInit()    {return mIsInstInit;}
  void SetIsInstInit() {mIsInstInit = true;}
  unsigned GetAttrsNum()           {return mAttrs.GetNum();}
  void     AddAttr(AttrId a)       {mAttrs.PushBack(a);}
  AttrId   AttrAtIndex(unsigned i) {return mAttrs.ValueAtIndex(i);}

  unsigned  GetChildrenNum()            {return mChildren.GetNum();}
  TreeNode* GetChildAtIndex(unsigned i) {return mChildren.ValueAtIndex(i);}
  void      AddChild(TreeNode *c)       {mChildren.PushBack(c);}

  void Release() {mChildren.Release();}
  void Dump(unsigned);
};

//////////////////////////////////////////////////////////////////////////
//                         Function Nodes
// Normal constructors are treated as function too, with mIsConstructor
// being true.
//////////////////////////////////////////////////////////////////////////

class ASTScope;

class FunctionNode : public TreeNode {
private:
  const char                 *mName;
  SmallVector<AttrId>         mAttrs;
  SmallVector<ExceptionNode*> mThrows; // exceptions it can throw
  TreeNode                   *mType;   // return type
  VarListNode                *mParams;
  BlockNode                  *mBody;
  DimensionNode              *mDims;
  bool                        mIsConstructor;

public:
  FunctionNode();
  ~FunctionNode() {Release();}

  BlockNode* GetBody() {return mBody;}
  void AddBody(BlockNode *b) {mBody = b;}
  void Construct();

  bool IsConstructor()    {return mIsConstructor;}
  void SetIsConstructor() {mIsConstructor = true;}

  // Attributes related
  unsigned GetAttrsNum()           {return mAttrs.GetNum();}
  void     AddAttr(AttrId a)       {mAttrs.PushBack(a);}
  AttrId   AttrAtIndex(unsigned i) {return mAttrs.ValueAtIndex(i);}

  // Exception/throw related
  unsigned       GetThrowNum()             {return mThrows.GetNum();}
  void           AddThrow(ExceptionNode *n){mThrows.PushBack(n);}
  ExceptionNode* ThrowAtIndex(unsigned i)  {return mThrows.ValueAtIndex(i);}

  const char* GetName()      {return mName;}
  void SetName(const char*s) {mName = s;}

  void SetType(TreeNode *t) {mType = t;}

  void SetDims(DimensionNode *t) {mDims = t;}
  unsigned GetDimsNum()          {return mDims->GetDimsNum();}
  bool     IsArray()             {return mDims && GetDimsNum() > 0;}
  unsigned AddDim(unsigned i = 0){mDims->AddDim(i);}           // 0 means unspecified
  unsigned GetNthNum(unsigned n) {return mDims->GetNthDim(n);} // 0 means unspecified.
  void     SetNthNum(unsigned n, unsigned i) {mDims->SetNthDim(n, i);}


  void Release() { mAttrs.Release();
                   if (mDims) mDims->Release();}
  void Dump(unsigned);
};

//////////////////////////////////////////////////////////////////////////
//                         Interface Nodes
//////////////////////////////////////////////////////////////////////////

class InterfaceNode : public TreeNode {
public:
  IdentifierNode *mName;
public:
  InterfaceNode() {mKind = NK_Interface;}
  ~InterfaceNode() {}

  void Dump();
};

//////////////////////////////////////////////////////////////////////////
//                         Class Nodes
//                             &
//                         ClassBody -->BlockNode
// In reality there is no such thing as ClassBody, since this 'body' will
// eventually become field and method of a class. However, during parsing
// the children are processed before parents, which means we could have
// all fields and members before the class is ready. So we come up with
// this ClassBody to temporarily hold these subtrees, and let the class
// to interpret it in the future. Once the class is done, this ClassBody
// is useless.
// In the real implementation, ClassBody is actually a BlockNode.
//////////////////////////////////////////////////////////////////////////

class ClassNode : public TreeNode {
private:
  const char                  *mName;
  SmallVector<ClassNode*>      mSuperClasses;
  SmallVector<InterfaceNode*>  mSuperInterfaces;
  SmallVector<AttrId>          mAttributes;
  BlockNode                   *mBody;

  SmallVector<IdentifierNode*> mMembers;
  SmallVector<FunctionNode*>   mMethods;
  SmallVector<FunctionNode*>   mConstructors;
  SmallVector<BlockNode*>      mInstInits;     // instance initializer
  SmallVector<ClassNode*>      mLocalClasses;
  SmallVector<InterfaceNode*>  mLocalInterfaces;

public:
  ClassNode(){mKind = NK_Class;}
  ~ClassNode() {Release();}

  void SetName(const char *n) {mName = n;}
  const char* GetName()       {return mName;}

  void AddSuperClass(ClassNode *n) {mSuperClasses.PushBack(n);}
  void AddSuperClass(InterfaceNode *n) {mSuperInterfaces.PushBack(n);}
  void AddAttribute(AttrId a) {mAttributes.PushBack(a);}
  void AddBody(BlockNode *b) {mBody = b;}

  void Construct();
  void Release();
  void Dump(unsigned);
};

//////////////////////////////////////////////////////////////////////////
//                  AnnotationTypeNode and AnnotationNode
//
// Annotation is quite common in modern languages, and we decided to support.
// Here we define two tree nodes, one for the type definition and one for
// the annotation usage.
//////////////////////////////////////////////////////////////////////////

class AnnotationTypeNode : public TreeNode {
private:
  IdentifierNode *mName;
public:
  void SetName(IdentifierNode *n) {mName = n;}
};

class AnnotationNode : public TreeNode {
private:
  AnnotationTypeNode *mType;
public:
};

//////////////////////////////////////////////////////////////////////////
//                         PassNode
// When Autogen creates rule tables, it could create some temp tables without
// any actions. The corresponding AppealNode will be given a PassNode to
// pass its subtrees to the parent.
//
// Also some rules in .spec don't have action at the beginning, and they
// will be given a PassNode too.
//////////////////////////////////////////////////////////////////////////

class PassNode : public TreeNode {
private:
  SmallVector<TreeNode*> mChildren;
public:
  PassNode() {mKind = NK_Pass;}
  ~PassNode() {Release();}

  unsigned  GetChildrenNum() {return mChildren.GetNum();}
  TreeNode* GetChild(unsigned idx) {return mChildren.ValueAtIndex(idx);}
  void SetChild(unsigned idx, TreeNode *t) {*(mChildren.RefAtIndex(idx)) = t;}

  void AddChild(TreeNode *c) {mChildren.PushBack(c);}
  void Release() {mChildren.Release();}
};

////////////////////////////////////////////////////////////////////////////
//                  The AST Tree
////////////////////////////////////////////////////////////////////////////

class AppealNode;
class ASTBuilder;

class ASTTree {
public:
  TreePool    mTreePool;
  TreeNode   *mRootNode;
  ASTBuilder *mBuilder;

public:
  ASTTree();
  ~ASTTree();

  TreeNode* NewTreeNode(AppealNode*, std::map<AppealNode*, TreeNode*> &);
  void      SetTraceBuild(bool b);

  TreeNode* BuildBinaryOperation(TreeNode *, TreeNode *, OprId);
  TreeNode* BuildPassNode();

  void Dump(unsigned);
};

#endif
