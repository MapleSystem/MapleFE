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
  -k | --keep            Keep generated files *.ts-[0-9]*.out.ts which fail to compile with tsc
  -A | --all             Process all .ts files in current directory excluding *.ts-[0-9]*.out.ts
  -C | --clean           Clean up generated files (*.ts-[0-9]*.out.ts)
  -n | --name            Keep original names by removing "__v[0-9]*" from generated code
  -t | --treediff        Compare the AST of generated TS code with the one of original TS code
  -T | --Treediff        Same as -t/--treediff except that it disables tsc for generated TS code
  -i | --ignore-imported Ignore all imported modules
  <file1> [<file2> ...]  Specify one or more TypeScript files to be processed
EOF
exit 1
}
CMDLINE="$0 $*"
GITTS=$(git ls-files "*.ts")
DOT= PRE= LIST= VIEWOP= HIGHLIGHT="cat" TSCERR= KEEP= CLEAN= NAME= TREEDIFF= TSC=yes NOIMPORTED=
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
        -A|--all)        LIST="$LIST $(echo $GITTS | xargs grep -l '^ *export ') "
                         LIST="$LIST $(echo $GITTS | xargs grep -L '^ *export ') " ;;
        -n|--name)       NAME="original" ;;
        -t|--treediff)   TREEDIFF="--emit-ts-only"; NAME="original"; TSC=yes ;;
        -T|--Treediff)   TREEDIFF="--emit-ts-only"; NAME="original"; TSC= ;;
        -i|--ignore-imported) NOIMPORTED="--no-imported" ;;
        -*)              echo "Unknown option $1"; usage;;
        *)               LIST="$LIST $1"
    esac
    shift
done
LIST=$(echo $LIST | xargs -n1 | grep -v '\.ts-[0-9][0-9]*\.out\.[d.]*ts' | grep -vF .ts.tmp.ts)
if [ -n "$CLEAN" ]; then
  echo Cleaning up generated files...
  find -maxdepth 1 -regex '.*\.ts-[0-9]+\.out.[ctj][ps]p*\|.*\.ts-[0-9]+\.[pd][no][gt]\|.*\.ts.[ca][ps][pt]' -exec rm '{}' \;
  rm -rf *.ts.orig *.ts.gen *.ts.tmp.ts *[0-9]-dump.out ts2cxx-lock-*
  for ts in $LIST $GITTS; do
    rm -rf $ts.orig $ts.gen $ts.tmp.ts $ts.*[0-9]-dump.out $ts-*[0-9].out.ts $ts-*[0-9].out.d.ts
  done
  echo Done.
fi
[ -n "$LIST" ] || { echo Please specify one or more TypeScript files.; usage; }
[ -z "$DOT" ] || [ -x /usr/bin/dot -a -x /usr/bin/viewnior -a -x /usr/bin/highlight ] || sudo apt install graphviz viewnior highlight
TSOUT=$(cd $(dirname $(realpath $0))/../; pwd)/output/typescript
TS2AST=$TSOUT/bin/ts2ast
AST2CPP=$TSOUT/bin/ast2cpp
[ -x "$TS2AST" ] || { echo Cannot execute $TS2AST; exit 1; }
[ -x "$AST2CPP" ] || { echo Cannot execute $AST2CPP; exit 1; }

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
trap "{ pstree -p $$ | tr ')' '\n' | sed 's/.*(//' | xargs kill -9 2> /dev/null; rm -f ts2cxx-lock-*; }" SIGINT SIGQUIT SIGKILL SIGTERM

