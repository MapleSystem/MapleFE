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

namespace maplefe {

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

class AnnotationNode;
class TreeNode {
protected:
  NodeKind  mKind;
  TreeNode *mParent;
  TreeNode *mLabel;   // label of a statement, or expression.
  const char *mName;
public:
  TreeNode() {mKind = NK_Null; mLabel = NULL; mParent = NULL; mName = NULL;}
  virtual ~TreeNode() {}

#undef  NODEKIND
#define NODEKIND(K) bool Is##K() const {return mKind == NK_##K;}
#include "ast_nk.def"

  bool IsScope() {return IsBlock() || IsClass() || IsFunction() || IsInterface();}
  bool TypeEquivalent(TreeNode*);

  NodeKind GetKind() {return mKind;}
  void SetParent(TreeNode *p) {mParent = p;}
  void SetLabel (TreeNode *p) {mLabel = p;}
  TreeNode* GetParent() {return mParent;}
  TreeNode* GetLabel()  {return mLabel;}

  virtual const char* GetName() {return mName;}
  virtual void SetName(const char *s) {mName = s;}
  virtual void ReplaceChild(TreeNode *oldchild, TreeNode *newchild){}
  virtual void AddAttr(AttrId) {}
  virtual void AddAnnotation(AnnotationNode *n){}
  virtual void Dump(unsigned){}

  void DumpIndentation(unsigned);
  void DumpLabel(unsigned);

  // Release the dynamically allocated memory by this tree node.
  virtual void Release(){}
};

//////////////////////////////////////////////////////////////////////////
//                     Package Node
//////////////////////////////////////////////////////////////////////////

class PackageNode : public TreeNode {
private:
  TreeNode *mPackage;
public:
  PackageNode(){mKind = NK_Package;}
  PackageNode(const char *s) {mKind = NK_Package; mName = s;}
  ~PackageNode() {}

  TreeNode* GetPackage() {return mPackage;}
  void SetPackage(TreeNode *t) {mPackage = t;}

  void SetName(const char *s) {mName = s;}
  void Dump(unsigned indent);
};

//////////////////////////////////////////////////////////////////////////
//                     Import Node
// Java import, c/c++ include, are the same scenarios. We just save the
// original string and let the language's verifier to check. We do borrow
// the idea from Java so as to separate type.vs.static and single.vs.ondemand.
// We also borrow the idea of system directory vs. local directory from c/c++.
//////////////////////////////////////////////////////////////////////////

enum ImportProperty {
  ImpNone = 0,
  ImpType = 1,        // Java like, import type
  ImpStatic = 1 << 1, // Java like, import static field.
                      // If we don't specify the data type of imported, it's
                      // everything. This is c/c++ style.
  ImpSingle = 1 << 2,
  ImpAll = 1 << 3,    // Java's OnDemand import. c/c++ style too.

  ImpLocal = 1 << 4,
  ImpSystem = 1 << 5
};

inline ImportProperty& operator|=(ImportProperty& t, ImportProperty p) {
    return t = static_cast<ImportProperty>(static_cast<unsigned>(t) | static_cast<unsigned>(p));
}

inline ImportProperty operator&(ImportProperty p, ImportProperty q) {
    return static_cast<ImportProperty>(static_cast<unsigned>(p) & static_cast<unsigned>(q));
}

class ImportNode : public TreeNode {
private:
  ImportProperty  mProperty;
  TreeNode       *mTarget;    // the imported target
public:
  ImportNode() {mName = NULL; mProperty = ImpNone; mKind = NK_Import;}
  ~ImportNode(){}

  void SetTarget(TreeNode *t) {mTarget = t;}
  TreeNode* GetTarget() {return mTarget;}

  void SetName(const char *s) {mName = s;}
  const char* GetName() {return mName;}

