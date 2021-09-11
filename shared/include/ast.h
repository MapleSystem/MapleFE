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

#include "stringpool.h"
#include "ast_mempool.h"
#include "container.h"

#include "supported.h"
#include "token.h"

namespace maplefe {

#define SETPARENT(n) if(n && n->GetKind() != NK_PrimType) n->SetParent(this)

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
class AsTypeNode;
class IdentifierNode;
class FunctionNode;
class UserTypeNode;
class ComputedNameNode;
class ASTScope;

class TreeNode {
protected:
  NodeKind  mKind;
  unsigned  mNodeId;
  TreeNode *mParent;
  TreeNode *mLabel;   // label of a statement, or expression.
  unsigned  mStrIdx;
  unsigned  mTypeIdx;     // typetable index
  TypeId    mTypeId;      // typeId of the node
  ASTScope *mScope;

  bool      mIsStmt;      // if a node is a statement
  bool      mIsOptional;  // if a node is optionally existing during runtime.
                          // This design is first coming from Javascript.
  bool      mIsNonNull;   // if a node is asserted to be non-null.
                          // This design is first coming from Typescript.
  bool      mIsRest;      // A spread or rest syntax in Javascript.
  bool      mIsConst;     // A constant node. Readonly.

  // This is a feature coming from TypeScript. Almost every expression in Typescript has
  // this information. So it's here.
  SmallVector<AsTypeNode*> mAsTypes;

public:
  TreeNode(NodeKind k, unsigned i)
    : mKind(k), mLabel(NULL), mParent(NULL), mStrIdx(i), mIsStmt(false), mTypeId(TY_None), mTypeIdx(0),
      mScope(NULL), mIsOptional(false), mIsNonNull(false), mIsRest(false), mIsConst(false) {}
  TreeNode(NodeKind k) : TreeNode(k, 0) {}
  //TreeNode() : TreeNode(NK_Null, 0) {}
  virtual ~TreeNode() {}

#undef  NODEKIND
#define NODEKIND(K) bool Is##K() const {return mKind == NK_##K;}
#include "ast_nk.def"

  bool IsScope() {return IsBlock() || IsClass() || IsFunction() || IsInterface() || IsModule();}
  bool TypeEquivalent(TreeNode*);

  void SetKind(NodeKind k) {} // Not allowed to change its kind
  void SetNodeId(unsigned id) {mNodeId = id;}
  void SetParent(TreeNode *p) {mParent = p;}
  void SetLabel (TreeNode *p) {mLabel = p; SETPARENT(p);}
  void SetTypeId(TypeId id)   {mTypeId = id;}
  void SetTypeIdx(unsigned id) {mTypeIdx = id;}
  void SetScope(ASTScope *s)   {mScope = s;}

  NodeKind GetKind()    {return mKind;}
  unsigned GetNodeId()  {return mNodeId;}
  TreeNode* GetParent() {return mParent;}
  TreeNode* GetLabel()  {return mLabel;}
  TypeId GetTypeId()    {return mTypeId;}
  unsigned GetTypeIdx() {return mTypeIdx;}
  ASTScope *GetScope()   {return mScope;}

  bool IsStmt()                    {return mIsStmt;}
  void SetIsStmt(bool b = true)    {mIsStmt = b;}
  bool IsOptional()                {return mIsOptional;}
  void SetIsOptional(bool b = true){mIsOptional = b;}
  bool IsNonNull()                 {return mIsNonNull;}
  void SetIsNonNull(bool b = true) {mIsNonNull = b;}
  bool IsRest()                    {return mIsRest;}
  void SetIsRest(bool b = true)    {mIsRest = b;}
  bool IsConst()                   {return mIsConst;}
  void SetIsConst(bool b = true)   {mIsConst = b;}

  virtual unsigned GetStrIdx() {return mStrIdx;}
  virtual void SetStrIdx(unsigned id) {mStrIdx = id;}
  virtual const char *GetName() {return gStringPool.GetStringFromStrIdx(mStrIdx);}
  virtual void SetStrIdx(std::string str) {mStrIdx = gStringPool.GetStrIdx(str);}
  virtual void ReplaceChild(TreeNode *oldchild, TreeNode *newchild){}
  virtual void AddAttr(AttrId) {}
  virtual void AddAnnotation(AnnotationNode *n){}

  // AsType related
  unsigned GetAsTypesNum()           {return mAsTypes.GetNum();}
  void     AddAsType(AsTypeNode *n)  {mAsTypes.PushBack(n);}
  void     AddAsTypes(TreeNode *n);
  AsTypeNode* GetAsTypeAtIndex(unsigned i) {return mAsTypes.ValueAtIndex(i);}
  void        SetAsTypeAtIndex(unsigned i, AsTypeNode* n) {*(mAsTypes.RefAtIndex(i)) = n;}

  virtual void Dump(unsigned){}
  virtual void D(){Dump(0); std::cout << std::endl;}
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
  PackageNode() : TreeNode(NK_Package) {}
  PackageNode(unsigned id) : TreeNode(NK_Package, id) {}
  ~PackageNode() {}

  TreeNode* GetPackage() {return mPackage;}
  void SetPackage(TreeNode *t) {mPackage = t;}

  void SetName(unsigned id) {mStrIdx = id;}
  void Dump(unsigned indent);
};

//////////////////////////////////////////////////////////////////////////
//                     XXportAsPair Node
// In JS, the import or export support: xxport {x as y}
// Its kind of like a mapping of internal name to extern name.
//
// Regarding 'default' and 'everything' (*), there are some rules of saving
// in the XXportAsPairNode. First, let's look at the variaties of combinations
// of default and *.
//
//   1) import * as x
//        mIsEverything = true
//        mAfter = x
//   2) import default as x
//   3) export x as default
//   4) export *
//        mIsEverything = true
//        mAfter = NULL, mBefore = NULL
//   5) export default declaration
//   6) export = x                 // This exports a single object.
//        mIsSingle = true
//        mBefore = x
//   7) import x = require("y")   // The counterpart of export = xxx
//        mIsSingle = true;
//        mBefore = y
//        mAfter = x
// For all these cases, we set mIsDefault or mIsEverything, and save the
// possible 'x' to mBefore.
//////////////////////////////////////////////////////////////////////////

class XXportAsPairNode : public TreeNode {
private:
  bool      mIsDefault;     // import or export 'default'
  bool      mIsEverything;  // import or export '*', which is everything
  bool      mIsSingle;      // export = xxx
  TreeNode *mBefore;        // In usual cases, name before 'as'
  TreeNode *mAfter;         // In usual cases, name after 'as'

public:
  XXportAsPairNode() : TreeNode(NK_XXportAsPair),
    mIsDefault(false), mIsEverything(false), mIsSingle(false), mBefore(NULL), mAfter(NULL) {}
  ~XXportAsPairNode() {}

  bool IsDefault()    {return mIsDefault;}
  void SetIsDefault(bool b = true) {mIsDefault = b;}

  bool IsEverything()    {return mIsEverything;}
  void SetIsEverything(bool b = true) {mIsEverything = b;}

  bool IsSingle()    {return mIsSingle;}
  void SetIsSingle(bool b = true) {mIsSingle = b;}

  TreeNode* GetBefore() {return mBefore;}
  void SetBefore(TreeNode *t) {mBefore = t; SETPARENT(t);}

  TreeNode* GetAfter() {return mAfter;}
  void SetAfter(TreeNode *t) {mAfter = t; SETPARENT(t);}

  void Dump(unsigned indent);
};

//////////////////////////////////////////////////////////////////////////
//                         Declare Nodes
// C/C++ extern decl,
// Typescript declare.
//////////////////////////////////////////////////////////////////////////

class DeclareNode : public TreeNode {
private:
  TreeNode *mDecl;    // the exported package in Java or module in JS
  SmallVector<AttrId> mAttrs;
public:
  DeclareNode() : TreeNode(NK_Declare), mDecl(NULL) {}
  ~DeclareNode(){mAttrs.Release();}

  void SetDecl(TreeNode *t) {mDecl = t;}
  TreeNode* GetDecl() {return mDecl;}

  // Attributes related
  unsigned GetAttrsNum() const        {return mAttrs.GetNum();}
  void     AddAttr(AttrId a)          {mAttrs.PushBack(a);}
  AttrId   GetAttrAtIndex(unsigned i) {return mAttrs.ValueAtIndex(i);}
  void     SetAttrAtIndex(unsigned i, AttrId n) {*(mAttrs.RefAtIndex(i)) = n;}

  void Dump(unsigned indent);
};

//////////////////////////////////////////////////////////////////////////
//                         Export Nodes
// export first comes from Javascript.
//
// If Export only exports a decl or a statement, it will be saved in
// mPairs as the only pair. This is the same in ImportNode.
//////////////////////////////////////////////////////////////////////////

class ExportNode : public TreeNode {
private:
  TreeNode       *mTarget;    // the exported package in Java or module in JS
  SmallVector<XXportAsPairNode*> mPairs;
  SmallVector<AnnotationNode*> mAnnotations; //annotation or pragma
public:
  ExportNode() : TreeNode(NK_Export), mTarget(NULL) {}
  ~ExportNode(){}

  void SetTarget(TreeNode *t) {mTarget = t; SETPARENT(t);}
  TreeNode* GetTarget() {return mTarget;}

  unsigned GetPairsNum() {return mPairs.GetNum();}
  XXportAsPairNode* GetPair(unsigned i) {return mPairs.ValueAtIndex(i);}
  void SetPair(unsigned i, XXportAsPairNode* n) {*(mPairs.RefAtIndex(i)) = n; SETPARENT(n);}
  void AddPair(TreeNode *p);
  void AddDefaultPair(TreeNode *p);
  void AddSinglePair(TreeNode *before, TreeNode *after);

