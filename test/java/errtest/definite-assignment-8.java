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
// 
// The analysis takes into account the structure of statements and expressions; it also
// provides a special treatment of the expression operators !, &&, ||, and ? :, and of
// boolean-valued constant expressions.
//
// Except for the special treatment of the conditional boolean operators &&, ||, and
// ? : and of boolean-valued constant expressions, the values of expressions are not
// taken into account in the flow analysis
//
// So in this case, !flag is not considered in analysis.

class Point {
  void flow(boolean flag) {
    int k;
    if (flag)
      k = 3;
    if (!flag)
      k = 4;
    System.out.println(k); /* k is not "definitely assigned" before this statement */
  }
}
