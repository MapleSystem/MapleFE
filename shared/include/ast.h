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
// 1. Operator
//
//    This is one of the operators defined in supported_operators.def
//    As you may know, operators, literals and variables (identifiers) are all preprocessed
//    to be a token. But we also define 'Token' as a NodeType. So please keep in mind, we
//    catagorize operator to a dedicated NodeType.
//
// 2. Function
//
//    A Function node have its arguments as children node. The return value is not counted.
//
// 3. Token.
//
//    This is the leaf node in an AST. It could be a variable, a literal.

enum NodeType {
  Operator,
  Function,
  Token
};

class TreeNode {
public:
  NodeType mType;
};

class OperatorNode : public TreeNode {
};

class FunctionNode : public TreeNode {
};

class TokenNode : public TreeNode {
};

#endif