  void SetImportType()   {mProperty |= ImpType;}
  void SetImportStatic() {mProperty |= ImpStatic;}
  void SetImportSingle() {mProperty |= ImpSingle;}
  void SetImportAll()    {mProperty |= ImpAll;}
  void SetImportLocal()  {mProperty |= ImpLocal;}
  void SetImportSystem() {mProperty |= ImpSystem;}
  bool IsImportType()   {return mProperty & ImpType;}
  bool IsImportStatic() {return mProperty & ImpStatic;}
  bool IsImportSingle() {return mProperty & ImpSingle;}
  bool IsImportAll()    {return mProperty & ImpAll;}
  bool IsImportLocal()  {return mProperty & ImpLocal;}
  bool IsImportSystem() {return mProperty & ImpSystem;}

  void Dump(unsigned indent);
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
  OprId     mOprId;
  unsigned  mDesc;  // The type was defined as OperatorProperty, but clang complains
                    // in the .cpp where Unary|Binary is treated as 'int'. Clang
                    // doesn't allow 'int' converted to OperatorProperty
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
  void SetOpnd(TreeNode* t) {mOpnd = t; t->SetParent(this);}
  void SetOprId(OprId o)    {mOprId = o;}

  bool      IsPost()  {return mIsPost;}
  TreeNode* GetOpnd() {return mOpnd;}
  OprId     GetOprId(){return mOprId;}

  void ReplaceChild(TreeNode*, TreeNode*);

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

  void ReplaceChild(TreeNode*, TreeNode*);
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
//                      New and Delete operation
// Java and C++ have new operation, with different syntax and semantics.
// C++ also has delete operation. The tree nodes below contains most of
// the information many similar languages need. But the semantic
// verification will have to be done specifically by each language.
//////////////////////////////////////////////////////////////////////////

class BlockNode;
class NewNode : public TreeNode {
private:
  TreeNode  *mId;                  // A name could be like Outer.Inner
                                   // Hard to give a const char* as name.
                                   // So give it an id.
  SmallVector<TreeNode*> mArgs;    //
  BlockNode *mBody;                // When body is not empty, it's an
                                   // anonymous class.
public:
  NewNode() : mId(NULL), mBody(NULL) {mKind = NK_New;}
  ~NewNode() {mArgs.Release();}

  TreeNode* GetId()          {return mId;}
  void SetId(TreeNode *n)    {mId = n;}
  BlockNode* GetBody()       {return mBody;}
  void SetBody(BlockNode *n) {mBody = n;}

  unsigned  GetArgsNum()        {return mArgs.GetNum();}
  TreeNode* GetArg(unsigned i)  {return mArgs.ValueAtIndex(i);}
  void      AddArg(TreeNode *t) {mArgs.PushBack(t);}

  void ReplaceChild(TreeNode *oldone, TreeNode *newone);
  void Dump(unsigned);
};

class DeleteNode : public TreeNode {
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
  void AddDim(unsigned i = 0) {mDimensions.PushBack(i);}
  void Merge(const TreeNode*);

  void Release() {mDimensions.Release();}
  void Dump();
};

// IdentifierNode is the most important node, and we usually call it symbol.
// It has most of the syntax information.
class IdentifierNode : public TreeNode {
private:
  SmallVector<AttrId> mAttrs;
  TreeNode      *mType; // PrimTypeNode or UserTypeNode
  TreeNode      *mInit; // Init value
  DimensionNode *mDims;
public:
  IdentifierNode(const char *s) : mType(NULL), mInit(NULL), mDims(NULL){
    mKind = NK_Identifier;
    SetName(s); }
  IdentifierNode(const char *s, TreeNode *t) : mType(t), mInit(NULL), mDims(NULL) {
    mKind = NK_Identifier;
    SetName(s);}
  ~IdentifierNode(){}

  TreeNode*   GetType() {return mType;}
  TreeNode*   GetInit() {return mInit;}
  DimensionNode* GetDims() {return mDims;}

  void SetType(TreeNode *t)      {mType = t;}
  void SetInit(TreeNode *t)      {mInit = t; t->SetParent(this);}
  void SetDims(DimensionNode *t) {mDims = t;}

