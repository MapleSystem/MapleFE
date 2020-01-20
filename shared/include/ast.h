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

// A node in AST could be one of the three types.
//
// Token
//    This is the leaf node in an AST. It could be a variable, a literal.
// Operator
//    This is one of the operators defined in supported_operators.def
//    As you may know, operators, literals and variables (identifiers) are all preprocessed
//    to be a token. But we also define 'Token' as a NodeKind. So please keep in mind, we
//    catagorize operator to a dedicated NodeKind.
// Construct
//    This is syntax construct, such as for(..) loop construct. This is language specific,
//    and most of these nodes are defined under each language, such as java/ directory.
//    However, I do define some popular construct in shared/ directory.
// Function
//    A Function node have its arguments as children node. The return value is not counted.
//

enum NodeKind {
  Token,
  Operator,
  Construct,
  Function,
};

class TreeNode {
public:
  NodeKind mType;
public:
  void Dump();
};

// OperatorNode-s are coming from operator token.
class OperatorNode : public TreeNode {
public:
  OprId mOprId;
public:
  void Dump();
};

class FunctionNode : public TreeNode {
public:
  TreeType mRetType;
  std::vector<TreeSymbol*> mParams;
public:
  void Dump();
};

class TokenNode : public TreeNode {
};

#endif
