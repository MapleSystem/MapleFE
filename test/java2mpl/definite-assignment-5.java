//
//Copyright (C) [2020] Futurewei Technologies, Inc. All rights reverved.
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

// Java has a rule 'Definite Assignment'.
// This case is from JLS 8.
//
// as far as definite assignment of k is concerned, k is assigned no matter whether the
// flag is true or false

class A{
  void flow(boolean flag) {
    int k;
    if (flag)
      k = 3;
    else
      k = 4;
    System.out.println(k);
  }
}