  unsigned GetDimsNum()          {return mDims->GetDimsNum();}
  bool     IsArray()             {return mDims && GetDimsNum() > 0;}
  unsigned AddDim(unsigned i = 0){mDims->AddDim(i);}           // 0 means unspecified
  unsigned GetNthNum(unsigned n) {return mDims->GetNthDim(n);} // 0 means unspecified.
  void     SetNthNum(unsigned n, unsigned i) {mDims->SetNthDim(n, i);}

  // Attributes related
  unsigned GetAttrsNum() const        {return mAttrs.GetNum();}
  void     AddAttr(AttrId a)          {mAttrs.PushBack(a);}
  AttrId   GetAttrAtIndex(unsigned i) {return mAttrs.ValueAtIndex(i);}

  void Release() { if (mDims) mDims->Release();}
  void Dump(unsigned);
};

//////////////////////////////////////////////////////////////////////////
//                       DeclNode
// A DeclNode defines one single variable or a VarList.
// The type info and init expr are both inside the IdentifierNode.
// DeclNode only tells this is a declaration.
//////////////////////////////////////////////////////////////////////////

// special property of Javascript
enum DeclProp {
  JS_Var,
  JS_Let,
  JS_Const,
  DP_NA
};

class DeclNode : public TreeNode {
private:
  TreeNode *mVar;
  DeclProp  mProp;
public:
  DeclNode() : mVar(NULL), mProp(DP_NA) {mKind = NK_Decl;}
  DeclNode(TreeNode *id) : mVar(id), mProp(DP_NA) {mKind = NK_Decl;}
  ~DeclNode(){}

  TreeNode* GetVar(){return mVar;}

  DeclProp GetProp() {return mProp;}
  void SetProp(DeclProp p) {mProp = p;}

  void Dump(unsigned);
};

//////////////////////////////////////////////////////////////////////////
//                         AnnotationNode
//
// Pragma or Annotation are language constructs to help (1) compiler
// (2) runtime (3) other tools, to better analyze or execute the program.
// It doesn't change the program behaviour, but may be improve static
// and/or dynamic analysis and/or performance.
//
// We have a dedicated AnnotationNode here. The program/annotation syntax
// could be complicated, eg. c99 introduce function call like pragma.
//
// The difference between Java annotation and C/c++ pragma is annotation
// is user defined and pragma is compiler/system defined. In another word
// Java annoation has unlimited number while pragmas are limited.
//////////////////////////////////////////////////////////////////////////

// AnnotationTypeNode defines a new Annotation
class AnnotationTypeNode : public TreeNode {
private:
  IdentifierNode *mId;
public:
  IdentifierNode* GetId() {return mId;}
  void SetId(IdentifierNode *n) {mId = n;}
  void Dump(unsigned);
};

// Annotation/Pragma is complicated, but everything starts from the
// name. So at construction, we will take in the name at first.
// Later we will initialize mType, mExpr based on the 'name' and other
// expression.
class AnnotationNode : public TreeNode {
private:
  IdentifierNode     *mId;
  AnnotationTypeNode *mType;
  TreeNode           *mExpr;
public:
  AnnotationNode() : mId(NULL), mType(NULL), mExpr(NULL) {
    mKind = NK_Annotation;}
  ~AnnotationNode(){}

  IdentifierNode* GetId() {return mId;}
  void SetId(IdentifierNode *n) {mId = n;}
  AnnotationTypeNode* GetType() {return mType;}
  void SetType(AnnotationTypeNode *n) {mType = n;}
  TreeNode* GetExpr() {return mExpr;}
  void SetExpr(TreeNode *n) {mExpr = n;}
};

//////////////////////////////////////////////////////////////////////////
//                           CastNode
// This node is for type casting node, implicit or explicit.
//////////////////////////////////////////////////////////////////////////

class CastNode : public TreeNode {
private:
  TreeNode *mDestType;
  TreeNode *mExpr;
public:
  CastNode() : mDestType(NULL), mExpr(NULL) {mKind = NK_Cast;}
  ~CastNode(){}

