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
trap "{ pstree -p $$ | tr ')' '\n' | sed 's/.*(//' | xargs kill -9 2> /dev/null; rm -f ts2cpp-lock-*; }" SIGINT SIGQUIT SIGKILL SIGTERM
rm -rf ts2cpp-lock-* *-ts2cpp.out ts2cpp.summary.out ts2cpp.failures*.out
cnt=0
if [ $# -gt 1 ]; then
list1=$(grep -l "^ *export " "$@")
list2=$(grep -L "^ *export " "$@")
else
  list1="$@" list2=
fi
for list in "$list1" "$list2"; do
for f in $list; do
  echo $((++cnt)): $f
  t=$(basename $f .ts)
  AcquireLock ts2cpp for_$t $(nproc)
  (set -x
  while true; do
    [ -f $t.ts ] && f=$t.ts
    $TS2AST $f || { echo "(ts2ast)$f" >> ts2cpp.failures.out; break; }
    dep=$(grep "^import.* from " "$f" | sed "s/^ *import.* from .\([^'\"]*\).*/\1.cpp/" | sort -u)
    for cpp in $dep; do
      $TS2AST $(sed 's/\.cpp/.ts/' <<< "$cpp")
    done
    $AST2CPP $f.ast || { echo "(ast2cpp)$f" >> ts2cpp.failures.out; break; }
    g++ $t.cpp $RTSRC/*.cpp $dep -o $t.out || { echo "(g++)$f" >> ts2cpp.failures2.out; break; }
    ./$t.out || { echo "(run)$f" >> ts2cpp.failures2.out; break; }
    echo $t >> ts2cpp.summary.out
    break
  done
  ReleaseLock ts2cpp
  ) >& $f-ts2cpp.out &
done 2>&1 
wait
done
if [ -f ts2cpp.summary.out ]; then
  echo -e "\nDate: $(date)\nTest cases passed:" | tee -a $log
  sort ts2cpp.summary.out | xargs -n1 | nl | tee -a $log
fi
if [ -f ts2cpp.failures2.out ]; then
  echo -e "\nTest cases failed due to g++ or run:" | tee -a $log
  sort ts2cpp.failures2.out | xargs -n1 | nl | tee -a $log
fi
if [ -f ts2cpp.failures.out ]; then
  echo -e "\nTest cases failed due to ts2ast or ast2cpp:" | tee -a $log
  sort ts2cpp.failures.out | xargs -n1 | nl | tee -a $log
fi