PROCID=$$
rm -rf *$PROCID-dump.out $PROCID-summary.out *$PROCID.out.ts ts2cxx-lock-*
cnt=0
for ts in $LIST; do
  ts=$(sed 's|^\./||' <<< "$ts")
  echo $((++cnt)): $ts
  AcquireLock ts2cxx for_$(basename $ts) $(nproc)
  (if true; then
  set -x
  echo ---------
  echo "$TS2AST" "$ts"
  out=$("$TS2AST" "$ts")
  if [ $? -ne 0 ]; then
    echo "MSG: Failed, test case (ts2ast)$ts"
  else
    echo "$AST2CPP" "$ts".ast --trace=2 --emit-ts $TREEDIFF $NOIMPORTED
    out=$("$AST2CPP" "$ts".ast --trace=2 --emit-ts $TREEDIFF $NOIMPORTED 2>&1)
    [ $? -eq 0 ] || echo "MSG: Failed, test case (ast2cpp)$ts"
  fi
  echo "$out"
  cmd=$(grep -n -e "^// .Beginning of Emitter:" -e "// End of Emitter.$" <<< "$out" |
    tail -2 | sed 's/:.*//' | xargs | sed 's/\([^ ]*\) \(.*\)/sed -n \1,$((\2))p/')
  if [ "x${cmd:0:4}" = "xsed " ]; then
    T=$(sed -e "s/\(.*\)\(\.d\)\(\.ts-$PROCID.out\)/\1\2\3\2/" <<< "$ts-$PROCID.out.ts")
    eval $cmd <<< "$out" > "$T"
    [ -z "$NAME" ] || sed -i 's/__v[0-9][0-9]*//g' "$T"
    clang-format-10 -i --style="{ColumnLimit: 120, JavaScriptWrapImports: false, AlignOperands: false}" "$T"
    sed -i -e 's/?? =/??=/g' -e 's/ int\[/ number[/g' "$T"
    echo -e "\n====== TS Reformatted ======\n"
    $HIGHLIGHT "$T"
    echo TREEDIFF=$TREEDIFF
    E=
    grep -qm1 "PassNode *{" <<< "$out" && E=",PassNode"
    if [ -z "$TREEDIFF" -o -n "$TSC" ]; then
      eval tsc -t es6 --lib es2015,es2017,dom -m commonjs --experimentalDecorators "$T" $TSCERR
    else
      echo Skipping tsc for tree diff
    fi
    # --strict  --downlevelIteration --esModuleInterop --noImplicitAny --isolatedModules "$T" $TSCERR
    if [ $? -ne 0 ]; then
      echo "MSG: Failed, test case (tsc-failed$E)$ts"
      [ -n "$KEEP" ] || rm -f "$T"
    elif [ -z $TREEDIFF ]; then
      echo "MSG: Passed, test case $ts"
      Passed="$Passed $ts"
      [ -n "$KEEP" ] || rm -f "$T"
    else
      cp $ts $ts.tmp.ts
      clang-format-10 -i --style="{ColumnLimit: 120, JavaScriptWrapImports: false, AlignOperands: false, JavaScriptQuotes: Double}" $ts.tmp.ts
      sed -i 's/?? =/??=/g' $ts.tmp.ts
      $TS2AST $ts.tmp.ts
      if [ $? -eq 0 ]; then
        $AST2CPP $ts.tmp.ts.ast $TREEDIFF | sed -n '/^AstDump:/,/^}/p' | sed 's/\(mStrIdx: unsigned int, \)[0-9]* =>/\1=>/'
      fi > $ts.orig
      $TS2AST $T
      if [ $? -eq 0 ]; then
        $AST2CPP $T.ast $TREEDIFF | sed -n '/^AstDump:/,/^}/p' | sed -e "s|/$T|/$ts.tmp.ts|" \
          -e 's/\(mStrIdx: unsigned int, \)[0-9]* =>/\1=>/'
      else
        E="$E,ts2ast"
      fi > $ts.gen
      diff $ts.orig $ts.gen
      if [ $? -eq 0 -a -s $ts.orig -a -s $ts.gen ]; then
        Passed="$Passed $ts"
        echo "MSG: Passed, test case $ts"
      else
        diff -I '[0-9]\. const char\*, "' $ts.orig $ts.gen
        if [ $? -eq 0 -a -s $ts.orig -a -s $ts.gen ]; then
          Passed="$Passed $ts"
          echo "MSG: Passed, test case (const-char*)$ts"
        else
          echo "MSG: Failed, test case (diff-ast$E)$ts"
        fi
      fi
      echo === "$T"; cat "$T"
      echo --- "$ts"; cat "$ts"
      echo ===
      if [ -n "$DOT" ]; then
        for tf in $ts.tmp.ts $T; do
          echo "=== $tf"
          cat $tf
          out="$out"$'\n'"$($TS2AST $tf --dump-dot)"
        done
      fi
      [ -n "$KEEP" ] || rm -f "$T" $ts.orig $ts.gen $ts.tmp.ts
    fi
  fi
  if [ -n "$DOT" ]; then
    echo --- "$ts"; cat "$ts"
    idx=0
    grep -n -e "^digraph $PRE[^{]* {" -e "^}" <<< "$out" | grep -A1 "digraph [^{]* {" |
      grep -v ^-- | sed 'N;s/\n/ /' | sed -e 's/:digraph [^{]* { */,/' -e 's/:.*/p/g' |
      { while read cmd; do
        idx=$((idx+1))
        sed -n $cmd <<< "$out" > "$ts"-$idx.dot
        dot -Tpng -o "$ts"-$idx.png "$ts"-$idx.dot
        env LC_ALL=C viewnior $VIEWOP "$ts"-$idx.png &
      done
      wait
      rm -f "$ts"-[0-9]*.png "$ts"-[0-9]*.dot; }
  fi
  ReleaseLock ts2cxx
  set +x
  fi >& $ts.$PROCID-dump.out
  grep -a "^MSG: [PF]a[si][sl]ed," $ts.$PROCID-dump.out >> $PROCID-summary.out
  ) &
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