  TreeNode* GetDestType() {return mDestType;}
  void SetDestType(TreeNode *t) {mDestType = t;}

  TreeNode* GetExpr() {return mExpr;}
  void SetExpr(TreeNode *t) {mExpr = t;}

  const char* GetName();
  void Dump(unsigned);
};

//////////////////////////////////////////////////////////////////////////
//                           ParenthesisNode
// In many languages like Java, c, c++, an Cast Expression has common
// patten like (TypeExpr)ValueExpr. The type is represented by a parenthesis
// expression. However, as language designers writing the Rules of spec,
// the (TypeExpr) and ValueExpr could be matched by different rules, which
// are not recoganized as a cast expression. It needs some manipulation
// after the tree is built at the first time.
//
// We define a Parenthesis node to help ASTTree::Manipulate2Cast().
//////////////////////////////////////////////////////////////////////////

class ParenthesisNode : public TreeNode {
private:
  TreeNode *mExpr;
public:
  ParenthesisNode() : mExpr(NULL) {mKind = NK_Parenthesis;}
  ~ParenthesisNode(){}

  TreeNode* GetExpr() {return mExpr;}
  void SetExpr(TreeNode *t) {mExpr = t; t->SetParent(this);}

  void Dump(unsigned);
};

//////////////////////////////////////////////////////////////////////////
//                           FieldNode
// This is used for field reference. The field could be a member field or
// a member function.
//////////////////////////////////////////////////////////////////////////

class FieldNode : public TreeNode {
private:
  TreeNode       *mUpper; // The upper enclosing structure
  IdentifierNode *mField;
public:
  FieldNode() : TreeNode(), mField(NULL), mUpper(NULL) {mKind = NK_Field;}
  ~FieldNode(){}

  IdentifierNode* GetField() {return mField;}
  void SetField(IdentifierNode *f) {mField = f;}

  TreeNode *GetUpper()       {return mUpper;}
  void SetUpper(TreeNode *n) {
    TreeNode *up = n;
    while (up->IsParenthesis()) {
      ParenthesisNode *pn = (ParenthesisNode*)up;
      up = pn->GetExpr();
    }
    mUpper = up;
  }

  void Dump(unsigned);
};

//////////////////////////////////////////////////////////////////////////
//                           Struct Node
// This is first coming from C struct. Typescript 'interface' has the
// similar structure.
//////////////////////////////////////////////////////////////////////////

enum StructProp {
  SProp_CStruct,
  SProp_TSInterface,
  SProp_NA
};

class StructNode : public TreeNode {
private:
  StructProp      mProp;
  IdentifierNode *mName;
  SmallVector<IdentifierNode*> mFields;
public:
  StructNode() {mKind = NK_Struct; mName = NULL; mProp = SProp_NA;}
  StructNode(IdentifierNode *n) {mKind = NK_Struct; mName = n; mProp = SProp_NA;}
  ~StructNode() {Release();}

  void SetName(IdentifierNode *n) {mName = n;}
  void SetProp(StructProp p) {mProp = p;}

  unsigned        GetFieldsNum() {return mFields.GetNum();}
  IdentifierNode* GetField(unsigned i) {return mFields.ValueAtIndex(i);}
  void            AddField(IdentifierNode *n) {mFields.PushBack(n);}

  void Release() {mFields.Release();}
  void Dump(unsigned);
};

// We define StructLiteral for C/C++ struct literal, TS/JS object literal.
// It contains a list of duple <fieldname, value>
class FieldLiteralNode : public TreeNode{
public:
  IdentifierNode *mFieldName;
  TreeNode       *mLiteral;

  void SetFieldName(IdentifierNode *id) {mFieldName = id;}
  void SetLiteral(TreeNode *id) {mLiteral = id;}

