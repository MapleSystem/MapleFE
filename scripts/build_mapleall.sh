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

set -e

if [ ! -d $MAPLEALL_ROOT ]; then
  cd $MAPLE_ROOT
  git clone https://gitee.com/openarkcompiler/OpenArkCompiler.git
fi

cd $MAPLEALL_ROOT
git checkout master
git pull
# use a version that is compatible at moment
git checkout 2a89d0a89
source build/envsetup.sh arm debug
make setup
make clobber
make maple
make irbuild
