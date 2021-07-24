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
FILES=( class-extends.ts class-extends2.ts class-generics-arrowfunc.ts class2.ts conditional-type2.ts construct-signature.ts delete-func.ts dynamic-import.ts generic-function.ts generic-type.ts generics-array.ts generics2.ts infer-type.ts interface-intersect-nonnullable-intf-fd.ts interface2.ts lambda3.ts mapped-type.ts non-null-assertion-this.ts prop-name-extends.ts property-deco2.ts record-parameter.ts semicolon-missing.ts semicolon-missing5.ts static.ts this-as-any.ts type-assertion-retval.ts type-assertion-this.ts union-type5.ts utility-type-instancetype.ts utility-type-nonnullable.ts)

for f in ${FILES[@]}
do
  echo "Generating result for $f ..."
  ../../../output/typescript/bin/ts2ast $f > $f.result
done