  FieldLiteralNode() {mKind = NK_FieldLiteral;}
  ~FieldLiteralNode(){}
};

class StructLiteralNode : public TreeNode {
private:
  SmallVector<FieldLiteralNode*> mFields;
public:
  StructLiteralNode() {mKind = NK_StructLiteral;}
  ~StructLiteralNode(){Release();}

  unsigned          GetFieldsNum() {return mFields.GetNum();}
  FieldLiteralNode* GetField(unsigned i) {return mFields.ValueAtIndex(i);}
  void              AddField(FieldLiteralNode *d) {mFields.PushBack(d);}

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
//                           ExprList Node
// It's for list of expressions. This covers more than VarList.
// This is used mostly as arguments of call site.
//////////////////////////////////////////////////////////////////////////

class ExprListNode : public TreeNode {
private:
  SmallVector<TreeNode*> mExprs;
public:
  ExprListNode() {mKind = NK_ExprList;}
  ~ExprListNode() {}

  unsigned GetNum() {return mExprs.GetNum();}
  TreeNode* ExprAtIndex(unsigned i) {return mExprs.ValueAtIndex(i);}

  void AddExpr(TreeNode *n) {mExprs.PushBack(n);}
  void Merge(TreeNode*);
  void Release() {mExprs.Release();}
  void Dump(unsigned);
};

//////////////////////////////////////////////////////////////////////////
//                         Literal Nodes
//////////////////////////////////////////////////////////////////////////

class LiteralNode : public TreeNode {
private:
  LitData mData;
private:
  void InitName();

public:
  LiteralNode(LitData d) : mData(d) {
    mKind = NK_Literal;
  }
  ~LiteralNode(){}

  LitData GetData() {return mData;}
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

  void SetCond(TreeNode *t)       {mCond = t; t->SetParent(this);}
  void SetTrueBranch(TreeNode *t) {mTrueBranch = t; t->SetParent(this);}
  void SetFalseBranch(TreeNode *t){mFalseBranch = t; t->SetParent(this);}

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
  SmallVector<TreeNode *> mInits;
  TreeNode               *mCond;
  SmallVector<TreeNode *> mUpdates;
  TreeNode               *mBody;   // This is a block node
public:
  ForLoopNode() {mCond = NULL; mBody = NULL; mKind = NK_ForLoop;}
  ~ForLoopNode() {Release();}

  void AddInit(TreeNode *t)   {mInits.PushBack(t);}
  void AddUpdate(TreeNode *t) {mUpdates.PushBack(t);}
  void SetCond(TreeNode *t)   {mCond = t;}
  void SetBody(TreeNode *t)   {mBody = t;}

  unsigned GetInitsNum()     {return mInits.GetNum();}
  unsigned GetUpdatesNum()   {return mUpdates.GetNum();}
  TreeNode* GetInitAtIndex(unsigned i)   {return mInits.ValueAtIndex(i);}
  TreeNode* GetUpdateAtIndex(unsigned i) {return mUpdates.ValueAtIndex(i);}
  TreeNode* GetCond() {return mCond;}
  TreeNode* GetBody() {return mBody;}

  void Release() {mInits.Release(); mUpdates.Release();}
  void Dump(unsigned);
};

class WhileLoopNode : public TreeNode {
private:
  TreeNode *mCond;
  TreeNode *mBody; // This is a block node
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
  TreeNode *mBody; // This is a block node
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

// This is for an 'assert' statement in languages like Java.
// It contains two parts, one evaluation expression, the other the detailed message.
//
// In languages like C, assert is defined as a normal function, and don't go
// through this node.

class AssertNode : public TreeNode {
private:
  TreeNode  *mExpr;
  TreeNode  *mMsg;
public:
  AssertNode() : mExpr(NULL), mMsg(NULL) {mKind = NK_Assert;}
  ~AssertNode(){}

  TreeNode* GetExpr() {return mExpr;}
  TreeNode* GetMsg() {return mMsg;}
  void SetExpr(TreeNode *t) {mExpr = t;}
  void SetMsg(TreeNode *t)  {mMsg = t;}