  // Annotation/Pragma related
  unsigned GetAnnotationsNum()           {return mAnnotations.GetNum();}
  void     AddAnnotation(AnnotationNode *n) {mAnnotations.PushBack(n);}
  AnnotationNode* GetAnnotationAtIndex(unsigned i) {return mAnnotations.ValueAtIndex(i);}
  void            SetAnnotationAtIndex(unsigned i, AnnotationNode* n) {*(mAnnotations.RefAtIndex(i)) = n;}
  void ClearAnnotation() {mAnnotations.Clear();}

  void Dump(unsigned indent);
};


//////////////////////////////////////////////////////////////////////////
//                     Import Node
// Java import, c/c++ include, are the same scenarios. We just save the
// original string and let the language's verifier to check. We do borrow
// the idea from Java so as to separate type.vs.static and single.vs.ondemand.
// We also borrow the idea of system directory vs. local directory from c/c++.
//////////////////////////////////////////////////////////////////////////

// The property is useful in Java right now. Javascript use the XXportAsPair.
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
  // Solely for Java.
  ImportProperty  mProperty;  // This is solely for Java.

  // Solely for javascript right now.
  SmallVector<XXportAsPairNode*> mPairs;

  // the imported target, a package in Java, or a module in JS
  TreeNode       *mTarget;

public:
  ImportNode() : TreeNode(NK_Import), mProperty(ImpNone), mTarget(nullptr) {}
  ~ImportNode(){mPairs.Release();}

  void SetProperty(ImportProperty p) {mProperty = p;}
  ImportProperty GetProperty() {return mProperty;}
  void SetTarget(TreeNode *t) {mTarget = t;}
  TreeNode* GetTarget() {return mTarget;}

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

  unsigned GetPairsNum() {return mPairs.GetNum();}
  XXportAsPairNode* GetPair(unsigned i) {return mPairs.ValueAtIndex(i);}
  void SetPair(unsigned i, XXportAsPairNode* n) {*(mPairs.RefAtIndex(i)) = n;}
  void AddPair(TreeNode *p);
  void AddDefaultPair(TreeNode *p);
  void AddSinglePair(TreeNode *before, TreeNode *after);

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
  UnaOperatorNode(OprId id) : TreeNode(NK_UnaOperator),
    mOprId(id), mOpnd(NULL), mIsPost(false) {}
  UnaOperatorNode() : UnaOperatorNode(OPR_NA) {}
  ~UnaOperatorNode() {}

  void SetIsPost(bool b)    {mIsPost = b;}
  void SetOpnd(TreeNode* t) {mOpnd = t; SETPARENT(t);}
  void SetOprId(OprId o)    {mOprId = o;}

  bool      IsPost()  {return mIsPost;}
  TreeNode* GetOpnd() {return mOpnd;}
  OprId     GetOprId(){return mOprId;}

  void ReplaceChild(TreeNode*, TreeNode*);

  void Dump(unsigned);
};

class BinOperatorNode : public TreeNode {
private:
  OprId     mOprId;
  TreeNode *mOpndA;
  TreeNode *mOpndB;
public:
  BinOperatorNode(OprId id) : TreeNode(NK_BinOperator), mOprId(id) {}
  BinOperatorNode() : BinOperatorNode(OPR_NA) {}
  ~BinOperatorNode() {}

  OprId     GetOprId() {return mOprId;}
  TreeNode* GetOpndA() {return mOpndA;}
  TreeNode* GetOpndB() {return mOpndB;}
  void SetOprId(OprId o)     {mOprId = o;}
  void SetOpndA(TreeNode* t) {mOpndA = t; SETPARENT(t);}
  void SetOpndB(TreeNode* t) {mOpndB = t; SETPARENT(t);}

  void ReplaceChild(TreeNode*, TreeNode*);
  void Dump(unsigned);
};

// TerOperatorNode is for an expression like
//   a > b ? c : d
class TerOperatorNode : public TreeNode {
private:
  TreeNode *mOpndA;
  TreeNode *mOpndB;
  TreeNode *mOpndC;
public:
  TerOperatorNode() : TreeNode(NK_TerOperator) {}
  ~TerOperatorNode() {}

  TreeNode* GetOpndA() {return mOpndA;}
  TreeNode* GetOpndB() {return mOpndB;}
  TreeNode* GetOpndC() {return mOpndC;}
  void SetOpndA(TreeNode* t) {mOpndA = t; SETPARENT(t);}
  void SetOpndB(TreeNode* t) {mOpndB = t; SETPARENT(t);}
  void SetOpndC(TreeNode* t) {mOpndC = t; SETPARENT(t);}

  void Dump(unsigned);
};

//////////////////////////////////////////////////////////////////////////
//                      TypeAliasNode
// The syntax is type alias in Typescript, typedef in c/c++
//////////////////////////////////////////////////////////////////////////

class TypeAliasNode : public TreeNode {
private:
  UserTypeNode *mId;
  TreeNode     *mAlias;
public:
  TypeAliasNode() : TreeNode(NK_TypeAlias), mId(NULL), mAlias(NULL){}
  ~TypeAliasNode() {}

  UserTypeNode* GetId() {return mId;}
  void SetId(UserTypeNode *id);

  TreeNode* GetAlias() {return mAlias;}
  void SetAlias(TreeNode *n);

  void Dump(unsigned);
};

//////////////////////////////////////////////////////////////////////////
//                      ConditionalType
// The syntax is n Typescript,
//   type-a extends type-b ? type-c : type-d
//////////////////////////////////////////////////////////////////////////

class ConditionalTypeNode : public TreeNode {
private:
  TreeNode *mTypeA;
  TreeNode *mTypeB;
  TreeNode *mTypeC;
  TreeNode *mTypeD;
public:
  ConditionalTypeNode() : TreeNode(NK_ConditionalType),
                          mTypeA(NULL), mTypeB(NULL), mTypeC(NULL), mTypeD(NULL){}
  ~ConditionalTypeNode() {}

  TreeNode* GetTypeA() {return mTypeA;}
  TreeNode* GetTypeB() {return mTypeB;}
  TreeNode* GetTypeC() {return mTypeC;}
  TreeNode* GetTypeD() {return mTypeD;}
  void SetTypeA(TreeNode *n) {mTypeA = n; SETPARENT(n);}
  void SetTypeB(TreeNode *n) {mTypeB = n; SETPARENT(n);}
  void SetTypeC(TreeNode *n) {mTypeC = n; SETPARENT(n);}
  void SetTypeD(TreeNode *n) {mTypeD = n; SETPARENT(n);}

  void Dump(unsigned);
};

//////////////////////////////////////////////////////////////////////////
//                      AstType
// The syntax is like: variable as type-a as type-b ...
// It tells what type is the 'variable'
//////////////////////////////////////////////////////////////////////////

class AsTypeNode : public TreeNode {
private:
  TreeNode *mType;
public:
  AsTypeNode() : TreeNode(NK_AsType), mType(NULL) {}
  ~AsTypeNode() {}

  TreeNode* GetType() {return mType;}
  void SetType(TreeNode *t) {mType = t; SETPARENT(t);}

  void Dump(unsigned indent);
};

//////////////////////////////////////////////////////////////////////////
//                      TypeParameter
//////////////////////////////////////////////////////////////////////////

class TypeParameterNode : public TreeNode {
private:
  TreeNode *mId;      // The name of the type parameter
  TreeNode *mDefault; // The default value of this type parameter.
                      // some languages support default value.
  TreeNode *mExtends; // The constraint of this type parameter.
                         // In Typescript, the syntax is like: T<X extends Y>

public:
  TypeParameterNode() : TreeNode(NK_TypeParameter), mId(NULL), mDefault(NULL),
                        mExtends(NULL) {}
  ~TypeParameterNode() {}

  TreeNode* GetId()            {return mId;}
  void      SetId(TreeNode* t) {mId = t; SETPARENT(t);}

  TreeNode* GetDefault()            {return mDefault;}
  void      SetDefault(TreeNode* t) {mDefault = t; SETPARENT(t);}

  TreeNode* GetExtends()            {return mExtends;}
  void      SetExtends(TreeNode* t) {mExtends = t; SETPARENT(t);}

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
                                   // In Typescript, it could be a lambda:
                                   //   new (...) => Type
                                   // in which mArgs and mBody are not used.
  SmallVector<TreeNode*> mArgs;    //
  BlockNode *mBody;                // When body is not empty, it's an
                                   // anonymous class.
public:
  NewNode() : TreeNode(NK_New), mId(NULL), mBody(NULL) {}
  ~NewNode() {mArgs.Release();}

  TreeNode* GetId()          {return mId;}
  void SetId(TreeNode *n)    {mId = n; SETPARENT(n);}
  BlockNode* GetBody()       {return mBody;}
  void SetBody(BlockNode *n) {mBody = n;}

  unsigned  GetArgsNum()        {return mArgs.GetNum();}
  TreeNode* GetArg(unsigned i)  {return mArgs.ValueAtIndex(i);}
  void      SetArg(unsigned i, TreeNode* n) {*(mArgs.RefAtIndex(i)) = n; SETPARENT(n);}
  void      AddArg(TreeNode *t) {mArgs.PushBack(t); SETPARENT(t);}

  void ReplaceChild(TreeNode *oldone, TreeNode *newone);
  void Dump(unsigned);
};

class DeleteNode : public TreeNode {
private:
  TreeNode *mExpr;
public:
  DeleteNode() : TreeNode(NK_Delete), mExpr(NULL) {}
  ~DeleteNode(){}

  TreeNode* GetExpr()            {return mExpr;}
  void      SetExpr(TreeNode *t) {mExpr = t; SETPARENT(t);}

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
  AnnotationTypeNode() : TreeNode(NK_AnnotationType) {}
  ~AnnotationTypeNode() {}
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
  TreeNode           *mId;
  AnnotationTypeNode *mType;
  SmallVector<TreeNode*> mArgs;
public:
  AnnotationNode() : TreeNode(NK_Annotation), mId(NULL), mType(NULL) {}
  ~AnnotationNode(){mArgs.Release();}

