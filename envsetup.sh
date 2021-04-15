#!/bin/bash
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

function print_usage {
  echo " "
  echo "usage: source envsetup.sh java/typescript"
  echo " "
}

if [ "$#" -gt 1 ]; then
  echo $#
  print_usage
  return
fi

LANG=java
if [ "$#" -eq 1 ]; then
  if [ $1 = "typescript" ]; then
    LANG=typescript
  fi
fi

export SRCLANG=${LANG}

pdir=$(cd ..; pwd)
unset MAPLE_ROOT
export MAPLE_ROOT=${pdir}

curdir=$(pwd)
unset MAPLEFE_ROOT
export MAPLEFE_ROOT=${curdir}

unset MAPLEALL_ROOT
export MAPLEALL_ROOT=${MAPLE_ROOT}/OpenArkCompiler

unset MAPLEALL_SRC
export MAPLEALL_SRC=${MAPLEALL_ROOT}/src/mapleall

unset BUILDDIR
export BUILDDIR=${MAPLEFE_ROOT}/output/${SRCLANG}

