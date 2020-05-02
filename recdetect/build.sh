# Copyright (C) [2020] Futurewei Technologies, Inc. All rights reverved.
# 
# OpenArkFE is licensed under the Mulan PSL v1.
# You can use this software according to the terms and conditions of the Mulan PSL v1.
# You may obtain a copy of Mulan PSL v1 at:
# 
#  http://license.coscl.org.cn/MulanPSL
# 
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR
# FIT FOR A PARTICULAR PURPOSE.
# See the Mulan PSL v1 for more details.
# 

#!/bin/bash

rm -rf $1
mkdir -p $1

cp ../$1/include/gen_*.h $1/
cp ../$1/src/gen_*.cpp $1/

mkdir -p ../build64/recdetect
mkdir -p ../build64/recdetect/$1
make LANG=$1
