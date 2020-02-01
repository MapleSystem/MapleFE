#!/bin/bash
FILES=$(pwd)/*.cpp
for f in $FILES
do
  echo "Generating copyright for $f ..."
  cp ~/MapleFE/Copyright.orig ~/MapleFE/Copyright
  cat $f >> ~/MapleFE/Copyright
  cp ~/MapleFE/Copyright $f
done
