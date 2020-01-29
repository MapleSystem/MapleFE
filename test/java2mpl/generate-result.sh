#!/bin/bash
FILES=$(pwd)/*.java
for f in $FILES
do
  echo "Generating result for $f ..."
  ../../build64/java/java2mpl $f > $f.result
done
