#!/bin/bash
# Usage: cd MapleFE/test/typescript/unit_tests; ../ts2cpp-test.sh *.ts
SUCC=
TSOUT=$(cd $(dirname $0)/../../; pwd)/output/typescript
RTSRC=$(cd $(dirname $0)/../../; pwd)/ast2cpp/runtime/src
TS2AST=$TSOUT/bin/ts2ast
AST2CPP=$TSOUT/bin/ast2cpp
log=cxx.log

# Acquire/release a lock
typeset -i LockVar
LockVar=1
function AcquireLock {
    while [[ $LockVar -ne 0 ]] || sleep 0.1; do
        ln -s Lock_$2 $1-lock-$((LockVar=(LockVar+1)%$3)) > /dev/null 2>&1 && break
    done
}
function ReleaseLock {
    rm -f $1-lock-$LockVar
}

rm -rf ts2cpp-lock-* *-ts2cpp.out ts2cpp.summary.out ts2cpp.failures.out
cnt=0
for f; do
  echo $((++cnt)): $f
  t=$(basename $f .ts)
  AcquireLock ts2cpp for_$t $(nproc)
  (set -x
  while true; do
    [ -f $t.ts ] && f=$t.ts
    $TS2AST $f || { echo "(ts2ast)$f" >> ts2cpp.failures.out; break; }
    $AST2CPP $f.ast || { echo "(ast2cpp)$f" >> ts2cpp.failures.out; break; }
    g++ $t.cpp $RTSRC/*.cpp -o $t || break
    ./$t || break
    echo $t >> ts2cpp.summary.out
    break
  done
  ReleaseLock ts2cpp
  ) >& $f-ts2cpp.out &
done 2>&1 
wait
echo -e "\nDate: $(date)\nTest cases passed:" | tee -a $log
sort ts2cpp.summary.out | xargs -n1 | nl | tee -a $log
if [ -f ts2cpp.failures.out ]; then
  echo -e "\nTest cases failed due to ts2ast or ast2cpp:" | tee -a $log
  sort ts2cpp.failures.out | xargs -n1 | nl | tee -a $log
fi
