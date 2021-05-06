#!/bin/bash
[ "x$1" = "x-dot" ] && { shift; DOT=true; } || DOT=
[ -z "$DOT" ] || [ -x /usr/bin/dot -a -x /usr/bin/viewnior ] || sudo apt install graphviz viewnior 
CMD=$(dirname $0)/../output/typescript/typescript/ts2cpp
[ -x "$CMD" ] || { echo Cannot execute $CMD; exit 1; }
for ts; do
  echo "$CMD" "$ts" --trace-a2c
  out=$("$CMD" "$ts" --trace-a2c 2>&1)
  echo "$out"
  if [ -n "$DOT" ]; then
    grep -n -e "^digraph [^{]* {" -e "^}" <<< "$out" | grep -A1 "digraph [^{]* {" |
      grep -v ^-- | sed 'N;s/\n/ /' | sed -e 's/:digraph [^{]* { */,/' -e 's/:.*/p/g' |
      while read cmd; do
        echo --- "$ts"; cat "$ts"
        sed -n $cmd <<< "$out" > "$ts".dot
        dot -Tpng -o "$ts".png "$ts".dot
        viewnior "$ts".png
        rm -f "$ts".png "$ts".dot
      done
  fi
done
