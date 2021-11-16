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
rm -rf -- tsc-lock-* *-tsc.out tsc.summary.out tsc.failures*.out

OPT="--target es6 \
     --lib es2015,es2017,dom \
     --module commonjs \
     --downlevelIteration \
     --esModuleInterop \
     --experimentalDecorators"
# --sourceMap \
# --isolatedModules \
while [ "x${1:0:1}" = "x-" ]; do
 OPT="$OPT $1"
 shift
done
i=0
for f; do
  echo $((++i)). $f
  js=$(dirname $f)/$(basename $f .ts).js
  rm -f $js
  AcquireLock tsc for_$t $(nproc)
  (bash -x -c "tsc --strict $OPT $f"
   if [ $? -ne 0 ]; then
     echo "(--strict)"$f >> tsc.failures.out
     bash -x -c "tsc $OPT $f" || echo "(non-strict)"$f >> tsc.failures.out
   fi
   if [ -f "$js" ]; then
     bash -x -c "node $js"
     [ $? -ne 0 ] && echo "(nodejs)"$f >> tsc.failures.out
   fi 2>&1 > $f-nodejs.out
   ReleaseLock tsc
  ) >& $f-tsc.out &
done
wait
rc=0
if [ -f tsc.failures.out ]; then
  echo -e "\nTest cases failed with tsc strict mode enabled:"
  sort tsc.failures.out | grep "(--strict)" | xargs -n1 | nl
  grep -q -e "(non-strict)" -e "(nodejs)" tsc.failures.out
  if [ $? -eq 0 ]; then
    echo -e "\nTest cases failed with non-strict mode or nodejs:"
    sort tsc.failures.out | grep -e "(non-strict)" -e "(nodejs)" | xargs -n1 | nl
    rc=2
  else
    echo -e "\nAll passed with tsc non-strict mode."
    rc=1
  fi
elif [ $# -gt 0 ]; then
  echo All passed
fi
[ $i -eq 1 -a -f $f-tsc.out ] && cat $f-tsc.out $f-nodejs.out
exit $rc
