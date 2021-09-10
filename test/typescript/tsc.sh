#!/bin/bash
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
rm -rf tsc-lock-* *-tsc.out tsc.summary.out tsc.failures*.out
i=0
for f; do
  echo $((++i)). $f
  AcquireLock tsc for_$t $(nproc)
  (bash -x -c "tsc \
    --target es6 \
    --lib es2015,es2017,dom \
    --module commonjs \
    --strict \
    --downlevelIteration \
    --esModuleInterop \
    --experimentalDecorators \
    $f" || echo $f >> tsc.failures.out
# --sourceMap \
# --isolatedModules \
  ReleaseLock tsc
  ) >& $f-tsc.out &
done
wait
if [ -f tsc.failures.out ]; then
  echo -e "\nTest cases failed to be compiled with tsc"
  sort tsc.failures.out | xargs -n1 | nl
  if [ $i -eq 1 -a -f $f-tsc.out ]; then
    echo
    cat $f-tsc.out
  fi
else
  echo All passed
fi
