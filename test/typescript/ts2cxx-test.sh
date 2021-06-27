#!/bin/bash -x
# Usage: cd MapleFE/test/typescript/unit_tests; ../ts2cpp-test.sh *.ts
SUCC=
log=cxx.log
for f; do
  t=$(basename $f .ts)
  [ -f $t.ts ] && f=$t.ts
  ts2ast $f || continue
  ast2cpp $f.ast || continue
  g++ $t.cpp -o $t || continue
  ./$t || continue
  SUCC="$SUCC $t"
done 2>&1 
echo "$SUCC" | xargs -n1 | nl | tee -a $log
