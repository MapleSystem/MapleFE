#!/bin/bash
# Usage: cd MapleFE/test/typescript/unit_tests; ../ts2cpp-test.sh *.ts
SUCC=
TSOUT=$(cd $(dirname $0)/../../; pwd)/output/typescript
RTSRC=$(cd $(dirname $0)/../../; pwd)/ast2cpp/runtime/src
TS2AST=$TSOUT/bin/ts2ast
AST2CPP=$TSOUT/bin/ast2cpp
TSCSH=$(dirname $0)/tsc.sh
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
  list1=$(grep -L -e "^ *import " -e "^ *export .* from " "$@")
  list2=$(grep -l -e "^ *import " -e "^ *export .* from " "$@")
else
  list1="$@" list2=
fi
single="no"
for list in "$list1" "$list2"; do
for f in $list; do
  echo $((++cnt)): $f
  t=$(basename $f .ts)
  AcquireLock ts2cpp for_$t $(nproc)
  (set -x
  while true; do
    [ -f $t.ts ] && f=$t.ts
    $TS2AST $f || { echo "(ts2ast)$f" >> ts2cpp.failures.out; break; }
    dep=$(grep "^[ei][xm]port.* from " "$f" | sed "s/^ *[ei][xm]port.* from .\([^'\"]*\).*/\1.cpp/" | sort -u)
    for cpp in $dep; do
      ts=$(sed 's/\.cpp/.ts/' <<< "$cpp")
      $TS2AST $ts
      dep="$dep "$(grep "^[ei][xm]port.* from " "$ts" | sed "s/^ *[ei][xm]port.* from .\([^'\"]*\).*/\1.cpp/" | sort -u)
    done
    dep=$(echo $dep | xargs -n1 | sort -u)
    $AST2CPP $f.ast || { echo "(ast2cpp)$f" >> ts2cpp.failures.out; break; }
    g++ -std=c++17 -g $t.cpp $RTSRC/*.cpp $dep -o $t.out || { echo "(g++)$f" >> ts2cpp.failures2.out; break; }
    ./$t.out 2>&1 > $f-run.out || { echo "(run)$f" >> ts2cpp.failures2.out; break; }
    $TSCSH $f
    diff $f-run.out $f-nodejs.out || { echo "(result)$f" >> ts2cpp.failures3.out; break; }
    echo $t >> ts2cpp.summary.out
    break
  done
  ReleaseLock ts2cpp
  ) >& $f-ts2cpp.out &
  if [ $single = "yes" ]; then
    wait
  fi
done 2>&1 
wait
single="yes"
done
if [ -f ts2cpp.summary.out ]; then
  echo -e "\nDate: $(date)\nTest cases passed:" | tee -a $log
  sort ts2cpp.summary.out | xargs -n1 | nl | tee -a $log
fi
if [ -f ts2cpp.failures3.out ]; then
  echo -e "\nTest cases failed due to unexpected results:" | tee -a $log
  sort ts2cpp.failures3.out | xargs -n1 | nl | tee -a $log
fi
if [ -f ts2cpp.failures2.out ]; then
  echo -e "\nTest cases failed due to g++ or run:" | tee -a $log
  sort ts2cpp.failures2.out | xargs -n1 | nl | tee -a $log
fi
if [ -f ts2cpp.failures.out ]; then
  echo -e "\nTest cases failed due to ts2ast or ast2cpp:" | tee -a $log
  sort ts2cpp.failures.out | xargs -n1 | nl | tee -a $log
fi