  TreeNode* GetId()       {return mId;}
  void SetId(TreeNode *n) {mId = n; SETPARENT(n);}
  AnnotationTypeNode* GetType() {return mType;}
  void SetType(AnnotationTypeNode *n) {mType = n; SETPARENT(n);}

  unsigned  GetArgsNum()       {return mArgs.GetNum();}
  TreeNode* GetArgAtIndex(unsigned i) {return mArgs.ValueAtIndex(i);}
  void      SetArgAtIndex(unsigned i, TreeNode* n) {*(mArgs.RefAtIndex(i)) = n;}
  void      AddArg(TreeNode *n){mArgs.PushBack(n); SETPARENT(n);}
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
  DimensionNode() : TreeNode(NK_Dimension) {}
  ~DimensionNode(){Release();}

  unsigned GetDimensionsNum() {return mDimensions.GetNum();}
  unsigned GetDimension(unsigned i) {return mDimensions.ValueAtIndex(i);} // 0 means unspecified.
  void     SetDimension(unsigned i, unsigned n) {*(mDimensions.RefAtIndex(i)) = n;}
  void AddDimension(unsigned i = 0) {mDimensions.PushBack(i);}
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

  SmallVector<AnnotationNode*> mAnnotations; //annotation or pragma


  bool           mOptionalParam; // A optional parameter.
public:
  IdentifierNode(unsigned id, TreeNode *t) : TreeNode(NK_Identifier, id),
    mType(t), mInit(NULL), mDims(NULL),
    mOptionalParam(false) {}
  IdentifierNode(unsigned id) : IdentifierNode(id, NULL) {}
  IdentifierNode() : IdentifierNode(0, NULL) {}
  ~IdentifierNode(){Release();}

  TreeNode*   GetType() {return mType;}
  TreeNode*   GetInit() {return mInit;}
  DimensionNode* GetDims() {return mDims;}

  void SetType(TreeNode *t)      {mType = t; SETPARENT(t);}
  void SetInit(TreeNode *t)      {mInit = t; SETPARENT(t);}
  void ClearInit()               {mInit = NULL;}
  void SetDims(DimensionNode *t) {mDims = t; SETPARENT(t);}

  bool IsOptionalParam()       {return mOptionalParam;}
  bool GetOptionalParam()      {return mOptionalParam;}
  void SetOptionalParam(bool b){mOptionalParam = b;}

  unsigned GetDimsNum()          {return mDims->GetDimensionsNum();}
  unsigned GetDim(unsigned n)    {return mDims->GetDimension(n);} // 0 means unspecified.
  bool     IsArray()             {return mDims && GetDimsNum() > 0;}
  unsigned AddDim(unsigned i = 0){mDims->AddDimension(i);}        // 0 means unspecified
  unsigned GetNthNum(unsigned n) {return mDims->GetDimension(n);} // 0 means unspecified.
  void     SetNthNum(unsigned n, unsigned i) {mDims->SetDimension(n, i);}

  // Attributes related
  unsigned GetAttrsNum() const        {return mAttrs.GetNum();}
  void     AddAttr(AttrId a)          {mAttrs.PushBack(a);}
  AttrId   GetAttrAtIndex(unsigned i) {return mAttrs.ValueAtIndex(i);}
  void     SetAttrAtIndex(unsigned i, AttrId n) {*(mAttrs.RefAtIndex(i)) = n;}

  // Annotation/Pragma related
  unsigned GetAnnotationsNum()           {return mAnnotations.GetNum();}
  void     AddAnnotation(AnnotationNode *n) {mAnnotations.PushBack(n); SETPARENT(n);}
  AnnotationNode* GetAnnotationAtIndex(unsigned i) {return mAnnotations.ValueAtIndex(i);}
  void            SetAnnotationAtIndex(unsigned i, AnnotationNode* n) {*(mAnnotations.RefAtIndex(i)) = n;}

  void Release();
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
  TreeNode *mInit; // Init value
  DeclProp  mProp;
public:
  DeclNode(TreeNode *t) : TreeNode(NK_Decl),
    mVar(t), mInit(nullptr), mProp(DP_NA) {SETPARENT(t);}
  DeclNode() : DeclNode(NULL) {}
  ~DeclNode(){}

  TreeNode* GetVar() {return mVar;}
  TreeNode* GetInit() {return mInit;}

  void SetVar(TreeNode *t) {mVar = t; SETPARENT(t);}
  void SetInit(TreeNode *t) {mInit = t; SETPARENT(t);}

  DeclProp GetProp() {return mProp;}
  void SetProp(DeclProp p) {mProp = p;}

  void Dump(unsigned);
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
  CastNode() : TreeNode(NK_Cast), mDestType(NULL), mExpr(NULL) {}
  ~CastNode(){}

  TreeNode* GetDestType() {return mDestType;}
  void SetDestType(TreeNode *t) {mDestType = t;}

  TreeNode* GetExpr() {return mExpr;}
  void SetExpr(TreeNode *t) {mExpr = t; SETPARENT(t);}

  const char* GetDumpName();
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
  ParenthesisNode() : TreeNode(NK_Parenthesis), mExpr(NULL) {}
  ~ParenthesisNode(){}

  TreeNode* GetExpr() {return mExpr;}
  void SetExpr(TreeNode *t) {mExpr = t; SETPARENT(t);}

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
  FieldNode() : TreeNode(NK_Field), mField(NULL), mUpper(NULL) {}
  ~FieldNode(){}

  IdentifierNode* GetField() {return mField;}
  void SetField(IdentifierNode *f) {mField = f; SETPARENT(f);}

  TreeNode *GetUpper()       {return mUpper;}
  void SetUpper(TreeNode *n) {
    TreeNode *up = n;
    while (up->IsParenthesis()) {
      ParenthesisNode *pn = (ParenthesisNode*)up;
      up = pn->GetExpr();
    }
    mUpper = up;
    SETPARENT(up);
  }

  void Dump(unsigned);
};

//////////////////////////////////////////////////////////////////////////
//                           Array Related Nodes
// ArrayElementNode, ArrayLiteralNode.
//////////////////////////////////////////////////////////////////////////

// Array element is a[b][c].
class ArrayElementNode : public TreeNode {
private:
  TreeNode              *mArray;
  SmallVector<TreeNode*> mExprs;  // index expressions.
public:
  ArrayElementNode() : TreeNode(NK_ArrayElement), mArray(NULL) {}
  ~ArrayElementNode() {Release();}

  TreeNode* GetArray()            {return mArray;}
  void      SetArray(TreeNode *n) {mArray = n; SETPARENT(n);}

  unsigned  GetExprsNum()       {return mExprs.GetNum();}
  TreeNode* GetExprAtIndex(unsigned i) {return mExprs.ValueAtIndex(i);}
  void      SetExprAtIndex(unsigned i, TreeNode* n) {*(mExprs.RefAtIndex(i)) = n; SETPARENT(n);}
  void      AddExpr(TreeNode *n){mExprs.PushBack(n); SETPARENT(n);}

  void Release() {mExprs.Release();}
  void Dump(unsigned);
};


// Array literal is [1, 2 , 0, -4]. It's an arrya of literals.
// It could also be multi-dim array literal like [[1,2],[2,3]]
class ArrayLiteralNode : public TreeNode {
private:
  SmallVector<TreeNode*> mLiterals;
public:
  ArrayLiteralNode() : TreeNode(NK_ArrayLiteral) {}
  ~ArrayLiteralNode() {Release();}

  unsigned  GetLiteralsNum()       {return mLiterals.GetNum();}
  TreeNode* GetLiteral(unsigned i) {return mLiterals.ValueAtIndex(i);}
  void      SetLiteral(unsigned i, TreeNode* n) {*(mLiterals.RefAtIndex(i)) = n; SETPARENT(n);}
  void      AddLiteral(TreeNode *n){mLiterals.PushBack(n); SETPARENT(n);}

  void Release() {mLiterals.Release();}
  void Dump(unsigned);
};

//////////////////////////////////////////////////////////////////////////
//                           BindingPattern
// It's used in Destructuring scenarios. It comes from Javascript.
// It takes out elements from structs or arrays and form a new one.
// It may bind elements to some new variables. I believe this is the reason
// it's called BindingXXX
//////////////////////////////////////////////////////////////////////////

class BindingElementNode : public TreeNode {
private:
  TreeNode *mVariable;  // The new variable to bind element to
  TreeNode *mElement;   // the elements in the source struct or array
public:
  BindingElementNode() : TreeNode(NK_BindingElement),
                         mVariable(NULL), mElement(NULL) {}
  ~BindingElementNode() {}

  TreeNode* GetVariable()            {return mVariable;}
  void      SetVariable(TreeNode* n) {mVariable = n; SETPARENT(n);}
  TreeNode* GetElement()             {return mElement;}
  void      SetElement(TreeNode* n)  {mElement = n; SETPARENT(n);}

  void Dump(unsigned);
};

enum BindPattProp {
  BPP_ArrayBinding,
  BPP_ObjectBinding,
  BPP_NA
};

class BindingPatternNode : public TreeNode {
private:
  BindPattProp           mProp;
  SmallVector<TreeNode*> mElements;    // mostly BindingElementNode, also could be
                                       // a nested BindingPatternNode.
  TreeNode              *mType;        // The type
  TreeNode              *mInit;        // An initializer
public:
  BindingPatternNode() :
      TreeNode(NK_BindingPattern), mInit(NULL), mType(NULL), mProp(BPP_NA) {}
  ~BindingPatternNode() {Release();}

  BindPattProp GetProp()                {return mProp;}
  void         SetProp(BindPattProp n)  {mProp = n;}

