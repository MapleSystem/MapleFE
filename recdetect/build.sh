# Copyright (C) [2020] Futurewei Technologies, Inc. All rights reverved.
# 
# OpenArkFE is licensed under the Mulan PSL v2.
# You can use this software according to the terms and conditions of the Mulan PSL v2.
# You may obtain a copy of Mulan PSL v2 at:
# 
#  http://license.coscl.org.cn/MulanPSL2
# 
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR
# FIT FOR A PARTICULAR PURPOSE.
# See the Mulan PSL v2 for more details.
# 

#!/bin/bash

# Usage:
#  ./build.sh [LANG]
#
# eg. ./build.sh java

rm -rf $BUILDDIR/recdetect
rm -rf $BUILDDIR/recdetect/$1

rm -rf $1
mkdir -p $1

cp ../$1/include/gen_*.h $1/
cp ../$1/src/gen_*.cpp $1/

# The four generated files shouldn't be taken in.
rm -f $1/gen_recursion.h
rm -f $1/gen_recursion.cpp
rm -f $1/gen_lookahead.h
rm -f $1/gen_lookahead.cpp

mkdir -p $BUILDDIR/recdetect
mkdir -p $BUILDDIR/recdetect/$1
make LANG=$1