  void Dump(unsigned);
};

// This is the node for a call site.
// 1. The method could be class method, a global method, or ...
// 2. The argument could be any expression even including another callsite.
// So I'm using TreeNode for all of them.
class CallNode : public TreeNode {
private:
  TreeNode    *mMethod;
  ExprListNode mArgs;
public:
  CallNode() : mMethod(NULL){mKind = NK_Call;}
  ~CallNode(){}

  void Init();

  TreeNode* GetMethod() {return mMethod;}
  void SetMethod(TreeNode *t) {mMethod = t;}

  void AddArg(TreeNode *t);
  unsigned GetArgsNum() {return mArgs.GetNum();}
  TreeNode* GetArg(unsigned index) {return mArgs.ExprAtIndex(index);}

  void Release() {mArgs.Release();}
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
public:
  SmallList<TreeNode*> mChildren;

  // Some blocks are instance initializer in languages like Java,
  // and they can have attributes, such as static instance initializer.
  bool                mIsInstInit; // Instance Initializer
  SmallVector<AttrId> mAttrs;

  const TreeNode     *mSync;       // Java allows a sync object on a Block.

public:
  BlockNode(){mKind = NK_Block; mIsInstInit = false; mSync = NULL;}
  ~BlockNode() {Release();}

  // Instance Initializer and Attributes related
  bool IsInstInit()    {return mIsInstInit;}
  void SetIsInstInit() {mIsInstInit = true;}
  unsigned GetAttrsNum()              {return mAttrs.GetNum();}
  void     AddAttr(AttrId a)          {mAttrs.PushBack(a);}
  AttrId   GetAttrAtIndex(unsigned i) {return mAttrs.ValueAtIndex(i);}

  void  SetSync(const TreeNode *n) {mSync = n;}
  const TreeNode* GetSync() {return mSync;}

  unsigned  GetChildrenNum()            {return mChildren.GetNum();}
  TreeNode* GetChildAtIndex(unsigned i) {return mChildren.ValueAtIndex(i);}
  void      AddChild(TreeNode *c)       {mChildren.PushBack(c); c->SetParent(this);}
  void      ClearChildren()             {mChildren.Clear();}

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
  SmallVector<AttrId>          mAttrs;
  SmallVector<AnnotationNode*> mAnnotations; //annotation or pragma
  SmallVector<ExceptionNode*>  mThrows;      // exceptions it can throw
  TreeNode                    *mType;        // return type
  SmallVector<TreeNode*>       mParams;      //
  BlockNode                   *mBody;
  DimensionNode               *mDims;
  bool                         mIsConstructor;

public:
  FunctionNode();
  ~FunctionNode() {Release();}

  // After function body is added, we need some clean up work, eg. cleaning
  // the PassNode in the tree.
  void CleanUp();

  BlockNode* GetBody() {return mBody;}
  void AddBody(BlockNode *b) {mBody = b; b->SetParent(this); CleanUp();}

  bool IsConstructor()    {return mIsConstructor;}
  void SetIsConstructor() {mIsConstructor = true;}

  unsigned  GetParamsNum()        {return mParams.GetNum();}
  TreeNode* GetParam(unsigned i)  {return mParams.ValueAtIndex(i);}
  void      AddParam(TreeNode *t) {mParams.PushBack(t); t->SetParent(this);}

  // Attributes related
  unsigned GetAttrsNum()              {return mAttrs.GetNum();}
  void     AddAttr(AttrId a)          {mAttrs.PushBack(a);}
  AttrId   GetAttrAtIndex(unsigned i) {return mAttrs.ValueAtIndex(i);}

  // Annotation/Pragma related
  unsigned GetAnnotationsNum()           {return mAnnotations.GetNum();}
  void     AddAnnotation(AnnotationNode *n) {mAnnotations.PushBack(n);}
  AnnotationNode* GetAnnotationAtIndex(unsigned i) {return mAnnotations.ValueAtIndex(i);}

