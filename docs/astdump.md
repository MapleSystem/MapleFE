## Overview

The `astdump.sh` tool is a bash script which executes the `ts2cpp` binary executable with option
`--trace-a2c` to dump the AST and CFG graphs of a TypeScript program.

It also dump the TypeScript code converted from the corresponding AST with AST emitter. 

## Usage to astdump.sh

```bash
Usage: astdump.sh [-dot] [-f|--fullscreen] [-p <PREFIX>|--pre <PREFIX>] [-a|--ast] [-c|--cfg] [-A|--all] [-C|--clean] <file1> [<file2> ...]

  -d | --dot             Use Graphviz dot to generate the graph and view it with viewnior
  -f | --fullscreen      View the generated graph in fullscreen mode. It implies option -dot
  -p | --pre <PREFIX>    Filter graphs with the specified <PREFIX>, e.g. -p "CFG_<function-name>"
  -a | --ast             Show AST graph. It is equivalent to options "-dot -p AST"
  -c | --cfg             Show CFG graph. It is equivalent to options "-dot -p CFG"
  -s | --syntax          Syntax highlighting the generated TypeScript code
  -A | --all             Process all .ts files in current directory excluding *.ts-[0-9]*.out.ts
  -C | --clean           Clean up generated files (*.ts-[0-9]*.out.ts)
  <file1> [<file2> ...]  Specify one or more TypeScript files to be processed
```
## Example with binary-search.ts

You can execute the following commands to get the CFG graph of the test case `binary-search.ts`, and the
TypeScript code converted from the corresponding AST.

### 1. Command lines
```bash
cd MapleFE/test/typescript/unit_tests
../../astdump.sh --cfg --syntax binary-search.ts
```

### 2. CFG graph of function "binarySearch"

This is the CFG graph of function `binarySearch`.

<img src="https://images.gitee.com/uploads/images/2021/0514/151722_a7245ff7_5583371.png" height="480">

### 3. TypeScript code converted from the corresponding AST

This is the TypeScript code generated from AST.

<img src="https://images.gitee.com/uploads/images/2021/0514/152039_16636b7d_5583371.png" height="360">

