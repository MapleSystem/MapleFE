#!/bin/bash
if [ $# -lt 1 ]; then
  out=$(cat)
  ts=/tmp/viewdot-$$
  grep -n -e "digraph [^{]* {" -e "^}" <<< "$out" | grep -A1 "digraph [^{]* {" |
    grep -v ^-- | sed 'N;s/\n/ /' | sed -e 's/:.*digraph [^{]* { */,/' -e 's/:.*/p/g' |
      { while read cmd; do
        idx=$((idx+1))
        sed -n $cmd <<< "$out" > "$ts"-$idx.dot
        dot -Tpng -o "$ts"-$idx.png "$ts"-$idx.dot
        env LC_ALL=C viewnior $VIEWOP "$ts"-$idx.png &
      done
      wait
      rm -f "$ts"-[0-9]*.png "$ts"-[0-9]*.dot; }
  exit
fi
for f; do
  dot -Tpng -o $f.png $f
  viewnior $f.png
  rm -f $f.png
done
