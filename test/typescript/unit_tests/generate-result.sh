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
FILES=( class-generics-arrowfunc.ts class-implements-interface.ts generic-prim-array.ts generic-type2.ts generic-typeof.ts generics-array.ts generics2.ts trailing-commas2.ts)


for f in ${FILES[@]}
do
  echo "Generating result for $f ..."
  ../../../output/typescript/bin/ts2ast $f > $f.result
done
