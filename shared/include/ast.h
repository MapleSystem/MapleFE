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

#include "ast_type.h"
#include "ast_mempool.h"
#include "container.h"

#include "ruletable.h"
#include "token.h"

#undef ATTRIBUTE
#define ATTRIBUTE(X) ATTR_##X,
enum ASTAttribute {
#include "supported_attributes.def"
};

enum NodeKind {
  NK_Identifier,
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
  bool IsVarList()    {return mKind == NK_VarList;}
  bool IsLiteral()    {return mKind == NK_Literal;}
  bool IsUnaOperator(){return mKind == NK_UnaOperator;}
  bool IsBinOperator(){return mKind == NK_BinOperator;}
  bool IsTerOperator(){return mKind == NK_TerOperator;}
  bool IsConstruct()  {return mKind == NK_Construct;}
  bool IsBlock()      {return mKind == NK_Block;}
  bool IsFunction()   {return mKind == NK_Function;}
  bool IsClass()      {return mKind == NK_Class;}
  bool IsInterface()      {return mKind == NK_Interface;}

  bool IsScope()      {return IsBlock() || IsFunction();}

  void SetParent(TreeNode *p) {mParent = p;}

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
//////////////////////////////////////////////////////////////////////////

class IdentifierNode : public TreeNode {
public:
  const char *mName; // In the lexer's StringPool
  ASTType    *mType;
  TreeNode   *mInit; // Init value
public:
  IdentifierNode(const char *s) : mName(s), mType(NULL), mInit(NULL) {
    mKind = NK_Identifier; }
  IdentifierNode(const char *s, ASTType *t) : mName(s), mType(t) {mKind = NK_Identifier;}
  ~IdentifierNode(){}

  const char* GetName()     {return mName;}
  void SetType(ASTType *t)  {mType = t;}
  void SetInit(TreeNode *t) {mInit = t;}
  void Dump(unsigned);
};

//////////////////////////////////////////////////////////////////////////
//                           VarList Node
// Why do we need a VarListNode? Often in the program we have multiple
// variables like parameters in function, or varable list in declaration.
//////////////////////////////////////////////////////////////////////////

class VarListNode : public TreeNode {
public:
  SmallVector<IdentifierNode*> mVars;
public:
  VarListNode() {mKind = NK_VarList;}
  ~VarListNode() {}

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
//////////////////////////////////////////////////////////////////////////

class BlockNode : public TreeNode {
};

//////////////////////////////////////////////////////////////////////////
//                         Function Nodes
//////////////////////////////////////////////////////////////////////////

class ASTScope;

class FunctionNode : public TreeNode {
public:
  ASTType      mRetType;
  VarListNode *mParams;
  ASTScope    *mScope;
  BlockNode   *mBody;
public:
  ASTScope* GetScope() {return mScope;}
  void      SetScope(ASTScope *s) {mScope = s;}

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
//////////////////////////////////////////////////////////////////////////

class ClassNode : public TreeNode {
public:
  IdentifierNode              *mName;
  SmallVector<ClassNode*>      mSuperClasses;
  SmallVector<InterfaceNode*>  mSuperInterfaces;
  SmallVector<ASTAttribute>    mAttributes;
  BlockNode                   *mBody;
public:
  ClassNode(){mKind = NK_Class;}
  ~ClassNode() {Release();}

  void SetName(IdentifierNode *n) {mName = n;}
  void AddSuperClass(ClassNode *n) {mSuperClasses.PushBack(n);}
  void AddSuperClass(InterfaceNode *n) {mSuperInterfaces.PushBack(n);}
  void AddAttribute(ASTAttribute a) {mAttributes.PushBack(a);}

  void Release();
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
  ASTBuilder *mBuilder;

public:
  ASTTree();
  ~ASTTree();

  TreeNode* NewTreeNode(AppealNode*, std::map<AppealNode*, TreeNode*> &);

  TreeNode* BuildBinaryOperation(TreeNode *, TreeNode *, OprId);

  void Dump(unsigned);
};

#endif
