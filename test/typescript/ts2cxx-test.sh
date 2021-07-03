#!/bin/bash -x
# Usage: cd MapleFE/test/typescript/unit_tests; ../ts2cpp-test.sh *.ts
SUCC=
TSOUT=$(cd $(dirname $0)/../../; pwd)/output/typescript
RTSRC=$(cd $(dirname $0)/../../; pwd)/ast2cpp/runtime/src
TS2AST=$TSOUT/bin/ts2ast
AST2CPP=$TSOUT/bin/ast2cpp
log=cxx.log
for f; do
  t=$(basename $f .ts)
  [ -f $t.ts ] && f=$t.ts
  $TS2AST $f || continue
  $AST2CPP $f.ast || continue
  g++ $t.cpp $RTSRC/*.cpp -o $t || continue
  ./$t || continue
  SUCC="$SUCC $t"
done 2>&1 
echo "$SUCC" | xargs -n1 | nl | tee -a $log
