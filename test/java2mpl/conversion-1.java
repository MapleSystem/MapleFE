//
//Copyright (C) [2020] Futurewei Technologies, Inc. All rights reverved.
//
//MapleFE is licensed under the Mulan PSL v2.
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

// This cases is taken from JLS 8.0 spec.
class Test {
  public static void main(String[] args) {
    // Casting conversion (5.4) of a float literal to
    // type int. Without the cast operator, this would
    // be a compile-time error, because this is a
    // narrowing conversion (5.1.3):
    int i = (int)12.5f;
    // String conversion (5.4) of i's int value:
     System.out.println("(int)12.5f==" + i);
    //System.out.println("int" + i);
    // Assignment conversion (5.2) of i's value to type
    // float. This is a widening conversion (5.1.2):
    float f = i;
    // String conversion of f's float value:
    //System.out.println("after float widening: " + f);
    // Numeric promotion (5.6) of i's value to type
    // float. This is a binary numeric promotion.
    // After promotion, the operation is float*float:
    System.out.print(f);
    f = f * i;
    // Two string conversions of i and f:
    //System.out.println("*" + i + "==" + f);
    // Invocation conversion (5.3) of f's value
    // to type double, needed because the method Math.sin
    // accepts only a double argument:
    double d = Math.sin(f);
    // Two string conversions of f and d:
    //System.out.println("Math.sin(" + f + ")==" + d);
  }
}
