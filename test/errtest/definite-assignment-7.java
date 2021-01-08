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
//  Definite Assignment Does Not Consider Values of Expressions
//even though the value of n is known at compile time, and in principle it can be known at
//compile time that the assignment to k will always be executed (more properly, evaluated).
//A Java compiler must operate according to the rules laid out in this section. The rules
//recognize only constant expressions; in this example, the expression n > 2 is not a constant
//expression as defined in §15.28.
// 
class Point {
  final int x;
  int foo() {
    int k;
    int n = 5;
    if (n > 2)
      k = 3;
    System.out.println(k); /* k is not "definitely assigned" before this statement */
  }
}