  // Exception/throw related
  unsigned       GetThrowsNum()               {return mThrows.GetNum();}
  void           AddThrow(ExceptionNode *n)   {mThrows.PushBack(n);}
  ExceptionNode* GetThrowAtIndex(unsigned i)  {return mThrows.ValueAtIndex(i);}

  void SetType(TreeNode *t) {mType = t;}
  TreeNode* GetType(){return mType;}

  void SetDims(DimensionNode *t) {mDims = t;}
  unsigned GetDimsNum()          {return mDims->GetDimsNum();}
  bool     IsArray()             {return mDims && GetDimsNum() > 0;}
  unsigned AddDim(unsigned i = 0){mDims->AddDim(i);}           // 0 means unspecified
  unsigned GetNthNum(unsigned n) {return mDims->GetNthDim(n);} // 0 means unspecified.
  void     SetNthNum(unsigned n, unsigned i) {mDims->SetNthDim(n, i);}

  // Override equivalent.
  bool OverrideEquivalent(FunctionNode*);

  void Release() { mAttrs.Release();
                   mParams.Release();
                   mAnnotations.Release();
                   if (mDims) mDims->Release();}
  void Dump(unsigned);
};

//////////////////////////////////////////////////////////////////////////
//                         Interface Nodes
// This is from Java originally. Java Annotation is also treated as an
// InterfaceNode.
//////////////////////////////////////////////////////////////////////////

class InterfaceNode : public TreeNode {
private:
  bool        mIsAnnotation;
  SmallVector<InterfaceNode*>  mSuperInterfaces;
  SmallVector<IdentifierNode*> mFields;
  SmallVector<FunctionNode*>   mMethods;
public:
  InterfaceNode() : mIsAnnotation(false) {mKind = NK_Interface;}
  ~InterfaceNode() {}

  void SetIsAnnotation(bool b) {mIsAnnotation = b;}

  void Construct(BlockNode *);
  void Dump(unsigned);
};

//////////////////////////////////////////////////////////////////////////
//                         Class Nodes
//                             &
//                         ClassBody -->BlockNode
// In reality there is no such thing as ClassBody, since this 'body' will
// eventually become fields and methods of a class. However, during parsing
// the children are processed before parents, which means we need to have
// all fields and members ready before the class. So we come up with
// this ClassBody to temporarily hold these subtrees, and let the class
// to interpret it in the future. Once the class is done, this ClassBody
// is useless. In the real implementation, ClassBody is actually a BlockNode.
//
// NOTE. There is one important thing to know is This design is following
//       the common rules of Java/C++, where a declaration of field or
//       method has the scope of the whole class. This means it can be
//       used before the declaratioin. There is no order of first decl then
//       use. So we can do a Construct() which collect all Fields & Methods.
//
//       However, we still keep mBody, which actually tells the order of
//       the original program. Any language requiring order could use mBody
//       to implement.
//////////////////////////////////////////////////////////////////////////

class ClassNode : public TreeNode {
private:
  // Java Enum is defined in a similar way as class, with many restrictions
  // and special semantic rules. We define JavaEnum here too. For other
  // languages like C/C++ which have very simply Enum, we will have a
  // dedicated EnumNode for them.
  bool                         mJavaEnum;

  SmallVector<ClassNode*>      mSuperClasses;
  SmallVector<InterfaceNode*>  mSuperInterfaces;
  SmallVector<AttrId>          mAttributes;
  BlockNode                   *mBody;

  SmallVector<IdentifierNode*> mFields;       // aka Field
  SmallVector<FunctionNode*>   mMethods;
  SmallVector<FunctionNode*>   mConstructors;
  SmallVector<BlockNode*>      mInstInits;     // instance initializer
  SmallVector<ClassNode*>      mLocalClasses;
  SmallVector<InterfaceNode*>  mLocalInterfaces;

public:
  ClassNode(){mKind = NK_Class; mJavaEnum = false; mBody = NULL;}
  ~ClassNode() {Release();}