  unsigned  GetElementsNum()       {return mElements.GetNum();}
  TreeNode* GetElement(unsigned i) {return mElements.ValueAtIndex(i);}
  void      SetElement(unsigned i, TreeNode* n) {*(mElements.RefAtIndex(i)) = n;}
  void      AddElement(TreeNode *n);

  TreeNode* GetType()             {return mType;}
  void      SetType(TreeNode* n)  {mType = n; SETPARENT(n);}
  TreeNode* GetInit()             {return mInit;}
  void      SetInit(TreeNode* n)  {mInit = n; SETPARENT(n);}

  void Release() {mElements.Release();}
  void Dump(unsigned);
};


//////////////////////////////////////////////////////////////////////////
//                           Struct Node
// This is first coming from C struct. Typescript 'interface' has the
// similar structure.
//
// Index signature of Typescript make it complicated. Here is an example.
//  interface Foo{
//    [key: string]: number;
//  }
//
//  let bar: Foo = {};
//  bar['key1'] = 1;
//
//////////////////////////////////////////////////////////////////////////

enum StructProp {
  SProp_CStruct,
  SProp_TSInterface,
  SProp_TSEnum,
  SProp_NA
};

class NumIndexSigNode : public TreeNode{
public:
  TreeNode *mKey;
  TreeNode *mDataType;

  void      SetKey(TreeNode *t) {mKey = t;}
  TreeNode* GetKey()            {return mKey;}
  void      SetDataType(TreeNode *t) {mDataType = t;}
  TreeNode* GetDataType()            {return mDataType;}

  NumIndexSigNode() : TreeNode(NK_NumIndexSig), mDataType(NULL), mKey(NULL) {}
  ~NumIndexSigNode(){}
  void Dump(unsigned);
};

class StrIndexSigNode : public TreeNode{
public:
  TreeNode *mKey;
  TreeNode *mDataType;

  void      SetKey(TreeNode *t) {mKey = t; SETPARENT(t);}
  TreeNode* GetKey()            {return mKey;}
  void      SetDataType(TreeNode *t) {mDataType = t; SETPARENT(t);}
  TreeNode* GetDataType()            {return mDataType;}

  StrIndexSigNode() : TreeNode(NK_StrIndexSig), mDataType(NULL), mKey(NULL) {}
  ~StrIndexSigNode(){}
  void Dump(unsigned);
};

// C++ struct or Typescript interface.
// The methods in Typescript interface has no function body.
class StructNode : public TreeNode {
private:
  StructProp      mProp;
  IdentifierNode *mStructId;
  SmallVector<TypeParameterNode*> mTypeParameters;
  SmallVector<TreeNode*>       mFields;
  SmallVector<FunctionNode*>   mMethods;
  SmallVector<TreeNode*>       mSupers;

  // These are for 'number' or 'string' index data type
  NumIndexSigNode *mNumIndexSig;
  StrIndexSigNode *mStrIndexSig;

public:
  StructNode(IdentifierNode *n) : TreeNode(NK_Struct), mStructId(n), mProp(SProp_NA),
                                  mNumIndexSig(NULL), mStrIndexSig(NULL) {SETPARENT(n);}
  StructNode() : StructNode(NULL) {}
  ~StructNode() {Release();}

  StructProp GetProp() {return mProp;}
  IdentifierNode* GetStructId() {return mStructId;}
  void SetProp(StructProp p) {mProp = p;}
  void SetStructId(IdentifierNode *n) {mStructId = n; SETPARENT(n);}

  NumIndexSigNode* GetNumIndexSig() {return mNumIndexSig;}
  StrIndexSigNode* GetStrIndexSig() {return mStrIndexSig;}
  void SetNumIndexSig(NumIndexSigNode *t) {mNumIndexSig = t;}
  void SetStrIndexSig(StrIndexSigNode *t) {mStrIndexSig = t;}

  // TypeParameter
  unsigned GetTypeParametersNum()           {return mTypeParameters.GetNum();}
  void     AddTypeParameter(TreeNode *n);
  TypeParameterNode* GetTypeParameterAtIndex(unsigned i) {return mTypeParameters.ValueAtIndex(i);}
  void               SetTypeParameterAtIndex(unsigned i, TypeParameterNode* n) {*(mTypeParameters.RefAtIndex(i)) = n;}

  unsigned  GetFieldsNum() {return mFields.GetNum();}
  TreeNode* GetField(unsigned i) {return mFields.ValueAtIndex(i);}
  void      SetField(unsigned i, TreeNode* n) {*(mFields.RefAtIndex(i)) = n; SETPARENT(n);}
  void      AddField(TreeNode *n) {mFields.PushBack(n); SETPARENT(n);}

  unsigned  GetSupersNum() {return mSupers.GetNum();}
  TreeNode* GetSuper(unsigned i) {return mSupers.ValueAtIndex(i);}
  void      SetSuper(unsigned i, TreeNode* n) {*(mSupers.RefAtIndex(i)) = n;}
  void      AddSuper(TreeNode *n);

  unsigned      GetMethodsNum() {return mMethods.GetNum();}
  FunctionNode* GetMethod(unsigned i) {return mMethods.ValueAtIndex(i);}
  void          SetMethod(unsigned i, FunctionNode* n) {*(mMethods.RefAtIndex(i)) = n;}
  void          AddMethod(FunctionNode *n) {mMethods.PushBack(n);}

  void AddChild(TreeNode *);

  void Release() {mFields.Release(); mMethods.Release(); mSupers.Release(); mTypeParameters.Release();}
  void Dump(unsigned);
};

// We define StructLiteral for C/C++ struct literal, TS/JS object literal.
// It contains a list of duple <fieldname, value>
//
// In Javascript, the GetAccessor/SetAccessor makes it complicated.
// We save the XetAccessor as a field literal with fieldname being func
// name and literal being function node itself.

// mFieldName could be NULL, like {3, 4} or {a, b}, 3, 4, a and b are literals without
// a name. but mLiteral may not be NULL.
class FieldLiteralNode : public TreeNode{
public:
  TreeNode *mFieldName;  // Generally a field is an identifier. However, in JS/TS
                         // it could be a literal string or numeric.
  TreeNode *mLiteral;

  void SetFieldName(TreeNode *id) {mFieldName = id; SETPARENT(id);}
  void SetLiteral(TreeNode *id) {mLiteral = id; SETPARENT(id);}

  TreeNode* GetFieldName() {return mFieldName;}
  TreeNode* GetLiteral() {return mLiteral;}

  FieldLiteralNode() : mFieldName(nullptr), TreeNode(NK_FieldLiteral) {}
  ~FieldLiteralNode(){}
};

class StructLiteralNode : public TreeNode {
private:
  SmallVector<FieldLiteralNode*> mFields;
public:
  StructLiteralNode() : TreeNode(NK_StructLiteral) {}
  ~StructLiteralNode(){Release();}

  unsigned          GetFieldsNum() {return mFields.GetNum();}
  FieldLiteralNode* GetField(unsigned i) {return mFields.ValueAtIndex(i);}
  void              SetField(unsigned i, FieldLiteralNode* n) {*(mFields.RefAtIndex(i)) = n; SETPARENT(n);}
  void              AddField(TreeNode *n);

  void Dump(unsigned);
};

//////////////////////////////////////////////////////////////////////////
//                           Namespace Node
// Typescript namespace has only a list of children which could be any
// kind of declaration or statement. So I simply keep their original
// nodes.
//////////////////////////////////////////////////////////////////////////

class NamespaceNode : public TreeNode {
private:
  SmallVector<TreeNode*> mElements;
  TreeNode *mId;   // the name of namespace
public:
  NamespaceNode() : TreeNode(NK_Namespace), mId(NULL) {}
  ~NamespaceNode() {Release();}

  void SetId(TreeNode *id) {mId = id; SETPARENT(id);}
  TreeNode* GetId() {return mId;}

  unsigned  GetElementsNum()              {return mElements.GetNum();}
  TreeNode* GetElementAtIndex(unsigned i) {return mElements.ValueAtIndex(i);}
  void      SetElementAtIndex(unsigned i, TreeNode* n) {*(mElements.RefAtIndex(i)) = n;}
  void      AddElement(TreeNode* n) {mElements.PushBack(n); SETPARENT(n);}

  void AddBody(TreeNode *);

  void Release() {mElements.Release();}
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
  VarListNode() : TreeNode(NK_VarList) {}
  ~VarListNode() {Release();}

  unsigned GetVarsNum() {return mVars.GetNum();}
  IdentifierNode* GetVarAtIndex(unsigned i) {return mVars.ValueAtIndex(i);}
  void            SetVarAtIndex(unsigned i, IdentifierNode* n) {*(mVars.RefAtIndex(i)) = n;}

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
  ExprListNode() : TreeNode(NK_ExprList) {}
  ~ExprListNode() {}

  unsigned GetExprsNum() {return mExprs.GetNum();}
  TreeNode* GetExprAtIndex(unsigned i) {return mExprs.ValueAtIndex(i);}
  void      SetExprAtIndex(unsigned i, TreeNode* n) {*(mExprs.RefAtIndex(i)) = n;}

  void AddExpr(TreeNode *n) {mExprs.PushBack(n);}
  void Merge(TreeNode*);
  void Release() {mExprs.Release();}
  void Dump(unsigned);
};

//////////////////////////////////////////////////////////////////////////
//                         TemplateLiteral Nodes
// TemplateLiteral node is created from the corresponding TempLit Token,
// copying the raw mStrings from token.
// Later on, call parser special API to create AST nodes for the patterns.
// After that, only mStrings and mTrees are used.
//////////////////////////////////////////////////////////////////////////

class TemplateLiteralNode : public TreeNode {
private:
  // mStrings save <format, placeholder> pairs.
  SmallVector<const char*> mStrings;

