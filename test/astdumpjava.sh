#!/bin/bash
function usage {
cat << EOF

Usage: astdump.sh [-d] [-f] [-p <PREFIX>] [-a] [-c] [-k] [-A] [-C] [-n] [-t|-T] <file1> [<file2> ...]

Short/long options:
  -d | --dot             Use Graphviz dot to generate the graph and view it with viewnior
  -f | --fullscreen      View the generated graph in fullscreen mode. It implies option -dot
  -p | --pre <PREFIX>    Filter graphs with the specified <PREFIX>, e.g. -p "CFG_<function-name>"
  -a | --ast             Show AST graph. It is equivalent to options "-d -p AST"
  -c | --cfg             Show CFG graph. It is equivalent to options "-d -p CFG"
  -s | --syntax          Syntax highlighting the generated TypeScript code
  -k | --keep            Keep generated files *.java-[0-9]*.out.java which fail to compile with tsc
  -A | --all             Process all .java files in current directory excluding *.java-[0-9]*.out.java
  -C | --clean           Clean up generated files (*.java-[0-9]*.out.java)
  -n | --name            Keep original names by removing "__v[0-9]*" from generated code
  -t | --treediff        Compare the AST of generated TS code with the one of original TS code
  -T | --Treediff        Same as -t/--treediff except that it disables tsc for generated TS code
  <file1> [<file2> ...]  Specify one or more TypeScript files to be processed
EOF
exit 1
}
CMDLINE="$0 $*"
DOT= PRE= LIST= VIEWOP= HIGHLIGHT="cat" TSCERR= KEEP= CLEAN= NAME= TREEDIFF= TSC=yes
while [ $# -gt 0 ]; do
    case $1 in
        -d|--dot)        DOT=dot;;
        -f|--fullscreen) VIEWOP="--fullscreen"; DOT=fullscreen;;
        -p|--pre)        [ $# -ge 2 ] && { PRE="$2"; shift; } || { echo "$1 needs an argument"; exit 1; } ;;
        -a|--ast)        PRE="AST" ; DOT=ast ; TSCERR=">& /dev/null" ;;
        -c|--cfg)        PRE="CFG" ; DOT=cfg ; TSCERR=">& /dev/null" ;;
        -s|--syntax)     HIGHLIGHT="highlight -O xterm256 --syntax ts" ;;
        -e|--tscerror)   TSCERR= ;;
        -k|--keep)       KEEP=keep ;;
        -C|--clean)      CLEAN=clean ;;
        -A|--all)        LIST="$LIST $(find -maxdepth 1 -name '*.java')" ;;
        -n|--name)       NAME="original" ;;
        -t|--treediff)   TREEDIFF="--emit-ts-only"; NAME="original"; TSC=yes ;;
        -T|--Treediff)   TREEDIFF="--emit-ts-only"; NAME="original"; TSC= ;;
        -*)              echo "Unknown option $1"; usage;;
        *)               LIST="$LIST $1"
    esac
    shift
done
LIST=$(echo $LIST | xargs -n1 | grep -v '\.java-[0-9][0-9]*\.out.java' | grep -vF .java.tmp.java)
if [ -n "$CLEAN" ]; then
  echo Cleaning up generated files...
  find -maxdepth 1 -regex '.*\.java-[0-9]+\.out.[ctj][ps]p*\|.*\.java-[0-9]+\.[pd][no][gt]\|.*\.java.[ca][ps][pt]' -exec rm '{}' \;
  rm -rf *.java.orig *.java.gen *.java.tmp.java *[0-9]-dump.out ts2cxx-lock-*
  for xx in $LIST; do
    rm -rf $xx.orig $xx.gen $xx.tmp.java $xx.*[0-9]-dump.out $xx-*[0-9].out.java
  done
  echo Done.
fi
[ -n "$LIST" ] || { echo Please specify one or more TypeScript files.; usage; }
[ -z "$DOT" ] || [ -x /usr/bin/dot -a -x /usr/bin/viewnior -a -x /usr/bin/highlight ] || sudo apt install graphviz viewnior highlight
JAVAOUT=$(cd $(dirname $(realpath $0))/../; pwd)/output/java
JAVA2AST=$JAVAOUT/bin/java2ast
AST2MPL=$JAVAOUT/bin/ast2mpl
[ -x "$JAVA2AST" ] || { echo Cannot execute $JAVA2AST; exit 1; }
[ -x "$AST2MPL" ] || { echo Cannot execute $AST2MPL; exit 1; }

# Acquire/release a lock
typeset -i LockVar
LockVar=1
function AcquireLock {
    while [[ $LockVar -ne 0 ]] || sleep 0.3; do
        ln -s Lock_$2 $1-lock-$((LockVar=(LockVar+1)%$3)) > /dev/null 2>&1 && break
    done
}
function ReleaseLock {
    rm -f $1-lock-$LockVar
}

