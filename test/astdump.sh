#!/bin/bash
function usage {
cat << EOF

Usage: astdump.sh [-dot] [-f|--fullscreen] [-p <PREFIX>|--pre <PREFIX>] [-a|--ast] [-c|--cfg] [-C|--clean] <file1> [<file2> ...]

  -d | --dot             Use Graphviz dot to generate the graph and view it with viewnior
  -f | --fullscreen      View the generated graph in fullscreen mode. It implies option -dot
  -p | --pre <PREFIX>    Filter graphs with the specified <PREFIX>, e.g. -p "CFG_<function-name>"
  -a | --ast             Show AST graph. It is equivalent to options "-dot -p AST"
  -c | --cfg             Show CFG graph. It is equivalent to options "-dot -p CFG"
  -s | --syntax          Syntax highlighting the generated TypeScript code
  -C | --clean           Clean up generated files (*.ts-[0-9]*.out.ts)
  <file1> [<file2> ...]  Specify one or more TypeScript files to be processed
EOF
exit 1
}

DOT= PRE= LIST= VIEWOP= HIGHLIGHT="cat" TSCERR= CLEAN=
while [ $# -gt 0 ]; do
    case $1 in
        -d|-dot|--dot)   DOT=true;;
        -f|--fullscreen) VIEWOP="--fullscreen"; DOT=true;;
        -p|-pre|--pre)   [ $# -ge 2 ] && { PRE="$2"; shift; } || { echo "$1 needs an argument"; exit 1; } ;;
        -a|-ast|--ast)   PRE="AST" ; DOT=true ; TSCERR=">& /dev/null" ;;
        -c|-cfg|--cfg)   PRE="CFG" ; DOT=true ; TSCERR=">& /dev/null" ;;
        -s|--syntax)     HIGHLIGHT="highlight -O xterm256 --syntax ts" ;;
        -e|--tscerror)   TSCERR= ;;
        -C|--clean)      CLEAN=true ;;
        -*)              usage;;
        *)               LIST="$LIST $1"
    esac
    shift
done
[ -z "$CLEAN" ] || { echo Cleaning up generated files...; find -maxdepth 1 -regex '.*\.ts-[0-9]+\.out.ts' -exec rm '{}' \;; echo Done.; }
[ -n "$LIST" ] || { echo Please specify one or more TypeScript files.; usage; }
[ -z "$DOT" ] || [ -x /usr/bin/dot -a -x /usr/bin/viewnior -x /usr/bin/highlight ] || sudo apt install graphviz viewnior highlight
CMD=$(dirname $0)/../output/typescript/typescript/ts2cpp
[ -x "$CMD" ] || { echo Cannot execute $CMD; exit 1; }
Failed=
for ts in $LIST; do
  echo "$CMD" "$ts" --trace-a2c
  out=$("$CMD" "$ts" --trace-a2c 2>&1)
  [ $? -eq 0 ] || Failed="$Failed $ts"
  echo "$out"
  cmd=$(grep -n -e "^// .Beginning of AstEmitter:" -e "// End of AstEmitter.$" <<< "$out" |
    tail -2 | sed 's/:.*//' | xargs | sed 's/\([^ ]*\) \(.*\)/sed -n \1,$((\2+1))p/')
  if [ "x${cmd:0:4}" = "xsed " ]; then
    T=$ts-$$.out.ts
    eval $cmd <<< "$out" > "$T"
    clang-format-10 -i --style="{ColumnLimit: 120}" "$T"
    echo -e "\n====== Reformated ======\n"
    $HIGHLIGHT "$T"
    eval tsc --noEmit --target es2015 "$T" $TSCERR
    if [ $? -ne 0 ]; then
      E="tsc"
      grep -qm1 "^PassNode {" <<< "$out" && E="$E,PassNode"
      Failed="$Failed ($E)$ts"
      echo Failed to compile "$T" with tsc
    else
      rm -f "$T"
    fi
  fi
  grep -n -e "^digraph $PRE[^{]* {" -e "^}" <<< "$out" | grep -A1 "digraph [^{]* {" |
  if [ -n "$DOT" ]; then
    grep -n -e "^digraph $PRE[^{]* {" -e "^}" <<< "$out" | grep -A1 "digraph [^{]* {" |
      grep -v ^-- | sed 'N;s/\n/ /' | sed -e 's/:digraph [^{]* { */,/' -e 's/:.*/p/g' |
      while read cmd; do
        echo --- "$ts"; cat "$ts"
        sed -n $cmd <<< "$out" > "$ts".dot
        dot -Tpng -o "$ts".png "$ts".dot
        viewnior $VIEWOP "$ts".png
        rm -f "$ts".png "$ts".dot
      done
  fi
done
echo
[ -n "$Failed" ] || exit 0
echo "Test case(s) failed:"
echo $Failed | xargs -n1 | env LC_ALL=C sort | nl
exit 1
