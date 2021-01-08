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

import java.util.ArrayList;
import java.util.function.Consumer;

class Point {
  void foo() {
    Consumer<Integer> m;
    m = () -> {};
    m = () -> 42;
    m = () -> null;
    m = () -> {return 42;};
    m = () -> {system.gc();};
    m = (int x, int y) -> x+y ;
  }
}