  // It's tree nodes of pairs of <format, placeholder>. So it would be pairs
  // of <TreeNode*, TreeNode*>, For any missing element, a NULL is saved
  // in its position.
  // Even index elements are for formats, Odd index elements are for placeholder.
  SmallVector<TreeNode*> mTrees;

public:
  TemplateLiteralNode() : TreeNode(NK_TemplateLiteral) {}
  ~TemplateLiteralNode(){mStrings.Release(); mTrees.Release();}

  unsigned    GetStringsNum() {return mStrings.GetNum();}
  const char* GetStringAtIndex(unsigned i) {return mStrings.ValueAtIndex(i);}
  void        SetStringAtIndex(unsigned i, const char* n) {*(mStrings.RefAtIndex(i)) = n;}
  void        AddString(const char *n) {mStrings.PushBack(n);}

  unsigned    GetTreesNum() {return mTrees.GetNum();}
  TreeNode*   GetTreeAtIndex(unsigned i) {return mTrees.ValueAtIndex(i);}
  void        SetTreeAtIndex(unsigned i, TreeNode *n) {*(mTrees.RefAtIndex(i)) = n; SETPARENT(n);}
  void        AddTree(TreeNode *n) {mTrees.PushBack(n); SETPARENT(n);}

  void Dump(unsigned);
};

// We define a global vector for TemplateLiteralNode created after all parsing.
extern SmallVector<TemplateLiteralNode*> gTemplateLiteralNodes;

//////////////////////////////////////////////////////////////////////////
//                         Literal Nodes
//////////////////////////////////////////////////////////////////////////

class LiteralNode : public TreeNode {
private:
  LitData   mData;

  // The regular type information is stored in LitData which is common practice.
  // However, in languages like Typescript, it does allow special literals like 'this'
  // to have a dedicated type. So here comes 'mType'.
  TreeNode *mType;

  // Typescript allows a string literal to be used as an Identifier, so it allows
  // an init value.
  TreeNode *mInit;

private:
  void InitName();

public:
  LiteralNode(LitData d) : TreeNode(NK_Literal), mData(d), mType(NULL), mInit(NULL) {}
  LiteralNode() : LiteralNode({.mType = LT_NA, .mData.mInt = 0}) {}
  ~LiteralNode(){}

  LitData GetData() {return mData;}
  void SetData(LitData d) {mData = d;}

  TreeNode* GetType()            {return mType;}
  void      SetType(TreeNode *t) {mType = t; SETPARENT(t);}
  TreeNode*   GetInit()                 {return mInit;}
  void        SetInit(TreeNode *t)      {mInit = t; SETPARENT(t);}

  bool IsThis() {return mData.mType == LT_ThisLiteral;}

  void Dump(unsigned);
};

//////////////////////////////////////////////////////////////////////////
//                         RegExpr Nodes
//////////////////////////////////////////////////////////////////////////

class RegExprNode : public TreeNode {
private:
  RegExprData mData;
public:
  RegExprNode() : TreeNode(NK_RegExpr) {mData.mExpr = NULL; mData.mFlags = NULL;}
  ~RegExprNode(){}

  RegExprData GetData() {return mData;}
  void SetData(RegExprData d) {mData = d;}

  void Dump(unsigned);
};

//////////////////////////////////////////////////////////////////////////
//                         ThrowNode
// This is the throw statement.
// In Java some functions throw exception in declaration, and they are
// saved in FunctionNode::mThrows.
//////////////////////////////////////////////////////////////////////////

class ThrowNode : public TreeNode {
private:
  SmallVector<TreeNode *> mExceptions;
public:
  ThrowNode() : TreeNode(NK_Throw) {}
  ~ThrowNode(){}

  unsigned  GetExceptionsNum() {return mExceptions.GetNum();}
  TreeNode* GetExceptionAtIndex(unsigned i) {return mExceptions.ValueAtIndex(i);}
  void      SetExceptionAtIndex(unsigned i, TreeNode* n) {*(mExceptions.RefAtIndex(i)) = n;}
  void      AddException(TreeNode *n);

  void Dump(unsigned);
};

//////////////////////////////////////////////////////////////////////////
//                         Try, Catch, Finally
// I can build one single TryNode to contain all the information. However,
// I built all three types of nodes, since some language could have complex
// syntax of catch or finally.
//////////////////////////////////////////////////////////////////////////

class CatchNode : public TreeNode {
private:
  SmallVector<TreeNode *> mParams;  // In Java, this sould be exception node.
  BlockNode              *mBlock;
public:
  CatchNode() : TreeNode(NK_Catch), mBlock(NULL) {}
  ~CatchNode(){}

  BlockNode* GetBlock()       {return mBlock;}
  void SetBlock(BlockNode *n) {mBlock = n;}

  unsigned  GetParamsNum() {return mParams.GetNum();}
  TreeNode* GetParamAtIndex(unsigned i) {return mParams.ValueAtIndex(i);}
  void      SetParamAtIndex(unsigned i, TreeNode* n) {*(mParams.RefAtIndex(i)) = n; SETPARENT(n);}
  void      AddParam(TreeNode *n);

  void Dump(unsigned);
};

class FinallyNode : public TreeNode {
private:
  BlockNode              *mBlock;
public:
  FinallyNode() : TreeNode(NK_Finally), mBlock(NULL) {}
  ~FinallyNode(){}

  BlockNode* GetBlock()       {return mBlock;}
  void SetBlock(BlockNode *n) {mBlock = n;}

  void Dump(unsigned);
};

class TryNode : public TreeNode {
private:
  BlockNode   *mBlock;
  FinallyNode *mFinally;
  SmallVector<CatchNode*> mCatches; // There could be >1 catches.

public:
  TryNode() : TreeNode(NK_Try), mBlock(NULL), mFinally(NULL) {}
  ~TryNode(){}

  BlockNode* GetBlock()       {return mBlock;}
  void SetBlock(BlockNode *n) {mBlock = n;}

  FinallyNode* GetFinally()       {return mFinally;}
  void SetFinally(FinallyNode *n) {mFinally = n;}

  unsigned   GetCatchesNum() {return mCatches.GetNum();}
  CatchNode* GetCatchAtIndex(unsigned i) {return mCatches.ValueAtIndex(i);}
  void       SetCatchAtIndex(unsigned i, CatchNode* n) {*(mCatches.RefAtIndex(i)) = n;}
  void       AddCatch(TreeNode *n);

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
  ExceptionNode(IdentifierNode *inode) : TreeNode(NK_Exception), mException(inode) {}
  ExceptionNode() : ExceptionNode(NULL) {}
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
  ReturnNode() : TreeNode(NK_Return), mResult(NULL) {}
  ~ReturnNode(){}

  void SetResult(TreeNode *t) {mResult = t; SETPARENT(t);}
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

  void SetCond(TreeNode *t)       {mCond = t; SETPARENT(t);}
  void SetTrueBranch(TreeNode *t) {mTrueBranch = t; SETPARENT(t);}
  void SetFalseBranch(TreeNode *t){mFalseBranch = t; SETPARENT(t);}

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
  BreakNode() : TreeNode(NK_Break), mTarget(NULL) {}
  ~BreakNode(){}

  TreeNode* GetTarget()           {return mTarget;}
  void      SetTarget(TreeNode* t){mTarget = t; SETPARENT(t);}
  void      Dump(unsigned);
};


// Continue statement. Continue targets could be one identifier or empty.
class ContinueNode : public TreeNode {
private:
  TreeNode* mTarget;
public:
  ContinueNode() : TreeNode(NK_Continue), mTarget(NULL) {}
  ~ContinueNode(){}

  TreeNode* GetTarget()           {return mTarget;}
  void      SetTarget(TreeNode* t){mTarget = t; SETPARENT(t);}
  void      Dump(unsigned);
};

// Javascript makes for loop complicated. It creates two special syntax as below
//   for (var in set) {...}
//   for (var of set) {...}
// We use  FL_Prop to differentiate them with regular for loop.
enum ForLoopProp {
  FLP_Regular,  // this is the default property
  FLP_JSIn,
  FLP_JSOf,
  FLP_NA
};

class ForLoopNode : public TreeNode {
private:
  ForLoopProp mProp;

  // Regular for loop
  SmallVector<TreeNode *> mInits;
  TreeNode               *mCond;
  SmallVector<TreeNode *> mUpdates;

  // JS In or JS Of
  TreeNode *mVariable;
  TreeNode *mSet;

  // shared by all kinds
  TreeNode *mBody;   // This could be a single statement, or a block node

public:
  ForLoopNode() : TreeNode(NK_ForLoop),
    mCond(NULL), mBody(NULL), mVariable(NULL), mSet(NULL), mProp(FLP_Regular) {}
  ~ForLoopNode() {Release();}

  void AddInit(TreeNode *t);
  void AddUpdate(TreeNode *t);
  void SetCond(TreeNode *t)   {mCond = t; SETPARENT(t);}
  void SetBody(TreeNode *t)   {mBody = t; SETPARENT(t);}

  unsigned GetInitsNum()     {return mInits.GetNum();}
  unsigned GetUpdatesNum()   {return mUpdates.GetNum();}
  TreeNode* GetInitAtIndex(unsigned i)   {return mInits.ValueAtIndex(i);}
  void      SetInitAtIndex(unsigned i, TreeNode* n) {*(mInits.RefAtIndex(i)) = n;}
  TreeNode* GetUpdateAtIndex(unsigned i) {return mUpdates.ValueAtIndex(i);}
  void      SetUpdateAtIndex(unsigned i, TreeNode* n) {*(mUpdates.RefAtIndex(i)) = n;}
  TreeNode* GetCond() {return mCond;}
  TreeNode* GetBody() {return mBody;}

  ForLoopProp GetProp() {return mProp;}
  void SetProp(ForLoopProp p) {mProp = p;}

