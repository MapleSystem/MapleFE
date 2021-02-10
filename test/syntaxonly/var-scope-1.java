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

class A {
  int a;

  int foo(int x) {
    a = x;              // field a
    if (a == 2) {
      int a = 2;        // local variable a #1
      x = a + this.a + 2;
      a = x + 2;
      if (a == 4) {
        int a = 4;      // local variable a #2
                        // javac will complain but valid in other languages
                        // so worth to try out the mpl
        x = a + this.a + 4;
      }
      x = a + 5;        // local variable a #1
    }

    a = x + this.a + a; // field a
    return a + 6;
  }
}
