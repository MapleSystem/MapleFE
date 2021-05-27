#!/bin/bash

[ -n "$MAPLEFE_ROOT" ] || { echo MAPLE_ROOT not set. Please source envsetup.sh.; exit 1; }
cd ${MAPLEFE_ROOT}/test

if [ ! -d ${MAPLEFE_ROOT}/test/TypeScript ]; then
  git clone -b release-1.8 https://github.com/microsoft/TypeScript.git
fi

if [ -f ${MAPLEFE_ROOT}/test/msts_testlist ]; then
  cat msts_testlist | xargs -n1 -I % sh -c '{ rm -f typescript/ms_tests/%; cp -f TypeScript/tests/cases/compiler/% typescript/ms_tests/; cp -f TypeScript/LICENSE.txt typescript/ms_tests/; }'
fi

# setup files only. do not run test
if [ "x$1" = "xsetup" ]; then
  exit 0
fi

# export is for sh -c in xargs
export TS2AST=$MAPLEFE_ROOT/output/typescript/bin/ts2ast
export PASS_LIST=msts_passed.txt
FAIL_LIST=msts_failed.txt
MSTEST_DIR=$MAPLEFE_ROOT/test/TypeScript/tests/cases/compiler
N_JOBS=16

[ -n "$MAPLE_ROOT" ] || { echo MAPLE_ROOT not set. Please source envsetup.sh.; exit 1; }
if [ ! -d $MSTEST_DIR ]; then
  echo "$MSTEST_DIR" does not exist. Please git clone https://github.com/microsoft/TypeScript.git under "$MAPLEFE_ROOT/test"
  exit 1
fi

cd $MAPLEFE_ROOT/test
rm -f $PASS_LIST
find $MSTEST_DIR -name "*.ts" | xargs -n1 -P$N_JOBS -I % sh -c '{ $TS2AST %; exitcode=$?; if [ $exitcode -eq 0 ]; then basename % >> $PASS_LIST; fi }'
cd $MSTEST_DIR
ls *.ts | grep -v -x -f $MAPLEFE_ROOT/test/$PASS_LIST /dev/stdin  > $MAPLEFE_ROOT/test/$FAIL_LIST
cd -
sort -o $PASS_LIST $PASS_LIST
echo
echo "Microsoft Typescript compiler testcases: Passed: " `wc -l < $PASS_LIST` " Failed: " `wc -l < $FAIL_LIST`
echo "List of passed and failed cases are in $PASS_LIST and $FAIL_LIST"
echo
echo "Note: Add testcase name from above lists into mstest_testlist if"
echo "      you want it included in make test"
echo
cd - > /dev/null
