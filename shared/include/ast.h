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
  NK_Identifier,
  NK_Dimension,
  NK_Attr,
  NK_PrimType,    // Primitive types. User types are indentifiers, functions, or etc.
                  // See ast_type.h for details.

  NK_VarList,     // A varialble list
  NK_Literal,
  NK_UnaOperator,
  NK_BinOperator,
  NK_TerOperator,
  NK_Construct,
  NK_Block,        // A block node
  NK_Function,
  NK_Class,
  NK_Interface,

  // These two are first coming from Java language. Annotation has no meaning
  // for execution, but has meanings for compiler or runtime.
  // AnnotationType : The definition of a type of annotation
  // Annotation     : The usage of an annotation type
  NK_AnnotationType,
  NK_Annotation,

  // Following are nodes to facilitate parsing.
  NK_Pass,         // see details in PassNode

  NK_Null,
};

// TreeNode has a VIRTUAL destructor. Why?
//
// Derived TreeNodes are allocated in a mempool and are referenced identically as TreeNode*,
// no matter what derived class they are. So when free the mempool, we call the destructor
// of each node through a TreeNode pointer. In this way, a virtual destructor of TreeNode
// if needed in order to invoke the derived class destructor.

class TreeNode {
public:
  NodeKind  mKind;
  TreeNode *mParent;
public:
  TreeNode() {mKind = NK_Null;}
  virtual ~TreeNode() {}

  bool IsIdentifier() {return mKind == NK_Identifier;}
  bool IsDimension()  {return mKind == NK_Dimension;}
  bool IsAttribute()  {return mKind == NK_Attr;}
  bool IsPrimType()       {return mKind == NK_PrimType;}
  bool IsVarList()    {return mKind == NK_VarList;}
  bool IsLiteral()    {return mKind == NK_Literal;}
  bool IsUnaOperator(){return mKind == NK_UnaOperator;}
  bool IsBinOperator(){return mKind == NK_BinOperator;}
  bool IsTerOperator(){return mKind == NK_TerOperator;}
  bool IsConstruct()  {return mKind == NK_Construct;}
  bool IsBlock()      {return mKind == NK_Block;}
  bool IsFunction()   {return mKind == NK_Function;}
  bool IsClass()      {return mKind == NK_Class;}
  bool IsInterface()  {return mKind == NK_Interface;}
  bool IsPass()       {return mKind == NK_Pass;}

  bool IsScope()      {return IsBlock() || IsFunction();}

  void SetParent(TreeNode *p) {mParent = p;}

  virtual const char* GetName() {return NULL;}
  virtual void Dump(unsigned){}
  void DumpIndentation(unsigned);

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

class UnaryOperatorNode : public TreeNode {
public:
  OprId     mOprId;
  TreeNode *mOpnd;
public:
  UnaryOperatorNode(OprId id) : mOprId(id) {mKind = NK_UnaOperator;}
  UnaryOperatorNode() {mKind = NK_UnaOperator;}
  ~UnaryOperatorNode() {}

  void Dump(unsigned);
};

class BinaryOperatorNode : public TreeNode {
public:
  OprId     mOprId;
  TreeNode *mOpndA;
  TreeNode *mOpndB;
public:
  BinaryOperatorNode(OprId id) : mOprId(id) {mKind = NK_BinOperator;}
  BinaryOperatorNode() {mKind = NK_BinOperator;}
  ~BinaryOperatorNode() {}

  void Dump(unsigned);
};

class TernaryOperatorNode : public TreeNode {
public:
  OprId     mOprId;
  TreeNode *mOpndA;
  TreeNode *mOpndB;
  TreeNode *mOpndC;
public:
  TernaryOperatorNode(OprId id) : mOprId(id) {mKind = NK_TerOperator;}
  TernaryOperatorNode() {mKind = NK_TerOperator;}
  ~TernaryOperatorNode() {}

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
public:
  const char    *mName; // In the lexer's StringPool
  TreeNode      *mType; // PrimTypeNode, or IdentifierNode
  TreeNode      *mInit; // Init value
  DimensionNode *mDims;
public:
  IdentifierNode(const char *s) : mName(s), mType(NULL), mInit(NULL), mDims(NULL) {
    mKind = NK_Identifier; }
  IdentifierNode(const char *s, TreeNode *t) : mName(s), mType(t) {mKind = NK_Identifier;}
  ~IdentifierNode(){}

  const char* GetName()     {return mName;}

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
//                         Block Nodes
// A block is common in all languages, and it serves different function in
// different context. Here is a few examples.
//   1) As function body, it could contain variable decl, statement, lambda,
//      inner function, inner class, ...
//   2) As a function block, it could contain variable declaration, statement
//      lambda, ...
//////////////////////////////////////////////////////////////////////////

class BlockNode : public TreeNode {
private:
  SmallVector<TreeNode*> mChildren;
public:
  BlockNode(){mKind = NK_Block;}
  ~BlockNode() {Release();}

  unsigned  GetChildrenNum()            {return mChildren.GetNum();}
  TreeNode* GetChildAtIndex(unsigned i) {return mChildren.ValueAtIndex(i);}

  void AddChild(TreeNode *c) {mChildren.PushBack(c);}
  void Release() {mChildren.Release();}
};

//////////////////////////////////////////////////////////////////////////
//                         Function Nodes
// Normal constructors are treated as function too, with mIsConstructor
// being true.
//////////////////////////////////////////////////////////////////////////

class ASTScope;

class FunctionNode : public TreeNode {
private:
  const char  *mName;
  SmallVector<AttrId> mAttrs;
  TreeNode    *mType;   // return type
  VarListNode *mParams;
  ASTScope    *mScope;
  BlockNode   *mBody;
  DimensionNode *mDims;
  bool         mIsConstructor;  // If it's a constructor of class.
public:
  FunctionNode();
  ~FunctionNode() {Release();}

  ASTScope* GetScope() {return mScope;}
  void      SetScope(ASTScope *s) {mScope = s;}

  void AddBody(BlockNode *b) {mBody = b;}
  void Construct();

  void SetIsConstructor() {mIsConstructor = true;}

  // Attributes related
  unsigned GetAttrsNum()           {return mAttrs.GetNum();}
  void     AddAttr(AttrId a)       {mAttrs.PushBack(a);}
  AttrId   AttrAtIndex(unsigned i) {return mAttrs.ValueAtIndex(i);}

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
  IdentifierNode           *mName;
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
