#!/bin/bash
DOT= PRE= LIST=
while [ $# -gt 0 ]; do
    case $1 in
        -d|--dot) DOT=true;;
        -p|--pre) [ $# -ge 2 ] && { PRE="$2"; shift; } || { echo "$1 needs an argument"; exit 1; } ;;
        -a|--ast) PRE="AST" ; DOT=true ;;
        -c|--cfg) PRE="CFG" ; DOT=true ;;
        -*)       echo "Usage: $0 [-dot] [-p <PREFIX>|--pre <PREFIX>] [-a|-ast] [-c|-cfg] <file1> [<file2> ...]" ;;
        *)        LIST="$LIST $1"
    esac
    shift
done
[ -z "$DOT" ] || [ -x /usr/bin/dot -a -x /usr/bin/viewnior ] || sudo apt install graphviz viewnior 
CMD=$(dirname $0)/../output/typescript/typescript/ts2cpp
[ -x "$CMD" ] || { echo Cannot execute $CMD; exit 1; }
Failed=
for ts in $LIST; do
  echo "$CMD" "$ts" --trace-a2c
  out=$("$CMD" "$ts" --trace-a2c 2>&1)
  [ $? -eq 0 ] || Failed="$Failed $ts"
  echo "$out"
  if [ -n "$DOT" ]; then
    grep -n -e "^digraph $PRE[^{]* {" -e "^}" <<< "$out" | grep -A1 "digraph [^{]* {" |
      grep -v ^-- | sed 'N;s/\n/ /' | sed -e 's/:digraph [^{]* { */,/' -e 's/:.*/p/g' |
      while read cmd; do
        echo --- "$ts"; cat "$ts"
        sed -n $cmd <<< "$out" > "$ts".dot
        dot -Tpng -o "$ts".png "$ts".dot
        viewnior --fullscreen "$ts".png
        rm -f "$ts".png "$ts".dot
      done
  fi
done
echo
[ -n "$Failed" ] || exit 0
echo "Test case(s) failed:"
echo $Failed | xargs -n1 | sort | nl
exit 1
