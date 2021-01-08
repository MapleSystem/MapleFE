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

class Point { int x, y; }
interface Colorable { void setColor(int color); }
class ColoredPoint extends Point implements Colorable {
  int color;
  public void setColor(int color) { this.color = color; }
}
final class EndPoint extends Point {}
class Test {
  public static void main(String[] args) {
    Point p = new Point();
    ColoredPoint cp = new ColoredPoint();
    Colorable c;
    // The following may cause errors at run time because
    // we cannot be sure they will succeed; this possibility
    // is suggested by the casts:
    cp = (ColoredPoint)p; // p might not reference an
    // object which is a ColoredPoint
    // or a subclass of ColoredPoint
    c = (Colorable)p; // p might not be Colorable
    // The following are incorrect at compile time because
    // they can never succeed as explained in the text:
    Long l = (Long)p; // compile-time error #1
    EndPoint e = new EndPoint();
    c = (Colorable)e; // compile-time error #2
  }
}
