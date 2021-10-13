import * as deco from "./deco-module";

class Klass {
  @deco.prop_deco("of")
  x: number;
  constructor(i: number) {
    this.x = i;
  }
}

var c = new Klass(3);
console.log(c.x);
