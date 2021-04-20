//
//Copyright (C) [2021] Futurewei Technologies, Inc. All rights reverved.
//
//OpenArkFE is licensed under the Mulan PSL v2.
//You can use this software according to the terms and conditions of the Mulan PSL v2.
//You may obtain a copy of Mulan PSL v2 at:
//
// http://license.coscl.org.cn/MulanPSL2
//
//THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER
//EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR
//FIT FOR A PARTICULAR PURPOSE.
//See the Mulan PSL v2 for more details.
//
class A {
  int f;
  int bar(int c, A a) { return c; }
  int bar(int c, int a) { return a; }
  int bar(A a) { return f; }
  int foo(A b) {
    int i = 0;
    int j = 3;
    int k = 4;
    i = bar(i);
    j = bar(j, k);
    k = bar(j, this);
    return i + j + k;
  }
}
