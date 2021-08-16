#!/bin/bash
# This script is to measure the runtime performance of ts2ast and other executables
# If perf is not installed yet, please install the package linux-tools-common with all dependencies
TS2AST=$(dirname $0)/../output/typescript/bin/ts2ast
CMD="sudo perf record -e cpu-cycles,cache-misses --call-graph fp -F 10000 -o perf.data"
if [ $# -eq 0 ]; then
  echo "Usage: $0 <TypeScript-filename.ts>"
  echo "       $0 <command-with-arguments>"
  exit 1
elif [ $# -eq 1 -a "$(basename $1)" != "$(basename $1 .ts)" ]; then
  echo $CMD $TS2AST "$@"
  $CMD $TS2AST "$@"
else
  echo $CMD "$@"
  $CMD "$@"
fi
echo sudo perf report
sudo perf report