  TreeNode* GetVariable() {return mVariable;}
  void SetVariable(TreeNode *t) {mVariable = t; SETPARENT(t);}

  TreeNode* GetSet() {return mSet;}
  void SetSet(TreeNode *t) {mSet = t; SETPARENT(t);}

  void Release() {mInits.Release(); mUpdates.Release();}
  void Dump(unsigned);
};

class WhileLoopNode : public TreeNode {
private:
  TreeNode *mCond;
  TreeNode *mBody; // This could be a single statement, or a block node
public:
  WhileLoopNode() : TreeNode(NK_WhileLoop), mCond(NULL), mBody(NULL) {}
  ~WhileLoopNode() {Release();}

  void SetCond(TreeNode *t) {mCond = t; SETPARENT(t);}
  void SetBody(TreeNode *t) {mBody = t; SETPARENT(t);}
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
  DoLoopNode() : TreeNode(NK_DoLoop), mCond(NULL), mBody(NULL) {}
  ~DoLoopNode(){Release();}

  void SetCond(TreeNode *t) {mCond = t; SETPARENT(t);}
  void SetBody(TreeNode *t) {mBody = t; SETPARENT(t);}
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
  SwitchLabelNode() : TreeNode(NK_SwitchLabel), mIsDefault(false), mValue(NULL) {}
  ~SwitchLabelNode(){}

  void SetIsDefault(bool b) {mIsDefault = b;}
  void SetValue(TreeNode *t){mValue = t; SETPARENT(t);}
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
  SwitchCaseNode() :TreeNode(NK_SwitchCase) {}
  ~SwitchCaseNode() {Release();}

  unsigned  GetLabelsNum()            {return mLabels.GetNum();}
  void      AddLabel(TreeNode*);
  SwitchLabelNode* GetLabelAtIndex(unsigned i) {return mLabels.ValueAtIndex(i);}
  void             SetLabelAtIndex(unsigned i, SwitchLabelNode* n) {*(mLabels.RefAtIndex(i)) = n;}

  unsigned  GetStmtsNum()            {return mStmts.GetNum();}
  TreeNode* GetStmtAtIndex(unsigned i) {return mStmts.ValueAtIndex(i);}
  void      SetStmtAtIndex(unsigned i, TreeNode* n) {*(mStmts.RefAtIndex(i)) = n; SETPARENT(n);}
  void      AddStmt(TreeNode*);
  void      PopStmt() {mStmts.PopBack();}

  void Release() {mStmts.Release(); mLabels.Release();}
  void Dump(unsigned);
};

class SwitchNode : public TreeNode {
private:
  TreeNode *mExpr;
  SmallVector<SwitchCaseNode*> mCases;
public:
  SwitchNode() : TreeNode(NK_Switch), mExpr(NULL) {}
  ~SwitchNode() {Release();}

  TreeNode* GetExpr() {return mExpr;}
  void SetExpr(TreeNode *c) {mExpr = c; SETPARENT(c);}

  unsigned  GetCasesNum() {return mCases.GetNum();}
  SwitchCaseNode* GetCaseAtIndex(unsigned i) {return mCases.ValueAtIndex(i);}
  void            SetCaseAtIndex(unsigned i, SwitchCaseNode* n) {*(mCases.RefAtIndex(i)) = n;}
  void AddCase(SwitchCaseNode* n) {mCases.PushBack(n); SETPARENT(n);}

  void            AddSwitchCase(TreeNode *t);
  SwitchCaseNode* SwitchLabelToCase(SwitchLabelNode*);

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
  AssertNode() : TreeNode(NK_Assert), mExpr(NULL), mMsg(NULL) {}
  ~AssertNode(){}

  TreeNode* GetExpr() {return mExpr;}
  TreeNode* GetMsg() {return mMsg;}
  void SetExpr(TreeNode *t) {mExpr = t; SETPARENT(t);}
  void SetMsg(TreeNode *t)  {mMsg = t; SETPARENT(t);}

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
  SmallVector<TreeNode*> mTypeArguments;
public:
  CallNode() : TreeNode(NK_Call), mMethod(NULL) {}
  ~CallNode(){Release();}

  void Init();

  TreeNode* GetMethod() {return mMethod;}
  void SetMethod(TreeNode *t) {mMethod = t; SETPARENT(t);}

  void AddArg(TreeNode *t) {mArgs.Merge(t); SETPARENT(t);}
  unsigned GetArgsNum() {return mArgs.GetExprsNum();}
  TreeNode* GetArg(unsigned index) {return mArgs.GetExprAtIndex(index);}
  void      SetArg(unsigned i, TreeNode* n) {mArgs.SetExprAtIndex(i, n); SETPARENT(n);}

  unsigned  GetTypeArgumentsNum()            {return mTypeArguments.GetNum();}
  TreeNode* GetTypeArgumentAtIndex(unsigned i) {return mTypeArguments.ValueAtIndex(i);}
  void      SetTypeArgumentAtIndex(unsigned i, TreeNode* n) {*(mTypeArguments.RefAtIndex(i)) = n; SETPARENT(n);}
  void      AddTypeArgument(TreeNode *);

  void Release() {mArgs.Release(); mTypeArguments.Release();}
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

  TreeNode           *mSync;       // Java allows a sync object on a Block.

public:
  BlockNode() : TreeNode(NK_Block), mIsInstInit(false), mSync(NULL) {}
  ~BlockNode() {Release();}

  // Instance Initializer and Attributes related
  bool IsInstInit()    {return mIsInstInit;}
  void SetIsInstInit(bool b = true) {mIsInstInit = b;}
  unsigned GetAttrsNum()              {return mAttrs.GetNum();}
  void     AddAttr(AttrId a)          {mAttrs.PushBack(a);}
  AttrId   GetAttrAtIndex(unsigned i) {return mAttrs.ValueAtIndex(i);}
  void     SetAttrAtIndex(unsigned i, AttrId n) {*(mAttrs.RefAtIndex(i)) = n;}

  void  SetSync(TreeNode *n) {mSync = n;}
  TreeNode* GetSync() {return mSync;}

  unsigned  GetChildrenNum()            {return mChildren.GetNum();}
  TreeNode* GetChildAtIndex(unsigned i) {return mChildren.ValueAtIndex(i);}
  void      SetChildAtIndex(unsigned i, TreeNode* n) {*(mChildren.RefAtIndex(i)) = n; SETPARENT(n);}
  void      AddChild(TreeNode *c);
  void      ClearChildren()             {mChildren.Clear();}

  void InsertStmtAfter(TreeNode *new_stmt, TreeNode *exist_stmt);
  void InsertStmtBefore(TreeNode *new_stmt, TreeNode *exist_stmt);

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
  SmallVector<TreeNode*>       mTypeParams;
  TreeNode                    *mFuncName;    // function name, usually an identifier
  TreeNode                    *mType;        // return type
  SmallVector<TreeNode*>       mParams;      //
  BlockNode                   *mBody;
  DimensionNode               *mDims;
  TreeNode                    *mAssert;      // In typescript, a function could have asserts
                                             // like: func () : asserts v is string
                                             // or a type predicate signature, like
                                             //    func() : v is string
                                             // So mAssert could be either an AssertNode or
                                             // an IsNode.
  bool mIsConstructor;
  bool mIsGetAccessor;
  bool mIsSetAccessor;
  bool mIsCallSignature; // no func name, no func body
  bool mIsConstructSignature; // no func name, no func body, and is a construct sig in TS

public:
  FunctionNode();
  ~FunctionNode() {Release();}

  // After function body is added, we need some clean up work, eg. cleaning
  // the PassNode in the tree.
  void CleanUp();

  TreeNode* GetFuncName() {return mFuncName;}
  void SetFuncName(TreeNode *n) {mFuncName = n; SETPARENT(n);}

  BlockNode* GetBody() {return mBody;}
  void SetBody(BlockNode *b) {mBody = b; SETPARENT(b); if(b) CleanUp();}

  TreeNode* GetAssert() {return mAssert;}
  void SetAssert(TreeNode *b) {mAssert = b; SETPARENT(b);}

  bool IsConstructor()    {return mIsConstructor;}
  void SetIsConstructor(bool b = true) {mIsConstructor = b;}
  bool IsGetAccessor()               {return mIsGetAccessor;}
  void SetIsGetAccessor(bool b = true) {mIsGetAccessor = b;}
  bool IsSetAccessor()                 {return mIsSetAccessor;}
  void SetIsSetAccessor(bool b = true) {mIsSetAccessor = b;}
  bool IsCallSignature()                 {return mIsCallSignature;}
  void SetIsCallSignature(bool b = true) {mIsCallSignature = b;}
  bool IsConstructSignature()                 {return mIsConstructSignature;}
  void SetIsConstructSignature(bool b = true) {mIsConstructSignature = b;}

  unsigned  GetParamsNum()        {return mParams.GetNum();}
  TreeNode* GetParam(unsigned i)  {return mParams.ValueAtIndex(i);}
  void      SetParam(unsigned i, TreeNode* n) {*(mParams.RefAtIndex(i)) = n; SETPARENT(n);}
  void      AddParam(TreeNode *t) {mParams.PushBack(t); SETPARENT(t);}

  // Attributes related
  unsigned GetAttrsNum()              {return mAttrs.GetNum();}
  void     AddAttr(AttrId a)          {mAttrs.PushBack(a);}
  AttrId   GetAttrAtIndex(unsigned i) {return mAttrs.ValueAtIndex(i);}
  void     SetAttrAtIndex(unsigned i, AttrId n) {*(mAttrs.RefAtIndex(i)) = n;}

