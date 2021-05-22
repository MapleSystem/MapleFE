## Overview of AstVisitor

The `AstVisitor` class is to let you be able to traverse an AST in a depth-first manner and call a visitor
function for each node in the AST. Each visitor function for a tree node  can be overridden by a customized
visitor in a derived class of `AstVisitor`. 

A customized visitor enable you to gather any interesting information of a tree node and replace this tree
node with another which the customized visitor returns.

## Creat your own visitors in a derived class of AstVisitor

First of all, you need to derive a class from `AstVisitor` and implement your own visitors for any nodes to
be processed. 

Here is an example. The class `MyVisitor` is derived from `AstVisitor` and has two customized visitors
implemented for `CondBranchNode`. 
 

```c
class MyVisitor : public AstVisitor {
  ... ...
  CondBranchNode *VisitCondBranchNode(CondBranchNode *node);
}
```

```c
CondBranchNode *MyVisitor::VisitCondBranchNode(CondBranchNode *node) {
  // Do something as needed
  ...
  // Call AstVisitor::VisitCondBranchNode to traverse subtrees of this node if needed
  AstVisitor::VisitCondBranchNode(node);
  return node;
}
```

## Replace a tree node with another one

You may replace a tree node with another one with your customized visitor. 

```c
CondBranchNode *MyVisitor::VisitCondBranchNode(CondBranchNode *node) {
  // Do something as needed
  ...
  // Create a new CondBranchNode node to replace the current one
  CondBranchNode *new_node = ...;
  return new_node;
}
```

The customized visitor `MyVisitor::VisitCondBranchNode()` visits a node which is the root of a subtree, 
and replace this node with `new_node` which is the root of another subtree. This provides a way to
transform a subtree into a new subtree when needed.

## How does the replacement work?

The `AstVisitor` class is declared and defined in `MapleFE/output/typescript/ast_gen/shared/gen_astvisitor.{h,cpp}`.

```c
BlockNode *AstVisitor::VisitBlockNode(BlockNode *node) {
  if (node != nullptr) {
    if (mTrace) {
      std::cout << "Visiting BlockNode, id=" << node->GetNodeId() << "..." << std::endl;
    }

    for (unsigned i = 0; i < node->GetChildrenNum(); ++i) {
      if (auto t = node->GetChildAtIndex(i)) {
        auto n = VisitTreeNode(t);
        if (n != t) {                     // If the returned node 'n' is not the same as 't'
          node->SetChildAtIndex(i, n);    // the current child node `t` of BlockNode is replaced with 'n'
        }
      }
    }

    if (auto t = node->GetSync()) {
      auto n = VisitTreeNode(t);
      if (n != t) {                       // Similar as mentioned above
        node->SetSync(n);
      }
    }
  }
  return node;
}
```

if a `CondBranchNode` node is a child of a `BlockNode` node, and `MyVisitor::VisitCondBranchNode()` returns
a new node, this child node of the `BlockNode` node will be replaced with the one returned.