  bool IsJavaEnum() {return mJavaEnum;}
  void SetJavaEnum(){mJavaEnum = true;}

  void AddSuperClass(ClassNode *n)         {mSuperClasses.PushBack(n);}
  void AddSuperInterface(InterfaceNode *n) {mSuperInterfaces.PushBack(n);}
  void AddAttr(AttrId a) {mAttributes.PushBack(a);}
  void AddBody(BlockNode *b) {mBody = b; b->SetParent(this);}

  unsigned GetFieldsNum()      {return mFields.GetNum();}
  unsigned GetMethodsNum()     {return mMethods.GetNum();}
  unsigned GetConstructorNum() {return mConstructors.GetNum();}
  unsigned GetInstInitsNum()   {return mInstInits.GetNum();}
  unsigned GetLocalClassesNum()   {return mLocalClasses.GetNum();}
  unsigned GetLocalInterfacesNum(){return mLocalInterfaces.GetNum();}
  IdentifierNode* GetField(unsigned i)     {return mFields.ValueAtIndex(i);}
  FunctionNode* GetMethod(unsigned i)      {return mMethods.ValueAtIndex(i);}
  FunctionNode* GetConstructor(unsigned i) {return mConstructors.ValueAtIndex(i);}
  BlockNode* GetInstInit(unsigned i)       {return mInstInits.ValueAtIndex(i);}
  ClassNode* GetLocalClass(unsigned i)     {return mLocalClasses.ValueAtIndex(i);}
  InterfaceNode* GetLocalInterface(unsigned i)  {return mLocalInterfaces.ValueAtIndex(i);}
  BlockNode* GetBody() {return mBody;}

  void Construct();
  void Release();
  void Dump(unsigned);
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

  void AddChild(TreeNode *c) {mChildren.PushBack(c); c->SetParent(this);}
  void Dump(unsigned);
  void Release() {mChildren.Release();}
};

////////////////////////////////////////////////////////////////////////////
//                  Lambda Expression
////////////////////////////////////////////////////////////////////////////

class LambdaNode : public TreeNode {
private:
  SmallVector<IdentifierNode*> mParams;
  TreeNode *mBody;  // the body is block.
public:
  LambdaNode() {mBody = NULL; mKind = NK_Lambda;}
  ~LambdaNode(){Release();}

  void AddParam(IdentifierNode *n) {mParams.PushBack(n); n->SetParent(this);}
  TreeNode* Getbody() {return mBody;}
  void SetBody(TreeNode *n) {mBody = n; n->SetParent(this);}

  void Release() {mParams.Release();}
  void Dump(unsigned);
};

////////////////////////////////////////////////////////////////////////////
//                  InstanceOf Expression
// This is first created for Java's instanceof operation. It has the form of
// left instanceof right.
////////////////////////////////////////////////////////////////////////////

class InstanceOfNode : public TreeNode {
private:
  TreeNode *mLeft;
  TreeNode *mRight;
public:
  InstanceOfNode() {mLeft = NULL; mRight = NULL; mKind = NK_InstanceOf;}
  ~InstanceOfNode(){Release();}

  TreeNode* GetLeft() {return mLeft;}
  void SetLeft(TreeNode *n) {mLeft = n;}
  TreeNode* GetRight() {return mRight;}
  void SetRight(TreeNode *n){mRight = n;}

  void Dump(unsigned);
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

private:
  // We need a set of functions to deal with some common manipulations of
  // most languages during AST Building. You can disable it if some functions
  // are not what you want.
  TreeNode* Manipulate(AppealNode*);
  TreeNode* Manipulate2Binary(TreeNode*, TreeNode*);
  TreeNode* Manipulate2Cast(TreeNode*, TreeNode*);

public:
  ASTTree();
  ~ASTTree();

  TreeNode* NewTreeNode(AppealNode*);

  TreeNode* BuildBinaryOperation(TreeNode *, TreeNode *, OprId);
  TreeNode* BuildPassNode();

  void Dump(unsigned);

};

}
#endif