  // Annotation/Pragma related
  unsigned GetAnnotationsNum()           {return mAnnotations.GetNum();}
  void     AddAnnotation(AnnotationNode *n) {mAnnotations.PushBack(n);}
  AnnotationNode* GetAnnotationAtIndex(unsigned i) {return mAnnotations.ValueAtIndex(i);}
  void            SetAnnotationAtIndex(unsigned i, AnnotationNode* n) {*(mAnnotations.RefAtIndex(i)) = n;}

  // Exception/throw related
  unsigned       GetThrowsNum()               {return mThrows.GetNum();}
  void           AddThrow(ExceptionNode *n)   {mThrows.PushBack(n);}
  ExceptionNode* GetThrowAtIndex(unsigned i)  {return mThrows.ValueAtIndex(i);}
  void           SetThrowAtIndex(unsigned i, ExceptionNode* n) {*(mThrows.RefAtIndex(i)) = n;}

  unsigned  GetTypeParamsNum()            {return mTypeParams.GetNum();}
  TreeNode* GetTypeParamAtIndex(unsigned i) {return mTypeParams.ValueAtIndex(i);}
  void      SetTypeParamAtIndex(unsigned i, TreeNode* n) {*(mTypeParams.RefAtIndex(i)) = n; SETPARENT(n);}
  void      AddTypeParam(TreeNode *);

  void SetType(TreeNode *t) {mType = t; SETPARENT(t);}
  TreeNode* GetType(){return mType;}

  DimensionNode* GetDims()       {return mDims;}
  void SetDims(DimensionNode *t) {mDims = t;}
  unsigned GetDimsNum()          {return mDims->GetDimensionsNum();}
  bool     IsArray()             {return mDims && mDims->GetDimensionsNum() > 0;}
  unsigned AddDim(unsigned i = 0){mDims->AddDimension(i);}        // 0 means unspecified
  unsigned GetNthDim(unsigned n) {return mDims->GetDimension(n);} // 0 means unspecified.
  void     SetNthDim(unsigned n, unsigned i) {mDims->SetDimension(n, i);}

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
  InterfaceNode() : TreeNode(NK_Interface), mIsAnnotation(false) {}
  ~InterfaceNode() {}

  unsigned       GetSuperInterfacesNum()              {return mSuperInterfaces.GetNum();}
  void           AddSuperInterface(InterfaceNode* a)  {mSuperInterfaces.PushBack(a);}
  InterfaceNode* GetSuperInterfaceAtIndex(unsigned i) {return mSuperInterfaces.ValueAtIndex(i);}
  void           SetSuperInterfaceAtIndex(unsigned i, InterfaceNode* n) {*(mSuperInterfaces.RefAtIndex(i)) = n;}

  unsigned        GetFieldsNum()              {return mFields.GetNum();}
  void            AddField(IdentifierNode* n) {mFields.PushBack(n); SETPARENT(n);}
  IdentifierNode* GetField(unsigned i) {return mFields.ValueAtIndex(i);}
  void            SetField(unsigned i, IdentifierNode* n) {*(mFields.RefAtIndex(i)) = n; SETPARENT(n);}

  unsigned      GetMethodsNum()              {return mMethods.GetNum();}
  void          AddMethod(FunctionNode* a)   {mMethods.PushBack(a);}
  FunctionNode* GetMethod(unsigned i) {return mMethods.ValueAtIndex(i);}
  void          SetMethod(unsigned i, FunctionNode* n) {*(mMethods.RefAtIndex(i)) = n;}

  void SetIsAnnotation(bool b) {mIsAnnotation = b;}
  bool IsAnnotation()          {return mIsAnnotation;}

  void Construct(BlockNode *);
  void Dump(unsigned);
};

//////////////////////////////////////////////////////////////////////////
//                         Class Nodes
//////////////////////////////////////////////////////////////////////////

class ClassNode : public TreeNode {
private:
  // Java Enum is defined in a similar way as class, with many restrictions
  // and special semantic rules. We define JavaEnum here too. For other
  // languages like C/C++ which have very simply Enum, we will have a
  // dedicated EnumNode for them.
  bool                         mIsJavaEnum;

  SmallVector<TreeNode*>          mSuperClasses;
  SmallVector<TreeNode*>          mSuperInterfaces;
  SmallVector<AttrId>             mAttributes;
  SmallVector<AnnotationNode*>    mAnnotations; //annotation or pragma
  SmallVector<TypeParameterNode*> mTypeParameters;

  SmallVector<TreeNode*>       mFields;  // a Field could be identifier or computed name
  SmallVector<FunctionNode*>   mMethods;
  SmallVector<FunctionNode*>   mConstructors;
  SmallVector<BlockNode*>      mInstInits;     // instance initializer
  SmallVector<ClassNode*>      mLocalClasses;
  SmallVector<InterfaceNode*>  mLocalInterfaces;
  SmallVector<ImportNode*>     mImports;
  SmallVector<ExportNode*>     mExports;

  SmallVector<DeclareNode*>    mDeclares; // First coming from TS.

public:
  ClassNode() : TreeNode(NK_Class), mIsJavaEnum(false) {}
  ~ClassNode() {Release();}

  bool IsJavaEnum()                {return mIsJavaEnum;}
  void SetIsJavaEnum(bool b = true){mIsJavaEnum = b;}

  // Annotation/Pragma related
  unsigned GetAnnotationsNum()           {return mAnnotations.GetNum();}
  void     AddAnnotation(AnnotationNode *n) {mAnnotations.PushBack(n);}
  AnnotationNode* GetAnnotationAtIndex(unsigned i) {return mAnnotations.ValueAtIndex(i);}
  void            SetAnnotationAtIndex(unsigned i, AnnotationNode* n) {*(mAnnotations.RefAtIndex(i)) = n;}

  // TypeParameter
  unsigned GetTypeParametersNum()           {return mTypeParameters.GetNum();}
  void     AddTypeParameter(TreeNode *n);
  TypeParameterNode* GetTypeParameterAtIndex(unsigned i) {return mTypeParameters.ValueAtIndex(i);}
  void               SetTypeParameterAtIndex(unsigned i, TypeParameterNode* n) {*(mTypeParameters.RefAtIndex(i)) = n;}

  void      AddSuperClass(TreeNode *n);
  unsigned  GetSuperClassesNum()        {return mSuperClasses.GetNum();}
  TreeNode* GetSuperClass(unsigned i)   {return mSuperClasses.ValueAtIndex(i);}
  void      SetSuperClass(unsigned i, TreeNode* n) {*(mSuperClasses.RefAtIndex(i)) = n;}

  void      AddSuperInterface(TreeNode *n);
  unsigned  GetSuperInterfacesNum()        {return mSuperInterfaces.GetNum();}
  TreeNode* GetSuperInterface(unsigned i)  {return mSuperInterfaces.ValueAtIndex(i);}
  void      SetSuperInterface(unsigned i, TreeNode* n) {*(mSuperInterfaces.RefAtIndex(i)) = n;}

  void AddAttr(AttrId a)      {mAttributes.PushBack(a);}
  void AddAttribute(AttrId a) {mAttributes.PushBack(a);}

  void AddField(TreeNode *n) {mFields.PushBack(n); SETPARENT(n);}
  void AddMethod(FunctionNode *n) {mMethods.PushBack(n);}
  void AddConstructor(FunctionNode *n) {mConstructors.PushBack(n);}
  void AddInstInit(BlockNode *n) {mInstInits.PushBack(n);}
  void AddLocalClass(ClassNode *n) {mLocalClasses.PushBack(n);}
  void AddLocalInterface(InterfaceNode *n) {mLocalInterfaces.PushBack(n);}
  void AddImport(ImportNode *n) {mImports.PushBack(n);}
  void AddExport(ExportNode *n) {mExports.PushBack(n);}
  void AddDeclare(DeclareNode *n) {mDeclares.PushBack(n);}


  unsigned GetAttributesNum()      {return mAttributes.GetNum();}
  unsigned GetFieldsNum()          {return mFields.GetNum();}
  unsigned GetMethodsNum()         {return mMethods.GetNum();}
  unsigned GetConstructorsNum()    {return mConstructors.GetNum();}
  unsigned GetInstInitsNum()       {return mInstInits.GetNum();}
  unsigned GetLocalClassesNum()    {return mLocalClasses.GetNum();}
  unsigned GetLocalInterfacesNum() {return mLocalInterfaces.GetNum();}
  unsigned GetImportsNum()         {return mImports.GetNum();}
  unsigned GetExportsNum()         {return mExports.GetNum();}
  unsigned GetDeclaresNum()        {return mDeclares.GetNum();}

  AttrId GetAttribute(unsigned i)              {return mAttributes.ValueAtIndex(i);}
  void   SetAttribute(unsigned i, AttrId n) {*(mAttributes.RefAtIndex(i)) = n;}
  TreeNode* GetField(unsigned i)         {return mFields.ValueAtIndex(i);}
  void      SetField(unsigned i, TreeNode* n) {*(mFields.RefAtIndex(i)) = n; SETPARENT(n);}
  FunctionNode* GetMethod(unsigned i)          {return mMethods.ValueAtIndex(i);}
  void          SetMethod(unsigned i, FunctionNode* n) {*(mMethods.RefAtIndex(i)) = n;}
  FunctionNode* GetConstructor(unsigned i)     {return mConstructors.ValueAtIndex(i);}
  void          SetConstructor(unsigned i, FunctionNode* n) {*(mConstructors.RefAtIndex(i)) = n;}
  BlockNode* GetInstInit(unsigned i)       {return mInstInits.ValueAtIndex(i);}
  void       SetInstInit(unsigned i, BlockNode* n) {*(mInstInits.RefAtIndex(i)) = n;}
  ClassNode* GetLocalClass(unsigned i)     {return mLocalClasses.ValueAtIndex(i);}
  void       SetLocalClass(unsigned i, ClassNode* n) {*(mLocalClasses.RefAtIndex(i)) = n;}
  InterfaceNode* GetLocalInterface(unsigned i)                   {return mLocalInterfaces.ValueAtIndex(i);}
  void           SetLocalInterface(unsigned i, InterfaceNode* n) {*(mLocalInterfaces.RefAtIndex(i)) = n;}
  ImportNode* GetImport(unsigned i)                {return mImports.ValueAtIndex(i);}
  void        SetImport(unsigned i, ImportNode* n) {*(mImports.RefAtIndex(i)) = n;}
  ExportNode* GetExport(unsigned i)                {return mExports.ValueAtIndex(i);}
  void        SetExport(unsigned i, ExportNode* n) {*(mExports.RefAtIndex(i)) = n;}
  DeclareNode* GetDeclare(unsigned i)                 {return mDeclares.ValueAtIndex(i);}
  void         SetDeclare(unsigned i, DeclareNode* n) {*(mDeclares.RefAtIndex(i)) = n;}