PROCID=$$
#rm -rf *$PROCID-dump.out $PROCID-summary.out *$PROCID.out.java ts2cxx-lock-*
echo PROCID-dump.out=$PROCID-dump.out $PROCID-summary.out *$PROCID.out.java ts2cxx-lock-*
rm -f ~/tmp/*
cnt=0
for tt in $LIST; do
  echo $((++cnt)): $tt
  set -x
  echo ---------
  echo "$JAVA2AST" "$tt"
  out=$("$JAVA2AST" "$tt")
  if [ $? -ne 0 ]; then
    echo "MSG: Failed, test case (java2ast)$tt"
  else
    echo "$AST2MPL" "$tt".ast --trace=2  $TREEDIFF
    out=$("$AST2MPL" "$tt".ast --trace=2  $TREEDIFF 2>&1)
    [ $? -eq 0 ] || echo "MSG: Failed, test case (ast2mpl)$tt"
  fi
  echo "$out"
  $JAVA2AST $tt.java
  if [ $? -eq 0 ]; then
    $AST2MPL $tt.java.ast $TREEDIFF | sed -n '/^AstDump:/,/^}/p' | sed 's/\(mStrIdx: unsigned int, \)[0-9]* =>/\1=>/'
  fi > $tt.orig
  $JAVA2AST $T
  if [ $? -eq 0 ]; then
    $AST2MPL $T.ast $TREEDIFF | sed -n '/^AstDump:/,/^}/p' | sed -e "s|$T|$tt.java|" \
      -e 's/\(mStrIdx: unsigned int, \)[0-9]* =>/\1=>/'
  else
    E="$E,java2ast"
  fi > $tt.gen
  diff $tt.orig $tt.gen
  if [ $? -eq 0 -a -s $tt.orig -a -s $tt.gen ]; then
    Passed="$Passed $tt"
    echo "MSG: Passed, test case $tt"
  else
    diff -I '[0-9]\. const char\*, "' $tt.orig $tt.gen
    if [ $? -eq 0 -a -s $tt.orig -a -s $tt.gen ]; then
      Passed="$Passed $tt"
      echo "MSG: Passed, test case (const-char*)$tt"
    else
      echo "MSG: Failed, test case (diff-ast$E)$tt"
    fi
  fi
  echo === "$T"; cat "$T"
  echo --- "$tt"; cat "$tt"
  echo ===
  if [ -n "$DOT" ]; then
    for tf in $tt.java $T; do
      echo "=== $tf"
      cat $tf
      out="$out"$'\n'"$($JAVA2AST $tf --dump-dot)"
    done
  fi
  [ -n "$KEEP" ] || rm -f "$T" $tt.orig $tt.gen $tt.java
  if [ -n "$DOT" ]; then
    echo --- "$tt"; cat "$tt"
    idx=0
    grep -n -e "^digraph $PRE[^{]* {" -e "^}" <<< "$out" | grep -A1 "digraph [^{]* {" |
      grep -v ^-- | sed 'N;s/\n/ /' | sed -e 's/:digraph [^{]* { */,/' -e 's/:.*/p/g' |
      { while read cmd; do
        idx=$((idx+1))
        sed -n $cmd <<< "$out" > "$tt"-$idx.dot
        dot -Tpng -o "$tt"-$idx.png "$tt"-$idx.dot
        # env LC_ALL=C viewnior $VIEWOP "$tt"-$idx.png &
      done
      wait
      rm -f "$tt"-[0-9]*.png "$tt"-[0-9]*.dot; }
  fi
done
wait
echo Done.
if [ -s $PROCID-summary.out ]; then
  msg=$(grep -a "^MSG: [PF]a[si][sl]ed," $PROCID-summary.out)
  echo "$CMDLINE" >> $PROCID-summary.out
  if true; then
    echo
    echo "Test case(s) passed:"
    grep -a "^MSG: Passed, test case " <<< "$msg" | sed 's/MSG: Passed, test case //' | env LC_ALL=C sort -r | nl
    grep -aq -m1 "^MSG: Failed, test case " <<< "$msg"
    if [ $? -eq 0 ]; then
      echo
      echo "Test case(s) failed:"
      grep -a "^MSG: Failed," <<< "$msg" | sed 's/MSG: Failed, test case //' | env LC_ALL=C sort | nl
    fi
    echo
    echo Total: $(wc -l <<< "$msg"), Passed: $(grep -ac "^MSG: Passed," <<< "$msg"), Failed: $(grep -ac "^MSG: Failed," <<< "$msg")
    grep -a "^MSG: Failed," <<< "$msg" | sed 's/MSG: Failed, test case (\([^)]*\).*/due to \1/' | sort | uniq -c
  fi | tee -a $PROCID-summary.out
fi
