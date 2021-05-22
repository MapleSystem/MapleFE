## Overview
The Python script `maplefe-autogen.py` is a tool to generate the code for class `AstVisitor`, `AstDump`, `AstGraph`,
etc. for AST tree.

Whenever a new kind of AST tree node is introduced for a new language feature in the future, or any changes are made
to the existing AST tree node, these classes will be updated automatically. It reduces the maintenance effort and
ensures the consistency and completeness of these classes.

## How does it work?

It makes use of `clang-doc-10` to parse source file `ast_builder.cpp` and get its documentation files in YAML format.
These YAML files are fed into `maplefe-autogen.py` to generate C++ header and source files as described below.

## What should I do to generate a new C++ class?

Let's use `AstVisitor` as an example to explain it.

### 1. Set filename, class name, prefix of function name and extra include directives

```python
gen_args = [
        "gen_astvisitor", # Filename
        "AstVisitor",     # Class name
        "Visit",          # Prefix of function name
        "",               # Extra include directives
        ]
```
The list `gen_args` contains the filename, class name, prefix of function name and extra include directives for
generating C++ header and source files. You have to use the list name `gen_args` for them.

### 2. Define the specific content in header file

```python
astvisitor_init = [
"""
private:
bool mTrace;

public:
{gen_args1}(bool t = false) : mTrace(t) {{}}

TreeNode* {gen_args2}(TreeNode* node) {{
  return {gen_args2}TreeNode(node);
}}
""".format(gen_args1=gen_args[1], gen_args2=gen_args[2])
] # astvisitor_init
```
The list `astvisitor_init` contains the specific content in C++ header file. The list name can be customized.

### 3. Set callback functions for each part of a C++ function for a AST node

The following callback functions are defined for generating the code for `AstVisitor`.

```python
gen_call_handle_values = lambda: False
```
This function returns true or false.
There are two kinds of fields in an AST node. One is a pointer to another AST node, and another is 
a value with a non-AST-node type, such as enum, int, etc.
If this function returns false, all non-AST-node values will be ignored.


```python
gen_func_declaration = lambda dictionary, node_name: ...
```
Function `gen_func_declaration` returns a string for the declaration of a function for an AST node `node_name`. The result will be in
the C++ header file.


```python
gen_func_definition = lambda dictionary, node_name: ...
```
Function `gen_func_definition` returns a string for the definition of a function for an AST node `node_name`. The result will be in
the C++ source file. Any code which occurs at the beginning of its function body can be placed here.


```python
gen_call_child_node = lambda dictionary, node_name, field_name, node_type, accessor: ...
```
Function `gen_call_child_node` returns a string with the statements to handle a child AST node `field_name`. The child node has
`node_type` and you can use `accessor` to get its pointer value. 


```python
gen_call_children_node = lambda dictionary, node_name, field_name, node_type, accessor: ''
```
Function `gen_call_children_node` returns a string with the statements to handle a SmallVector or SmallList node `field_name` with
`node_type`. The `accessor` can be used to get its pointer value. It returns an empty string for `AstVisitor`.


```python
gen_call_children_node_end = lambda dictionary, node_name, field_name, node_type, accessor: ''
```
Function `gen_call_children_node_end` returns a string with the statements following the for-loop for each value stored in the
SmallVector or SmallList node `field_name`. It returns an empty string for `AstVisitor`. 


```python
gen_func_definition_end = lambda dictionary, node_name: '}\nreturn node;\n}'
```
Function `gen_func_definition_end` returns a string with the statements at the end of the function body.

Since it does not need to handle any non-AST-node values, therefore some functions for values are missing for `AstVisitor`.

### 4. Generate the code in C++ header and source files 

Here is the code block for generating the C++ code.
```python
handle_src_include_files(Initialization)
append(include_file, astvisitor_init)
handle_yaml(initial_yaml, gen_handler)
handle_src_include_files(Finalization)
```

It has to call `handle_src_include_files(Initialization)` and `handle_src_include_files(Finalization)` at the beginning and end of the block as shown above.

```python
append(include_file, astvisitor_init)
```
This is to append the code in list `astvisitor_init` to the C++ header file. 

```python
handle_yaml(initial_yaml, gen_handler)
```
This function call `handle_yaml(initial_yaml, gen_handler)` is to handle all related YAML file, call the `gen_func_*` and `gen_call_*` callback functions defined at step 3, and generate the code in C++ header and source files with them.

## Where are the generated files located?

They can be found under directory `MapleFE/output/typescript/ast_gen/shared/`.
```bash
MapleFE/output/typescript/ast_gen/shared/gen_astvisitor.h
MapleFE/output/typescript/ast_gen/shared/gen_astvisitor.cpp
```

## Format of the generated C++ header and source files

The generated C++ header and source files will be formatted with clang-format-10. You do not
have to format the generated code manually.