  void Construct(BlockNode*);
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
  PassNode() : TreeNode(NK_Pass) {}
  ~PassNode() {Release();}

  unsigned  GetChildrenNum() {return mChildren.GetNum();}
  TreeNode* GetChild(unsigned i) {return mChildren.ValueAtIndex(i);}
  void      SetChild(unsigned i, TreeNode* n) {*(mChildren.RefAtIndex(i)) = n;}

  void AddChild(TreeNode *c) {mChildren.PushBack(c); SETPARENT(c);}
  void Dump(unsigned);
  void Release() {mChildren.Release();}
};

////////////////////////////////////////////////////////////////////////////
//                  Lambda Expression
// Java Lambda expression and JS arrow function have similar syntax.
// Also in Typescript, FunctionType and Constructor Type have the similar syntax.
// We put them in the same node.
////////////////////////////////////////////////////////////////////////////

// This property tells categories of LambdaNode
enum LambdaProperty {
  LP_JavaLambda,
  LP_JSArrowFunction,
  LP_NA
};

class LambdaNode : public TreeNode {
private:
  LambdaProperty         mProperty;
  TreeNode              *mType;         // The return type. NULL as Java Lambda.
  SmallVector<TreeNode*> mParams;       // A param could be an IdentifierNode or DeclNode.
  TreeNode              *mBody;         // the body could be an expression, or block.
                                        // NULL as TS FunctionType and ConstructorType
public:
  LambdaNode() : TreeNode(NK_Lambda),
    mBody(NULL), mProperty(LP_JSArrowFunction), mType(NULL) {}
  ~LambdaNode(){Release();}


  TreeNode* GetBody()            {return mBody;}
  void      SetBody(TreeNode *n) {mBody = n; SETPARENT(n);}

  LambdaProperty  GetProperty()                 {return mProperty;}
  void            SetProperty(LambdaProperty p) {mProperty = p;}

  TreeNode* GetType()            {return mType;}
  void      SetType(TreeNode* t) {mType = t; SETPARENT(t);}

  unsigned  GetParamsNum()        {return mParams.GetNum();}
  TreeNode* GetParam(unsigned i)  {return mParams.ValueAtIndex(i);}
  void      SetParam(unsigned i, TreeNode* n) {*(mParams.RefAtIndex(i)) = n; SETPARENT(n);}
  void      AddParam(TreeNode *n) {mParams.PushBack(n); SETPARENT(n);}

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
  InstanceOfNode() : TreeNode(NK_InstanceOf), mLeft(NULL), mRight(NULL) {}
  ~InstanceOfNode(){Release();}

  TreeNode* GetLeft() {return mLeft;}
  void SetLeft(TreeNode *n) {mLeft = n; SETPARENT(n);}
  TreeNode* GetRight() {return mRight;}
  void SetRight(TreeNode *n){mRight = n; SETPARENT(n);}

  void Dump(unsigned);
};

////////////////////////////////////////////////////////////////////////////
//                  TypeOf Expression
// First coming from typescript.
////////////////////////////////////////////////////////////////////////////

class TypeOfNode : public TreeNode {
private:
  TreeNode *mExpr;
public:
  TypeOfNode() : TreeNode(NK_TypeOf), mExpr(NULL) {}
  ~TypeOfNode(){Release();}

  TreeNode* GetExpr()            {return mExpr;}
  void      SetExpr(TreeNode *n) {mExpr = n; SETPARENT(n);}

  void Dump(unsigned);
};

////////////////////////////////////////////////////////////////////////////
//                  KeyOf Expression
// First coming from typescript.
////////////////////////////////////////////////////////////////////////////

class KeyOfNode : public TreeNode {
private:
  TreeNode *mExpr;
public:
  KeyOfNode() : TreeNode(NK_KeyOf), mExpr(NULL) {}
  ~KeyOfNode(){Release();}

  TreeNode* GetExpr()            {return mExpr;}
  void      SetExpr(TreeNode *n) {mExpr = n; SETPARENT(n);}

  void Dump(unsigned);
};

////////////////////////////////////////////////////////////////////////////
//                  Infer Expression
// First coming from typescript.
////////////////////////////////////////////////////////////////////////////

class InferNode : public TreeNode {
private:
  TreeNode *mExpr;
public:
  InferNode() : TreeNode(NK_Infer), mExpr(NULL) {}
  ~InferNode(){Release();}

  TreeNode* GetExpr()            {return mExpr;}
  void      SetExpr(TreeNode *n) {mExpr = n; SETPARENT(n);}

  void Dump(unsigned);
};

////////////////////////////////////////////////////////////////////////////
//                  In Expression
// First coming from Javascript. It's like
// A is IN B.
// B is a set of properties. A is one of the properties.
////////////////////////////////////////////////////////////////////////////

class InNode : public TreeNode {
private:
  TreeNode *mLeft;
  TreeNode *mRight;
public:
  InNode() : TreeNode(NK_In), mLeft(NULL), mRight(NULL) {}
  ~InNode(){Release();}

  TreeNode* GetLeft() {return mLeft;}
  void SetLeft(TreeNode *n) {mLeft = n; SETPARENT(n);}
  TreeNode* GetRight() {return mRight;}
  void SetRight(TreeNode *n){mRight = n; SETPARENT(n);}

  void Dump(unsigned);
};

////////////////////////////////////////////////////////////////////////////
//                  ComputedName Expression
// First coming from Javascript. It's like
// [ xxx ]
////////////////////////////////////////////////////////////////////////////

enum CompNameProp {
  CNP_Rem_ReadOnly = 1,
  CNP_Add_ReadOnly = 1 << 1,
  CNP_Rem_Optional = 1 << 2,
  CNP_Add_Optional = 1 << 3,
  CNP_NA = 0
};

class ComputedNameNode : public TreeNode {
private:
  unsigned  mProp;
  TreeNode *mExpr;
  TreeNode *mInit;
  TreeNode *mExtendType;  // This is the type extending expression
                          // of the mapped property
public:
  ComputedNameNode() : TreeNode(NK_ComputedName),
    mProp(CNP_NA), mExpr(NULL), mExtendType(NULL), mInit(NULL) {}
  ~ComputedNameNode(){Release();}

  unsigned GetProp()               {return mProp;}
  void     SetProp(CompNameProp p) {mProp = mProp | (unsigned)p;}

  TreeNode* GetExpr()            {return mExpr;}
  void      SetExpr(TreeNode *n) {mExpr = n; SETPARENT(n);}
  TreeNode* GetInit()            {return mInit;}
  void      SetInit(TreeNode *n) {mInit = n;}
  TreeNode* GetExtendType()            {return mExtendType;}
  void      SetExtendType(TreeNode *n) {mExtendType = n;}

  void Dump(unsigned);
};

////////////////////////////////////////////////////////////////////////////
//                  Is Expression
// First coming from Typescript. It's like
// A is B.
// B is usually a type.
////////////////////////////////////////////////////////////////////////////

class IsNode : public TreeNode {
private:
  TreeNode *mLeft;
  TreeNode *mRight;
public:
  IsNode() : TreeNode(NK_Is), mLeft(NULL), mRight(NULL) {}
  ~IsNode(){Release();}

  TreeNode* GetLeft() {return mLeft;}
  void SetLeft(TreeNode *n) {mLeft = n; SETPARENT(n);}
  TreeNode* GetRight() {return mRight;}
  void SetRight(TreeNode *n){mRight = n; SETPARENT(n);}

  void Dump(unsigned);
};

////////////////////////////////////////////////////////////////////////////
//                  Tuple Type
// [label : type, label : type, ...]
////////////////////////////////////////////////////////////////////////////

class NameTypePairNode : public TreeNode {
private:
  TreeNode *mVar;
  TreeNode *mType;
public:
  NameTypePairNode() : TreeNode(NK_NameTypePair), mVar(NULL), mType(NULL) {}
  ~NameTypePairNode() {}

  TreeNode* GetVar()  {return mVar;}
  TreeNode* GetType() {return mType;}
  void SetVar(TreeNode* t)  {mVar = t; SETPARENT(t);}
  void SetType(TreeNode* t) {mType = t; SETPARENT(t);}

  void Dump(unsigned);
};

class TupleTypeNode : public TreeNode {
private:
  SmallVector<NameTypePairNode*>       mFields;
public:
  TupleTypeNode() : TreeNode(NK_TupleType) {}
  ~TupleTypeNode() {Release();}

  unsigned  GetFieldsNum() {return mFields.GetNum();}
  NameTypePairNode* GetField(unsigned i) {return mFields.ValueAtIndex(i);}
  void      SetField(unsigned i, NameTypePairNode* n) {*(mFields.RefAtIndex(i)) = n; SETPARENT(n);}
  void      AddField(NameTypePairNode *n) {mFields.PushBack(n); SETPARENT(n);}
  void      AddChild(TreeNode *n);

  void Release() {mFields.Release();}
  void Dump(unsigned);
};

}
#endif
